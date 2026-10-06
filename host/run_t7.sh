#!/usr/bin/env bash
# T7: run the test cases on RV32_ISS and RV32_5S with the CLI build of
# target/search.s (RENDER blocks stripped). Raw Ripes output is kept under
# measurements/t7/raw/.
# Prints one line per run: "input|proc|exit_code|iret|cycles|cpi|solution".
set -u
RIPES=${RIPES:-$HOME/tools/ripes/Ripes-v2.2.6-106-g5b8a616-linux-x86_64.AppImage}
CASES=${CASES:-"12345671111111 62345713133111 21345671111111 54721631111111"}
PROCS=${PROCS:-"RV32_ISS RV32_5S"}
OUT=measurements/t7/raw
mkdir -p build/t7 "$OUT"
CLI_SRC=build/search_cli.s
awk '/^# RENDER_BEGIN/ { skip = 1; next } /^# RENDER_END/ { skip = 0; next } !skip' \
    target/search.s > "$CLI_SRC"
if grep -q 'LED_MATRIX' "$CLI_SRC"; then
    echo "error: RENDER blocks were not stripped from $CLI_SRC" >&2
    exit 1
fi
export RIPES CLI_SRC OUT

run_one() {
    s=$1
    proc=$2
    src=build/t7/search_${s}_$proc.s
    raw=$OUT/${s}_$proc.txt
    sed "s/21345671111111/$s/" "$CLI_SRC" | cat target/tables.s - > "$src"
    "$RIPES" --mode cli --src "$src" -t asm --proc "$proc" \
        --cycles --iret --cpi > "$raw" 2>/dev/null
    code=$(sed -n 's/^Program exited with code: //p' "$raw")
    cycles=$(sed -n '/^===== cycles$/{n;p;}' "$raw")
    iret=$(sed -n '/^===== instructions retired$/{n;p;}' "$raw")
    cpi=$(sed -n '/^===== cycles per instruction/{n;p;}' "$raw")
    sol=$(sed -n '/^Program exited/q;/./p' "$raw" | head -n 1)
    printf '%s|%s|%s|%s|%s|%s|%s\n' "$s" "$proc" "${code:-NA}" "${iret:-NA}" \
        "${cycles:-NA}" "${cpi:-NA}" "${sol:--}"
    rm -f "$src"
}
export -f run_one

for s in $CASES; do
    for p in $PROCS; do
        printf '%s %s\n' "$s" "$p"
    done
done | xargs -P "${JOBS:-8}" -n 2 bash -c 'run_one "$1" "$2"' _ | sort
