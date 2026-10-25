#pragma once
#include "esphome/core/component.h"

namespace esphome::button {
class Button {
 public:
  virtual ~Button() = default;
  void press() { this->press_action(); }
 protected:
  virtual void press_action() = 0;
};
}
