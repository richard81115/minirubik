.text

li t0, 584390 # loop counter N

loop:
addi t0, t0, -1
bnez t0, loop

addi a7, x0, 10
ecall
