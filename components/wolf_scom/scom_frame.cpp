#include "scom_frame.h"

namespace scom {

namespace {
constexpr uint8_t kWriteHeader = 0x53;
constexpr uint8_t kReadHeader = 0x44;
}  // namespace

std::optional<Frame> decode(const std::array<uint8_t, 8> &image) {
  if (image[3] != image[2]) return std::nullopt;
  if (image[0] == kWriteHeader && image[1] == kWriteHeader) {
    if (image[5] != image[4] || image[6] != 0x06 || image[7] != image[4]) return std::nullopt;
    return Frame{Kind::Write, image[2], image[4]};
  }
  if (image[0] != kReadHeader || image[1] != kReadHeader) return std::nullopt;
  if (image[4] != image[2]) return std::nullopt;
  if (image[7] != 0x06) return std::nullopt;
  return Frame{Kind::Read, image[2], image[5]};
}

}  // namespace scom
