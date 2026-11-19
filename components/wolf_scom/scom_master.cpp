#include "scom_master.h"

namespace scom {

namespace {
constexpr uint8_t FlowTemperatureCmd = 0x68;
constexpr uint32_t GuardListenMs = 9000;
}

Master::Master(BytePort &port, Listener &listener, Config config)
    : port_(port), listener_(listener), config_(config) {}

void Master::tick(uint32_t now_ms) {
  if (refused_) return;
  if (!first_tick_ms_) first_tick_ms_ = now_ms;
  if (!started_) {
    if (now_ms - *first_tick_ms_ >= GuardListenMs) start_transaction(now_ms);
    return;
  }
  if (!transaction_) return;
  transaction_->tick(now_ms);
  deliver_if_ended();
}

void Master::on_rx(uint8_t byte, uint32_t now_ms) {
  if (!started_) return refuse();
  if (!transaction_) return;
  transaction_->on_rx(byte, now_ms);
  deliver_if_ended();
}

void Master::refuse() {
  if (refused_) return;
  refused_ = true;
  listener_.on_refused();
}

void Master::start_transaction(uint32_t now_ms) {
  started_ = true;
  transaction_.emplace(port_, Kind::Read, FlowTemperatureCmd, config_.loopback);
  transaction_->begin(now_ms);
}

void Master::deliver_if_ended() {
  const Transaction &t = *transaction_;
  if (t.state() == Transaction::State::Running) return;
  if (t.state() == Transaction::State::Done) {
    listener_.on_frame(*t.frame());
  } else {
    listener_.on_failure(t.fail_reason(), FlowTemperatureCmd);
  }
  listener_.on_trace(t.trace());
  transaction_.reset();
}

}  // namespace scom
