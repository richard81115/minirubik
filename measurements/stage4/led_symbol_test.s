.data
test_word: .word LED_MATRIX_0_WIDTH*4

.text
.global main
main:
    li t0, LED_MATRIX_0_WIDTH

#    li t1, LED_MATRIX_0_WIDTH*4

    li a7, 10
    ecall
