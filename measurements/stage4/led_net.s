.data
    .align 2
    
    face_color: # 白橙綠紅藍黃
        .word 0xFFFFFF, 0xFFA500, 0x00FF00, 0xFF0000, 0x0000FF, 0xFFFF00
    face_origin:
        .word 36, 980, 1016, 1052, 1088, 1996
    facelets:
        .byte 0, 0, 0, 0, 1, 1, 1, 1, 2, 2, 2, 2, 3, 3, 3, 3, 4, 4, 4, 4, 5, 5, 5, 5

.text
    jal ra, draw_cube
    li a7, 10
    ecall

    draw_cube:
    li t0, LED_MATRIX_0_BASE
    la t1, facelets
    la t2, face_color
    la t3, face_origin
    li t4, 0

    draw_loop:
    andi a3, t4, -4
    add a3, a3, t3
    lw t5, 0(a3)
    andi a4, t4, 3
    andi a5, a4, 1 
    slli a5, a5, 4 # column * 16
    add t5, t5, a5
    srli a4, a4, 1 # row
    beq a4, x0, is_zero
    addi t5, t5, 420
    
    is_zero:
    add t5, t5, t0
    
    add t6, t1, t4
    lbu t6, 0(t6)
    slli t6, t6, 2
    add t6, t2, t6
    lw t6, 0(t6)

    sw t6, 0(t5)
    sw t6, 4(t5)
    sw t6, 8(t5)
    sw t6, 12(t5)
    sw t6, 140(t5)
    sw t6, 144(t5)
    sw t6, 148(t5)
    sw t6, 152(t5)
    sw t6, 280(t5)
    sw t6, 284(t5)
    sw t6, 288(t5)
    sw t6, 292(t5)
    

    addi t4, t4, 1
    li a3, 24
    bne t4, a3, draw_loop
    
    ret
