#pragma once
#include <cstdint>
#include <optional>
#include <vector>

#include "scom_frame.h"
#include "scom_transaction.h"

namespace scom {

class Listener {
 public:
  virtual ~Listener() = default;
  virtual void on_frame(const Frame &frame) = 0;
  virtual void on_failure(FailReason reason, uint8_t cmd) = 0;
  virtual void on_trace(const std::vector<TraceEntry> &trace) = 0;
  virtual void on_refused() = 0;
};

class Master {
 public:
  struct Config {
    bool loopback;
  };

  Master(BytePort &port, Listener &listener, Config config);
  void on_rx(uint8_t byte, uint32_t now_ms);
  void tick(uint32_t now_ms);

 private:
  void refuse();
  void start_transaction(uint32_t now_ms);
  void deliver_if_ended();

  BytePort &port_;
  Listener &listener_;
  Config config_;
  bool started_ = false;
  bool refused_ = false;
  std::optional<uint32_t> first_tick_ms_;
  std::optional<Transaction> transaction_;
};

}  // namespace scom
