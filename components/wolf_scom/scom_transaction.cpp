#include "scom_transaction.h"

namespace scom {

enum class Action { Send, Expect };
enum class Byte { Header, Cmd, Value, ValueEcho, Ack };

struct Step {
  Action action;
  Byte byte;
};

namespace {

constexpr uint8_t ReadHeader = 0x44;
constexpr uint32_t EchoTimeoutMs = 150;
constexpr uint8_t AckByte = 0x06;

// Read step tables v1. [hypothesis] which bytes the master drives.
// Loopback: every own Send is mirrored onto RX first and must not be taken for the boiler echo.
constexpr Step ReadSteps[] = {
    {Action::Send, Byte::Header}, {Action::Expect, Byte::Header}, {Action::Send, Byte::Cmd},
    {Action::Expect, Byte::Cmd},  {Action::Send, Byte::Cmd},      {Action::Expect, Byte::Value},
    {Action::Expect, Byte::ValueEcho}, {Action::Expect, Byte::Ack},
};
constexpr Step ReadStepsLoopback[] = {
    {Action::Send, Byte::Header}, {Action::Expect, Byte::Header}, {Action::Expect, Byte::Header},
    {Action::Send, Byte::Cmd},    {Action::Expect, Byte::Cmd},    {Action::Expect, Byte::Cmd},
    {Action::Send, Byte::Cmd},    {Action::Expect, Byte::Cmd},    {Action::Expect, Byte::Value},
    {Action::Expect, Byte::ValueEcho}, {Action::Expect, Byte::Ack},
};

}  // namespace

Transaction::Transaction(BytePort &port, Kind kind, uint8_t cmd, bool loopback)
    : port_(port),
      kind_(kind),
      cmd_(cmd),
      steps_(loopback ? ReadStepsLoopback : ReadSteps),
      step_count_(loopback ? sizeof(ReadStepsLoopback) / sizeof(Step) : sizeof(ReadSteps) / sizeof(Step)) {}

void Transaction::begin(uint32_t now_ms) {
  last_event_ms_ = now_ms;
  advance();
}

void Transaction::tick(uint32_t now_ms) {
  if (state_ == State::Running && now_ms - last_event_ms_ >= EchoTimeoutMs) fail(FailReason::Timeout);
}

void Transaction::on_rx(uint8_t byte, uint32_t now_ms) {
  if (state_ != State::Running) return;
  last_event_ms_ = now_ms;
  if (steps_[step_].byte == Byte::Ack && byte != AckByte) return fail(FailReason::NoAck);
  if (!echoes_expected(byte)) return fail(FailReason::WrongEcho);
  if (steps_[step_].byte == Byte::Value) value_ = byte;
  ++step_;
  if (step_ == step_count_) {
    state_ = State::Done;
    frame_ = Frame{kind_, cmd_, value_};
    return;
  }
  advance();
}

bool Transaction::echoes_expected(uint8_t byte) const {
  switch (steps_[step_].byte) {
    case Byte::Header: return byte == ReadHeader;
    case Byte::Cmd: return byte == cmd_;
    case Byte::ValueEcho: return byte == value_;
    default: return true;
  }
}

void Transaction::fail(FailReason reason) {
  state_ = State::Failed;
  fail_reason_ = reason;
}

void Transaction::advance() {
  while (step_ < step_count_ && steps_[step_].action == Action::Send) {
    port_.write(steps_[step_].byte == Byte::Header ? ReadHeader : cmd_);
    ++step_;
  }
}

}  // namespace scom
