#pragma once
#include <string>
#include <functional>
namespace esphome::text_sensor {
class TextSensor {
 public:
  std::string state;
  std::function<void(const std::string &)> callback;
  bool has_state() const { return has_state_; }
  void publish_state(const char *value) {
    state = value;
    has_state_ = true;
    if (callback) callback(state);
  }
 protected:
  bool has_state_{false};
};
}
