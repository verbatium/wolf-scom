#include "scom_transaction.h"

namespace scom {

namespace {
constexpr uint8_t ReadHeader = 0x44;  // [hypothesis]
}

Transaction::Transaction(BytePort &port, Kind, uint8_t) : port_(port) {}

void Transaction::begin(uint32_t) { port_.write(ReadHeader); }

}  // namespace scom
