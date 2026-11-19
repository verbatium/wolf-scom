#pragma once
#include <optional>
#include <vector>

#include "scom_frame.h"

namespace scom {

enum class Sensor { FlowTemperature };

struct Publication {
  Sensor sensor;
  float value;
};

class Publisher {
 public:
  std::vector<Publication> on_frame(const Frame &frame);

 private:
  std::optional<uint8_t> last_flow_temperature_;
};

}  // namespace scom
