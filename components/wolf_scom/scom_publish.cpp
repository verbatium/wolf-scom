#include "scom_publish.h"

namespace scom {

std::vector<Publication> Publisher::on_frame(const Frame &frame) {
  if (last_flow_temperature_ == frame.value) return {};
  last_flow_temperature_ = frame.value;
  return {Publication{Sensor::FlowTemperature, static_cast<float>(frame.value)}};
}

}  // namespace scom
