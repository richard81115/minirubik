#!/usr/bin/env bash
# Run renderer-free ELF builds on ISS and a pipeline model.
# Output: input|proc|exit_code|iret|cycles|cpi|solution
set -euo pipefail
cd "$(dirname "$0")/.."

RIPES=${RIPES:-$HOME/tools/ripes/Ripes-v2.2.6-106-g5b8a616-linux-x86_64.AppImage}
CASES=${CASES:-"12345671111111 25371462312233 62345713133111 21345671111111 54721631111111"}
PROCS=${PROCS:-"RV32_ISS RV32_5S"}
OUT=${OUT:-measurements/t7/readonly_raw}
mkdir -p build/t7_elf "$OUT"
export RIPES OUT

run_one() {
    local s=$1 proc=$2
    local elf="build/t7_elf/search_${s}_$proc.elf"
    local raw="$OUT/${s}_$proc.txt"
    local code cycles iret cpi sol expected

    if ! bash host/build_target.sh "$s" "$elf" cli \
        > "$elf.build.log" 2>&1; then
        echo "build failed: $s $proc; see $elf.build.log" >&2
        return 1
    fi

    if ! "$RIPES" --mode cli --src "$elf" -t elf --proc "$proc" \
        --cycles --iret --cpi > "$raw" 2> "$raw.stderr"; then
        echo "Ripes failed: $s $proc; see $raw.stderr" >&2
        return 1
    fi

    code=$(sed -n 's/^Program exited with code: //p' "$raw")
    cycles=$(sed -n '/^===== cycles$/{n;p;}' "$raw")
    iret=$(sed -n '/^===== instructions retired$/{n;p;}' "$raw")
    cpi=$(sed -n '/^===== cycles per instruction/{n;p;}' "$raw")
    sol=$(sed -n '/^Program exited/q;/./p' "$raw" | head -n 1)

    printf '%s|%s|%s|%s|%s|%s|%s\n' \
        "$s" "$proc" "${code:-NA}" "${iret:-NA}" \
        "${cycles:-NA}" "${cpi:-NA}" "${sol:--}"

    case "$s" in
        12345671111111) expected=0 ;;
        25371462312233) expected=2 ;;
        62345713133111) expected=8 ;;
        21345671111111|54721631111111) expected=11 ;;
        *) expected="" ;;
    esac

    if [[ ! "$code" =~ ^[0-9]+$ ||
          ! "$iret" =~ ^[1-9][0-9]*$ ||
          ! "$cycles" =~ ^[1-9][0-9]*$ ]]; then
        echo "missing or invalid counters: $s $proc" >&2
        return 1
    fi
    if (( code > 11 )); then
        echo "invalid solution length: $s $proc" >&2
        return 1
    fi
    if [[ -n "$expected" && "$code" != "$expected" ]]; then
        echo "wrong solution length: $s $proc" >&2
        return 1
    fi
}
export -f run_one

for s in $CASES; do
    for p in $PROCS; do
        printf '%s %s\n' "$s" "$p"
    done
done | xargs -P "${JOBS:-6}" -n 2 \
    bash -c 'run_one "$1" "$2"' _ | sort
