#include "test_switch.h"

namespace esphome {
namespace kalfire {

void TestSwitch::write_state(bool state) {
  this->publish_state(state);
//   this->parent_->set_underlying_open_function(state);
}

}  // namespace kalfire
}  // namespace esphome