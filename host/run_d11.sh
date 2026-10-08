#!/usr/bin/env bash
# Run the renderer-free, read-only-table ELF on every listed input.
# Output: input exit_code retired_instructions
set -euo pipefail
cd "$(dirname "$0")/.."

RIPES=${RIPES:-$HOME/tools/ripes/Ripes-v2.2.6-106-g5b8a616-linux-x86_64.AppImage}
LIST=${1:-measurements/stage4/d11_inputs.txt}
JOBS=${JOBS:-6}
mkdir -p build/d11_elf
export RIPES

run_one() {
    local s=$1
    local elf="build/d11_elf/search_$s.elf"
    local code iret

    if ! bash host/build_target.sh "$s" "$elf" cli \
        > "$elf.build.log" 2>&1; then
        echo "build failed: $s; see $elf.build.log" >&2
        printf '%s NA NA\n' "$s"
        return 1
    fi

    if ! "$RIPES" --mode cli --src "$elf" -t elf \
        --proc RV32_ISS --iret > "$elf.stdout" 2> "$elf.stderr"; then
        echo "Ripes failed: $s; see $elf.stderr" >&2
        printf '%s NA NA\n' "$s"
        return 1
    fi

    code=$(sed -n 's/^Program exited with code: //p' "$elf.stdout")
    iret=$(sed -n '/^===== instructions retired$/{n;p;}' "$elf.stdout")
    printf '%s %s %s\n' "$s" "${code:-NA}" "${iret:-NA}"

    if [[ "$code" != 11 || ! "$iret" =~ ^[1-9][0-9]*$ ]]; then
        echo "invalid result: $s; evidence retained in build/d11_elf" >&2
        return 1
    fi
    if (( iret > 50000000 )); then
        echo "instruction budget exceeded: $s" >&2
        return 1
    fi

    rm -f "$elf" "$elf.s" "$elf.o" \
        "$elf.build.log" "$elf.stdout" "$elf.stderr"
}
export -f run_one

xargs -P "$JOBS" -I{} bash -c 'run_one "$1"' _ {} < "$LIST"
