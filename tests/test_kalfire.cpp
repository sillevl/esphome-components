// Actual component sources, deterministic clock, ESPHome test doubles; no I2C.
#include <cmath>
#include <iostream>
#include <limits>
#include <string>
#include "esphome/core/hal.h"
#include "components/kalfire/kalfire.h"
#include "components/kalfire/select/mode_select.h"
#include "components/kalfire/number/flame_height.h"

using namespace esphome;
using namespace esphome::kalfire;

class TestNumber : public FlameHeightNumber { public: using FlameHeightNumber::control; };
class TestSelect : public ModeSelect { public: using ModeSelect::control; };

struct Fixture {
  output::FloatOutput dac;
  Kalfire controller;
  TestSelect mode;
  TestNumber height;
  sensor::Sensor temperature, remaining;
  text_sensor::TextSensor reason, status;
  Fixture(uint32_t timeout = 0, bool limit = false, uint32_t max_age = 1000, uint32_t grace = 500) {
    test_millis = 0;
    controller.add_output(&dac);
    mode.set_parent(&controller);
    height.set_parent(&controller);
    controller.set_mode_select(&mode);
    controller.set_flame_height_number(&height);
    controller.set_session_timeout(timeout);
    controller.set_time_remaining_sensor(&remaining);
    controller.set_last_stop_reason_sensor(&reason);
    controller.set_temperature_limit_status_sensor(&status);
    if (limit) controller.set_temperature_limit(&temperature, 23, 0.5f, max_age, grace);
    controller.setup();
  }
  void choose(Mode value) { mode.control(static_cast<size_t>(value)); }
  void at(uint32_t now) { test_millis = now; controller.loop(); }
  bool off() const { return controller.get_mode() == Mode::OFF && voltage() == 0; }
  float voltage() const { return dac.writes.empty() ? -1.0f : dac.writes.back() * 10.0f; }
};

int failures = 0, checks = 0;
void check(bool condition, const std::string &message) {
  ++checks;
  if (!condition) { std::cerr << "FAIL: " << message << '\n'; ++failures; }
}
bool close(float a, float b) { return std::abs(a - b) < 0.0001f; }

int main() {
  {
    Fixture f;
    check(f.off(), "startup writes zero volts");
    check(f.mode.active_index() == 0, "initial mode Off is published");
    check(f.height.state == 1, "initial height 1 is published");
    check(f.status.state == "Disabled" && std::isnan(f.remaining.state), "limits omitted by default");
    check(f.controller.get_setup_priority() < setup_priority::DATA, "setup runs after DAC");
    f.choose(Mode::MANUAL);
    float previous = -1;
    for (int h = 1; h <= 8; ++h) {
      f.height.control(h);
      check(close(f.voltage(), 3.0f + 6.0f * (h - 1) / 7.0f), "height " + std::to_string(h));
      check(f.voltage() > previous, "eight distinct heights");
      previous = f.voltage();
    }
    f.choose(Mode::ECO);
    check(close(f.voltage(), 9.5f), "ECO requests 9.5 V");
    f.height.control(3);
    check(close(f.voltage(), 9.5f), "height selection does not override ECO");
    f.choose(Mode::MANUAL);
    check(close(f.voltage(), 3.0f + 12.0f / 7.0f), "Manual restores selected height");
    f.choose(Mode::OFF);
    f.height.control(8);
    check(f.off() && f.reason.state == "User", "height cannot ignite while Off");
    f.choose(Mode::MANUAL);
    f.at(24 * 60 * 60 * 1000);
    check(!f.off(), "without limits session is manual-only");
  }
  {
    Fixture f;
    f.choose(Mode::MANUAL);
    f.height.control(4);
    auto writes = f.dac.writes.size();
    for (float value : {0.0f, 9.0f, -1.0f, 2.5f, NAN, INFINITY}) {
      f.height.control(value);
      check(f.height.state == 4 && f.dac.writes.size() == writes, "invalid height leaves request unchanged");
    }
    f.mode.control(3);
    check(f.controller.get_mode() == Mode::MANUAL, "invalid mode is ignored");
  }
  {
    Fixture f(5000);
    f.choose(Mode::MANUAL);
    check(f.remaining.state == 5, "timer starts with session");
    f.at(1000); f.choose(Mode::ECO);
    f.at(2000); f.height.control(8);
    f.at(3000); f.choose(Mode::ECO);
    check(f.remaining.state == 2, "mode/height/repeated on do not reset deadline");
    f.at(4999);
    check(!f.off() && f.remaining.state == 1, "timer rounds display up");
    f.at(5000);
    check(f.off() && f.reason.state == "Timeout" && f.remaining.state == 0, "local expiry stops session");
    f.at(20000);
    check(f.off(), "no autonomous restart");
    f.choose(Mode::MANUAL);
    check(!f.off() && f.remaining.state == 5, "new explicit start creates a new session");
  }
  {
    Fixture f(1000);
    f.choose(Mode::ECO);
    test_millis = 1000;
    f.choose(Mode::MANUAL);
    check(f.off() && f.reason.state == "Timeout", "late mode command cannot revive expired session");
  }
  {
    Fixture f(1000);
    f.choose(Mode::MANUAL);
    f.at(500); f.choose(Mode::OFF);
    f.at(700); f.choose(Mode::ECO);
    f.at(1699); check(!f.off(), "new session gets its own deadline");
    f.at(1700); check(f.off(), "new deadline expires");
  }
  {
    Fixture f(0, true);
    f.choose(Mode::MANUAL);
    check(f.off() && f.status.state == "Unavailable", "missing initial temperature blocks start");
    f.temperature.publish_state(22);
    f.choose(Mode::MANUAL);
    check(!f.off() && f.status.state == "Ready", "fresh cool reading permits start");
    f.temperature.publish_state(23);
    check(f.off() && f.reason.state == "Temperature ceiling", "ceiling immediately ends session");
    f.temperature.publish_state(22.8f); f.choose(Mode::ECO);
    check(f.off() && f.status.state == "Too warm", "restart margin blocks early restart");
    f.temperature.publish_state(22.5f);
    check(f.off() && f.status.state == "Ready", "cooling does not restart");
    f.choose(Mode::ECO);
    check(close(f.voltage(), 9.5f), "explicit restart after cooldown accepted");
  }
  {
    Fixture f(0, true);
    f.temperature.publish_state(23); f.choose(Mode::ECO);
    check(f.off(), "already hot temperature blocks initial start");
  }
  {
    Fixture f(0, true);
    f.temperature.publish_state(22); f.choose(Mode::MANUAL);
    f.at(1000);
    check(!f.off() && f.status.state == "Unavailable", "stale temperature starts grace");
    f.at(1499); check(!f.off(), "session continues only during grace");
    f.at(1500); check(f.off() && f.reason.state == "Temperature unavailable", "stale grace expires");
    f.temperature.publish_state(22);
    check(f.off(), "sensor recovery never restarts session");
  }
  {
    Fixture f(0, true);
    f.temperature.publish_state(22); f.choose(Mode::MANUAL);
    f.at(1000); f.at(1200); f.temperature.publish_state(22);
    f.at(1500); check(!f.off() && f.status.state == "Ready", "recovery within grace preserves session");
    f.at(2700); check(f.off(), "new stale interval has new grace");
  }
  {
    Fixture f(0, true);
    f.temperature.publish_state(22); f.choose(Mode::MANUAL);
    f.at(100); f.temperature.publish_state(NAN);
    f.at(400); f.temperature.publish_state(INFINITY);
    f.at(599); check(!f.off(), "invalid samples start one grace interval");
    f.at(600); check(f.off(), "repeated invalid samples do not extend grace");
  }
  {
    Fixture f(0, true);
    f.temperature.publish_state(22); f.choose(Mode::MANUAL);
    // A delayed loop or a recovery sample must not extend an overdue cutoff.
    test_millis = 2000; f.temperature.publish_state(22);
    check(f.off() && f.reason.state == "Temperature unavailable", "late recovery cannot conceal an expired grace");
  }
  {
    Fixture f(0, true, 1000, 0);
    f.temperature.publish_state(22); f.choose(Mode::MANUAL);
    f.temperature.publish_state(NAN);
    check(f.off(), "zero grace stops immediately");
  }
  {
    Fixture f(1000);
    test_millis = UINT32_MAX - 499;
    f.choose(Mode::MANUAL);
    f.at(499); check(!f.off(), "timer works immediately before wrapped deadline");
    f.at(500); check(f.off(), "timer expires across millis wrap");
  }
  {
    Fixture f(0, true, 1000, 500);
    test_millis = UINT32_MAX - 499;
    f.temperature.publish_state(22); f.choose(Mode::MANUAL);
    f.at(999); check(!f.off(), "temperature grace works across millis wrap");
    f.at(1000); check(f.off(), "temperature grace expires across wrap");
  }
  {
    Fixture f;
    f.mode.callback = [&](size_t index) { if (index != 0) f.choose(Mode::OFF); };
    f.choose(Mode::MANUAL);
    check(f.off() && f.mode.active_index() == 0, "nested Off not overwritten by outer Manual");
  }
  {
    Fixture f;
    f.choose(Mode::MANUAL);
    f.height.callback = [&](float h) { if (h == 8) f.height.control(1); };
    f.height.control(8);
    check(f.height.state == 1 && close(f.voltage(), 3), "nested height command is not overwritten");
  }
  {
    Fixture f(1000);
    f.choose(Mode::MANUAL);
    f.mode.callback = [&](size_t index) { if (index == 0) f.choose(Mode::ECO); };
    f.reason.callback = [&](const std::string &) { f.choose(Mode::MANUAL); };
    f.at(1000);
    check(f.off(), "limit-stop callbacks cannot synchronously reignite");
  }
  {
    Fixture f(1000);
    f.choose(Mode::MANUAL);
    f.at(500);
    f.controller.setup();  // Represents new startup; firmware stores no session in flash.
    check(f.off() && f.reason.state == "Startup", "Atom restart returns to Off, never restores session");
  }
  {
    Fixture f(5000);
    f.choose(Mode::MANUAL);
    f.remaining.add_on_state_callback([&](float value) { if (value == 4) f.choose(Mode::OFF); });
    test_millis = 1000;
    f.choose(Mode::ECO);
    check(f.off(), "newer Off from diagnostic callback wins over outer mode request");
  }
  {
    Fixture f(0, true);
    f.temperature.publish_state(22);
    f.at(1000);
    // Model the clock returning to the old timestamp after a full wrap.
    f.at(0); f.choose(Mode::MANUAL);
    check(f.off() && f.status.state == "Unavailable", "expired sample never becomes fresh again on wrap");
  }
  {
    Fixture f(0, true, 1000, 500);
    f.temperature.publish_state(22); f.choose(Mode::MANUAL);
    f.at(999); f.temperature.publish_state(22);
    f.at(1500);
    check(!f.off(), "periodic equal-valued real publications refresh freshness");
  }
  std::cout << checks << " checks: " << failures << " failures\n";
  return failures ? 1 : 0;
}
