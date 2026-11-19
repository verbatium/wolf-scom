#!/usr/bin/env bash
# B2: a failing host assertion must make scripts/gate.sh print GATE FAIL and exit non-zero.
set -u
cd "$(dirname "$0")/.."
tmp="$(mktemp -d)"
trap 'rm -rf "$tmp"' EXIT
cp tests/mini_test.h tests/test_main.cpp "$tmp"/
cat > "$tmp/test_failing.cpp" <<'CPP'
#include "mini_test.h"
TEST(deliberately_failing) { CHECK_EQ(1, 2); }
CPP
out="$(GATE_TESTS_DIR="$tmp" scripts/gate.sh 2>&1)"; rc=$?
echo "$out"
[ "$rc" -ne 0 ] || { echo "SELFTEST FAIL: gate exited 0 on a failing assertion"; exit 1; }
echo "$out" | grep -qx "GATE FAIL" || { echo "SELFTEST FAIL: no GATE FAIL line"; exit 1; }
echo "$out" | grep -q "GATE PASS" && { echo "SELFTEST FAIL: GATE PASS printed"; exit 1; }
echo "SELFTEST OK"
