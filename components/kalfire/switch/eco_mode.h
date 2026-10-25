#pragma once

#include "esphome/components/switch/switch.h"
#include "../kalfire.h"

namespace esphome {
namespace kalfire {

class EcoModeSwitch : public switch_::Switch, public Parented<Kalfire> {
 public:
  EcoModeSwitch() = default;
  bool assumed_state() override { return true; }

 protected:
  void write_state(bool state) override;
};

}  // namespace kalfire
}  // namespace esphome
