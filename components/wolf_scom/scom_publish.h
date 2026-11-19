#pragma once
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
};

}  // namespace scom
