#ifndef CONFIG_H
#define CONFIG_H
#include <stdint.h>
#define MEMORY_SIZE 65536u
#ifndef BLOCK_SIZE
#define BLOCK_SIZE 16u
#endif
#ifndef L1_SIZE
#define L1_SIZE 256u
#endif
#ifndef VICTIM_BLOCKS
#define VICTIM_BLOCKS 2u
#endif
#define L1_LINES (L1_SIZE / BLOCK_SIZE)
#define L1_CYCLES UINT64_C(1)
#define VICTIM_CYCLES UINT64_C(2)
#define MEMORY_READ_CYCLES UINT64_C(50)
#define MEMORY_WRITE_CYCLES UINT64_C(50)
_Static_assert(BLOCK_SIZE == 8 || BLOCK_SIZE == 16 || BLOCK_SIZE == 32,
               "BLOCK_SIZE must be 8, 16, or 32");
_Static_assert(L1_SIZE == 32 || L1_SIZE == 64 || L1_SIZE == 128 || L1_SIZE == 256,
               "L1_SIZE must be 32, 64, 128, or 256");
_Static_assert(L1_SIZE >= 2 * BLOCK_SIZE && L1_SIZE % (2 * BLOCK_SIZE) == 0,
               "L1 must contain an even number of at least two blocks");
_Static_assert(VICTIM_BLOCKS == 1 || VICTIM_BLOCKS == 2 || VICTIM_BLOCKS == 4,
               "VICTIM_BLOCKS must be 1, 2, or 4");
#endif
