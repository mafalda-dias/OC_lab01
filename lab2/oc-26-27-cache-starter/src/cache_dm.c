/* STUDENT FILE: Part 1 - Direct-mapped cache.
 * Implement only this mode here; cache.c supplies initialization/dispatch.
 * Keep helpers static and in this file. Do not call another student module.
 * Record CPU and lookup events through stats_record(). Memory transfers
 * record their own events automatically; never record those twice.
 */
#include "cache_impl.h"
#include "memory.h"
#include "stats.h"

static L1Line *dm_access(Cache *cache, uint32_t address) 
{
    uint32_t block_number = address / BLOCK_SIZE;
    uint32_t index = block_number % L1_LINES;
    uint32_t tag = block_number / L1_LINES;
    L1Line *line = &cache->l1[index];

    if (line->valid && line->tag == tag) {
        stats_record(EVENT_L1_HIT);
        return line;
    }
    stats_record(EVENT_L1_MISS);

    if (line->valid && line->dirty) {
        uint32_t old_base = (line->tag * L1_LINES + index) * BLOCK_SIZE;
        memory_write_block(old_base, line->data);
    }
    
    memory_read_block(block_number * BLOCK_SIZE, line->data);

    line->valid = true;
    line->tag = tag;
    line->dirty = false;

    return line;
    
}

uint8_t dm_read(Cache *cache, uint32_t address)
{
    stats_record(EVENT_CPU_READ);
    L1Line *line = dm_access(cache, address);
    return line->data[address % BLOCK_SIZE];

}

void dm_write(Cache *cache, uint32_t address, uint8_t value)
{
    stats_record(EVENT_CPU_WRITE);
    L1Line *line = dm_access(cache, address);
    line->data[address % BLOCK_SIZE] = value;
    line->dirty = true;

}

void dm_flush(Cache *cache)
{
    for (uint32_t i = 0; i < L1_LINES; i++) {
        L1Line *line = &cache->l1[i];
        if (line->valid && line->dirty) {
            uint32_t base = (line->tag * L1_LINES + i) * BLOCK_SIZE;
            memory_write_block(base, line->data);
            line->dirty = false;
        }
    }

}

