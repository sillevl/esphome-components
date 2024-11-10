#pragma once

#include "esphome/components/switch/switch.h"
#include "../kalfire.h"

namespace esphome {
namespace kalfire {

class FlameSwitch : public switch_::Switch, public Parented<Kalfire> {
 public:
  FlameSwitch() = default;

 protected:
  void write_state(bool state) override;
};

}  // namespace kalfire
}  // namespace esphome
