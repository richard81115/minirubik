/* Host-only H2 checks against the baseline cubie model. */
#define main baseline_program_main
#include "../solver.c"
#undef main

#include "../target/tables.h"

static const uint16_t *const h2_perm_rows[3] = {
    perm_move[0], perm_move[1], perm_move[2]
};
static const uint16_t *const h2_orient_rows[3] = {
    orient_move[0], orient_move[1], orient_move[2]
};

static uint16_t reference_move(uint16_t x, uint8_t face,
                               int permutation_component)
{
    state_t state;
    uint32_t rank = permutation_component
                        ? (uint32_t) x * ORIENTATIONS
                        : x;
    unrank_state(rank, &state);
    state = quarter_turn(state, face);
    rank = rank_state(&state);
    return (uint16_t) (permutation_component
                          ? rank / ORIENTATIONS
                          : rank % ORIENTATIONS);
}

static unsigned check_moves(const char *name,
                            const uint16_t *const rows[3],
                            uint16_t n, int permutation_component)
{
    uint8_t seen[PERMUTATIONS];
    unsigned errors = 0;

    for (uint8_t face = 0; face < 3; ++face) {
        unsigned row_errors = 0;
        uint16_t max_value = 0;
        memset(seen, 0, sizeof seen);

        for (uint16_t x = 0; x < n; ++x) {
            uint16_t v = rows[face][x];
            if (v >= n) {
                ++row_errors;
                continue;
            }
            if (seen[v])
                ++row_errors;
            seen[v] = 1;
            if (v > max_value)
                max_value = v;

            if (v != reference_move(x, face, permutation_component))
                ++row_errors;
        }

        for (uint16_t x = 0; x < n; ++x)
            if (!seen[x])
                ++row_errors;

        if (max_value != n - 1U)
            ++row_errors;

        /* Follow transitions only after this row has passed validation. */
        if (row_errors == 0) {
            for (uint16_t x = 0; x < n; ++x) {
                uint16_t y = x;
                for (unsigned k = 0; k < 4; ++k)
                    y = rows[face][y];
                if (y != x)
                    ++row_errors;
            }
        }

        printf("%s[%u]: max=%u, solved=%u, expected_solved=%u, errors=%u\n",
               name, (unsigned) face, (unsigned) max_value,
               (unsigned) rows[face][0],
               (unsigned) reference_move(0, face, permutation_component),
               row_errors);
        errors += row_errors;
    }
    return errors;
}

static unsigned check_distances(const char *name, const uint8_t *dist,
                                const uint16_t *const rows[3],
                                uint16_t n, uint8_t expected_max)
{
    unsigned errors = 0;
    uint8_t max_value = 0;

    for (uint16_t x = 0; x < n; ++x) {
        uint8_t d = dist[x];
        if (d == UINT8_MAX || (d == 0) != (x == 0))
            ++errors;
        if (d > max_value)
            max_value = d;

        int has_closer = 0;
        for (uint8_t face = 0; face < 3; ++face) {
            uint16_t y = x;
            for (unsigned turn = 0; turn < 3; ++turn) {
                y = rows[face][y];
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

    if (max_value != expected_max)
        ++errors;

    printf("%s: max=%u, expected_max=%u, solved=%u, errors=%u\n",
           name, (unsigned) max_value, (unsigned) expected_max,
           (unsigned) dist[0], errors);
    return errors;
}

int main(void)
{
    unsigned errors = check_moves("perm_move", h2_perm_rows,
                                  PERMUTATIONS, 1);
    errors += check_moves("orient_move", h2_orient_rows,
                          ORIENTATIONS, 0);
    if (errors) {
        printf("H2 transition errors: %u; distance checks skipped\n", errors);
        return 1;
    }

    errors += check_distances("perm_dist", perm_dist, h2_perm_rows,
                              PERMUTATIONS, 7);
    errors += check_distances("orient_dist", orient_dist, h2_orient_rows,
                              ORIENTATIONS, 6);
    printf("H2 total errors: %u\n", errors);
    return errors ? 1 : 0;
}
