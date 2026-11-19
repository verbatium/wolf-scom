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
  local remote_dir
  remote_dir="$(ssh "$ESPHOME_REMOTE" 'mktemp -d')" || fail "cannot reach ESPHOME_REMOTE"
  [ -n "$remote_dir" ] || fail "remote mktemp returned nothing"
  tar cf - components/wolf_scom tests/esphome-test*.yaml \
    | ssh "$ESPHOME_REMOTE" "cd '$remote_dir' && tar xf - && mv tests/esphome-test*.yaml ." \
    || fail "upload to ESPHOME_REMOTE failed"
  local status=0 yaml
  for yaml in tests/esphome-test*.yaml; do
    ssh "$ESPHOME_REMOTE" "docker exec -w '$remote_dir' '$ESPHOME_CONTAINER' esphome compile '$(basename "$yaml")'" || status=1
  done
  ssh "$ESPHOME_REMOTE" "rm -rf '$remote_dir'"
  [ "$status" -eq 0 ] || fail "esphome compile failed"
}

if [ "${1:-}" = "--esphome" ]; then
  run_esphome_compile
else
  run_host_tests
fi
echo "GATE PASS"
