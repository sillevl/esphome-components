#include "kalfire.h"
#include <algorithm>

namespace esphome {
namespace kalfire {

const float Kalfire::MIN_FLAME_HEIGHT_VOLTAGE = 3.0;
const float Kalfire::MAX_FLAME_HEIGHT_VOLTAGE = 9.0;
const uint8_t Kalfire::MIN_FLAME_HEIGHT = 0;
const uint8_t Kalfire::MAX_FLAME_HEIGHT = 7;

Kalfire::Kalfire() {
    // Constructor
}

float Kalfire::get_voltage() {
    if(!this->power_on) { return 0; }
    if(this->eco_mode) { return 95; }

    return this->flame_height;
}

float Kalfire::flame_height_to_voltage(uint8_t flame_height) {
    flame_height = std::min(flame_height, this->MAX_FLAME_HEIGHT);
    flame_height = std::max(flame_height, this->MIN_FLAME_HEIGHT);

    float a = MIN_FLAME_HEIGHT_VOLTAGE;
    float b = (MAX_FLAME_HEIGHT_VOLTAGE - MIN_FLAME_HEIGHT_VOLTAGE) / MAX_FLAME_HEIGHT;

    return a + (b * flame_height);
}

} // namespace kalfire
} // namespace esphome