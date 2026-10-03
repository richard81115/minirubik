/* Host harness for target/solve.c: H2 table checks, input-parser check,
 * and full-domain H3 using the generated tables. */
#define SOLVE_COUNT_NODES
#include "../target/solve.c"

#include <stdio.h>
#include <string.h>
#include <time.h>

enum {
    CUBIES = 7,
    PERMUTATIONS = 5040,
    ORIENTATIONS = 729,
    STATES = PERMUTATIONS * ORIENTATIONS
};

/* Adapted from solver.c's unrank_state: write the 14-character input string. */
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

/* H2: every face row is a bijection, and four quarter turns return to the start. */
static unsigned check_move_table(const uint16_t *const row[3], uint16_t n)
{
    static uint8_t seen[PERMUTATIONS];
    unsigned errors = 0;
    for (uint8_t face = 0; face < 3; ++face) {
        memset(seen, 0, n);
        for (uint16_t x = 0; x < n; ++x) {
            uint16_t v = row[face][x];
            if (v >= n || seen[v]++) {
                ++errors;
                continue;
            }
            uint16_t y = x;
            for (uint8_t k = 0; k < 4 && y < n; ++k)
                y = row[face][y];
            if (y != x)
                ++errors;
        }
    }
    return errors;
}

/* H2: the table equals the exact BFS distance if only entry 0 is zero, neighbours
 * differ by at most one, and every nonzero entry has a neighbour one closer. */
static unsigned check_dist_table(const char *name, const uint8_t *dist,
                                 const uint16_t *const row[3], uint16_t n,
                                 uint8_t expected_max)
{
    unsigned errors = 0;
    uint8_t max = 0;
    for (uint16_t x = 0; x < n; ++x) {
        uint8_t d = dist[x];
        if (d == 0xFF || (d == 0) != (x == 0)) {
            ++errors;
            continue;
        }
        if (d > max)
            max = d;
        int has_closer = 0;
        for (uint8_t face = 0; face < 3; ++face) {
            uint16_t y = x;
            for (uint8_t turn = 0; turn < 3; ++turn) {
                y = row[face][y];
                uint8_t e = dist[y];
                if (e + 1 < d || d + 1 < e)
                    ++errors;
                if (e + 1 == d)
                    has_closer = 1;
            }
        }
        if (d != 0 && !has_closer)
            ++errors;
    }
    if (max != expected_max)
        ++errors;
    printf("%s: max %u, solved entry %u\n", name, (unsigned) max, (unsigned) dist[0]);
    return errors;
}

static uint8_t full_dist[STATES];
static uint32_t full_queue[STATES];

/* Exact BFS over all states, using the generated transition tables. */
static uint32_t full_bfs(void)
{
    uint32_t head = 0, tail = 1;
    memset(full_dist, 0xFF, sizeof full_dist);
    full_dist[0] = 0;
    full_queue[0] = 0;
    while (head < tail) {
        uint32_t here = full_queue[head++];
        uint16_t p = (uint16_t) (here / ORIENTATIONS), o = (uint16_t) (here % ORIENTATIONS);
        for (uint8_t face = 0; face < 3; ++face) {
            uint16_t np = p, no = o;
            for (uint8_t turn = 0; turn < 3; ++turn) {
                np = perm_row[face][np];
                no = orient_row[face][no];
                uint32_t there = (uint32_t) np * ORIENTATIONS + no;
                if (full_dist[there] == 0xFF) {
                    full_dist[there] = (uint8_t) (full_dist[here] + 1U);
                    full_queue[tail++] = there;
                }
            }
        }
    }
    return tail;
}

/* Apply path[0 .. length - 1] and report whether it reaches the solved state. */
static int path_solves(uint16_t p, uint16_t o, uint8_t length)
{
    for (uint8_t i = 0; i < length; ++i) {
        uint8_t face = (uint8_t) (path[i] / 3U), turns = (uint8_t) (path[i] % 3U + 1U);
        for (uint8_t t = 0; t < turns; ++t) {
            p = perm_row[face][p];
            o = orient_row[face][o];
        }
    }
    return p == 0 && o == 0;
}

int main(void)
{
    unsigned h2 = check_move_table(perm_row, PERMUTATIONS) +
                  check_move_table(orient_row, ORIENTATIONS) +
                  check_dist_table("perm_dist", perm_dist, perm_row, PERMUTATIONS, 7) +
                  check_dist_table("orient_dist", orient_dist, orient_row, ORIENTATIONS, 6);
    printf("H2 table errors: %u\n", h2);
    printf("full BFS with generated tables: %u states reached\n", (unsigned) full_bfs());

    uint32_t parse_errors = 0, wrong_length = 0, bad_path = 0, count11 = 0, max_rank = 0;
    uint64_t sum11 = 0, max11 = 0;
    char input[15];
    clock_t start = clock();
    for (uint32_t r = 0; r < STATES; ++r) {
        uint16_t p, o;
        rank_to_input(r, input);
        parse_ranks(input, &p, &o);
        if (p != r / ORIENTATIONS || o != r % ORIENTATIONS) {
            ++parse_errors;
            continue;
        }
        nodes = 0;
        uint8_t length = solve_ranks(p, o);
        if (length != full_dist[r])
            ++wrong_length;
        if (!path_solves(p, o, length))
            ++bad_path;
        if (full_dist[r] == 11) {
            ++count11;
            sum11 += nodes;
            if (nodes > max11) {
                max11 = nodes;
                max_rank = r;
            }
        }
    }
    double seconds = (double) (clock() - start) / CLOCKS_PER_SEC;
    printf("parser errors: %u\n", (unsigned) parse_errors);
    printf("H3 over %u states: %u wrong lengths, %u paths not reaching solved\n",
           (unsigned) STATES, (unsigned) wrong_length, (unsigned) bad_path);
    printf("H3 CPU time: %.1f s\n", seconds);
    rank_to_input(max_rank, input);
    printf("distance-11 nodes: %u states, max %llu (%s), mean %.0f\n", (unsigned) count11,
           (unsigned long long) max11, input, (double) sum11 / count11);
    return (h2 || parse_errors || wrong_length || bad_path) ? 1 : 0;
}
