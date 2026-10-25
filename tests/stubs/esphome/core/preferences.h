#pragma once
#include <cstdint>
#include <optional>

namespace esphome {
// One entity's fake persistent storage, shared by fresh instances across a reboot.
inline std::optional<uint8_t> test_saved_preference;
inline unsigned test_preference_writes = 0;
class ESPPreferenceObject {
 public:
  template<class T> bool load(T *value) const {
    if (!test_saved_preference) return false;
    *value = *test_saved_preference;
    return true;
  }
  template<class T> bool save(const T *value) {
    test_saved_preference = *value;
    ++test_preference_writes;
    return true;
  }
};
}
