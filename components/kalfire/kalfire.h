#pragma once

#include <cmath>
#include "esphome/core/component.h"
#include "esphome/components/output/float_output.h"
#include "esphome/components/number/number.h"
#include "esphome/components/select/select.h"
#include "esphome/components/sensor/sensor.h"
#include "esphome/components/text_sensor/text_sensor.h"

namespace esphome::kalfire {

enum class Mode : uint8_t { OFF = 0, MANUAL = 1, ECO = 2 };

class Kalfire : public Component {
 public:
  void setup() override;
  void loop() override;
  void dump_config() override;
  float get_setup_priority() const override { return setup_priority::PROCESSOR; }

  bool request_mode(Mode mode);
  bool set_flame_height(float height);
  Mode get_mode() const { return this->mode_; }

  void add_output(output::FloatOutput *output) { this->output_ = output; }
  void set_mode_select(select::Select *mode) { this->mode_select_ = mode; }
  void set_flame_height_number(number::Number *height) { this->height_number_ = height; }
  void set_session_timeout(uint32_t timeout) { this->session_timeout_ms_ = timeout; }
  void set_temperature_limit(sensor::Sensor *sensor, float ceiling, float restart_margin,
                             uint32_t max_age, uint32_t unavailable_grace);
  void set_time_remaining_sensor(sensor::Sensor *sensor) { this->remaining_sensor_ = sensor; }
  void set_last_stop_reason_sensor(text_sensor::TextSensor *sensor) { this->stop_reason_sensor_ = sensor; }
  void set_temperature_limit_status_sensor(text_sensor::TextSensor *sensor) { this->temperature_status_sensor_ = sensor; }

 protected:
  void on_temperature_(float value);
  bool temperature_available_(uint32_t now) const;
  bool evaluate_limits_(uint32_t now);
  void stop_(const char *reason, uint32_t now);
  void update_output_();
  void publish_mode_();
  void publish_diagnostics_(uint32_t now);

  output::FloatOutput *output_{nullptr};
  select::Select *mode_select_{nullptr};
  number::Number *height_number_{nullptr};
  sensor::Sensor *temperature_sensor_{nullptr};
  sensor::Sensor *remaining_sensor_{nullptr};
  text_sensor::TextSensor *stop_reason_sensor_{nullptr};
  text_sensor::TextSensor *temperature_status_sensor_{nullptr};

  Mode mode_{Mode::OFF};
  uint8_t flame_height_{1};
  bool ready_{false};
  bool stopping_{false};
  uint32_t mode_revision_{0};
  uint32_t session_timeout_ms_{0};
  uint32_t session_started_ms_{0};
  const char *last_stop_reason_{"Startup"};

  float ceiling_{0};
  float restart_margin_{0.5f};
  uint32_t max_age_ms_{300000};
  uint32_t unavailable_grace_ms_{60000};
  float temperature_{NAN};
  uint32_t sample_ms_{0};
  bool sample_seen_{false};
  bool sample_expired_{true};
  bool too_warm_{false};
  bool unavailable_tracking_{true};
  uint32_t unavailable_since_ms_{0};
};

}  // namespace esphome::kalfire
