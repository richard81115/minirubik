#!/usr/bin/env bash
# Run target/search.s on every input in a list on RV32_ISS.
# Prints one line per input: "input exit_code retired_instructions".
set -u
RIPES=${RIPES:-$HOME/tools/ripes/Ripes-v2.2.6-106-g5b8a616-linux-x86_64.AppImage}
LIST=${1:-measurements/stage4/d11_inputs.txt}
JOBS=${JOBS:-6}
mkdir -p build/d11
export RIPES

run_one() {
    s=$1
    src=build/d11/search_$s.s
    sed "s/21345671111111/$s/" target/search.s | cat target/tables.s - > "$src"
    out=$("$RIPES" --mode cli --src "$src" -t asm --proc RV32_ISS --iret 2>/dev/null)
    code=$(printf '%s\n' "$out" | sed -n 's/^Program exited with code: //p')
    iret=$(printf '%s\n' "$out" | sed -n '/instructions retired/{n;p;}')
    printf '%s %s %s\n' "$s" "${code:-NA}" "${iret:-NA}"
    rm -f "$src"
}
export -f run_one
xargs -P "$JOBS" -I{} bash -c 'run_one "$1"' _ {} < "$LIST"
