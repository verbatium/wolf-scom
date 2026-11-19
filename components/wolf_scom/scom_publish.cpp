#include "scom_publish.h"

namespace scom {

std::vector<Publication> Publisher::on_frame(const Frame &frame) {
  return {Publication{Sensor::FlowTemperature, static_cast<float>(frame.value)}};
}

}  // namespace scom
