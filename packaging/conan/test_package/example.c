/* Minimal consumer of the packaged d4np-c library: include the umbrella header, touch a few
 * modules, and return non-zero on any unexpected result so `conan create` fails loudly. */
#include <stdio.h>

#include "d4np_c.h"

int main(void)
{
    d4np_arena_t arena;
    if (d4np_arena_init(&arena, NULL, 1024) != D4NP_OK) {
        return 1;
    }

    int64_t value = 0;
    d4np_str_view_t sv = d4np_str_view_from_str("1234");
    if (d4np_str_parse_int(sv, 10, &value) != D4NP_OK || value != 1234) {
        d4np_arena_destroy(&arena);
        return 2;
    }

    uint64_t h = d4np_hash_fnv1a_str("d4np-c");
    d4np_arena_destroy(&arena);

    printf("d4np-c OK: parsed=%lld hash=%llu\n", (long long)value, (unsigned long long)h);
    return 0;
}
