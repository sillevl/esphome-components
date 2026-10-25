#pragma once

#include "kalfire.h"
#include "esphome/components/button/button.h"

namespace esphome::kalfire {
class BoostButton : public button::Button, public Parented<Kalfire> {
 protected:
  void press_action() override { this->parent_->request_boost(); }
};
}  // namespace esphome::kalfire
