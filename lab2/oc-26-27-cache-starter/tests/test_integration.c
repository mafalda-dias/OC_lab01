#include "cache.h"
#include "memory.h"
#include "stats.h"
#include "check.h"
static void run(CacheMode mode)
{
    Cache c = {0}; memory_init(); stats_reset(); cache_init(&c, mode);
    cache_write(&c, 0, 0xAA);
    (void)cache_read(&c, L1_SIZE);
    CHECK_EQ("integration read value", cache_read(&c, 0), 0xAA);
    cache_flush(&c);
    Stats s = stats_get();
    CHECK_EQ("integration reads", s.reads, 2); CHECK_EQ("integration writes", s.writes, 1);
    CHECK_EQ("integration L1 hits", s.l1_hits, mode == MODE_SA2 ? 1 : 0);
    CHECK_EQ("integration L1 misses", s.l1_misses, mode == MODE_SA2 ? 2 : 3);
    CHECK_EQ("integration victim hits", s.victim_hits, mode == MODE_VICTIM ? 1 : 0);
    CHECK_EQ("integration victim misses", s.victim_misses, mode == MODE_VICTIM ? 2 : 0);
    CHECK_EQ("integration memory reads", s.memory_reads, mode == MODE_DM ? 3 : 2);
    CHECK_EQ("integration memory writes", s.memory_writes, 1);
    CHECK_EQ("integration cycles", s.cycles, mode == MODE_DM ? 203 : mode == MODE_SA2 ? 153 : 159);
    CHECK_EQ("memory events match real calls", s.memory_reads, memory_read_count());
    CHECK_EQ("write events match real calls", s.memory_writes, memory_write_count());
    stats_reset();
    CHECK_EQ("reset preserves cache data", cache_read(&c, 0), 0xAA);
    cache_flush(&c);
    s = stats_get();
    CHECK_EQ("reset retains L1 block", s.l1_hits, 1);
    CHECK_EQ("no transfer after clean flush", s.memory_reads + s.memory_writes, 0);
    CHECK_EQ("reset interval cycles", s.cycles, 1);
    /* A reset with a dirty block must preserve the pending write-back. */
    cache_write(&c, 0, 0xBB); stats_reset(); cache_flush(&c);
    CHECK_EQ("reset preserves dirty block", memory_peek(0), 0xBB);
    CHECK_EQ("dirty flush after reset", stats_get().memory_writes, 1);
    CHECK_EQ("dirty flush cycles only", stats_get().cycles, 50);
}
int main(void)
{
    run(MODE_DM); run(MODE_SA2); run(MODE_VICTIM);
    return finish("Integration tests");
}
