#include "capture_frames.h"
#include "mini_test.h"
#include "scom_frame.h"

TEST(decode_captured_read_68) {
  auto f = scom::decode(capture::kRead68);
  CHECK(f.has_value());
  if (!f) return;
  CHECK(f->kind == scom::Kind::Read);
  CHECK_EQ(0x68, f->cmd);
  CHECK_EQ(0x1F, f->value);
}

TEST(decode_captured_write_6E) {
  auto f = scom::decode(capture::kWrite6E);
  CHECK(f.has_value());
  if (!f) return;
  CHECK(f->kind == scom::Kind::Write);
  CHECK_EQ(0x6E, f->cmd);
  CHECK_EQ(0x27, f->value);
}
