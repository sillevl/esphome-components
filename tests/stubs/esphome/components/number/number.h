#pragma once
#include <functional>
#include "esphome/core/component.h"
namespace esphome::number {
class Number {
 public:
  virtual ~Number() = default;
  float state = 0;
  bool has_state = false;
  std::function<void(float)> callback;
  void publish_state(float value) {
    state = value;
    has_state = true;
    if (callback) callback(value);
  }
 protected:
  virtual void control(float value) = 0;
};
}
