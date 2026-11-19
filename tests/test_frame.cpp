#include <vector>

#include "capture_frames.h"
#include "mini_test.h"
#include "scom_frame.h"

TEST(decode_captured_read_68) {
  auto f = scom::decode(capture::kRead68);
  CHECK(f.has_value());
  if (!f) return;
  CHECK(f->kind == scom::Kind::Read);
  CHECK_EQ(0x68, f->cmd);
  CHECK_EQ(0x1F, f->value);
}

TEST(decode_captured_write_6E) {
  auto f = scom::decode(capture::kWrite6E);
  CHECK(f.has_value());
  if (!f) return;
  CHECK(f->kind == scom::Kind::Write);
  CHECK_EQ(0x6E, f->cmd);
  CHECK_EQ(0x27, f->value);
}

TEST(decode_rejects_echo_byte_differing_from_command) {
  // captured images with byte 3 (the echo of the command byte) corrupted
  auto read = capture::kRead68;
  read[3] = 0x69;
  CHECK(!scom::decode(read).has_value());
  auto write = capture::kWrite6E;
  write[3] = 0x6F;
  CHECK(!scom::decode(write).has_value());
}

TEST(decode_rejects_read_frame_whose_ack_is_not_06) {
  auto read = capture::kRead68;
  read[7] = 0x15;  // NAK-looking byte instead of ACK
  CHECK(!scom::decode(read).has_value());
  read[7] = 0x00;
  CHECK(!scom::decode(read).has_value());
}

TEST(decode_is_lenient_on_read_value_echo_like_old_process_packet) {
  // synthetic: byte6 (value echo) != byte5; old code never looked at byte6, value is byte5
  auto read = capture::kRead68;
  read[6] = 0x77;
  auto f = scom::decode(read);
  CHECK(f.has_value());
  if (!f) return;
  CHECK(f->kind == scom::Kind::Read);
  CHECK_EQ(0x68, f->cmd);
  CHECK_EQ(0x1F, f->value);
}

TEST(decode_accepts_structurally_valid_frame_with_unknown_command) {
  // synthetic: the capture contains no unknown code; old code consumed all 8 bytes
  const std::array<uint8_t, 8> unknown = {0x44, 0x44, 0x99, 0x99, 0x99, 0x00, 0x00, 0x06};
  auto f = scom::decode(unknown);
  CHECK(f.has_value());
  if (!f) return;
  CHECK(f->kind == scom::Kind::Read);
  CHECK_EQ(0x99, f->cmd);
  CHECK_EQ(0x00, f->value);
}

TEST(decode_rejects_read_frame_whose_third_command_byte_differs) {
  // old process_packet required command == buffer[4] for reads (parity, added to the plan as B7c)
  auto read = capture::kRead68;
  read[4] = 0x69;
  CHECK(!scom::decode(read).has_value());
}

TEST(decode_rejects_write_frame_with_inconsistent_value_or_ack) {
  // old process_packet: write needs byte5 == byte4, byte6 == 06, byte7 == byte4 (parity, added as B7d)
  auto bad_echo = capture::kWrite6E;
  bad_echo[5] = 0x28;
  CHECK(!scom::decode(bad_echo).has_value());
  auto bad_ack = capture::kWrite6E;
  bad_ack[6] = 0x15;
  CHECK(!scom::decode(bad_ack).has_value());
  auto bad_trailer = capture::kWrite6E;
  bad_trailer[7] = 0x28;
  CHECK(!scom::decode(bad_trailer).has_value());
}

TEST(decode_rejects_images_without_a_valid_header) {
  // old process_packet only looked at images starting 53 53 or 44 44 (added as B7e)
  auto junk_head = capture::kRead68;
  junk_head[0] = 0x00;
  CHECK(!scom::decode(junk_head).has_value());
  auto half_head = capture::kRead68;
  half_head[1] = 0x53;
  CHECK(!scom::decode(half_head).has_value());
  auto write_half_head = capture::kWrite6E;
  write_half_head[1] = 0x44;
  CHECK(!scom::decode(write_half_head).has_value());
}

TEST(stream_parser_resyncs_after_stray_junk_byte) {
  scom::StreamParser parser;
  std::vector<scom::Frame> frames;
  auto feed = [&](uint8_t b) {
    if (auto f = parser.push(b)) frames.push_back(*f);
  };
  feed(0x00);  // stray byte before the frame
  for (uint8_t b : capture::kRead68) feed(b);
  CHECK_EQ(1, frames.size());
  if (frames.empty()) return;
  CHECK(frames[0].kind == scom::Kind::Read);
  CHECK_EQ(0x68, frames[0].cmd);
  CHECK_EQ(0x1F, frames[0].value);
}

TEST(stream_parser_consumes_all_eight_bytes_of_unknown_command_frame) {
  // synthetic. Unknown frame A = 44 44 99 99 99 44 44 06 whose tail "44 44 06" plus the following
  // bytes "06 06 00 00 06" would form a second, overlapping valid read frame (cmd 06).
  // A parser that kept A's bytes after accepting it (slide-by-one) would report that phantom frame.
  const uint8_t stream[] = {0x44, 0x44, 0x99, 0x99, 0x99, 0x44, 0x44, 0x06, 0x06, 0x06, 0x00, 0x00, 0x06};
  scom::StreamParser parser;
  std::vector<scom::Frame> frames;
  for (uint8_t b : stream) if (auto f = parser.push(b)) frames.push_back(*f);
  CHECK_EQ(1, frames.size());
  if (frames.empty()) return;
  CHECK_EQ(0x99, frames[0].cmd);
  CHECK_EQ(0x44, frames[0].value);
  // and the next captured frame is still found right after
  for (uint8_t b : capture::kRead68) if (auto f = parser.push(b)) frames.push_back(*f);
  CHECK_EQ(2, frames.size());
}

TEST(stream_parser_replays_full_capture_as_198_frames_in_cycle_order) {
  static_assert(capture::kCaptureLength == 1619, "capture has 1619 bytes");
  const uint8_t cycle[12] = {0x6E, 0x72, 0x74, 0x6C, 0x71, 0x68, 0x6A, 0x6F, 0x65, 0xA0, 0x70, 0x67};
  scom::StreamParser parser;
  std::vector<scom::Frame> frames;
  for (uint8_t b : capture::kCaptureBytes) if (auto f = parser.push(b)) frames.push_back(*f);
  CHECK_EQ(198, frames.size());
  int wrong_order = 0;
  for (size_t i = 0; i < frames.size(); ++i) {
    if (frames[i].cmd != cycle[i % 12]) ++wrong_order;
  }
  CHECK_EQ(0, wrong_order);
}
