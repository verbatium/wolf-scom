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
