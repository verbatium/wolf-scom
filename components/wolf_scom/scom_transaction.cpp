#include "scom_transaction.h"

namespace scom {

namespace {
constexpr uint8_t ReadHeader = 0x44;  // [hypothesis]
}

Transaction::Transaction(BytePort &port, Kind, uint8_t cmd) : port_(port), cmd_(cmd) {}

void Transaction::begin(uint32_t) { port_.write(ReadHeader); }

void Transaction::on_rx(uint8_t, uint32_t) { port_.write(cmd_); }

}  // namespace scom
