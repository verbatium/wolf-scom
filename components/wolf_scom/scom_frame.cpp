#include "scom_frame.h"

namespace scom {

std::optional<Frame> decode(const std::array<uint8_t, 8> &image) {
  return Frame{Kind::Read, image[2], image[5]};
}

}  // namespace scom
