#include "scom_publish.h"

namespace scom {

namespace {
struct Route {
  Kind kind;
  uint8_t cmd;
  Sensor sensor;
};

constexpr Route kRoutes[] = {
    {Kind::Read, 0x68, Sensor::FlowTemperature},
    {Kind::Read, 0x6C, Sensor::OutsideTemperature},
    {Kind::Read, 0x6A, Sensor::WaterTemperature},
    {Kind::Read, 0x6F, Sensor::TargetWaterTemperature},
    {Kind::Read, 0x65, Sensor::ErrorCode},
    {Kind::Read, 0xA0, Sensor::Rpm},
    {Kind::Write, 0x6E, Sensor::TargetFlowTemperature},
    {Kind::Write, 0x72, Sensor::HeatingDay},
    {Kind::Write, 0x74, Sensor::HotWaterDay},
    {Kind::Write, 0x67, Sensor::OperationMode},
    {Kind::Write, 0x71, Sensor::DwtOutsideTemperature},
};

std::optional<Sensor> sensor_for(const Frame &frame) {
  for (const Route &route : kRoutes) {
    if (route.kind == frame.kind && route.cmd == frame.cmd) return route.sensor;
  }
  return std::nullopt;
}
}  // namespace

std::vector<Publication> Publisher::on_frame(const Frame &frame) {
  std::optional<Sensor> sensor = sensor_for(frame);
  if (!sensor) return {};
  std::optional<uint8_t> &last = last_value_[static_cast<size_t>(*sensor)];
  if (last == frame.value) return {};
  last = frame.value;
  return {Publication{*sensor, static_cast<float>(frame.value)}};
}

}  // namespace scom
