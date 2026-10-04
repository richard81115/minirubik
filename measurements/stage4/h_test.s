.text
    li t0, 0
    li t1, 0

    
    slli t4, t0, 1
    slli t5, t1, 1
    la t2, perm_move_0
    la t3, orient_move_0
    add t2, t2, t4
    add t3, t3, t5
    lhu a2, 0(t2)
    lhu a3, 0(t3)

    la t2, perm_dist
    la t3, orient_dist
    add t2, t2, a2
    add t3, t3, a3
    lbu a4, 0(t2)
    lbu a5, 0(t3)


    bltu a4, a5, set_orient_as_bigger
    mv a0, a4
    j end_program

set_orient_as_bigger:
    mv a0, a5

end_program:
    li a7, 93
    ecall
