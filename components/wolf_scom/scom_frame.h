#pragma once
#include <array>
#include <cstdint>
#include <optional>

namespace scom {

enum class Kind { Read, Write };

struct Frame {
  Kind kind;
  uint8_t cmd;
  uint8_t value;
};

std::optional<Frame> decode(const std::array<uint8_t, 8> &image);

}  // namespace scom
