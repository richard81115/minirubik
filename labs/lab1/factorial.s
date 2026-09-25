        .data
argument:
        .word   15                       # compute the factorial of this value
str1:   .string "Factorial value of "
str2:   .string " is "
nl:     .string "\n"

        .text
        .globl main

main:
        lw    a0, argument
        jal   fact                      # a0 = fact(7)

        mv    a1, a0                    # a1 = the result
        lw    a0, argument              # a0 = the original input
        jal   print_result

        li    a7, 10                    # Exit
        ecall

# fact(n) returns n! for n >= 0, and 1 for every n <= 0.
# The argument and the return value both use a0.
fact:
        addi  sp, sp, -16               # 16-byte frame, per the RISC-V ABI
        sw    ra, 8(sp)                 # save the return address
        sw    a0, 0(sp)                 # save n across the recursive call

        blez  a0, base                  # test n itself, not n - 1

        addi  a0, a0, -1
        jal   fact                      # a0 = fact(n - 1)

        lw    t0, 0(sp)                 # reload n
        lw    ra, 8(sp)
        addi  sp, sp, 16

        mul   a0, t0, a0                # n * fact(n - 1), needs the M extension
        ret

base:
        li    a0, 1                     # 0! = 1
        addi  sp, sp, 16
        ret

# print_result(a0 = n, a1 = n!) prints "Factorial value of <n> is <n!>".
# Both arguments have to survive four ecalls, so they are held in callee-saved
# registers, whose previous contents are restored before returning.
print_result:
        addi  sp, sp, -16
        sw    ra, 12(sp)
        sw    s0, 8(sp)
        sw    s1, 4(sp)
        mv    s0, a0
        mv    s1, a1

        la    a0, str1
        li    a7, 4                     # PrintStr
        ecall

        mv    a0, s0
        li    a7, 1                     # PrintInt
        ecall

        la    a0, str2
        li    a7, 4
        ecall

        mv    a0, s1
        li    a7, 1
        ecall

        la    a0, nl
        li    a7, 4
        ecall

        lw    s1, 4(sp)
        lw    s0, 8(sp)
        lw    ra, 12(sp)
        addi  sp, sp, 16
        ret
