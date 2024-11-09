#pragma once

#include "esphome/core/component.h"


namespace esphome {
namespace kalfire {

class Kalfire : public Component {

    public:
        Kalfire();

    private:
        float flame_height_to_voltage(uint8_t flame_height);

    protected:
        float get_voltage();
        uint8_t flame_height = 0;
        bool power_on = false;
        bool eco_mode = false;
        static const float MAX_FLAME_HEIGHT_VOLTAGE;
        static const float MIN_FLAME_HEIGHT_VOLTAGE;
        static const uint8_t MAX_FLAME_HEIGHT;
        static const uint8_t MIN_FLAME_HEIGHT;
};

} // namespace status_led
} // namespace esphome