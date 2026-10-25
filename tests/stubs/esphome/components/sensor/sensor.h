#pragma once
#include <cmath>
#include <functional>
#include <vector>
namespace esphome::sensor {
class Sensor {
 public:
  float state = NAN;
  bool has_state() const { return has_state_; }
  void publish_state(float value) {
    state = value;
    has_state_ = true;
    for (auto &callback : callbacks_) callback(value);
  }
  template<class F> void add_on_state_callback(F callback) { callbacks_.push_back(callback); }
 protected:
  bool has_state_{false};
  std::vector<std::function<void(float)>> callbacks_;
};
}
