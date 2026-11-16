#include "stats.h"
#include "check.h"
static void check_zero(Stats s)
{
    CHECK_EQ("reset reads", s.reads, 0); CHECK_EQ("reset writes", s.writes, 0);
    CHECK_EQ("reset L1 hits", s.l1_hits, 0); CHECK_EQ("reset L1 misses", s.l1_misses, 0);
    CHECK_EQ("reset victim hits", s.victim_hits, 0); CHECK_EQ("reset victim misses", s.victim_misses, 0);
    CHECK_EQ("reset memory reads", s.memory_reads, 0); CHECK_EQ("reset memory writes", s.memory_writes, 0);
    CHECK_EQ("reset time", s.cycles, 0);
}
int main(void)
{
    stats_reset(); check_zero(stats_get());
    const uint64_t cost[] = {0, 0, 1, 1, 2, 2, 50, 50};
    for (int e = 0; e < EVENT_COUNT; ++e) {
        stats_reset(); stats_record((Event)e); Stats s = stats_get();
        uint64_t count[] = {s.reads, s.writes, s.l1_hits, s.l1_misses,
                            s.victim_hits, s.victim_misses, s.memory_reads, s.memory_writes};
        for (int j = 0; j < EVENT_COUNT; ++j)
            CHECK_EQ("one event changes exactly one counter", count[j], j == e);
        CHECK_EQ("individual event cost", s.cycles, cost[e]);
    }
    stats_reset();
    const Event events[] = {EVENT_CPU_WRITE, EVENT_L1_MISS, EVENT_VICTIM_MISS,
        EVENT_MEMORY_READ, EVENT_CPU_READ, EVENT_L1_MISS, EVENT_VICTIM_MISS,
        EVENT_MEMORY_READ, EVENT_CPU_READ, EVENT_L1_MISS, EVENT_VICTIM_HIT,
        EVENT_MEMORY_WRITE};
    for (unsigned i = 0; i < sizeof events / sizeof events[0]; ++i) stats_record(events[i]);
    Stats s = stats_get();
    CHECK_EQ("sequence reads", s.reads, 2); CHECK_EQ("sequence writes", s.writes, 1);
    CHECK_EQ("sequence L1 misses", s.l1_misses, 3);
    CHECK_EQ("sequence victim hits", s.victim_hits, 1); CHECK_EQ("sequence victim misses", s.victim_misses, 2);
    CHECK_EQ("sequence memory reads", s.memory_reads, 2); CHECK_EQ("sequence memory writes", s.memory_writes, 1);
    CHECK_EQ("sequence cycles", s.cycles, 159);
    stats_reset(); check_zero(stats_get());
    stats_record(EVENT_CPU_READ); stats_record(EVENT_L1_HIT);
    CHECK_EQ("new interval cycles", stats_get().cycles, 1);
    CHECK_EQ("new interval reads", stats_get().reads, 1);
    return finish("Accounting tests");
}
