// Recording Listener fake + helpers for Master tests.
#pragma once
#include <cstdint>
#include <vector>

#include "fake_port.h"
#include "scom_master.h"

struct RecordingListener : scom::Listener {
  std::vector<scom::Frame> frames;
  std::vector<std::pair<scom::FailReason, uint8_t>> failures;
  std::vector<std::vector<scom::TraceEntry>> traces;
  int refused = 0;
  void on_frame(const scom::Frame &f) override { frames.push_back(f); }
  void on_failure(scom::FailReason r, uint8_t cmd) override { failures.emplace_back(r, cmd); }
  void on_trace(const std::vector<scom::TraceEntry> &t) override { traces.push_back(t); }
  void on_refused() override { ++refused; }
};

// Ticks the master every millisecond from `from` until it writes its first byte; returns that time
// (or UINT32_MAX after 30 s). Independent of the exact start-up guard length.
inline uint32_t tick_until_first_tx(scom::Master &m, RecordingPort &port, uint32_t from = 0) {
  for (uint32_t now = from; now < from + 30000; ++now) {
    m.tick(now);
    if (!port.written.empty()) return now;
  }
  return UINT32_MAX;
}
