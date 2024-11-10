#include "flame_height.h"
#include "esphome/core/log.h"

namespace esphome {
namespace kalfire {

static const char *const TAG = "kalfire.flameheight";

void FlameHeightNumber::control(float value) {
    this->publish_state(value);
    this->parent_->set_flame_height(value);
}

} // namespace kalfire
} // namespace esphome