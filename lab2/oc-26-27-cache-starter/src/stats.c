/* STUDENT FILE: Part 4. Do not print to stdout. */
#include "config.h"
#include "stats.h"

static Stats counters;

void stats_reset(void)
{
    Stats zero = {0};
    counters = zero;
}

void stats_record(Event event)
{
    switch (event) {
        case EVENT_CPU_READ:
            counters.reads++;
            break;
        case EVENT_CPU_WRITE:
            counters.writes++;
            break;
        case EVENT_L1_HIT:
            counters.l1_hits++;
            counters.cycles += L1_CYCLES;
            break;
        case EVENT_L1_MISS:
            counters.l1_misses++;
            counters.cycles += L1_CYCLES;
            break;
        case EVENT_VICTIM_HIT:
            counters.victim_hits++;
            counters.cycles += VICTIM_CYCLES;
            break;
        case EVENT_VICTIM_MISS:
            counters.victim_misses++;
            counters.cycles += VICTIM_CYCLES;
            break;
        case EVENT_MEMORY_READ:
            counters.memory_reads++;
            counters.cycles += MEMORY_READ_CYCLES;
            break;
        case EVENT_MEMORY_WRITE:
            counters.memory_writes++;
            counters.cycles += MEMORY_WRITE_CYCLES;
            break;
        default:
            break;
    }
}

Stats stats_get(void)
{
    /* Supplied accessor. The student code above maintains counters. */
    return counters;
}
