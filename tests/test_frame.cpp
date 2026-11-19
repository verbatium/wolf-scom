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
