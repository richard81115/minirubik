.data
    .align 2

    perm_rows:
        .word perm_move_0, perm_move_1, perm_move_2
    orient_rows:
        .word orient_move_0, orient_move_1, orient_move_2
    records:
        .zero 144

.text
    la s5, perm_rows
    li t0, 1
    slli t0, t0, 2
    add t1, t0, s5
    lw t2, 0(t1)
    
    la s9, perm_move_1
    
    bne t2, s9, set_a0_0
    
    la s10, records
    addi t2, zero, 1234
    sh t2, 4(s10)
    lhu t6, 4(s10)

    bne t2, t6, set_a0_0
    
    li a0, 1
    j end_program

set_a0_0:
    li a0, 0

end_program:
    
    li a7, 93
    ecall
