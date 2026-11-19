#include "scom_transaction.h"

namespace scom {

namespace {

constexpr uint8_t ReadHeader = 0x44;

enum class Action { Send, Expect };
enum class Byte { Header, Cmd, Value, ValueEcho, Ack };

struct Step {
  Action action;
  Byte byte;
};

// Read step table v1, no-loopback variant. [hypothesis] which bytes the master drives.
constexpr Step ReadSteps[] = {
    {Action::Send, Byte::Header},   {Action::Expect, Byte::Header},    {Action::Send, Byte::Cmd},
    {Action::Expect, Byte::Cmd},    {Action::Send, Byte::Cmd},         {Action::Expect, Byte::Value},
    {Action::Expect, Byte::ValueEcho}, {Action::Expect, Byte::Ack},
};
constexpr size_t ReadStepCount = sizeof(ReadSteps) / sizeof(ReadSteps[0]);

}  // namespace

Transaction::Transaction(BytePort &port, Kind kind, uint8_t cmd) : port_(port), kind_(kind), cmd_(cmd) {}

void Transaction::begin(uint32_t) { advance(); }

void Transaction::on_rx(uint8_t byte, uint32_t) {
  if (state_ != State::Running) return;
  if (!echoes_expected(byte)) return fail(FailReason::WrongEcho);
  if (ReadSteps[step_].byte == Byte::Value) value_ = byte;
  ++step_;
  if (step_ == ReadStepCount) {
    state_ = State::Done;
    frame_ = Frame{kind_, cmd_, value_};
    return;
  }
  advance();
}

bool Transaction::echoes_expected(uint8_t byte) const {
  switch (ReadSteps[step_].byte) {
    case Byte::Header: return byte == ReadHeader;
    case Byte::Cmd: return byte == cmd_;
    default: return true;
  }
}

void Transaction::fail(FailReason reason) {
  state_ = State::Failed;
  fail_reason_ = reason;
}

void Transaction::advance() {
  while (step_ < ReadStepCount && ReadSteps[step_].action == Action::Send) {
    port_.write(ReadSteps[step_].byte == Byte::Header ? ReadHeader : cmd_);
    ++step_;
  }
}

}  // namespace scom
