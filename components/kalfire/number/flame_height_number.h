#pragma once

#include "esphome/components/number/number.h"
#include "kalfire.h"

namespace esphome {
namespace kalfire {

class FlameHeightNumber : public number::Number, public Parented<Kalfire> {
    public:
        FlameHeightNumber() = default;

    protected:
        void control(float value) override;
};

} // namespace kalfire
} // namespace esphome