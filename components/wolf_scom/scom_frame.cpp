#include "scom_frame.h"

namespace scom {

std::optional<Frame> decode(const std::array<uint8_t, 8> &image) {
  if (image[3] != image[2]) return std::nullopt;
  if (image[0] == 0x53) {
    if (image[5] != image[4] || image[6] != 0x06 || image[7] != image[4]) return std::nullopt;
    return Frame{Kind::Write, image[2], image[4]};
  }
  if (image[4] != image[2]) return std::nullopt;
  if (image[7] != 0x06) return std::nullopt;
  return Frame{Kind::Read, image[2], image[5]};
}

}  // namespace scom
