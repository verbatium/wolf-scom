#include "scom_publish.h"

namespace scom {

namespace {
struct Route {
  Kind kind;
  uint8_t cmd;
  Sensor sensor;
};

struct Candidate {
  Sensor sensor;
  uint8_t value;
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

float published_value(Sensor sensor, uint8_t raw) {
  switch (sensor) {
    case Sensor::OutsideTemperature:
    case Sensor::DwtOutsideTemperature:
      return static_cast<float>(static_cast<int8_t>(raw));
    case Sensor::HeatingDay:
    case Sensor::HotWaterDay:
      return raw != 0 ? 1.0f : 0.0f;
    default:
      return static_cast<float>(raw);
  }
}

// Parity with the legacy int8 cache compared against a uint8 value: a negative outside
// temperature never equals its cache, so it was republished on every frame.
bool republishes_every_frame(Sensor sensor, uint8_t raw) {
  bool signed_sensor = sensor == Sensor::OutsideTemperature || sensor == Sensor::DwtOutsideTemperature;
  return signed_sensor && raw >= 0x80;
}

constexpr uint8_t kBoilerStatusCmd = 0x70;

std::vector<Candidate> candidates_for(const Frame &frame) {
  if (frame.kind == Kind::Read && frame.cmd == kBoilerStatusCmd) {
    return {{Sensor::BoilerStatus, frame.value},
            {Sensor::Pump, static_cast<uint8_t>((frame.value >> 2) & 1)},
            {Sensor::Heat, static_cast<uint8_t>(frame.value & 1)},
            {Sensor::ThreeWayValve, static_cast<uint8_t>((frame.value >> 1) & 1)}};
  }
  std::optional<Sensor> sensor = sensor_for(frame);
  if (!sensor) return {};
  return {{*sensor, frame.value}};
}
}  // namespace

std::vector<Publication> Publisher::on_frame(const Frame &frame) {
  std::vector<Publication> published;
  for (const Candidate &candidate : candidates_for(frame)) {
    std::optional<uint8_t> &last = last_value_[static_cast<size_t>(candidate.sensor)];
    if (last == candidate.value && !republishes_every_frame(candidate.sensor, candidate.value)) continue;
    last = candidate.value;
    published.push_back({candidate.sensor, published_value(candidate.sensor, candidate.value)});
  }
  return published;
}

}  // namespace scom
