#pragma once
#include <vector>
namespace esphome::output {
class FloatOutput {
 public:
  std::vector<float> writes;
  void set_level(float value) { writes.push_back(value); }
};
}
