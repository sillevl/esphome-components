#include "mode_select.h"

namespace esphome::kalfire {
void ModeSelect::control(size_t index) {
  if (index <= static_cast<size_t>(Mode::ECO)) this->parent_->request_mode(static_cast<Mode>(index));
}
}  // namespace esphome::kalfire
