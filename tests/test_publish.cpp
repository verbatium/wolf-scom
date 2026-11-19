#include <vector>

#include "mini_test.h"
#include "scom_frame.h"
#include "scom_publish.h"

namespace {
using scom::Frame;
using scom::Kind;
using scom::Publication;
using scom::Sensor;

bool is(const Publication &p, Sensor s, float v) { return p.sensor == s && p.value == v; }
}  // namespace

TEST(publisher_maps_flow_temperature_frame_to_sensor_publication) {
  scom::Publisher publisher;
  std::vector<Publication> out = publisher.on_frame(Frame{Kind::Read, 0x68, 0x1F});
  CHECK_EQ(1, out.size());
  if (out.size() != 1) return;
  CHECK(is(out[0], Sensor::FlowTemperature, 31.0f));
}

TEST(publisher_dedupes_unchanged_value_and_republishes_on_change) {
  scom::Publisher publisher;
  CHECK_EQ(1, publisher.on_frame(Frame{Kind::Read, 0x68, 0x1F}).size());
  CHECK_EQ(0, publisher.on_frame(Frame{Kind::Read, 0x68, 0x1F}).size());
  std::vector<Publication> changed = publisher.on_frame(Frame{Kind::Read, 0x68, 0x20});
  CHECK_EQ(1, changed.size());
  if (changed.size() != 1) return;
  CHECK(is(changed[0], Sensor::FlowTemperature, 32.0f));
  CHECK_EQ(0, publisher.on_frame(Frame{Kind::Read, 0x68, 0x20}).size());
}
