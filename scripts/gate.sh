#!/usr/bin/env bash
set -u
cd "$(dirname "$0")/.."

TESTS_DIR="${GATE_TESTS_DIR:-tests}"
BUILD_DIR="$(mktemp -d)"
trap 'rm -rf "$BUILD_DIR"' EXIT

fail() { echo "GATE FAIL"; exit 1; }

core_sources=()
while IFS= read -r f; do core_sources+=("$f"); done < <(find components/wolf_scom -name 'scom_*.cpp')

g++ -std=c++17 -Wall -Werror -Icomponents/wolf_scom -I"$TESTS_DIR" \
  "$TESTS_DIR"/*.cpp ${core_sources[@]+"${core_sources[@]}"} -o "$BUILD_DIR/host_tests" || fail
"$BUILD_DIR/host_tests" || fail

echo "GATE PASS"
