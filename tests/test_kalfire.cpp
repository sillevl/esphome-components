// Compile the real component against small ESPHome test doubles.
#include <cmath>
#include <iostream>
#include <limits>
#include <string>
#include "components/kalfire/kalfire.h"
#include "components/kalfire/switch/flame_switch.h"
#include "components/kalfire/switch/eco_mode.h"
#include "components/kalfire/number/flame_height.h"

using namespace esphome;
using namespace esphome::kalfire;

class TestNumber : public FlameHeightNumber {
 public:
  using FlameHeightNumber::control;
};

struct Fixture {
  output::FloatOutput dac;
  Kalfire controller;
  FlameSwitch flame;
  EcoModeSwitch eco;
  TestNumber height;
  Fixture() {
    controller.add_output(&dac);
    flame.set_parent(&controller);
    eco.set_parent(&controller);
    height.set_parent(&controller);
    controller.set_flame_switch(&flame);
    controller.set_eco_mode_switch(&eco);
    controller.set_flame_height_number(&height);
  }
  float voltage() const { return dac.writes.empty() ? -1.0f : dac.writes.back() * 10.0f; }
};

int failures = 0;
void check(bool condition, const std::string &message) {
  if (!condition) { std::cerr << "FAIL: " << message << '\n'; ++failures; }
}
bool close(float a, float b) { return std::abs(a - b) < 0.0001f; }

int main() {
  {
    Fixture f;
    f.controller.setup();
    check(close(f.voltage(), 0), "startup explicitly writes zero volts");
    check(f.flame.has_state && !f.flame.state, "initial flame request is published off");
    check(f.eco.has_state && !f.eco.state, "initial ECO request is published off");
    check(f.height.has_state && f.height.state == 1, "initial height 1 is published");
    check(f.flame.assumed_state() && f.eco.assumed_state(), "switches report assumed state");
    check(f.controller.get_setup_priority() < setup_priority::DATA,
          "controller initializes after the DAC's default DATA priority");
  }
  {
    Fixture f;
    f.flame.turn_on();
    check(close(f.voltage(), 3), "initial height is minimum");
    float previous = -1;
    for (int h = 1; h <= 8; ++h) {
      f.height.control(h);
      check(close(f.voltage(), 3.0f + 6.0f * (h - 1) / 7.0f), "height " + std::to_string(h));
      check(f.voltage() > previous, "all eight levels are distinct");
      previous = f.voltage();
    }
    f.eco.turn_on();
    check(close(f.voltage(), 9.5f), "ECO requests 9.5 V");
    f.height.control(3);
    check(close(f.voltage(), 9.5f), "height selection does not override ECO");
    f.eco.turn_off();
    check(close(f.voltage(), 3.0f + 12.0f / 7.0f), "leaving ECO restores requested height");
    f.flame.turn_off();
    f.eco.turn_on();
    f.height.control(8);
    check(close(f.voltage(), 0), "off has priority over ECO and height");
  }
  {
    Fixture f;
    f.flame.turn_on();
    f.height.control(4);
    auto count = f.dac.writes.size();
    for (float value : {0.0f, 9.0f, -1.0f, 2.5f,
                        std::numeric_limits<float>::quiet_NaN(),
                        std::numeric_limits<float>::infinity()}) {
      f.height.control(value);
      check(f.height.state == 4, "invalid height is not published");
      check(f.dac.writes.size() == count, "invalid height does not change output");
    }
  }
  {
    Fixture f;
    // publish_state callbacks are synchronous in ESPHome.
    f.flame.callback = [&](bool on) { if (on) f.flame.turn_off(); };
    f.flame.turn_on();
    check(!f.flame.state && close(f.voltage(), 0), "nested off is not overwritten by outer on");
  }
  {
    Fixture f;
    f.flame.turn_on();
    f.eco.callback = [&](bool on) { if (on) f.eco.turn_off(); };
    f.eco.turn_on();
    check(!f.eco.state && close(f.voltage(), 3), "nested ECO off is not overwritten");
  }
  {
    Fixture f;
    f.flame.turn_on();
    f.height.callback = [&](float h) { if (h == 8) f.height.control(1); };
    f.height.control(8);
    check(f.height.state == 1 && close(f.voltage(), 3), "nested height command is not overwritten");
  }
  std::cout << (failures ? "FAILED: " : "PASS: ") << failures << " failing checks\n";
  return failures ? 1 : 0;
}
