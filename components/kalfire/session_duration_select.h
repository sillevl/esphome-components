#pragma once

#include "kalfire.h"
#include "esphome/core/preferences.h"

namespace esphome::kalfire {

// Options/order are paired with SESSION_DURATION_OPTIONS in __init__.py.
class SessionDurationSelect : public select::Select, public Component, public Parented<Kalfire> {
 public:
  void setup() override;
  float get_setup_priority() const override { return setup_priority::HARDWARE; }

 protected:
  void control(size_t index) override;
  void apply_(uint8_t index);
  ESPPreferenceObject pref_;
  bool initialized_{false};
  static constexpr uint8_t DEFAULT_INDEX = 3;  // 1h
  static constexpr uint32_t DURATIONS_MS[] = {
      15 * 60000, 30 * 60000, 45 * 60000, 60 * 60000,
      90 * 60000, 120 * 60000, 180 * 60000, 240 * 60000,
  };
};

}  // namespace esphome::kalfire
