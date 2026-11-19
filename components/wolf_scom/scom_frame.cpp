#include "scom_frame.h"

namespace scom {

std::optional<Frame> decode(const std::array<uint8_t, 8> &image) {
  if (image[0] == 0x53) return Frame{Kind::Write, image[2], image[4]};
  return Frame{Kind::Read, image[2], image[5]};
}

}  // namespace scom
