.equ N, 3000000
.text
.globl main

main:
    
    li t0, 0x20000000 # set pointer

    li t1, N # set N
    
    beqz t1, end_program  

write_loop:
    sw x0, 0(t0) # write 0

    addi t0, t0, 4 # move to the next memory

    addi t1, t1, -1 # counter decrease

    bnez t1, write_loop # branch if counter is not 0

end_program:
    li a7, 10
    ecall
