#pragma once
#include <cstdint>

#include "scom_frame.h"

namespace scom {

class BytePort {
 public:
  virtual ~BytePort() = default;
  virtual void write(uint8_t byte) = 0;
};

class Transaction {
 public:
  Transaction(BytePort &port, Kind kind, uint8_t cmd);
  void begin(uint32_t now_ms);

 private:
  BytePort &port_;
};

}  // namespace scom
