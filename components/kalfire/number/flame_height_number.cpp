#include "flame_height_number.h"

namespace esphome {
namespace kalfire {

void FlameHeightNumber::control(float value) {
    this->parent_->set_flame_height(value);
}

} // namespace kalfire
} // namespace esphome