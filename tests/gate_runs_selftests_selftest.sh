#!/usr/bin/env bash
# F2: the default gate must run the host-only tests/*selftest.sh scripts, so they are not dead tests.
# Proof: point GATE_SELFTESTS_DIR at a dir holding a failing selftest; the gate must print GATE FAIL.
# (Recursion safety: gate runs selftests only when GATE_TESTS_DIR is unset; gate_selftest.sh sets it.)
set -u
cd "$(dirname "$0")/.."
tmp="$(mktemp -d)"
trap 'rm -rf "$tmp"' EXIT
printf '#!/usr/bin/env bash\necho deliberately failing selftest\nexit 1\n' > "$tmp/x_selftest.sh"
chmod +x "$tmp/x_selftest.sh"
out="$(GATE_SELFTESTS_DIR="$tmp" scripts/gate.sh 2>&1)"; rc=$?
echo "$out" | tail -4
[ "$rc" -ne 0 ] || { echo "SELFTEST FAIL: gate exited 0 with a failing selftest"; exit 1; }
echo "$out" | grep -qx "GATE FAIL" || { echo "SELFTEST FAIL: no GATE FAIL line"; exit 1; }
echo "$out" | grep -q "deliberately failing selftest" || { echo "SELFTEST FAIL: selftest was not executed"; exit 1; }
echo "SELFTEST OK"
