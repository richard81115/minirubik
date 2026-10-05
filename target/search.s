.data
    .align 2

    perm_rows:
        .word perm_move_0, perm_move_1, perm_move_2
    orient_rows:
        .word orient_move_0, orient_move_1, orient_move_2
    records:
        .zero 144

.text
    la s1, records
    la s5, perm_rows
    la s6, orient_rows
    la s7, perm_dist
    la s8, orient_dist

    li a1, 720 # permutation rank
    li a2, 0 # orientation rank
    li a3, 3 # constant 3

    add t0, s7, a1
    add t1, s8, a2
    lbu t2, 0(t0)
    lbu t3, 0(t1)
    
    bltu t2, t3, set_orient_as_bigger
    mv s2, t2
    bne s2, x0, round_start # check if already solved
    j end_program

set_orient_as_bigger:
    mv s2, t3

    bne s2, x0, round_start # check if already solved
    j end_program

round_start:
    mv s0, s1
    li s3, 255
    li s4, 0

    sh a1, 0(s0) # level_p
    sh a2, 2(s0) # level_o
    sh a1, 4(s0) # chain_p
    sh a2, 6(s0) # chain_o
    sb x0, 8(s0) # level_face
    sb x0, 9(s0) # level_turn
    sb a3, 10(s0) # level_prev

next_child:
    lbu t0, 8(s0) # 從記錄讀出 level_face（位移 8）
    addi t4, x0, 3
    bltu t0, t4, level_face_not_yet_three

    beq s0, s1, not_found
    addi s0, s0, -12
    addi s4, s4, -1
    j next_child

level_face_not_yet_three:
    mv a6, t0
    slli t0, t0, 2 # face = face * 4
    add t1, t0, s5
    lw t1, 0(t1) # 用 face 從 perm_rows（s5）選出這一面的排列表開頭
    add t2, t0, s6
    lw t2, 0(t2) # 用 face 從 orient_rows（s6）選出方向表開頭

    lhu t0, 4(s0)
    slli t0, t0, 1
    add a4, t0, t1 # get np
    lhu a4, 0(a4)

    lhu t0, 6(s0)
    slli t0, t0, 1
    add a5, t0, t2 # get no
    lhu a5, 0(a5)

    sh a4, 4(s0) # store np back to chain_p
    sh a5, 6(s0) # store no back to chain_o

    lbu t0, 9(s0) # read level_turn
    lbu t1, 8(s0) # read level_face
    slli t2, t1,1 # face * 3
    add t2, t1, t2
    add t2, t0, t2 # face * 3 + turn
    sb t2, 11(s0)

    addi t0, t0, 1 # ++level_turn
    addi t2, x0, 3
    bne t0, t2, not_three_yet # 轉數還不到 3，跳走 
    
to_next_face:
    addi t1, t1, 1 # next_face
    lbu t2, 10(s0)
    beq t1, t2, to_next_face
    sb t1, 8(s0) # 把 next_face 寫回 level_face
    sb x0, 9(s0) # 把 level_turn 寫成 0
    
    lhu t0, 0(s0)
    sh t0, 4(s0)
    lhu t0, 2(s0)
    sh t0, 6(s0)
    
    j b2_done

not_three_yet:
    sb t0, 9(s0) # write number of turn back to level_turn
    
b2_done:
    add t0, s7, a4
    lbu t0, 0(t0)
    add t1, s8, a5
    lbu t1, 0(t1)

    add t2, x0, t1
    bgeu t1, t0, h_ready
    add t2, x0, t0

h_ready:

    addi t3, s4, 1 # g+1
    add t3, t3, t2 # f = (g+1) + h

    bgeu s2, t3, check_whether_solved

    bgeu t3, s3, no_update_s3
    mv s3, t3
no_update_s3:
    j next_child

check_whether_solved:
    bne a4, x0, to_next_depth
    bne a5, x0, to_next_depth
    j found

to_next_depth:
    addi s0, s0, 12
    addi s4, s4, 1
    sh a4, 0(s0)
    sh a5, 2(s0)
    sh a4, 4(s0)
    sh a5, 6(s0)
    sb x0, 9(s0)
    sb a6, 10(s0)
   
    sb x0, 8(s0) 
    bne a6, x0, store_0_to_level_face
    addi t0, x0, 1
    sb t0, 8(s0)
store_0_to_level_face:
    j next_child

found:
    addi t0, s4, 1
    mv a0, t0
    li a7, 93
    ecall

not_found:
    mv s2, s3
    j round_start


end_program:
    li a0, 0
    li a7, 93
    ecall
