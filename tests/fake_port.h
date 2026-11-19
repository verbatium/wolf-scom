// Recording BytePort fake shared by transaction and master tests.
#pragma once
#include <cstdint>
#include <vector>

#include "scom_transaction.h"

struct RecordingPort : scom::BytePort {
  std::vector<uint8_t> written;
  void write(uint8_t b) override { written.push_back(b); }
};
