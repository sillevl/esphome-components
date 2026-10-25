#pragma once
#include <functional>
#include "esphome/core/component.h"
namespace esphome::switch_ {
class Switch {
 public:
  virtual ~Switch() = default;
  bool state = false;
  bool has_state = false;
  std::function<void(bool)> callback;
  void turn_on() { write_state(true); }
  void turn_off() { write_state(false); }
  void publish_state(bool value) {
    if (has_state && state == value) return;
    state = value;
    has_state = true;
    if (callback) callback(value);
  }
  virtual bool assumed_state() { return false; }
 protected:
  virtual void write_state(bool state) = 0;
};
}
