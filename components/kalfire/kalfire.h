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
        bool set_flame_height(float flame_height);

        void setup() override;
        float get_setup_priority() const override { return setup_priority::PROCESSOR; }
        void dump_config() override;

        void set_flame_switch(esphome::switch_::Switch* flame_switch) { this->flame_switch = flame_switch; }
        void set_eco_mode_switch(esphome::switch_::Switch* eco_mode_switch) { this->eco_mode_switch = eco_mode_switch; }
        void set_flame_height_number(esphome::number::Number* number) { this->flame_height_number = number; }

        void add_output(esphome::output::FloatOutput* output);

    private:
        float flame_height_to_voltage(uint8_t flame_height);

    protected:
        float get_voltage();
        uint8_t flame_height = 1;
        bool power_on = false;
        bool eco_mode = false;

        void update_output();

        esphome::output::FloatOutput* output = nullptr;
        esphome::number::Number* flame_height_number = nullptr;
        esphome::switch_::Switch* flame_switch = nullptr;
        esphome::switch_::Switch* eco_mode_switch = nullptr;

};

} // namespace kalfire
} // namespace esphome