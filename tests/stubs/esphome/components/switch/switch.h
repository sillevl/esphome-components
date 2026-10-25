#pragma once
#include <functional>
#include "esphome/core/component.h"

namespace esphome::switch_ {
class Switch {
 public:
  virtual ~Switch() = default;
  bool state = false;
  std::function<void(bool)> callback;
  void turn_on() { write_state(true); }
  void turn_off() { write_state(false); }
  void toggle() { write_state(!state); }
  void publish_state(bool value) {
    if (published_ && state == value) return;
    published_ = true;
    state = value;
    if (callback) callback(value);
  }
 protected:
  virtual void write_state(bool value) = 0;
  bool published_{false};
};
}
