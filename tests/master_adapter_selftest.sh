#!/usr/bin/env bash
# Host-checkable part of the [compile-verified] master adapter behaviors (B25..B29).
# Proof that the result compiles is tests/esphome-test-master.yaml via `gate.sh --esphome` (needs-compile).
set -u
cd "$(dirname "$0")/.."
init=components/wolf_scom/__init__.py
cpp=components/wolf_scom/wolf_scom.cpp
hdr=components/wolf_scom/wolf_scom.h
yaml=tests/esphome-test-master.yaml
bad=0
check() { if ! eval "$2"; then echo "SELFTEST FAIL: $1"; bad=1; fi; }

# B25: master option in the schema, default false, forwarded to the component; test YAML turns it on
check "B25 schema declares the master option" "grep -Eq \"cv\\.Optional\\(CONF_MASTER, default=False\\): *cv\\.boolean\" $init"
check "B25 to_code forwards master to the component" "grep -q 'set_master' $init"
check "B25 master YAML enables master" "grep -Eq '^  master: true' $yaml"
check "B25 passive YAML keeps master unset" "! grep -Eq '^ *master:' tests/esphome-test.yaml"

[ "$bad" -eq 0 ] && echo "SELFTEST OK"
exit "$bad"
