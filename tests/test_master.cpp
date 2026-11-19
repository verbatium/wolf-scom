#include "capture_frames.h"
#include "fake_listener.h"
#include "mini_test.h"
#include "scom_master.h"

// B19a: the trace is delivered only when the transaction has ended
TEST(b19a_trace_is_delivered_only_after_done) {
  const auto &img = capture::kRead68;
  RecordingPort port;
  RecordingListener l;
  scom::Master m(port, l, scom::Master::Config{/*loopback=*/false});
  uint32_t t0 = tick_until_first_tx(m, port);
  CHECK(t0 != UINT32_MAX);
  m.on_rx(img[1], t0 + 10);
  m.on_rx(img[3], t0 + 20);
  m.on_rx(img[5], t0 + 30);
  m.on_rx(img[6], t0 + 40);
  CHECK_EQ(0, (int) l.traces.size());
  CHECK_EQ(0, (int) l.frames.size());
  m.on_rx(img[7], t0 + 50);
  CHECK_EQ(1, (int) l.frames.size());
  CHECK_EQ(1, (int) l.traces.size());
  if (l.traces.size() == 1) CHECK_EQ(8, (int) l.traces[0].size());
  m.tick(t0 + 51);
  CHECK_EQ(1, (int) l.traces.size());  // not delivered twice
}

TEST(b19a_trace_is_delivered_after_failure_not_during) {
  RecordingPort port;
  RecordingListener l;
  scom::Master m(port, l, scom::Master::Config{false});
  uint32_t t0 = tick_until_first_tx(m, port);
  CHECK(t0 != UINT32_MAX);
  m.on_rx(0x44, t0 + 10);
  CHECK_EQ(0, (int) l.traces.size());
  m.on_rx(0x69, t0 + 20);  // wrong command echo
  CHECK_EQ(1, (int) l.failures.size());
  CHECK_EQ(1, (int) l.traces.size());
  if (l.traces.size() == 1) CHECK_EQ(4, (int) l.traces[0].size());  // TX 44, RX 44, TX 68, RX 69
}

// B20: 9 s start-up listen window (GuardListenMs), measured from the first tick
TEST(b20_master_emits_nothing_during_the_listen_window) {
  RecordingPort port;
  RecordingListener l;
  scom::Master m(port, l, scom::Master::Config{false});
  const uint32_t first = 5000;
  for (uint32_t now = first; now < first + 9000; now += 10) m.tick(now);
  m.tick(first + 8999);
  CHECK_EQ(0, (int) port.written.size());
  CHECK_EQ(0, l.refused);
}
