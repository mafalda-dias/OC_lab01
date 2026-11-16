#ifndef CACHE_H
#define CACHE_H
#include <stdbool.h>
#include <stdint.h>
#include "config.h"
typedef enum { MODE_DM, MODE_SA2, MODE_VICTIM } CacheMode;
typedef struct {
    bool valid, dirty;
    uint32_t tag;
    uint8_t data[BLOCK_SIZE];
    uint64_t last_used;
} L1Line;
typedef struct {
    bool valid, dirty;
    uint32_t block_number; /* Full address / BLOCK_SIZE, NOT a byte address. */
    uint8_t data[BLOCK_SIZE];
    uint64_t last_used;
} VictimEntry;
typedef struct {
    CacheMode mode;
    L1Line l1[L1_LINES];
    VictimEntry victim[VICTIM_BLOCKS];
    uint64_t clock; /* Replacement timestamp; independent of stats.cycles. */
} Cache;
/* L1 array layout:
 * dm/victim: l1[index].  sa2: l1[2 * set + way], with way 0 or 1.
 * tag = block_number / number_of_sets in the selected mode.
 * last_used: larger means more recent; clock is the last assigned timestamp.
 * Initialization: mode set; all other fields and arrays zero.
 * Tests do not prescribe how many increments an operation uses. Each new
 * timestamp must be greater than all earlier timestamps in that cache.
 * Invalid entries' other fields are ignored after initialization.
 * No evaluated trace causes a 64-bit replacement clock to overflow.
 */
void cache_init(Cache *cache, CacheMode mode);
uint8_t cache_read(Cache *cache, uint32_t address);
void cache_write(Cache *cache, uint32_t address, uint8_t value);
void cache_flush(Cache *cache);
#endif
