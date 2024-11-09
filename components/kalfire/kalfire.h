#pragma once

#include "esphome/core/component.h"
#include "esphome/components/switch/switch.h"


namespace esphome {
namespace kalfire {

class Kalfire : public Component {

    public:
        Kalfire();
        void set_enable_flame_state(bool state);
        void set_eco_mode_state(bool state);
        void set_flame_height(uint8_t flame_height);

        void dump_config() override;

    private:
        float flame_height_to_voltage(uint8_t flame_height);

    protected:
        float get_voltage();
        uint8_t flame_height = 0;
        bool power_on = false;
        bool eco_mode = false;

        void update_output();

};

} // namespace status_led
} // namespace esphome