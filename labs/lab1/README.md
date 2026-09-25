# Lab 1: RISC-V Assembly with Ripes

This note records my practice with the examples from [Lab 1: RV32I Simulator](https://hackmd.io/@sysprog/H1TpVYMdB), including factorial calculation, LED animation, and instruction tracing.

Date: September 25, 2026

## Environment

| Item | Configuration |
|---|---|
| Operating system | Windows 11 with WSL / Ubuntu 26.04.1 LTS |
| Architecture | x86_64 |
| Ripes build | v2.2.6-106-g5b8a616 |
| Processor model | Single-cycle processor |
| ISA for the factorial example | RV32IM |
| LED Matrix | Width 35, height 25 |

The Ripes AppImage initially failed to launch because `libOpenGL.so.0` was missing. Installing `libopengl0` resolved the issue.

## Factorial Example

I ran the recursive factorial example with three inputs:

| Input | Expected result | Observed result |
|---:|---:|---:|
| 5 | 120 | 120 |
| 7 | 5040 | 5040 |
| 10 | 3628800 | 3628800 |

All three runs produced the expected output and exited with code 0.

The example helped me understand how:

- `a0` carries the argument when entering `fact` and the result when returning.
- Each recursive call reserves 16 bytes on the stack and saves its argument and return address.
- The base case returns 1 for an input of zero.
- Each returning call multiplies its saved argument by the result from the next call.
- Environment calls print strings and integers.

This example uses `mul`, which requires the M extension. It is separate from the RV32I-only implementation required for Assignment 1.

## LED Matrix Animation

I ran the lab's LED animation and observed the pattern moving right and downward, with the old position restored to the blue background.

Each pixel occupies four bytes. For a matrix with width 35, the address of pixel `(x, y)` is:

```text
address = base + (y * 35 + x) * 4
```

This gives the following offsets:

| Movement | Address increase |
|---|---:|
| One pixel right | 4 bytes |
| One pixel down | 140 bytes |
| Two pixels right and one down | 148 bytes |

Writing a color to a new pixel does not clear the previous pixel. Animation therefore requires clearing the old pattern and drawing it at the new position.

Initially, I only saw a stationary pattern because I switched to the I/O tab after execution had finished. Keeping the LED Matrix in a separate window allowed me to watch the animation during execution.

## Breakpoints and Single-Stepping

I set a breakpoint at the multiplication instruction in `fact` and inspected the registers. At the first multiplication, both `t0` and `a0` contained 1, corresponding to the calculation of `1!`.

I also used F5 to advance execution one step at a time. This showed that executing an instruction does not always change a register's value: multiplying 1 by 1 still leaves 1 in `a0`.

## Current Progress

Completed:

- Launched Ripes successfully.
- Tested the factorial example with three inputs.
- Ran and observed the LED animation.
- Practiced setting a breakpoint and stepping through instructions.


## Sources

The factorial and LED examples come from the instructor's Lab 1 material. My work in this session consisted of running the examples, changing factorial inputs, observing results, and practicing debugging.
