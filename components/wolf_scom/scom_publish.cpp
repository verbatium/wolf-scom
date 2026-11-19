#include "scom_publish.h"

namespace scom {

namespace {
struct Route {
  uint8_t cmd;
  Sensor sensor;
};

constexpr Route kRoutes[] = {
    {0x68, Sensor::FlowTemperature},  {0x6C, Sensor::OutsideTemperature}, {0x6A, Sensor::WaterTemperature},
    {0x6F, Sensor::TargetWaterTemperature}, {0x65, Sensor::ErrorCode},   {0xA0, Sensor::Rpm},
};

std::optional<Sensor> sensor_for(uint8_t cmd) {
  for (const Route &route : kRoutes) {
    if (route.cmd == cmd) return route.sensor;
  }
  return std::nullopt;
}
}  // namespace

std::vector<Publication> Publisher::on_frame(const Frame &frame) {
  std::optional<Sensor> sensor = sensor_for(frame.cmd);
  if (!sensor) return {};
  std::optional<uint8_t> &last = last_value_[static_cast<size_t>(*sensor)];
  if (last == frame.value) return {};
  last = frame.value;
  return {Publication{*sensor, static_cast<float>(frame.value)}};
}

}  // namespace scom
