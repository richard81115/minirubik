

.data
.align 2
perm_rows:
    .word perm_move_0, perm_move_1, perm_move_2
orient_rows:
    .word orient_move_0, orient_move_1, orient_move_2
records:
    .zero 144
input:
    .string "21345671111111"

.align 1
factorial:
    .half 720, 120, 24, 6, 2, 1, 1

face_letters: # printing R, B, D
    .byte 82, 66, 68
turn_letters: # printing blank, 2, '
    .byte 32, 50, 39

# LED
# RENDER_BEGIN
.align 2
face_color: # 白橙綠紅藍黃
    .word 0xFFFFFF, 0xFFA500, 0x00FF00, 0xFF0000, 0x0000FF, 0xFFFF00
face_origin:
    .byte  9,  0    # U
    .byte  0,  7    # L
    .byte  9,  7    # F
    .byte 18,  7    # R
    .byte 27,  7    # B
    .byte  9, 14    # D
facelets:
    .byte 0, 0, 0, 0, 1, 1, 1, 1, 2, 2, 2, 2, 3, 3, 3, 3, 4, 4, 4, 4, 5, 5, 5, 5
turn_cycles:
    .byte 12, 13, 15, 14,   3, 16, 23, 11,   1, 18, 21,  9
    .byte 16, 17, 19, 18,   1,  4, 22, 15,   0,  6, 23, 13
    .byte 20, 21, 23, 22,  10, 14, 18,  6,   7, 11, 15, 19
# RENDER_END

.text
# finding permutation rank
    li a1, 0
    la t0, input # 外層指標
    la t2, factorial #factorial 指標
    addi t6, t0, 7 # ending position

outer_loop:
    lbu t1, 0(t0) # input[i]
    lhu t3, 0(t2) # factorail[i]
    addi t4, t0, 1 # 內層指標
    lbu t5, 0(t4) # input[j]

inner_loop:
    beq t4, t6, inner_done # whether j equals 7
    bgeu t5, t1, inputJ_greater_than_inputI
    add a1, a1, t3 # p += factorial[i]
inputJ_greater_than_inputI:
    addi t4, t4, 1
    lbu t5, 0(t4)
    j inner_loop

inner_done:
    addi t0, t0, 1
    addi t2, t2, 2 # reset factorial[i]
    bne t0, t6, outer_loop

# finding orientation rank
    li a2, 0
    la t0, input
    addi t6, t0, 13 # ending position
    addi t0, t0, 7 # start position

loop_start:
    beq t0, t6, loop_done

    lbu t5, 0(t0)
    addi t5, t5, -49
    slli t3, a2, 1
    add t3, t3, a2
    add a2, t3, t5
    addi t0, t0, 1

    j loop_start

loop_done:
    la s1, records
    la s5, perm_rows
    la s6, orient_rows
    la s7, perm_dist
    la s8, orient_dist

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
    bltu t0, a3, level_face_not_yet_three

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
    slli t2, t1, 1 # face * 3
    add t2, t1, t2
    add t2, t0, t2 # face * 3 + turn
    sb t2, 11(s0)

    addi t0, t0, 1 # ++level_turn
    bne t0, a3, not_three_yet # 轉數還不到 3，跳走

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
    mv t0, s1
    addi t6, s4, 1

verify_loop:
    beq t6, x0, verify_check
    lbu t1, 11(t0)
    li t2, 0 # face
    li t3, 3 # constant 3 for comparison
    bltu t1, t3, decode_done
    li t2, 1
    addi t1, t1, -3
    bltu t1, t3, decode_done
    li t2, 2
    addi t1, t1, -3
decode_done:
    slli a4, t2, 2
    add t5, s5, a4
    lw t5, 0(t5)
    add t4, s6, a4
    lw t4, 0(t4)

    addi t1, t1, 1

apply_loop:
    slli a1, a1, 1
    add a1, a1, t5
    lhu a1, 0(a1)
    slli a2, a2, 1
    add a2, a2, t4
    lhu a2, 0(a2)

    addi t1, t1, -1
    bne t1, zero, apply_loop

    addi t0, t0, 12
    addi t6, t6, -1
    j verify_loop

verify_check:
    bne a1, zero, exit_with_255
    bne a2, zero, exit_with_255
    j print_path

exit_with_255:
    li a0, 255
    li a7, 93
    ecall

print_path:
    mv t0, s1
    addi t6, s4, 1

print_loop:
    beq t6, x0, print_done
    lbu t1, 11(t0)
    li t2, 0 # face
    li t3, 3 # constant 3 for comparison
    la t4, face_letters
    la t5, turn_letters

    bltu t1, t3, print_out
    li t2, 1
    addi t1, t1, -3
    bltu t1, t3, print_out
    li t2, 2
    addi t1, t1, -3

print_out:
    add t4, t4, t2
    lbu t4, 0(t4)
    mv a0, t4
    li a7, 11
    ecall

    beq t1, x0, label_for_turn_equals_zero
    add t5, t5, t1
    lbu t5, 0(t5)
    mv a0, t5
    li a7, 11
    ecall

label_for_turn_equals_zero:
    addi t0, t0, 12
    addi t6, t6, -1
    beq t6, x0, print_loop

    # if t6 not zero print blank
    la a0, turn_letters
    lbu a0, 0(a0)
    li a7, 11
    ecall
    j print_loop

print_done:
    li a0, 10
    li a7, 11
    ecall

# RENDER_BEGIN
    mv s9, s0

rev_loop:
    lbu t1, 11(s9)
    li a6, 0 # face
    li t3, 3 # constant 3 for comparison
    bltu t1, t3, rev_decoded
    li a6, 1
    addi t1, t1, -3
    bltu t1, t3, rev_decoded
    li a6, 2
    addi t1, t1, -3

rev_decoded:
    sub s11, t3, t1

rev_turn:
    jal ra, quarter_turn
    addi s11, s11, -1
    bne s11, x0, rev_turn
    beq s9, s1, draw_start_point
    addi s9, s9, -12
    j rev_loop

draw_start_point:
    jal ra, draw_cube

    mv s9, s1
    addi s10, s4, 1

fwd_loop:
    beq s10, x0, anim_done
    lbu t1, 11(s9)
    li a6, 0 # face
    li t3, 3 # constant 3 for comparison
    bltu t1, t3, fwd_decoded
    li a6, 1
    addi t1, t1, -3
    bltu t1, t3, fwd_decoded
    li a6, 2
    addi t1, t1, -3

fwd_decoded:
    addi s11, t1, 1

fwd_turn:
    jal ra, quarter_turn
    addi s11, s11, -1
    bne s11, x0, fwd_turn
    jal ra, draw_cube
    addi s9, s9, 12
    addi s10, s10, -1
    j fwd_loop

anim_done:
# RENDER_END

    addi a0, s4, 1
    li a7, 93
    ecall

not_found:
    mv s2, s3
    j round_start

end_program:
    li a0, 0
    li a7, 93
    ecall

# RENDER_BEGIN
quarter_turn:
    slli t4, a6, 3
    slli t5, a6, 2
    add t4, t4, t5
    la t0, turn_cycles
    add t0, t0, t4
    la t1, facelets
    li t2, 3

cycle_loop:
    lbu a3, 0(t0)
    add a3, a3, t1
    lbu a4, 1(t0)
    add a4, a4, t1
    lbu a5, 2(t0)
    add a5, a5, t1
    lbu t3, 3(t0)
    add t3, t3, t1

    lbu t5, 0(t3)
    lbu t6, 0(a5)
    sb t6, 0(t3)
    lbu t6, 0(a4)
    sb t6, 0(a5)
    lbu t6, 0(a3)
    sb t6, 0(a4)
    sb t5, 0(a3)

    addi t0, t0, 4
    addi t2, t2, -1
    bne t2, x0, cycle_loop
    ret

draw_cube:
    li t0, LED_MATRIX_0_BASE
    la t1, facelets
    la t2, face_color
    la t3, face_origin

    li a0, LED_MATRIX_0_WIDTH
    slli a0, a0, 2          # a0 = 4W

    slli a1, a0, 1         # a1 = 8W
    add a1, a1, a0         # a1 = 12W

    li a2, 7
    li t4, 0               # Facelet index

draw_loop:
    # Each face has four facelets and two coordinate bytes.
    srli a3, t4, 2         # Face index
    slli a3, a3, 1         # Face index * 2
    add a3, t3, a3

    lbu a4, 1(a3)          # y
    lbu a3, 0(a3)          # x

    # Row offset: 4 * W * y.
    # This layout uses only y = 0, 7, or 14.
    beq a4, x0, draw_y_zero

    slli t5, a0, 3         # 32W
    sub t5, t5, a0         # 28W
    beq a4, a2, draw_y_ready

    slli t5, t5, 1         # 56W for y = 14
    j draw_y_ready

draw_y_zero:
    li t5, 0

draw_y_ready:
    slli a3, a3, 2         # 4x
    add t5, t5, a3         # Face origin byte offset

    # Position within the face: 0, 1, 2, or 3.
    andi a4, t4, 3
    andi a5, a4, 1
    slli a5, a5, 4         # Right column: +16 bytes
    add t5, t5, a5

    srli a4, a4, 1
    beq a4, x0, draw_address_ready
    add t5, t5, a1         # Lower row: +12W bytes

draw_address_ready:
    add t5, t5, t0         # Absolute pixel address

    # Load the RGB color for this facelet.
    add t6, t1, t4
    lbu t6, 0(t6)
    slli t6, t6, 2
    add t6, t2, t6
    lw t6, 0(t6)

    # First pixel row.
    sw t6, 0(t5)
    sw t6, 4(t5)
    sw t6, 8(t5)
    sw t6, 12(t5)

    # Second pixel row.
    add t5, t5, a0
    sw t6, 0(t5)
    sw t6, 4(t5)
    sw t6, 8(t5)
    sw t6, 12(t5)

    # Third pixel row.
    add t5, t5, a0
    sw t6, 0(t5)
    sw t6, 4(t5)
    sw t6, 8(t5)
    sw t6, 12(t5)

    addi t4, t4, 1
    li a3, 24
    bne t4, a3, draw_loop

    ret

# RENDER_END
