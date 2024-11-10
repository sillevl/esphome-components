#pragma once

#include "esphome/core/component.h"
#include "esphome/components/switch/switch.h"
#include "esphome/components/output/float_output.h"
#include "esphome/components/number/number.h"


namespace esphome {
namespace kalfire {

class Kalfire : public Component {

    public:
        Kalfire();
        void set_enable_flame_state(bool state);
        void set_eco_mode_state(bool state);
        void set_flame_height(uint8_t flame_height);

        void dump_config() override;

        void add_output(esphome::output::FloatOutput* output);
        void add_flame_height_number(esphome::number::Number* flame_height_number);
        void add_enable_flame_switch(switch_::Switch *enable_flame_switch);
        void add_eco_mode_switch(switch_::Switch *eco_mode_switch);

    private:
        float flame_height_to_voltage(uint8_t flame_height);

    protected:
        float get_voltage();
        uint8_t flame_height = 0;
        bool power_on = false;
        bool eco_mode = false;

        void update_output();

        esphome::output::FloatOutput* output = nullptr;
        esphome::number::Number* flame_height_number = nullptr;

};

} // namespace status_led
} // namespace esphome