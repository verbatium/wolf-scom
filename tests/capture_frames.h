// Byte images taken verbatim from docs/scom-capture-2026-11-19.log (cycle #1 of the capture).
#pragma once
#include <array>
#include <cstdint>

namespace capture {
using Image = std::array<uint8_t, 8>;
// 44 44 68 68 68 1F 1F 06 : read 0x68 flow temperature = 0x1F (31 C)
constexpr Image kRead68 = {0x44, 0x44, 0x68, 0x68, 0x68, 0x1F, 0x1F, 0x06};
}  // namespace capture
