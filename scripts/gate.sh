#!/usr/bin/env bash
set -u
cd "$(dirname "$0")/.."

fail() { echo "${1:-}"; echo "GATE FAIL"; exit 1; }

run_host_tests() {
  local tests_dir="${GATE_TESTS_DIR:-tests}"
  build_dir="$(mktemp -d)"
  trap 'rm -rf "$build_dir"' EXIT
  local core_sources=()
  while IFS= read -r f; do core_sources+=("$f"); done < <(find components/wolf_scom -name 'scom_*.cpp')
  g++ -std=c++17 -Wall -Werror -Icomponents/wolf_scom -I"$tests_dir" \
    "$tests_dir"/*.cpp ${core_sources[@]+"${core_sources[@]}"} -o "$build_dir/host_tests" || fail
  "$build_dir/host_tests" || fail
}

run_esphome_compile() {
  [ -n "${ESPHOME_REMOTE:-}" ] && [ -n "${ESPHOME_CONTAINER:-}" ] \
    || fail "--esphome needs env ESPHOME_REMOTE (ssh host) and ESPHOME_CONTAINER (docker container)"
  local in_container="docker exec -i $ESPHOME_CONTAINER"
  local workdir
  workdir="$(ssh "$ESPHOME_REMOTE" "$in_container mktemp -d")" || fail "cannot create work dir in ESPHOME_CONTAINER"
  [ -n "$workdir" ] || fail "container mktemp returned nothing"
  COPYFILE_DISABLE=1 tar -c -f - --exclude='._*' --exclude='.idea' --exclude='__pycache__' components/wolf_scom tests/esphome-test*.yaml \
    | ssh "$ESPHOME_REMOTE" "$in_container sh -c 'cd $workdir && tar xf - && mv tests/esphome-test*.yaml .'" \
    || fail "upload into ESPHOME_CONTAINER failed"
  local status=0 yaml
  for yaml in tests/esphome-test*.yaml; do
    ssh "$ESPHOME_REMOTE" "$in_container sh -c 'cd $workdir && esphome compile $(basename "$yaml")'" || status=1
  done
  ssh "$ESPHOME_REMOTE" "$in_container rm -rf $workdir"
  [ "$status" -eq 0 ] || fail "esphome compile failed"
}

run_selftests() {
  [ -z "${GATE_TESTS_DIR:-}" ] || return 0
  local script
  for script in "${GATE_SELFTESTS_DIR:-tests}"/*selftest.sh; do
    [ -e "$script" ] || continue
    bash "$script" || fail "selftest failed: $script"
  done
}

if [ "${1:-}" = "--esphome" ]; then
  run_esphome_compile
else
  run_host_tests
  run_selftests
fi
echo "GATE PASS"
