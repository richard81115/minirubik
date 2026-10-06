.text
    li t0, LED_MATRIX_0_BASE

    li t1, 0xFF0000
    sw t1, 0(t0)
    li t1, 0x00FF00
    sw t1, 136(t0)
    li t1, 0x0000FF
    li t2, 3360
    add t3, t0, t2
    sw t1, 0(t3)

    li a7, 10
    ecall