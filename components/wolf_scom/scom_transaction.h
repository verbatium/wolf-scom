#pragma once
#include <cstddef>
#include <cstdint>
#include <optional>

#include "scom_frame.h"

namespace scom {

class BytePort {
 public:
  virtual ~BytePort() = default;
  virtual void write(uint8_t byte) = 0;
};

class Transaction {
 public:
  enum class State { Running, Done, Failed };

  Transaction(BytePort &port, Kind kind, uint8_t cmd);
  void begin(uint32_t now_ms);
  void on_rx(uint8_t byte, uint32_t now_ms);
  State state() const { return state_; }
  std::optional<Frame> frame() const { return frame_; }

 private:
  void advance();

  BytePort &port_;
  Kind kind_;
  uint8_t cmd_;
  size_t step_ = 0;
  uint8_t value_ = 0;
  State state_ = State::Running;
  std::optional<Frame> frame_;
};

}  // namespace scom
