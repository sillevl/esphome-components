#include "kalfire.h"
#include <algorithm>
#include "esphome/core/hal.h"
#include "esphome/core/log.h"

namespace esphome::kalfire {

static const char *const TAG = "kalfire";

void Kalfire::setup() {
  // GP8403/GP8413 initializes at DATA priority. No previous session is restored.
  this->ready_ = false;
  this->mode_ = Mode::OFF;
  this->flame_height_ = 1;
  this->last_stop_reason_ = "Startup";
  this->sample_seen_ = false;
  this->sample_expired_ = true;
  this->temperature_ = NAN;
  this->too_warm_ = false;
  this->unavailable_tracking_ = true;
  this->unavailable_since_ms_ = millis();
  this->update_output_();
  if (this->temperature_sensor_ != nullptr) {
    // Require a publication after setup; do not adopt a possibly cached state.
    this->temperature_sensor_->add_on_state_callback([this](float value) { this->on_temperature_(value); });
  }
  if (this->height_number_ != nullptr) this->height_number_->publish_state(this->flame_height_);
  this->publish_mode_();
  this->publish_diagnostics_(millis());
  this->ready_ = true;
}

void Kalfire::set_temperature_limit(sensor::Sensor *sensor, float ceiling, float restart_margin,
                                   uint32_t max_age, uint32_t unavailable_grace) {
  this->temperature_sensor_ = sensor;
  this->ceiling_ = ceiling;
  this->restart_margin_ = restart_margin;
  this->max_age_ms_ = max_age;
  this->unavailable_grace_ms_ = unavailable_grace;
}

bool Kalfire::request_mode(Mode requested) {
  if (!this->ready_ || requested > Mode::ECO || (this->stopping_ && requested != Mode::OFF)) return false;
  const uint32_t now = millis();
  // An overdue session cannot be extended by a repeated mode command. Also
  // preserve any newer command issued by synchronous diagnostic callbacks.
  const uint32_t revision = this->mode_revision_;
  const bool stopped = this->evaluate_limits_(now);
  if (requested != Mode::OFF && (stopped || revision != this->mode_revision_)) return false;
  if (requested == Mode::OFF) {
    if (this->mode_ != Mode::OFF) this->stop_("User", now);
    return true;
  }
  if (this->temperature_sensor_ != nullptr && (!this->temperature_available_(now) || this->too_warm_)) {
    ESP_LOGW(TAG, "Start/mode request rejected by temperature limit");
    return false;
  }
  if (requested == this->mode_) return true;
  if (this->mode_ == Mode::OFF) {
    this->session_started_ms_ = now;
    this->active_session_timeout_ms_ = this->session_timeout_ms_;
  }
  this->mode_ = requested;
  ++this->mode_revision_;
  this->update_output_();
  this->publish_mode_();
  this->publish_diagnostics_(now);
  return this->mode_ == requested;
}

bool Kalfire::set_flame_height(float height) {
  if (!this->ready_ || !std::isfinite(height) || height < 1 || height > 8 || std::floor(height) != height) {
    ESP_LOGW(TAG, "Ignoring invalid/not-ready flame height request; expected an integer from 1 to 8");
    return false;
  }
  this->evaluate_limits_(millis());
  this->flame_height_ = static_cast<uint8_t>(height);
  this->update_output_();
  return true;
}

bool Kalfire::temperature_available_(uint32_t now) const {
  return this->sample_seen_ && !this->sample_expired_ && std::isfinite(this->temperature_) &&
         static_cast<uint32_t>(now - this->sample_ms_) < this->max_age_ms_;
}

void Kalfire::on_temperature_(float value) {
  const uint32_t now = millis();
  // Enforce any deadline already reached before accepting a recovery sample.
  this->evaluate_limits_(now);
  this->temperature_ = value;
  this->sample_ms_ = now;
  this->sample_seen_ = true;
  this->sample_expired_ = false;
  this->evaluate_limits_(now);
}

bool Kalfire::evaluate_limits_(uint32_t now) {
  if (this->temperature_sensor_ != nullptr) {
    // Once expired, an old sample stays expired even after another millis wrap.
    if (this->sample_seen_ && static_cast<uint32_t>(now - this->sample_ms_) >= this->max_age_ms_) {
      this->sample_expired_ = true;
    }
    if (this->temperature_available_(now)) {
      this->unavailable_tracking_ = false;
      if (this->temperature_ >= this->ceiling_) this->too_warm_ = true;
      else if (this->temperature_ <= this->ceiling_ - this->restart_margin_) this->too_warm_ = false;
    } else if (!this->unavailable_tracking_) {
      this->unavailable_tracking_ = true;
      // Staleness begins at max_age, not when a later loop happens to notice it.
      this->unavailable_since_ms_ = this->sample_seen_ && std::isfinite(this->temperature_)
                                       ? this->sample_ms_ + this->max_age_ms_ : now;
    }
  }
  if (this->mode_ != Mode::OFF) {
    if (this->active_session_timeout_ms_ != 0 &&
        static_cast<uint32_t>(now - this->session_started_ms_) >= this->active_session_timeout_ms_) {
      this->stop_("Timeout", now);
      return true;
    }
    if (this->temperature_sensor_ != nullptr) {
      if (this->temperature_available_(now) && this->too_warm_) {
        this->stop_("Temperature ceiling", now);
        return true;
      }
      if (this->unavailable_tracking_ &&
          static_cast<uint32_t>(now - this->unavailable_since_ms_) >= this->unavailable_grace_ms_) {
        this->stop_("Temperature unavailable", now);
        return true;
      }
    }
  }
  this->publish_diagnostics_(now);
  return false;
}

void Kalfire::loop() { this->evaluate_limits_(millis()); }

void Kalfire::stop_(const char *reason, uint32_t now) {
  // State and output are committed before callbacks. Refuse synchronous restart
  // attempts from Off/stop-reason automations during this transition.
  this->stopping_ = true;
  this->mode_ = Mode::OFF;
  ++this->mode_revision_;
  this->last_stop_reason_ = reason;
  this->update_output_();
  this->publish_mode_();
  this->publish_diagnostics_(now);
  this->stopping_ = false;
}

void Kalfire::update_output_() {
  float voltage = 0.0f;
  if (this->mode_ == Mode::ECO) voltage = 9.5f;
  else if (this->mode_ == Mode::MANUAL) voltage = 3.0f + 6.0f * (this->flame_height_ - 1) / 7.0f;
  ESP_LOGI(TAG, "Requested voltage: %.3f", voltage);
  if (this->output_ != nullptr) this->output_->set_level(voltage / 10.0f);
}

void Kalfire::publish_mode_() {
  const size_t index = static_cast<size_t>(this->mode_);
  if (this->mode_select_ != nullptr &&
      (!this->mode_select_->has_state() || this->mode_select_->active_index().value() != index)) {
    this->mode_select_->publish_state(index);
  }
}

void Kalfire::publish_diagnostics_(uint32_t now) {
  if (this->remaining_sensor_ != nullptr) {
    float remaining = NAN;  // Unlimited, not zero seconds remaining.
    const uint32_t timeout =
        this->mode_ == Mode::OFF ? this->session_timeout_ms_ : this->active_session_timeout_ms_;
    if (timeout != 0) {
      const uint32_t elapsed = this->mode_ == Mode::OFF ? timeout : now - this->session_started_ms_;
      remaining = (timeout - std::min(elapsed, timeout) + 999) / 1000;
    }
    const float old = this->remaining_sensor_->state;
    if (!this->remaining_sensor_->has_state() ||
        (std::isnan(remaining) ? !std::isnan(old) : remaining != old)) {
      this->remaining_sensor_->publish_state(remaining);
    }
  }
  if (this->stop_reason_sensor_ != nullptr &&
      (!this->stop_reason_sensor_->has_state() || this->stop_reason_sensor_->state != this->last_stop_reason_)) {
    this->stop_reason_sensor_->publish_state(this->last_stop_reason_);
  }
  const char *status = "Disabled";
  if (this->temperature_sensor_ != nullptr) {
    status = !this->temperature_available_(now) ? "Unavailable" : this->too_warm_ ? "Too warm" : "Ready";
  }
  if (this->temperature_status_sensor_ != nullptr &&
      (!this->temperature_status_sensor_->has_state() || this->temperature_status_sensor_->state != status)) {
    this->temperature_status_sensor_->publish_state(status);
  }
}

void Kalfire::dump_config() {
  ESP_LOGCONFIG(TAG, "Kalfire manual sessions (requested states only):");
  ESP_LOGCONFIG(TAG, "  Next session timeout: %u ms (0 = disabled)", this->session_timeout_ms_);
  ESP_LOGCONFIG(TAG, "  Temperature limit: %s", this->temperature_sensor_ != nullptr ? "enabled" : "disabled");
}

}  // namespace esphome::kalfire
