/* Apply HTM moves to the solved state using solver.c's source/twist tables. */
#define main solver_main
#include "../solver.c"
#undef main
#include <stdio.h>
#include <string.h>

int main(int argc, char **argv)
{
    int p[CUBIES], o[CUBIES];
    for (int i = 0; i < CUBIES; i++) { p[i] = i; o[i] = 0; }
    for (int a = 1; a < argc; a++) {
        const char *m = argv[a];
        int face = m[0] == 'R' ? 0 : m[0] == 'B' ? 1 : m[0] == 'D' ? 2 : -1;
        if (face < 0) { fprintf(stderr, "bad move: %s\n", m); return 1; }
        int q = m[1] == '2' ? 2 : m[1] == '\'' ? 3 : 1;
        while (q--) {
            int np[CUBIES], no[CUBIES];
            for (int i = 0; i < CUBIES; i++) {
                int from = source[face][i];
                np[i] = p[from];
                no[i] = (o[from] + twist[face][i]) % 3;
            }
            memcpy(p, np, sizeof p);
            memcpy(o, no, sizeof o);
        }
    }
    for (int i = 0; i < CUBIES; i++) putchar('1' + p[i]);
    for (int i = 0; i < CUBIES; i++) putchar('1' + o[i]);
    putchar('\n');
    return 0;
}
