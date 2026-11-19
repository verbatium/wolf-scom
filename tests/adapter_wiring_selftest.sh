#!/usr/bin/env bash
# B10 (host-checkable part): the ESPHome adapter must delegate to the pure core instead of owning the
# decode/dedupe logic. Proof that it still compiles is tests/esphome-test.yaml via `gate.sh --esphome`.
set -u
cd "$(dirname "$0")/.."
cpp=components/wolf_scom/wolf_scom.cpp
hdr=components/wolf_scom/wolf_scom.h
bad=0
check() { if ! eval "$2"; then echo "SELFTEST FAIL: $1"; bad=1; fi; }
check "wolf_scom.cpp uses scom::StreamParser (via member)" "grep -q 'StreamParser' $cpp $hdr"
check "wolf_scom.h or .cpp uses scom::Publisher" "grep -q 'Publisher' $cpp $hdr"
check "wolf_scom.cpp includes scom_publish.h or scom_frame.h" "grep -Eq '#include \"scom_(publish|frame)\.h\"' $cpp $hdr"
check "old process_packet removed from the adapter" "! grep -q 'process_packet' $cpp $hdr"
check "old last_* dedupe caches removed from the adapter" "! grep -Eq 'last_(flow|rpm|pump|heat)' $cpp $hdr"
check "core stays free of ESPHome headers" "! grep -lE 'esphome|ESP_LOG' components/wolf_scom/scom_*.h components/wolf_scom/scom_*.cpp"
[ "$bad" -eq 0 ] && echo "SELFTEST OK"
exit "$bad"
