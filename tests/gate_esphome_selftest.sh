#!/usr/bin/env bash
# B3 (host-checkable part): `gate.sh --esphome` takes host/container only from the environment.
# Without ESPHOME_REMOTE / ESPHOME_CONTAINER it must refuse (GATE FAIL, non-zero) and name both variables.
# The actual remote compile of tests/esphome-test*.yaml is run by the orchestrator (needs ssh).
set -u
cd "$(dirname "$0")/.."
[ -f tests/esphome-test.yaml ] || { echo "SELFTEST FAIL: tests/esphome-test.yaml missing"; exit 1; }
grep -q '^wolf_scom:' tests/esphome-test.yaml || { echo "SELFTEST FAIL: yaml has no wolf_scom block"; exit 1; }
out="$(env -u ESPHOME_REMOTE -u ESPHOME_CONTAINER scripts/gate.sh --esphome 2>&1)"; rc=$?
echo "$out"
[ "$rc" -ne 0 ] || { echo "SELFTEST FAIL: exit 0 without env"; exit 1; }
echo "$out" | grep -qx "GATE FAIL" || { echo "SELFTEST FAIL: no GATE FAIL line"; exit 1; }
echo "$out" | grep -q "ESPHOME_REMOTE" || { echo "SELFTEST FAIL: message lacks ESPHOME_REMOTE"; exit 1; }
echo "$out" | grep -q "ESPHOME_CONTAINER" || { echo "SELFTEST FAIL: message lacks ESPHOME_CONTAINER"; exit 1; }
echo "SELFTEST OK"
