#pragma once
#include "esphome/components/select/select.h"
#include "../kalfire.h"

namespace esphome::kalfire {
class ModeSelect : public select::Select, public Parented<Kalfire> {
 protected:
  void control(size_t index) override;
};
}  // namespace esphome::kalfire
