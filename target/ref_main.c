/* Reference build of the target C solver. It performs the same observable
 * work as the handwritten RV32I version: parse, search, replay check, and
 * printing through ecall 11. */
#include "solve.c"

#ifndef INPUT
#define INPUT "21345671111111"
#endif
static const char input[] = INPUT;

static const char face_letter[3] = {'R', 'B', 'D'};
static const char turn_suffix[3] = {' ', '2', '\''};

/* Print one character with the Ripes ecall 11. */
static void put_char(int c)
{
    register int a0 __asm__("a0") = c;
    register int a7 __asm__("a7") = 11;
    __asm__ volatile("ecall" : "+r"(a0) : "r"(a7) : "memory");
}

/* Split a move code into face and turn by comparison and subtraction,
 * matching the handwritten version. */
static uint8_t decode(uint8_t code, uint8_t *turn)
{
    uint8_t face = 0;
    if (code >= 3) {
        face = 1;
        code = (uint8_t) (code - 3U);
        if (code >= 3) {
            face = 2;
            code = (uint8_t) (code - 3U);
        }
    }
    *turn = code;
    return face;
}

int main(void)
{
    uint16_t p, o;
    parse_ranks(input, &p, &o);
    uint8_t length = solve_ranks(p, o);
    if (length == 0)
        return 0;

    for (uint8_t i = 0; i < length; ++i) {
        uint8_t turn, face = decode(path[i], &turn);
        for (uint8_t t = 0; t <= turn; ++t) {
            p = perm_row[face][p];
            o = orient_row[face][o];
        }
    }
    if (p != 0 || o != 0)
        return 255;

    for (uint8_t i = 0; i < length; ++i) {
        uint8_t turn, face = decode(path[i], &turn);
        put_char(face_letter[face]);
        if (turn != 0)
            put_char(turn_suffix[turn]);
        if (i + 1 < length)
            put_char(' ');
    }
    put_char('\n');
    return length;
}
