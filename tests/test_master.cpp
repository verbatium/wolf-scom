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

// B21: any RX byte inside the listen window means a foreign master (DWT) is on the bus
TEST(b21_rx_byte_in_listen_window_refuses_start_and_never_transmits) {
  RecordingPort port;
  RecordingListener l;
  scom::Master m(port, l, scom::Master::Config{false});
  m.tick(1000);
  m.on_rx(0x44, 3000);
  for (uint32_t now = 3000; now < 30000; now += 10) m.tick(now);
  CHECK_EQ(0, (int) port.written.size());
  CHECK_EQ(1, l.refused);
  CHECK_EQ(0, (int) l.frames.size());
}

// B21a: refusal is permanent until reboot, however long the bus stays silent afterwards
TEST(b21a_refusal_is_permanent_after_long_silence) {
  RecordingPort port;
  RecordingListener l;
  scom::Master m(port, l, scom::Master::Config{false});
  m.tick(0);
  m.on_rx(0x53, 100);
  for (uint32_t now = 100; now < 300000; now += 7) m.tick(now);
  CHECK_EQ(0, (int) port.written.size());
  CHECK_EQ(1, l.refused);
}

// B22: exactly 9 s of silence start the first transaction
TEST(b22_first_transaction_starts_after_9s_of_silence) {
  RecordingPort port;
  RecordingListener l;
  scom::Master m(port, l, scom::Master::Config{false});
  m.tick(2000);
  m.tick(2000 + 8999);
  CHECK_EQ(0, (int) port.written.size());
  m.tick(2000 + 9000);
  CHECK_EQ(1, (int) port.written.size());
  if (!port.written.empty()) CHECK_EQ(0x44, port.written[0]);
  CHECK_EQ(0, l.refused);
}

// B22a: an RX byte at 8.9 s is still inside the window
TEST(b22a_rx_byte_at_8900ms_still_refuses_start) {
  RecordingPort port;
  RecordingListener l;
  scom::Master m(port, l, scom::Master::Config{false});
  m.tick(2000);
  m.on_rx(0x44, 2000 + 8900);
  for (uint32_t now = 2000 + 8900; now < 2000 + 40000; now += 10) m.tick(now);
  CHECK_EQ(0, (int) port.written.size());
  CHECK_EQ(1, l.refused);
}

// B23: the S2 cycle is a single READ 0x68; the Frame arrives through on_frame
TEST(b23_cycle_is_read_0x68_only_and_delivers_the_frame) {
  const auto &img = capture::kRead68;
  RecordingPort port;
  RecordingListener l;
  scom::Master m(port, l, scom::Master::Config{false});
  uint32_t t0 = tick_until_first_tx(m, port);
  m.on_rx(img[1], t0 + 10);
  m.on_rx(img[3], t0 + 20);
  m.on_rx(img[5], t0 + 30);
  m.on_rx(img[6], t0 + 40);
  m.on_rx(img[7], t0 + 50);
  CHECK_EQ(1, (int) l.frames.size());
  if (l.frames.size() == 1) {
    CHECK(l.frames[0].kind == scom::Kind::Read);
    CHECK_EQ(0x68, l.frames[0].cmd);
    CHECK_EQ(0x1F, l.frames[0].value);
  }
  CHECK_EQ(0, (int) l.failures.size());
  CHECK_EQ(3, (int) port.written.size());
  for (uint8_t b : port.written) CHECK(b != 0x53);  // no write frame in S2
}

// B24: the next cycle starts 7.8 s after the previous cycle STARTED
TEST(b24_next_cycle_starts_7800ms_after_previous_cycle_start) {
  const auto &img = capture::kRead68;
  RecordingPort port;
  RecordingListener l;
  scom::Master m(port, l, scom::Master::Config{false});
  uint32_t t0 = tick_until_first_tx(m, port);
  m.on_rx(img[1], t0 + 10);
  m.on_rx(img[3], t0 + 20);
  m.on_rx(img[5], t0 + 30);
  m.on_rx(img[6], t0 + 40);
  m.on_rx(img[7], t0 + 50);
  CHECK_EQ(3, (int) port.written.size());
  m.tick(t0 + 7799);
  CHECK_EQ(3, (int) port.written.size());
  m.tick(t0 + 7800);
  CHECK_EQ(4, (int) port.written.size());
  if (port.written.size() == 4) CHECK_EQ(0x44, port.written[3]);
}
