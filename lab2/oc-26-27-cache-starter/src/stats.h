#ifndef STATS_H
#define STATS_H
#include <stdint.h>
typedef enum {
    EVENT_CPU_READ, EVENT_CPU_WRITE,
    EVENT_L1_HIT, EVENT_L1_MISS,
    EVENT_VICTIM_HIT, EVENT_VICTIM_MISS,
    EVENT_MEMORY_READ, EVENT_MEMORY_WRITE,
    EVENT_COUNT
} Event;
typedef struct {
    uint64_t reads, writes;
    uint64_t l1_hits, l1_misses;
    uint64_t victim_hits, victim_misses;
    uint64_t memory_reads, memory_writes;
    uint64_t cycles;
} Stats;
/* Part 4. A reset affects neither memory nor any Cache object. */
void stats_reset(void);
void stats_record(Event event);
Stats stats_get(void);
#endif
