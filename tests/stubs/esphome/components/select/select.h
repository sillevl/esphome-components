#pragma once
#include <functional>
#include <optional>
#include <cstddef>
#include "esphome/core/component.h"
namespace esphome::select {
class Select {
 public:
  virtual ~Select() = default;
  bool has_state() const { return has_state_; }
  std::optional<size_t> active_index() const { return has_state_ ? std::optional<size_t>(index_) : std::nullopt; }
  std::function<void(size_t)> callback;
  void publish_state(size_t index) {
    index_ = index;
    has_state_ = true;
    if (callback) callback(index);
  }
 protected:
  virtual void control(size_t index) = 0;
  bool has_state_{false};
  size_t index_{0};
};
}
