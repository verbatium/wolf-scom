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
