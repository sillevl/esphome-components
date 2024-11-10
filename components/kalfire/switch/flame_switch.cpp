#include "flame_switch.h"

namespace esphome {
namespace kalfire {

void FlameSwitch::write_state(bool state) {
  this->publish_state(state);
  this->parent_->set_enable_flame_state(state);
}

}  // namespace kalfire
}  // namespace esphome