/* Target-side solver core: table-driven IDA* with no heap, no recursion,
 * and no multiplication, division, or remainder. */
#include <stdint.h>

#include "tables.h"

enum { MAX_DEPTH = 12 };

/* Row pointers replace face * 5040 and face * 729 index arithmetic. */
static const uint16_t *const perm_row[3] = {perm_move[0], perm_move[1], perm_move[2]};
static const uint16_t *const orient_row[3] = {orient_move[0], orient_move[1], orient_move[2]};

/* (6 - i)! for position i of the permutation. */
static const uint16_t factorial[7] = {720, 120, 24, 6, 2, 1, 1};

static uint16_t level_p[MAX_DEPTH], level_o[MAX_DEPTH];
static uint16_t chain_p[MAX_DEPTH], chain_o[MAX_DEPTH];
static uint8_t level_face[MAX_DEPTH], level_turn[MAX_DEPTH];
static uint8_t level_prev[MAX_DEPTH];
static uint8_t path[MAX_DEPTH];

#ifdef SOLVE_COUNT_NODES
static uint64_t nodes;
#define COUNT_NODE() (++nodes)
#else
#define COUNT_NODE() ((void) 0)
#endif

static uint8_t heuristic(uint16_t p, uint16_t o)
{
    uint8_t a = perm_dist[p], b = orient_dist[o];
    return a > b ? a : b;
}

/* Convert "PPPPPPPOOOOOOO" (digits from '1') into the two ranks.
 * The input is assumed to be a valid state. */
static void parse_ranks(const char *input, uint16_t *p_out, uint16_t *o_out)
{
    uint16_t p = 0, o = 0;
    for (uint8_t i = 0; i < 7; ++i) {
        uint8_t smaller = 0;
        for (uint8_t j = (uint8_t) (i + 1U); j < 7; ++j)
            if (input[j] < input[i])
                ++smaller;
        for (; smaller; --smaller)
            p = (uint16_t) (p + factorial[i]);
    }
    for (uint8_t i = 7; i < 13; ++i)
        o = (uint16_t) ((o << 1) + o + (uint8_t) (input[i] - '1'));
    *p_out = p;
    *o_out = o;
}

static void start_level(uint8_t d, uint16_t p, uint16_t o, uint8_t prev)
{
    level_p[d] = chain_p[d] = p;
    level_o[d] = chain_o[d] = o;
    level_prev[d] = prev;
    level_face[d] = (uint8_t) (prev == 0 ? 1 : 0);
    level_turn[d] = 0;
}

/* One bounded pass, in the same order as host/ida_check.c. */
static int bounded_search(uint16_t p0, uint16_t o0, uint8_t bound,
                          uint8_t *next_bound, uint8_t *length)
{
    COUNT_NODE();
    uint8_t f = heuristic(p0, o0);
    if (f > bound) {
        if (f < *next_bound)
            *next_bound = f;
        return 0;
    }
    if (p0 == 0 && o0 == 0) {
        *length = 0;
        return 1;
    }
    uint8_t d = 0;
    start_level(0, p0, o0, 3);
    for (;;) {
        if (level_face[d] >= 3) {
            if (d == 0)
                return 0;
            --d;
            continue;
        }
        uint8_t face = level_face[d];
        uint16_t np = perm_row[face][chain_p[d]];
        uint16_t no = orient_row[face][chain_o[d]];
        chain_p[d] = np;
        chain_o[d] = no;
        path[d] = (uint8_t) ((face << 1) + face + level_turn[d]);
        if (++level_turn[d] == 3) {
            uint8_t next_face = (uint8_t) (face + 1U);
            if (next_face == level_prev[d])
                ++next_face;
            level_face[d] = next_face;
            level_turn[d] = 0;
            chain_p[d] = level_p[d];
            chain_o[d] = level_o[d];
        }
        COUNT_NODE();
        uint8_t g = (uint8_t) (d + 1U);
        f = (uint8_t) (g + heuristic(np, no));
        if (f > bound) {
            if (f < *next_bound)
                *next_bound = f;
            continue;
        }
        if (np == 0 && no == 0) {
            *length = g;
            return 1;
        }
        d = g;
        start_level(d, np, no, face);
    }
}

/* Solve from the two ranks. The moves are left in path[0 .. length - 1]. */
static uint8_t solve_ranks(uint16_t p, uint16_t o)
{
    uint8_t bound = heuristic(p, o), length = 0;
    for (;;) {
        uint8_t next_bound = UINT8_MAX;
        if (bounded_search(p, o, bound, &next_bound, &length))
            return length;
        bound = next_bound;
    }
}
