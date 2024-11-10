#include "eco_mode.h"

namespace esphome {
namespace kalfire {

void EcoModeSwitch::write_state(bool state) {
  this->publish_state(state);
  this->parent_->set_eco_mode_state(state);
}

}  // namespace kalfire
}  // namespace esphome