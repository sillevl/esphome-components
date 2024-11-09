#include "kalfire.h"
#include <algorithm>
#include "esphome/core/log.h"

namespace esphome {
namespace kalfire {

static const char *const TAG = "kalfire";

static const float ECO_MODE_VOLTAGE = 9.5;
static const float POWER_OFF_VOLTAGE = 0.0;
static const float MIN_FLAME_HEIGHT_VOLTAGE = 3.0;
static const float MAX_FLAME_HEIGHT_VOLTAGE = 9.0;
static const uint8_t MIN_FLAME_HEIGHT = 0;
static const uint8_t MAX_FLAME_HEIGHT = 7;

Kalfire::Kalfire() {
    // Constructor
}

void Kalfire::set_enable_flame_state(bool state) {
    this->power_on = state;
    ESP_LOGI(TAG, "Power: %s", this->power_on ? "ON" : "OFF");
    update_output();
}

void Kalfire::set_eco_mode_state(bool state) {
    this->eco_mode = state;
    ESP_LOGI(TAG, "Eco mode: %s", this->eco_mode ? "ON" : "OFF");
    update_output();
}

void Kalfire::set_flame_height(uint8_t flame_height) {
    this->flame_height = flame_height;
    ESP_LOGI(TAG, "Flame height: %d", this->flame_height);
    update_output();
}

void Kalfire::update_output() {
    float voltage = this->get_voltage();
    ESP_LOGI(TAG, "Voltage: %f", voltage);
}

float Kalfire::get_voltage() {
    if(!this->power_on) { return POWER_OFF_VOLTAGE; }
    if(this->eco_mode) { return ECO_MODE_VOLTAGE; }

    return flame_height_to_voltage(this->flame_height);
}

float Kalfire::flame_height_to_voltage(uint8_t flame_height) {
    flame_height = std::min(flame_height, MAX_FLAME_HEIGHT);
    flame_height = std::max(flame_height, MIN_FLAME_HEIGHT);

    float a = MIN_FLAME_HEIGHT_VOLTAGE;
    float b = (MAX_FLAME_HEIGHT_VOLTAGE - MIN_FLAME_HEIGHT_VOLTAGE) / MAX_FLAME_HEIGHT;

    return a + (b * flame_height);
}

void Kalfire::dump_config() {
    ESP_LOGCONFIG(TAG, "Kalfire:");
    ESP_LOGCONFIG(TAG, "  Power: %s", this->power_on ? "ON" : "OFF");
    ESP_LOGCONFIG(TAG, "  Eco mode: %s", this->eco_mode ? "ON" : "OFF");
    ESP_LOGCONFIG(TAG, "  Flame height: %d (if power would be ON and eco_mode OFF)", this->flame_height);
}

} // namespace kalfire
} // namespace esphome