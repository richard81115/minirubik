.text
    li t0, 6
    li t1, 7
    mul a0, t0, t1   # M-extension instruction; should be rejected under RV32I
    li a7, 10
    ecall
