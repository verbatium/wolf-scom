#pragma once
#include <array>
#include <optional>
#include <vector>

#include "scom_frame.h"

namespace scom {

enum class Sensor {
  FlowTemperature,
  OutsideTemperature,
  WaterTemperature,
  TargetWaterTemperature,
  ErrorCode,
  Rpm,
  TargetFlowTemperature,
  HeatingDay,
  HotWaterDay,
  OperationMode,
  DwtOutsideTemperature,
  BoilerStatus,
  Pump,
  Heat,
  ThreeWayValve,
  Count
};

struct Publication {
  Sensor sensor;
  float value;
};

class Publisher {
 public:
  std::vector<Publication> on_frame(const Frame &frame);

 private:
  std::array<std::optional<uint8_t>, static_cast<size_t>(Sensor::Count)> last_value_;
};

}  // namespace scom
