.text
.global _start

_start:
    call main

end_program:
    addi a7, zero, 93
    ecall
