/* List every distance-11 state as a 14-character input, one per line,
 * using the generated transition tables. */
#include <stdio.h>
#include <string.h>

#include "../target/tables.h"

enum {
    CUBIES = 7,
    PERMUTATIONS = 5040,
    ORIENTATIONS = 729,
    STATES = PERMUTATIONS * ORIENTATIONS
};

static uint8_t dist[STATES];
static uint32_t queue[STATES];

/* Exact BFS over all states, as in host/target_check.c. */
static uint32_t full_bfs(void)
{
    uint32_t head = 0, tail = 1;
    memset(dist, 0xFF, sizeof dist);
    dist[0] = 0;
    queue[0] = 0;
    while (head < tail) {
        uint32_t here = queue[head++];
        uint16_t p = (uint16_t) (here / ORIENTATIONS), o = (uint16_t) (here % ORIENTATIONS);
        for (uint8_t face = 0; face < 3; ++face) {
            uint16_t np = p, no = o;
            for (uint8_t turn = 0; turn < 3; ++turn) {
                np = perm_move[face][np];
                no = orient_move[face][no];
                uint32_t there = (uint32_t) np * ORIENTATIONS + no;
                if (dist[there] == 0xFF) {
                    dist[there] = (uint8_t) (dist[here] + 1U);
                    queue[tail++] = there;
                }
            }
        }
    }
    return tail;
}

/* Same as host/target_check.c: write the 14-character input for a rank. */
static void rank_to_input(uint32_t rank, char *out)
{
    uint8_t available[CUBIES] = {0, 1, 2, 3, 4, 5, 6}, ori[CUBIES], sum = 0;
    uint32_t p = rank / ORIENTATIONS, o = rank % ORIENTATIONS, f = 720;
    for (uint8_t i = 0; i < CUBIES; ++i) {
        uint8_t q = (uint8_t) (p / f);
        p %= f;
        out[i] = (char) ('1' + available[q]);
        for (uint8_t j = q; j + 1U < (unsigned) (CUBIES - i); ++j)
            available[j] = available[j + 1U];
        if (i < 5)
            f /= 6U - i;
    }
    for (uint8_t i = 6; i-- > 0;) {
        ori[i] = (uint8_t) (o % 3U);
        sum = (uint8_t) (sum + ori[i]);
        o /= 3U;
    }
    ori[6] = (uint8_t) ((3U - sum % 3U) % 3U);
    for (uint8_t i = 0; i < CUBIES; ++i)
        out[CUBIES + i] = (char) ('1' + ori[i]);
    out[14] = '\0';
}

int main(void)
{
    char input[15];
    uint32_t count = 0;
    if (full_bfs() != STATES) {
        fputs("BFS did not reach every state\n", stderr);
        return 1;
    }
    for (uint32_t r = 0; r < STATES; ++r) {
        if (dist[r] != 11)
            continue;
        rank_to_input(r, input);
        puts(input);
        ++count;
    }
    fprintf(stderr, "%u distance-11 states\n", (unsigned) count);
    return count == 2644 ? 0 : 1;
}
