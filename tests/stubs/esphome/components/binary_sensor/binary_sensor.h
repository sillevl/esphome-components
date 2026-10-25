#pragma once
#include <functional>

namespace esphome::binary_sensor {
class BinarySensor {
 public:
  bool state = false;
  bool has_state() const { return has_state_; }
  std::function<void(bool)> callback;
  void publish_state(bool value) {
    state = value;
    has_state_ = true;
    if (callback) callback(value);
  }
 protected:
  bool has_state_{false};
};
}
