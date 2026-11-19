#include <cstddef>
#include <vector>

#include "capture_frames.h"
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

TEST(publisher_splits_boiler_status_into_status_and_three_bit_sensors) {
  // 0x0C = pump on, idle (captured). Old order: status, pump (bit2), heat (bit0), three-way valve (bit1)
  scom::Publisher publisher;
  std::vector<Publication> out = publisher.on_frame(Frame{Kind::Read, 0x70, 0x0C});
  CHECK_EQ(4, out.size());
  if (out.size() != 4) return;
  CHECK(is(out[0], Sensor::BoilerStatus, 12.0f));
  CHECK(is(out[1], Sensor::Pump, 1.0f));
  CHECK(is(out[2], Sensor::Heat, 0.0f));
  CHECK(is(out[3], Sensor::ThreeWayValve, 0.0f));
}

TEST(publisher_dedupes_each_status_bit_independently) {
  scom::Publisher publisher;
  publisher.on_frame(Frame{Kind::Read, 0x70, 0x0C});
  // 0x0C -> 0x0D: burner on. status and heat change, pump and valve do not
  std::vector<Publication> out = publisher.on_frame(Frame{Kind::Read, 0x70, 0x0D});
  CHECK_EQ(2, out.size());
  if (out.size() != 2) return;
  CHECK(is(out[0], Sensor::BoilerStatus, 13.0f));
  CHECK(is(out[1], Sensor::Heat, 1.0f));
  // 0x0D -> 0x0E: heat off, valve on
  out = publisher.on_frame(Frame{Kind::Read, 0x70, 0x0E});
  CHECK_EQ(3, out.size());
  if (out.size() != 3) return;
  CHECK(is(out[0], Sensor::BoilerStatus, 14.0f));
  CHECK(is(out[1], Sensor::Heat, 0.0f));
  CHECK(is(out[2], Sensor::ThreeWayValve, 1.0f));
  // 0x0E -> 0x0A: pump off only visible bit change besides status (0x0A = 1010b: bit1 stays)
  out = publisher.on_frame(Frame{Kind::Read, 0x70, 0x0A});
  CHECK_EQ(2, out.size());
  if (out.size() != 2) return;
  CHECK(is(out[0], Sensor::BoilerStatus, 10.0f));
  CHECK(is(out[1], Sensor::Pump, 0.0f));
}

TEST(publisher_publishes_outside_temperatures_as_signed_bytes) {
  // 6C and 71 are int8 degrees C: 0xFB = -5, 0x80 = -128
  const Row rows[] = {
      {Kind::Read, 0x6C, 0xFB, Sensor::OutsideTemperature, -5.0f},
      {Kind::Write, 0x71, 0xFB, Sensor::DwtOutsideTemperature, -5.0f},
      {Kind::Read, 0x6C, 0x7F, Sensor::OutsideTemperature, 127.0f},
  };
  check_rows(rows, sizeof(rows) / sizeof(rows[0]));
}

TEST(publisher_publishes_binary_day_flags_as_zero_or_one) {
  // old code passed the raw byte to a bool sensor: any non-zero value is true
  const Row rows[] = {
      {Kind::Write, 0x72, 0x02, Sensor::HeatingDay, 1.0f},
      {Kind::Write, 0x74, 0x05, Sensor::HotWaterDay, 1.0f},
  };
  check_rows(rows, sizeof(rows) / sizeof(rows[0]));
}

// ---------------------------------------------------------------------------------------------
// Characterization oracle: the pre-extraction WolfScom::loop/process_packet logic, copied verbatim
// from the code at commit 0d7cc28 (components/wolf_scom/wolf_scom.cpp/.h). Only the ESPHome sensor
// objects are replaced by recording stubs (publish_state records instead of publishing).
// ---------------------------------------------------------------------------------------------
#define ESP_LOGW(...) \
  do {                \
  } while (0)

namespace oracle {
using Seq = std::vector<Publication>;

struct RecSensor {
  Seq *seq;
  Sensor id;
  void publish_state(float v) { seq->push_back(Publication{id, v}); }
};
struct RecBinarySensor {
  Seq *seq;
  Sensor id;
  void publish_state(bool v) { seq->push_back(Publication{id, v ? 1.0f : 0.0f}); }
};

class OldWolfScom {
 public:
  explicit OldWolfScom(Seq *seq)
      : target_flow_temperature_sensor_{seq, Sensor::TargetFlowTemperature},
        heating_day_sensor_{seq, Sensor::HeatingDay},
        hot_water_day_sensor_{seq, Sensor::HotWaterDay},
        operation_mode_sensor_{seq, Sensor::OperationMode},
        outside_temperature_sensor_{seq, Sensor::OutsideTemperature},
        dwt_outside_temperature_sensor_{seq, Sensor::DwtOutsideTemperature},
        flow_temperature_sensor_{seq, Sensor::FlowTemperature},
        water_temperature_sensor_{seq, Sensor::WaterTemperature},
        target_water_temperature_sensor_{seq, Sensor::TargetWaterTemperature},
        error_code_sensor_{seq, Sensor::ErrorCode},
        rpm_sensor_{seq, Sensor::Rpm},
        boiler_status_sensor_{seq, Sensor::BoilerStatus},
        pump_sensor_{seq, Sensor::Pump},
        heat_sensor_{seq, Sensor::Heat},
        three_way_valve_sensor_{seq, Sensor::ThreeWayValve} {
    target_flow_temperature_sensor = &target_flow_temperature_sensor_;
    heating_day_sensor = &heating_day_sensor_;
    hot_water_day_sensor = &hot_water_day_sensor_;
    operation_mode_sensor = &operation_mode_sensor_;
    outside_temperature_sensor = &outside_temperature_sensor_;
    dwt_outside_temperature_sensor = &dwt_outside_temperature_sensor_;
    flow_temperature_sensor = &flow_temperature_sensor_;
    water_temperature_sensor = &water_temperature_sensor_;
    target_water_temperature_sensor = &target_water_temperature_sensor_;
    error_code_sensor = &error_code_sensor_;
    rpm_sensor = &rpm_sensor_;
    boiler_status_sensor = &boiler_status_sensor_;
    pump_sensor = &pump_sensor_;
    heat_sensor = &heat_sensor_;
    three_way_valve_sensor = &three_way_valve_sensor_;
  }

  // old WolfScom::loop body for one received byte
  void feed(uint8_t byte) {
    buffer_.push_back(byte);
    if (buffer_.size() >= 8) {
      if (process_packet()) {
        buffer_.erase(buffer_.begin(), buffer_.begin() + 8);
      } else {
        buffer_.erase(buffer_.begin());
      }
    }
  }

 private:
  uint8_t last_target_flow_temperature = -1;
  int8_t last_outside_temperature = -128;
  int8_t last_dwt_outside_temperature = -128;
  uint8_t last_flow_temperature = -1;
  uint8_t last_water_temperature = -1;
  uint8_t last_target_water_temperature = -1;
  uint8_t last_error_code = -1;
  uint8_t last_rpm = -1;
  uint8_t last_boiler_status = -1;
  uint8_t last_operation_mode = -1;
  uint8_t last_heating_day = -1;
  uint8_t last_hot_water_day = -1;
  uint8_t last_pump = -1;
  uint8_t last_three_way_valve = -1;
  uint8_t last_heat = -1;

  std::vector<uint8_t> buffer_;

  RecSensor target_flow_temperature_sensor_;
  RecBinarySensor heating_day_sensor_;
  RecBinarySensor hot_water_day_sensor_;
  RecSensor operation_mode_sensor_;
  RecSensor outside_temperature_sensor_;
  RecSensor dwt_outside_temperature_sensor_;
  RecSensor flow_temperature_sensor_;
  RecSensor water_temperature_sensor_;
  RecSensor target_water_temperature_sensor_;
  RecSensor error_code_sensor_;
  RecSensor rpm_sensor_;
  RecSensor boiler_status_sensor_;
  RecBinarySensor pump_sensor_;
  RecBinarySensor heat_sensor_;
  RecBinarySensor three_way_valve_sensor_;

  RecSensor *target_flow_temperature_sensor = nullptr;
  RecBinarySensor *heating_day_sensor = nullptr;
  RecBinarySensor *hot_water_day_sensor = nullptr;
  RecSensor *operation_mode_sensor = nullptr;
  RecSensor *outside_temperature_sensor = nullptr;
  RecSensor *dwt_outside_temperature_sensor = nullptr;
  RecSensor *flow_temperature_sensor = nullptr;
  RecSensor *water_temperature_sensor = nullptr;
  RecSensor *target_water_temperature_sensor = nullptr;
  RecSensor *error_code_sensor = nullptr;
  RecSensor *rpm_sensor = nullptr;
  RecSensor *boiler_status_sensor = nullptr;
  RecBinarySensor *pump_sensor = nullptr;
  RecBinarySensor *heat_sensor = nullptr;
  RecBinarySensor *three_way_valve_sensor = nullptr;

  bool process_packet() {
    if (buffer_.size() < 8) {
      return false;
    }

    uint8_t *buffer = buffer_.data();
    if (buffer[0] == 0x53 && buffer[1] == 0x53) {
      uint8_t command = buffer[2];
      uint8_t value = buffer[4];

      if (command == buffer[3] && value == buffer[5] && buffer[6] == 0x06 && value == buffer[7]) {
        switch (command) {
          case 0x6E:
            if (this->target_flow_temperature_sensor != nullptr && this->last_target_flow_temperature != value) {
              this->target_flow_temperature_sensor->publish_state(value);
              this->last_target_flow_temperature = value;
            }
            break;
          case 0x72:
            if (this->heating_day_sensor != nullptr && this->last_heating_day != value) {
              this->heating_day_sensor->publish_state(value);
              this->last_heating_day = value;
            }
            break;
          case 0x74:
            if (this->hot_water_day_sensor != nullptr && this->last_hot_water_day != value) {
              this->hot_water_day_sensor->publish_state(value);
              this->last_hot_water_day = value;
            }
            break;
          case 0x67:
            if (this->operation_mode_sensor != nullptr && this->last_operation_mode != value) {
              this->operation_mode_sensor->publish_state(value);
              this->last_operation_mode = value;
            }
            break;
          case 0x71:
            if (this->dwt_outside_temperature_sensor != nullptr && this->last_dwt_outside_temperature != value) {
              this->dwt_outside_temperature_sensor->publish_state(static_cast<int8_t>(value));
              this->last_dwt_outside_temperature = value;
            }
            break;
          default:
            ESP_LOGW("uart_protocol_sensor", "Unknown command: 0x%02X", command);
            break;
        }
        return true;
      }
    } else if (buffer[0] == 0x44 && buffer[1] == 0x44) {
      uint8_t command = buffer[2];
      uint8_t value = buffer[5];

      if (command == buffer[3] && command == buffer[4] && buffer[7] == 0x06) {
        switch (command) {
          case 0x6C:
            if (this->outside_temperature_sensor != nullptr && this->last_outside_temperature != value) {
              this->outside_temperature_sensor->publish_state(static_cast<int8_t>(value));
              this->last_outside_temperature = value;
            }
            break;
          case 0x68:
            if (this->flow_temperature_sensor != nullptr && this->last_flow_temperature != value) {
              this->flow_temperature_sensor->publish_state(value);
              this->last_flow_temperature = value;
            }
            break;
          case 0x6A:
            if (this->water_temperature_sensor != nullptr && this->last_water_temperature != value) {
              this->water_temperature_sensor->publish_state(value);
              this->last_water_temperature = value;
            }
            break;
          case 0x6F:
            if (this->target_water_temperature_sensor != nullptr && this->last_target_water_temperature != value) {
              this->target_water_temperature_sensor->publish_state(value);
              this->last_target_water_temperature = value;
            }
            break;
          case 0x65:
            if (this->error_code_sensor != nullptr && this->last_error_code != value) {
              this->error_code_sensor->publish_state(value);
              this->last_error_code = value;
            }
            break;
          case 0xA0:
            if (this->rpm_sensor != nullptr && this->last_rpm != value) {
              this->rpm_sensor->publish_state(value);
              this->last_rpm = value;
            }
            break;
          case 0x70:
            if (this->boiler_status_sensor != nullptr && this->last_boiler_status != value) {
              this->boiler_status_sensor->publish_state(value);
              this->last_boiler_status = value;
            }
            if (this->pump_sensor != nullptr && (this->last_pump != (value & (1 << 2)))) {
              this->pump_sensor->publish_state((value >> 2) & 1);
              this->last_pump = value & (1 << 2);
            }
            if (this->heat_sensor != nullptr && this->last_heat != (value & (1 << 0))) {
              this->heat_sensor->publish_state(value & 1);
              this->last_heat = value & (1 << 0);
            }
            if (this->three_way_valve_sensor != nullptr && this->last_three_way_valve != (value & (1 << 1))) {
              this->three_way_valve_sensor->publish_state((value >> 1) & 1);
              this->last_three_way_valve = value & (1 << 1);
            }
            break;
          default:
            ESP_LOGW("uart_protocol_sensor", "Unknown command: 0x%02X", command);
            break;
        }
        return true;
      }
    }

    return false;
  }
};

Seq run_old(const uint8_t *bytes, size_t n) {
  Seq seq;
  OldWolfScom old(&seq);
  for (size_t i = 0; i < n; ++i) old.feed(bytes[i]);
  return seq;
}
}  // namespace oracle

namespace {
oracle::Seq run_new(const uint8_t *bytes, size_t n) {
  oracle::Seq seq;
  scom::StreamParser parser;
  scom::Publisher publisher;
  for (size_t i = 0; i < n; ++i) {
    if (auto frame = parser.push(bytes[i])) {
      for (const Publication &p : publisher.on_frame(*frame)) seq.push_back(p);
    }
  }
  return seq;
}

// Returns the index of the first difference, or -1 when the two sequences are identical.
long first_difference(const oracle::Seq &a, const oracle::Seq &b) {
  size_t n = a.size() < b.size() ? a.size() : b.size();
  for (size_t i = 0; i < n; ++i) {
    if (a[i].sensor != b[i].sensor || a[i].value != b[i].value) return static_cast<long>(i);
  }
  return a.size() == b.size() ? -1 : static_cast<long>(n);
}
}  // namespace

TEST(characterization_capture_replay_matches_old_process_packet) {
  oracle::Seq old_seq = oracle::run_old(capture::kCaptureBytes, capture::kCaptureLength);
  oracle::Seq new_seq = run_new(capture::kCaptureBytes, capture::kCaptureLength);
  // guard against two empty sequences agreeing: the capture does produce publications (first cycle alone: 14)
  CHECK(old_seq.size() >= 14);
  CHECK_EQ(old_seq.size(), new_seq.size());
  CHECK_EQ(-1, first_difference(old_seq, new_seq));
}

namespace {
void append_read(std::vector<uint8_t> &s, uint8_t cmd, uint8_t v) {
  const uint8_t f[8] = {0x44, 0x44, cmd, cmd, cmd, v, v, 0x06};
  s.insert(s.end(), f, f + 8);
}
void append_write(std::vector<uint8_t> &s, uint8_t cmd, uint8_t v) {
  const uint8_t f[8] = {0x53, 0x53, cmd, cmd, v, v, 0x06, v};
  s.insert(s.end(), f, f + 8);
}
}  // namespace

TEST(characterization_synthetic_stream_with_value_changes_junk_and_unknown_codes_matches_old) {
  // The capture only ever shows status 0x0C and constant values; this stream exercises every mapping,
  // every status bit, repeated and changed values, stray bytes and an unknown code (all synthetic).
  std::vector<uint8_t> s;
  const uint8_t statuses[] = {0x0C, 0x0D, 0x0D, 0x0E, 0x0F, 0x08, 0x0C, 0x0C, 0x0A, 0x01, 0x02, 0x04};
  for (uint8_t st : statuses) {
    append_read(s, 0x70, st);
    s.push_back(0x00);  // stray byte between frames
  }
  const uint8_t values[] = {0x10, 0x10, 0x11, 0x00, 0x00, 0x7F, 0x01, 0x02};
  const uint8_t read_cmds[] = {0x68, 0x6A, 0x6F, 0x65, 0xA0, 0x6C};
  const uint8_t write_cmds[] = {0x6E, 0x72, 0x74, 0x67, 0x71};
  for (uint8_t v : values) {
    for (uint8_t c : read_cmds) append_read(s, c, v);
    for (uint8_t c : write_cmds) append_write(s, c, v);
    append_read(s, 0x99, v);   // unknown read code
    append_write(s, 0x99, v);  // unknown write code
  }
  oracle::Seq old_seq = oracle::run_old(s.data(), s.size());
  oracle::Seq new_seq = run_new(s.data(), s.size());
  CHECK(old_seq.size() >= 60);
  CHECK_EQ(old_seq.size(), new_seq.size());
  CHECK_EQ(-1, first_difference(old_seq, new_seq));
}

TEST(negative_outside_temperatures_republish_every_frame_like_old_int8_dedupe_quirk) {
  // Pinned quirk (synthetic; the capture only has +5 C). Old code kept int8_t caches but compared them with the
  // uint8_t value: for bytes 128..255 (negative temperatures) the comparison never matched, so every frame
  // republished. Pre-existing bug, preserved by this extraction; registered separately as an issue.
  scom::Publisher publisher;
  for (int i = 0; i < 3; ++i) {
    std::vector<Publication> out = publisher.on_frame(Frame{Kind::Read, 0x6C, 0xFB});
    CHECK_EQ(1, out.size());
    if (out.size() == 1) CHECK(is(out[0], Sensor::OutsideTemperature, -5.0f));
  }
  for (int i = 0; i < 3; ++i) {
    std::vector<Publication> out = publisher.on_frame(Frame{Kind::Write, 0x71, 0xFB});
    CHECK_EQ(1, out.size());
    if (out.size() == 1) CHECK(is(out[0], Sensor::DwtOutsideTemperature, -5.0f));
  }
  // non-negative values still dedupe
  CHECK_EQ(1, publisher.on_frame(Frame{Kind::Read, 0x6C, 0x05}).size());
  CHECK_EQ(0, publisher.on_frame(Frame{Kind::Read, 0x6C, 0x05}).size());
  // -128 (0x80) is the old cache's initial value, still republished every frame
  CHECK_EQ(1, publisher.on_frame(Frame{Kind::Read, 0x6C, 0x80}).size());
  CHECK_EQ(1, publisher.on_frame(Frame{Kind::Read, 0x6C, 0x80}).size());
}

TEST(characterization_negative_temperature_stream_matches_old) {
  std::vector<uint8_t> s;
  const uint8_t temps[] = {0x05, 0x05, 0xFB, 0xFB, 0xFB, 0x80, 0x80, 0x7F, 0x7F, 0x00, 0x00, 0xFF, 0xFE, 0xFE};
  for (uint8_t t : temps) {
    append_read(s, 0x6C, t);
    append_write(s, 0x71, t);
  }
  oracle::Seq old_seq = oracle::run_old(s.data(), s.size());
  oracle::Seq new_seq = run_new(s.data(), s.size());
  CHECK_EQ(22, old_seq.size());  // 2 sensors x (14 frames - 3 deduped non-negative repeats)
  CHECK_EQ(old_seq.size(), new_seq.size());
  CHECK_EQ(-1, first_difference(old_seq, new_seq));
}

TEST(publisher_reports_which_frames_carry_a_known_command) {
  // The adapter logs "Unknown command" (owner's discovery signal, see #4) when this is false; core stays log-free.
  CHECK(scom::Publisher::is_known_command(Frame{Kind::Read, 0x68, 0x1F}));
  CHECK(scom::Publisher::is_known_command(Frame{Kind::Read, 0x70, 0x0C}));
  CHECK(scom::Publisher::is_known_command(Frame{Kind::Write, 0x6E, 0x27}));
  CHECK(!scom::Publisher::is_known_command(Frame{Kind::Read, 0x99, 0x00}));
  CHECK(!scom::Publisher::is_known_command(Frame{Kind::Write, 0x99, 0x00}));
  CHECK(!scom::Publisher::is_known_command(Frame{Kind::Write, 0x68, 0x1F}));  // read code in a write frame
  CHECK(!scom::Publisher::is_known_command(Frame{Kind::Read, 0x6E, 0x27}));   // write code in a read frame
}

TEST(first_0xFF_value_on_an_unsigned_sensor_is_published) {
  // B9d, owner-ratified bug fix: the old code initialised its uint8 caches to 255 (-1), so a first value of
  // 0xFF was swallowed. The Publisher publishes it (and then dedupes the repeat).
  scom::Publisher publisher;
  std::vector<Publication> out = publisher.on_frame(Frame{Kind::Read, 0x65, 0xFF});
  CHECK_EQ(1, out.size());
  if (out.size() == 1) CHECK(is(out[0], Sensor::ErrorCode, 255.0f));
  CHECK_EQ(0, publisher.on_frame(Frame{Kind::Read, 0x65, 0xFF}).size());
  out = publisher.on_frame(Frame{Kind::Read, 0xA0, 0xFF});
  CHECK_EQ(1, out.size());
  if (out.size() == 1) CHECK(is(out[0], Sensor::Rpm, 255.0f));
}
