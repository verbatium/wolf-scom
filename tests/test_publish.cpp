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

namespace {
// one fresh Publisher per row: first frame of a command yields exactly this publication
struct Row { Kind kind; uint8_t cmd; uint8_t value; Sensor sensor; float expected; };
void check_rows(const Row *rows, size_t n) {
  for (size_t i = 0; i < n; ++i) {
    scom::Publisher publisher;
    std::vector<Publication> out = publisher.on_frame(Frame{rows[i].kind, rows[i].cmd, rows[i].value});
    if (out.size() != 1) {
      std::printf("  row %zu (cmd 0x%02X): expected 1 publication, got %zu\n", i, rows[i].cmd, out.size());
      ++mini_test::failures();
      continue;
    }
    if (!is(out[0], rows[i].sensor, rows[i].expected)) {
      std::printf("  row %zu (cmd 0x%02X): wrong sensor or value %g\n", i, rows[i].cmd, out[0].value);
      ++mini_test::failures();
    }
  }
}
}  // namespace

TEST(publisher_maps_captured_read_commands_to_their_sensors) {
  // values are the first captured cycle of docs/scom-capture-2026-11-19.log
  const Row rows[] = {
      {Kind::Read, 0x6C, 0x05, Sensor::OutsideTemperature, 5.0f},
      {Kind::Read, 0x6A, 0x20, Sensor::WaterTemperature, 32.0f},
      {Kind::Read, 0x6F, 0x23, Sensor::TargetWaterTemperature, 35.0f},
      {Kind::Read, 0x65, 0x00, Sensor::ErrorCode, 0.0f},
      {Kind::Read, 0xA0, 0x00, Sensor::Rpm, 0.0f},
  };
  check_rows(rows, sizeof(rows) / sizeof(rows[0]));
}

TEST(publisher_ignores_unknown_command) {
  scom::Publisher publisher;
  CHECK_EQ(0, publisher.on_frame(Frame{Kind::Read, 0x99, 0x00}).size());
}

TEST(publisher_maps_captured_write_commands_to_their_sensors) {
  const Row rows[] = {
      {Kind::Write, 0x6E, 0x27, Sensor::TargetFlowTemperature, 39.0f},
      {Kind::Write, 0x72, 0x01, Sensor::HeatingDay, 1.0f},
      {Kind::Write, 0x74, 0x00, Sensor::HotWaterDay, 0.0f},  // first value 0 is still published
      {Kind::Write, 0x67, 0x14, Sensor::OperationMode, 20.0f},
      {Kind::Write, 0x71, 0x05, Sensor::DwtOutsideTemperature, 5.0f},
  };
  check_rows(rows, sizeof(rows) / sizeof(rows[0]));
}

TEST(publisher_ignores_command_arriving_with_the_wrong_frame_kind) {
  // old process_packet switched on the 53/44 header: a write code in a read frame (and vice versa) was unknown
  scom::Publisher publisher;
  CHECK_EQ(0, publisher.on_frame(Frame{Kind::Read, 0x6E, 0x27}).size());
  CHECK_EQ(0, publisher.on_frame(Frame{Kind::Write, 0x68, 0x1F}).size());
}
