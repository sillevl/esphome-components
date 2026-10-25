#include "session_duration_select.h"
#include "esphome/core/log.h"

namespace esphome::kalfire {

static const char *const TAG = "kalfire.duration";

void SessionDurationSelect::setup() {
  this->initialized_ = false;
  this->pref_ = this->make_entity_preference<uint8_t>();
  uint8_t index;
  if (!this->pref_.load(&index) || index >= sizeof(DURATIONS_MS) / sizeof(DURATIONS_MS[0])) {
    index = DEFAULT_INDEX;
  }
  // Restore only the next duration, never the mode or an active session.
  this->parent_->set_session_timeout(DURATIONS_MS[index]);
  this->initialized_ = true;
  this->publish_state(static_cast<size_t>(index));
}

void SessionDurationSelect::control(size_t index) {
  if (!this->initialized_ || index >= sizeof(DURATIONS_MS) / sizeof(DURATIONS_MS[0])) return;
  if (this->has_state() && this->active_index().value() == index) return;
  this->apply_(static_cast<uint8_t>(index));
}

void SessionDurationSelect::apply_(uint8_t index) {
  this->parent_->set_session_timeout(DURATIONS_MS[index]);
  // Save before publishing: a nested, newer selection must win in RAM and flash.
  // ESPHome's normal flash_write_interval controls when pending writes persist.
  if (!this->pref_.save(&index)) ESP_LOGW(TAG, "Could not save session duration preference");
  this->publish_state(static_cast<size_t>(index));
}

}  // namespace esphome::kalfire
