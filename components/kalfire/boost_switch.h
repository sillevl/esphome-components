#pragma once

#include "kalfire.h"
#include "esphome/components/switch/switch.h"

namespace esphome::kalfire {
class BoostSwitch : public switch_::Switch, public Parented<Kalfire> {
 protected:
  void write_state(bool state) override {
    // The controller alone publishes the actual internal boost state. Never
    // optimistically publish the requested value: limits/callbacks can reject it.
    if (state) this->parent_->request_boost();
    else this->parent_->cancel_boost();
  }
};
}  // namespace esphome::kalfire
