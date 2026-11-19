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

// B15
TEST(b15_wrong_echo_byte_aborts_with_wrong_echo) {
  RecordingPort port;
  scom::Transaction t(port, scom::Kind::Read, 0x68);
  t.begin(1000);
  t.on_rx(0x44, 1010);
  t.on_rx(0x69, 1020);  // echo of the command byte is wrong
  CHECK(t.state() == scom::Transaction::State::Failed);
  CHECK(t.fail_reason() == scom::FailReason::WrongEcho);
  CHECK(!t.frame().has_value());
  size_t sent = port.written.size();
  t.on_rx(0x68, 1030);  // a terminal transaction ignores late bytes and transmits nothing
  t.on_rx(0x1F, 1040);
  CHECK(t.state() == scom::Transaction::State::Failed);
  CHECK_EQ((int) sent, (int) port.written.size());
}

TEST(b15_wrong_first_echo_aborts_before_command_is_sent) {
  RecordingPort port;
  scom::Transaction t(port, scom::Kind::Read, 0x68);
  t.begin(1000);
  t.on_rx(0x45, 1010);
  CHECK(t.state() == scom::Transaction::State::Failed);
  CHECK(t.fail_reason() == scom::FailReason::WrongEcho);
  CHECK_EQ(1, (int) port.written.size());
}

// A finished transaction must not walk off the end of its step table (found reading B14a implementation).
TEST(done_transaction_ignores_late_rx_bytes) {
  const auto &img = capture::kRead68;
  RecordingPort port;
  scom::Transaction t(port, scom::Kind::Read, 0x68);
  t.begin(1000);
  for (int i : {1, 3, 5, 6, 7}) t.on_rx(img[i], 1010 + i);
  CHECK(t.state() == scom::Transaction::State::Done);
  t.on_rx(0x00, 1100);
  t.on_rx(0x99, 1110);
  CHECK(t.state() == scom::Transaction::State::Done);
  CHECK_EQ(0x1F, t.frame()->value);
  CHECK_EQ(3, (int) port.written.size());
}

// B14b: loopback variant (primary hypothesis for TSS721-style transceivers: own TX mirrored onto RX).
// The ESP sees every byte of the captured image 44 44 68 68 68 1F 1F 06; idx0, idx2, idx4 are its own TX
// looped back, idx1, idx3 the boiler echoes, idx5..7 value, value echo, ACK.
TEST(b14b_loopback_captured_read_completes_done_without_premature_echo) {
  const auto &img = capture::kRead68;
  RecordingPort port;
  scom::Transaction t(port, scom::Kind::Read, 0x68, /*loopback=*/true);
  t.begin(1000);
  CHECK_EQ(1, (int) port.written.size());
  t.on_rx(img[0], 1001);  // own 44 looped back: NOT the boiler echo yet
  CHECK_EQ(1, (int) port.written.size());
  CHECK(t.state() == scom::Transaction::State::Running);
  t.on_rx(img[1], 1010);  // boiler echo
  CHECK_EQ(2, (int) port.written.size());
  t.on_rx(img[2], 1011);  // own cmd looped back
  CHECK_EQ(2, (int) port.written.size());
  t.on_rx(img[3], 1020);  // boiler echo
  CHECK_EQ(3, (int) port.written.size());
  t.on_rx(img[4], 1021);  // own third cmd looped back
  CHECK(t.state() == scom::Transaction::State::Running);
  t.on_rx(img[5], 1030);
  t.on_rx(img[6], 1040);
  CHECK(t.state() == scom::Transaction::State::Running);
  t.on_rx(img[7], 1050);
  CHECK(t.state() == scom::Transaction::State::Done);
  auto frame = t.frame();
  CHECK(frame.has_value());
  if (frame) {
    CHECK_EQ(0x68, frame->cmd);
    CHECK_EQ(0x1F, frame->value);
  }
  CHECK_EQ(3, (int) port.written.size());
}

// a loopback transaction must not accept the no-loopback byte sequence (own TX is NOT the echo)
TEST(b14b_loopback_does_not_treat_own_tx_as_boiler_echo) {
  RecordingPort port;
  scom::Transaction t(port, scom::Kind::Read, 0x68, /*loopback=*/true);
  t.begin(1000);
  t.on_rx(0x44, 1001);
  t.on_rx(0x45, 1010);  // boiler echo must equal 0x44
  CHECK(t.state() == scom::Transaction::State::Failed);
  CHECK(t.fail_reason() == scom::FailReason::WrongEcho);
}

// B16: echo timeout is 150 ms, measured from the last byte sent or received
TEST(b16_no_echo_within_150ms_aborts_with_timeout) {
  RecordingPort port;
  scom::Transaction t(port, scom::Kind::Read, 0x68);
  t.begin(1000);
  t.tick(1149);
  CHECK(t.state() == scom::Transaction::State::Running);
  t.tick(1150);
  CHECK(t.state() == scom::Transaction::State::Failed);
  CHECK(t.fail_reason() == scom::FailReason::Timeout);
  CHECK(!t.frame().has_value());
}

TEST(b16_timeout_is_measured_from_the_last_rx_byte) {
  RecordingPort port;
  scom::Transaction t(port, scom::Kind::Read, 0x68);
  t.begin(1000);
  t.on_rx(0x44, 1100);  // echo arrives late but in time; the wait for the command echo restarts here
  t.tick(1249);
  CHECK(t.state() == scom::Transaction::State::Running);
  t.tick(1250);
  CHECK(t.state() == scom::Transaction::State::Failed);
  CHECK(t.fail_reason() == scom::FailReason::Timeout);
}

// B17
TEST(b17_last_byte_other_than_06_aborts_with_no_ack) {
  RecordingPort port;
  scom::Transaction t(port, scom::Kind::Read, 0x68);
  t.begin(1000);
  for (uint8_t b : {0x44, 0x68, 0x1F, 0x1F}) t.on_rx(b, 1010);
  CHECK(t.state() == scom::Transaction::State::Running);
  t.on_rx(0x07, 1050);
  CHECK(t.state() == scom::Transaction::State::Failed);
  CHECK(t.fail_reason() == scom::FailReason::NoAck);
  CHECK(!t.frame().has_value());
}
