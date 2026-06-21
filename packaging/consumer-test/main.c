/* Consumes an installed d4np-c via find_package + d4np::d4np. Touches several modules through
 * both include styles (umbrella + per-module) and returns non-zero on any unexpected result. */
#include <stdio.h>

#include "d4np/sys/hash.h"
#include "d4np_c.h"

int main(void)
{
    d4np_vector_t v;
    if (d4np_vector_init(&v, NULL, sizeof(int), 0) != D4NP_OK) {
        return 1;
    }
    int x = 7;
    if (d4np_vector_push(&v, &x) != D4NP_OK || d4np_vector_len(&v) != 1) {
        d4np_vector_destroy(&v);
        return 2;
    }
    d4np_vector_destroy(&v);

    uint64_t h = d4np_hash_fnv1a_str("d4np-c");
    if (h == 0) {
        return 3;
    }

    printf("consumer OK: hash=%llu\n", (unsigned long long)h);
    return 0;
}
