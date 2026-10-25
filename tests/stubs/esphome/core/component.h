#pragma once
#include <cstdint>
namespace esphome {
namespace setup_priority { constexpr float DATA = 600.0f; constexpr float PROCESSOR = 400.0f; }
class Component {
 public:
  virtual ~Component() = default;
  virtual void setup() {}
  virtual void dump_config() {}
  virtual float get_setup_priority() const { return setup_priority::DATA; }
};
template<class T> class Parented {
 public:
  void set_parent(T *parent) { parent_ = parent; }
 protected:
  T *parent_ = nullptr;
};
}
