/* Reference build of the target C solver for helper-call inspection and
 * comparison with the hand-written RV32I version. */
#include "solve.c"

static const char input[] = "21345671111111";

/* volatile keeps the result observable, so the compiler cannot discard the search. */
volatile uint8_t result_length;

int main(void)
{
    uint16_t p, o;
    parse_ranks(input, &p, &o);
    result_length = solve_ranks(p, o);
    return 0;
}
