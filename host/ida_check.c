#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <time.h>

enum { CUBIES = 7, PERMUTATIONS = 5040, ORIENTATIONS = 729 };

typedef struct {
    uint8_t p[CUBIES], o[CUBIES];
} state_t;

/* Copied from solver.c: move definitions. */
static const uint8_t source[3][CUBIES] = {
    {1, 4, 2, 0, 3, 5, 6},
    {0, 1, 2, 4, 5, 6, 3},
    {0, 2, 5, 3, 1, 4, 6},
};
static const uint8_t twist[3][CUBIES] = {
    {1, 2, 0, 2, 1, 0, 0},
    {0, 0, 0, 1, 2, 1, 2},
    {0, 0, 0, 0, 0, 0, 0},
};

/* Copied from solver.c. */
static state_t quarter_turn(state_t state, uint8_t face)
{
    state_t result;
    for (uint8_t i = 0; i < CUBIES; ++i) {
        uint8_t from = source[face][i];
        result.p[i] = state.p[from];
        result.o[i] = (uint8_t) ((state.o[from] + twist[face][i]) % 3U);
    }
    return result;
}

/* Copied from solver.c. */
static uint32_t rank_state(const state_t *state)
{
    uint32_t p = 0, o = 0;
    for (uint8_t i = 0; i < CUBIES; ++i) {
        uint8_t smaller = 0;
        for (uint8_t j = (uint8_t) (i + 1U); j < CUBIES; ++j)
            if (state->p[j] < state->p[i])
                ++smaller;
        p = p * (CUBIES - i) + smaller;
    }
    for (uint8_t i = 0; i < 6; ++i)
        o = o * 3U + state->o[i];
    return p * ORIENTATIONS + o;
}

/* Copied from solver.c; a cast removes the -Wsign-compare warning. */
static void unrank_state(uint32_t rank, state_t *state)
{
    uint8_t available[CUBIES] = {0, 1, 2, 3, 4, 5, 6};
    uint32_t p = rank / ORIENTATIONS, o = rank % ORIENTATIONS, f = 720;
    uint8_t sum = 0;
    for (uint8_t i = 0; i < CUBIES; ++i) {
        uint8_t q = (uint8_t) (p / f);
        p %= f;
        state->p[i] = available[q];
        for (uint8_t j = q; j + 1U < (unsigned) (CUBIES - i); ++j)
            available[j] = available[j + 1U];
        if (i < 5)
            f /= 6U - i;
    }
    for (uint8_t i = 6; i-- > 0;) {
        state->o[i] = (uint8_t) (o % 3U);
        sum = (uint8_t) (sum + state->o[i]);
        o /= 3U;
    }
    state->o[6] = (uint8_t) ((3U - sum % 3U) % 3U);
}

static uint16_t perm_move[3][PERMUTATIONS];
static uint16_t orient_move[3][ORIENTATIONS];
static uint8_t perm_dist[PERMUTATIONS];
static uint8_t orient_dist[ORIENTATIONS];

/* Same construction as the first half of build_table in solver.c. */
static void build_moves(void)
{
    state_t state;
    for (uint16_t r = 0; r < PERMUTATIONS; ++r) {
        unrank_state((uint32_t) r * ORIENTATIONS, &state);
        for (uint8_t face = 0; face < 3; ++face) {
            state_t next = quarter_turn(state, face);
            perm_move[face][r] = (uint16_t) (rank_state(&next) / ORIENTATIONS);
        }
    }
    for (uint16_t r = 0; r < ORIENTATIONS; ++r) {
        unrank_state(r, &state);
        for (uint8_t face = 0; face < 3; ++face) {
            state_t next = quarter_turn(state, face);
            orient_move[face][r] = (uint16_t) (rank_state(&next) % ORIENTATIONS);
        }
    }
}

/* BFS from rank 0 over one projection. move is a [3][n] table stored row by row.
 * Writes the distance of every rank into dist and returns how many were reached. */
static uint16_t bfs(uint16_t n, const uint16_t *move, uint8_t *dist)
{
    uint16_t queue[PERMUTATIONS];
    uint16_t head = 0, tail = 1;
    memset(dist, 0xFF, n);
    dist[0] = 0;
    queue[0] = 0;
    while (head < tail) {
        uint16_t here = queue[head++];
        for (uint8_t face = 0; face < 3; ++face) {
            uint16_t next = here;
            for (uint8_t turn = 0; turn < 3; ++turn) {
                next = move[face * n + next];
                if (dist[next] == 0xFF) {
                    dist[next] = (uint8_t) (dist[here] + 1U);
                    queue[tail++] = next;
                }
            }
        }
    }
    return tail;
}

static void report(const char *name, uint16_t n, uint16_t reached, const uint8_t *dist)
{
    uint32_t count[16] = {0};
    uint8_t max = 0;
    for (uint16_t i = 0; i < n; ++i) {
        ++count[dist[i]];
        if (dist[i] > max)
            max = dist[i];
    }
    printf("%s: %u states, %u reached, max distance %u\n", name, (unsigned) n,
           (unsigned) reached, (unsigned) max);
    for (uint8_t d = 0; d <= max; ++d)
        printf("  distance %2u: %6u\n", (unsigned) d, (unsigned) count[d]);
}

enum { STATES = PERMUTATIONS * ORIENTATIONS };

static uint8_t full_dist[STATES];
static uint32_t full_queue[STATES];

/* Exact BFS over all states, as in solver.c, but storing distances. */
static uint32_t full_bfs(void)
{
    uint32_t head = 0, tail = 1;
    memset(full_dist, 0xFF, sizeof full_dist);
    full_dist[0] = 0;
    full_queue[0] = 0;
    while (head < tail) {
        uint32_t here = full_queue[head++];
        uint16_t p = (uint16_t) (here / ORIENTATIONS);
        uint16_t o = (uint16_t) (here % ORIENTATIONS);
        for (uint8_t face = 0; face < 3; ++face) {
            uint16_t np = p, no = o;
            for (uint8_t turn = 0; turn < 3; ++turn) {
                np = perm_move[face][np];
                no = orient_move[face][no];
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

/* H1: compare h = max(perm_dist, orient_dist) with the exact distance d. */
static void check_heuristic(void)
{
    uint32_t violations = 0, gap[16] = {0};
    uint32_t count[16] = {0}, h_sum[16] = {0};
    uint8_t h_min[16], h_max[16] = {0};
    memset(h_min, 0xFF, sizeof h_min);
    for (uint32_t r = 0; r < STATES; ++r) {
        uint8_t hp = perm_dist[r / ORIENTATIONS];
        uint8_t ho = orient_dist[r % ORIENTATIONS];
        uint8_t h = hp > ho ? hp : ho;
        uint8_t d = full_dist[r];
        if (h > d)
            ++violations;
        else
            ++gap[d - h];
        ++count[d];
        h_sum[d] += h;
        if (h < h_min[d])
            h_min[d] = h;
        if (h > h_max[d])
            h_max[d] = h;
    }
    printf("H1 violations (h > d): %u\n", (unsigned) violations);
    printf("gap d - h:\n");
    for (uint8_t g = 0; g < 16; ++g)
        if (gap[g])
            printf("  %2u: %7u\n", (unsigned) g, (unsigned) gap[g]);
    printf("by exact distance d: states, min h, max h, mean h\n");
    for (uint8_t d = 0; d < 16; ++d)
        if (count[d])
            printf("  d=%2u: %7u  %u  %u  %.2f\n", (unsigned) d, (unsigned) count[d],
                   (unsigned) h_min[d], (unsigned) h_max[d],
                   (double) h_sum[d] / count[d]);
}

static uint64_t nodes;

static uint8_t heuristic(uint16_t p, uint16_t o)
{
    return perm_dist[p] > orient_dist[o] ? perm_dist[p] : orient_dist[o];
}

/* One bounded depth-first pass of IDA*. prev_face == 3 means no previous move.
 * Returns 1 when the solved state is reached within the bound. */
static int dfs(uint16_t p, uint16_t o, uint8_t g, uint8_t bound,
               uint8_t prev_face, uint8_t *next_bound)
{
    ++nodes;
    uint8_t f = (uint8_t) (g + heuristic(p, o));
    if (f > bound) {
        if (f < *next_bound)
            *next_bound = f;
        return 0;
    }
    if (p == 0 && o == 0)
        return 1;
    for (uint8_t face = 0; face < 3; ++face) {
        if (face == prev_face)
            continue;
        uint16_t np = p, no = o;
        for (uint8_t turn = 0; turn < 3; ++turn) {
            np = perm_move[face][np];
            no = orient_move[face][no];
            if (dfs(np, no, (uint8_t) (g + 1U), bound, face, next_bound))
                return 1;
        }
    }
    return 0;
}

/* Returns the length of the solution found. */
static uint8_t ida(uint16_t p, uint16_t o)
{
    uint8_t bound = heuristic(p, o);
    for (;;) {
        uint8_t next_bound = UINT8_MAX;
        if (dfs(p, o, 0, bound, 3, &next_bound))
            return bound;
        bound = next_bound;
    }
}

static void measure_ida(void)
{
    uint64_t total = 0, max_nodes = 0;
    uint32_t count = 0, wrong = 0, max_rank = 0;
    for (uint32_t r = 0; r < STATES; ++r) {
        if (full_dist[r] != 11)
            continue;
        nodes = 0;
        if (ida((uint16_t) (r / ORIENTATIONS), (uint16_t) (r % ORIENTATIONS)) != 11)
            ++wrong;
        total += nodes;
        ++count;
        if (nodes > max_nodes) {
            max_nodes = nodes;
            max_rank = r;
        }
    }
    printf("IDA* on distance-11 states: %u states, %u wrong lengths\n",
           (unsigned) count, (unsigned) wrong);
    printf("  nodes: max %llu (rank %u), mean %.0f\n",
           (unsigned long long) max_nodes, (unsigned) max_rank,
           (double) total / count);
    nodes = 0;
    ida(720, 0); /* 21345671111111 has rank 524880 = 720 * 729 + 0 */
    printf("  21345671111111: %llu nodes\n", (unsigned long long) nodes);
}

/* Non-recursive IDA*: explicit per-depth storage replaces recursion.
 * Distances never exceed 11, so depths 0..11 need 12 levels. */
enum { MAX_DEPTH = 12 };

static uint16_t level_p[MAX_DEPTH], level_o[MAX_DEPTH];
static uint16_t chain_p[MAX_DEPTH], chain_o[MAX_DEPTH];
static uint8_t level_face[MAX_DEPTH], level_turn[MAX_DEPTH];
static uint8_t level_prev[MAX_DEPTH];
static uint8_t path[MAX_DEPTH];

/* Enter depth d at state (p, o), reached by a move on face prev (3 = none). */
static void start_level(uint8_t d, uint16_t p, uint16_t o, uint8_t prev)
{
    level_p[d] = chain_p[d] = p;
    level_o[d] = chain_o[d] = o;
    level_prev[d] = prev;
    level_face[d] = (uint8_t) (prev == 0 ? 1 : 0);
    level_turn[d] = 0;
}

/* One bounded pass. Visits nodes in the same order as the recursive dfs. */
static int bounded_search(uint16_t p0, uint16_t o0, uint8_t bound,
                          uint8_t *next_bound, uint8_t *length)
{
    ++nodes;
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
        if (level_face[d] >= 3) {  /* every move at this depth has been tried */
            if (d == 0)
                return 0;
            --d;
            continue;
        }
        uint8_t face = level_face[d];
        uint16_t np = perm_move[face][chain_p[d]];
        uint16_t no = orient_move[face][chain_o[d]];
        chain_p[d] = np;
        chain_o[d] = no;
        path[d] = (uint8_t) (face * 3U + level_turn[d]);
        if (++level_turn[d] == 3) {  /* this face is finished: move to the next one */
            uint8_t next_face = (uint8_t) (face + 1U);
            if (next_face == level_prev[d])
                ++next_face;
            level_face[d] = next_face;
            level_turn[d] = 0;
            chain_p[d] = level_p[d];
            chain_o[d] = level_o[d];
        }
        ++nodes;
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

static uint8_t ida_iter(uint16_t p, uint16_t o)
{
    uint8_t bound = heuristic(p, o), length = 0;
    for (;;) {
        uint8_t next_bound = UINT8_MAX;
        if (bounded_search(p, o, bound, &next_bound, &length))
            return length;
        bound = next_bound;
    }
}

/* Apply the recorded path and report whether it reaches the solved state. */
static int path_solves(uint16_t p, uint16_t o, uint8_t length)
{
    for (uint8_t i = 0; i < length; ++i) {
        uint8_t face = (uint8_t) (path[i] / 3U);
        uint8_t turns = (uint8_t) (path[i] % 3U + 1U);
        for (uint8_t t = 0; t < turns; ++t) {
            p = perm_move[face][p];
            o = orient_move[face][o];
        }
    }
    return p == 0 && o == 0;
}

static void compare_iterative(void)
{
    uint32_t count = 0, node_mismatch = 0, bad = 0;
    for (uint32_t r = 0; r < STATES; ++r) {
        if (full_dist[r] != 11)
            continue;
        uint16_t p = (uint16_t) (r / ORIENTATIONS), o = (uint16_t) (r % ORIENTATIONS);
        nodes = 0;
        ida(p, o);
        uint64_t recursive_nodes = nodes;
        nodes = 0;
        uint8_t length = ida_iter(p, o);
        if (nodes != recursive_nodes)
            ++node_mismatch;
        if (length != 11 || !path_solves(p, o, length))
            ++bad;
        ++count;
    }
    printf("iterative vs recursive on %u distance-11 states: "
           "%u node-count mismatches, %u bad solutions\n",
           (unsigned) count, (unsigned) node_mismatch, (unsigned) bad);
}

/* H3: the search must return the exact distance for every state,
 * and the returned path must reach the solved state. */
static void full_h3(void)
{
    uint32_t wrong_length = 0, bad_path = 0, count[16] = {0};
    uint64_t sum_nodes[16] = {0}, max_nodes[16] = {0};
    clock_t start = clock();
    for (uint32_t r = 0; r < STATES; ++r) {
        uint16_t p = (uint16_t) (r / ORIENTATIONS), o = (uint16_t) (r % ORIENTATIONS);
        uint8_t d = full_dist[r];
        nodes = 0;
        uint8_t length = ida_iter(p, o);
        if (length != d)
            ++wrong_length;
        if (!path_solves(p, o, length))
            ++bad_path;
        ++count[d];
        sum_nodes[d] += nodes;
        if (nodes > max_nodes[d])
            max_nodes[d] = nodes;
    }
    double seconds = (double) (clock() - start) / CLOCKS_PER_SEC;
    printf("H3 over %u states: %u wrong lengths, %u paths not reaching solved\n",
           (unsigned) STATES, (unsigned) wrong_length, (unsigned) bad_path);
    printf("H3 CPU time: %.1f s\n", seconds);
    printf("nodes by exact distance d: states, mean, max\n");
    for (uint8_t d = 0; d < 16; ++d)
        if (count[d])
            printf("  d=%2u: %7u  %10.0f  %8llu\n", (unsigned) d, (unsigned) count[d],
                   (double) sum_nodes[d] / count[d], (unsigned long long) max_nodes[d]);
}

int main(void)
{
    build_moves();
    uint16_t reached = bfs(PERMUTATIONS, &perm_move[0][0], perm_dist);
    report("permutation", PERMUTATIONS, reached, perm_dist);
    reached = bfs(ORIENTATIONS, &orient_move[0][0], orient_dist);
    report("orientation", ORIENTATIONS, reached, orient_dist);
    printf("full BFS: %u states reached\n", (unsigned) full_bfs());
    check_heuristic();
    measure_ida();
    compare_iterative();
    full_h3();
    return 0;
}
