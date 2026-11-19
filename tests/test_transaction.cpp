#include <vector>

#include "capture_frames.h"
#include "fake_port.h"
#include "mini_test.h"
#include "scom_transaction.h"

// TX byte sequences are [hypothesis]: the capture only shows the bus image, not who drives which byte.

// B11
TEST(b11_read_begin_sends_first_byte_0x44) {
  RecordingPort port;
  scom::Transaction t(port, scom::Kind::Read, 0x68);
  CHECK(port.written.empty());
  t.begin(1000);
  CHECK_EQ(1, (int) port.written.size());
  if (!port.written.empty()) CHECK_EQ(capture::kRead68[0], port.written[0]);
}

// B12
TEST(b12_after_echo_of_0x44_sends_command_byte) {
  RecordingPort port;
  scom::Transaction t(port, scom::Kind::Read, 0x68);
  t.begin(1000);
  t.on_rx(capture::kRead68[1], 1010);  // boiler echo of 0x44
  CHECK_EQ(2, (int) port.written.size());
  if (port.written.size() >= 2) CHECK_EQ(capture::kRead68[2], port.written[1]);
}

// B13 [hypothesis]: master sends the third command byte; command 0x6A (capture: 44 44 6A 6A 6A 20 20 06)
TEST(b13_after_echo_of_command_sends_third_command_byte) {
  RecordingPort port;
  scom::Transaction t(port, scom::Kind::Read, 0x6A);
  t.begin(1000);
  t.on_rx(0x44, 1010);
  t.on_rx(0x6A, 1020);
  CHECK_EQ(3, (int) port.written.size());
  if (port.written.size() >= 3) {
    CHECK_EQ(0x44, port.written[0]);
    CHECK_EQ(0x6A, port.written[1]);
    CHECK_EQ(0x6A, port.written[2]);
  }
}

// B14a: no-loopback variant. The ESP only sees the boiler's bytes of the captured image
// 44 44 68 68 68 1F 1F 06: idx1 (echo of 44), idx3 (echo of cmd), idx5..7 (value, value echo, ACK);
// idx0, idx2, idx4 are its own TX.
TEST(b14a_no_loopback_captured_read_completes_done_with_value) {
  const auto &img = capture::kRead68;
  RecordingPort port;
  scom::Transaction t(port, scom::Kind::Read, 0x68);
  t.begin(1000);
  CHECK(t.state() == scom::Transaction::State::Running);
  t.on_rx(img[1], 1010);
  t.on_rx(img[3], 1020);
  CHECK(t.state() == scom::Transaction::State::Running);
  t.on_rx(img[5], 1030);
  t.on_rx(img[6], 1040);
  CHECK(t.state() == scom::Transaction::State::Running);
  t.on_rx(img[7], 1050);
  CHECK(t.state() == scom::Transaction::State::Done);
  auto frame = t.frame();
  CHECK(frame.has_value());
  if (frame) {
    CHECK(frame->kind == scom::Kind::Read);
    CHECK_EQ(0x68, frame->cmd);
    CHECK_EQ(0x1F, frame->value);
  }
  // exactly the three own-TX bytes, nothing more
  CHECK_EQ(3, (int) port.written.size());
  if (port.written.size() == 3) {
    CHECK_EQ(img[0], port.written[0]);
    CHECK_EQ(img[2], port.written[1]);
    CHECK_EQ(img[4], port.written[2]);
  }
}
