/* Reference build of the target C solver for helper-call inspection and
 * comparison with the hand-written RV32I version. */
#include "solve.c"

#ifndef INPUT
#define INPUT "21345671111111"
#endif
static const char input[] = INPUT;

int main(void)
{
    uint16_t p, o;
    parse_ranks(input, &p, &o);
    return solve_ranks(p, o);
}
