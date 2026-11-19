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
