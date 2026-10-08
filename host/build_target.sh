#!/usr/bin/env bash
# Usage: bash host/build_target.sh INPUT OUTPUT.elf [cli|gui]
# GUI dimensions can be set through LED_WIDTH and LED_HEIGHT.
set -euo pipefail
cd "$(dirname "$0")/.."

state=${1:?Missing input state}
output=${2:?Missing output ELF path}
mode=${3:-cli}

[[ "$state" =~ ^[1-7]{7}[1-3]{7}$ ]] || {
    echo "error: expected a 14-digit cube input" >&2
    exit 1
}
[[ "$output" == *.elf ]] || {
    echo "error: output path must end in .elf" >&2
    exit 1
}
[[ "$mode" == cli || "$mode" == gui ]] || {
    echo "error: mode must be cli or gui" >&2
    exit 1
}
[[ "$(sed -n '2p' target/tables.s)" == ".data" ]] || {
    echo "error: unexpected table section declaration" >&2
    exit 1
}
grep -q '"21345671111111"' target/search.s || {
    echo "error: default input string was not found" >&2
    exit 1
}

mkdir -p "$(dirname "$output")"
src="$output.s"
obj="$output.o"

{
    # Only generated tables move to the read-only section.
    sed '2s/^\.data$/.section .rodata\n.balign 2/' target/tables.s

    awk -v mode="$mode" '
        /^# RENDER_BEGIN/ { skip=1; next }
        /^# RENDER_END/   { skip=0; next }
        mode == "cli" && skip { next }
        { print }
    ' target/search.s |
    sed "s/\"21345671111111\"/\"$state\"/" |
    awk '
        /^[[:space:]]*\.text[[:space:]]*$/ {
            print
            print ".globl _start"
            print "_start:"
            next
        }
        { print }
    '
} > "$src"

if [[ "$mode" == cli ]] && grep -q 'LED_MATRIX' "$src"; then
    echo "error: CLI source still references LED symbols" >&2
    exit 1
fi

defs=()
if [[ "$mode" == gui ]]; then
    defs=(
        --defsym "LED_MATRIX_0_BASE=${LED_BASE:-0xF0000000}"
        --defsym "LED_MATRIX_0_WIDTH=${LED_WIDTH:-35}"
        --defsym "LED_MATRIX_0_HEIGHT=${LED_HEIGHT:-25}"
    )
fi

riscv64-unknown-elf-as \
    -march=rv32i -mabi=ilp32 -mno-relax \
    "${defs[@]}" "$src" -o "$obj"

riscv64-unknown-elf-ld \
    -m elf32lriscv --no-relax -e _start \
    "$obj" -o "$output"
