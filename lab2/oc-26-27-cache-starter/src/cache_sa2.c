/* STUDENT FILE: Part 2 - Two-way set-associative cache.
 * Implement only this mode here; cache.c supplies initialization/dispatch.
 * Keep helpers static and in this file. Do not call another student module.
 * Record CPU and lookup events through stats_record(). Memory transfers
 * record their own events automatically; never record those twice.
 */
#include "cache_impl.h"
#include "memory.h"
#include "stats.h"

#define NUM_SETS (L1_LINES / 2)

static L1Line *sa2_access(Cache *cache, uint32_t address)
{
    uint32_t block_number = address / BLOCK_SIZE;
    uint32_t set = block_number % NUM_SETS;
    uint32_t tag = block_number / NUM_SETS;
    L1Line *ways = &cache->l1[2 * set];   // ways[0] e ways[1]

    for (int w = 0; w < 2; w++) {
        if (ways[w].valid && ways[w].tag == tag) {
            stats_record(EVENT_L1_HIT);
            ways[w].last_used = ++cache->clock;
            return &ways[w];   
        }
    }
    stats_record(EVENT_L1_MISS);

    L1Line *line;
    if (!ways[0].valid)
        line = &ways[0];
    else if (!ways[1].valid)
        line = &ways[1];
    else
        line = (ways[0].last_used <= ways[1].last_used) ? &ways[0] : &ways[1];

    if (line->valid && line->dirty) {
        uint32_t old_base = (line->tag * NUM_SETS + set) * BLOCK_SIZE;
        memory_write_block(old_base, line->data);
    }

    memory_read_block(block_number * BLOCK_SIZE, line->data);
    line->valid = true;
    line->dirty = false;
    line->tag = tag;
    line->last_used = ++cache->clock;

    return line;

}

uint8_t sa2_read(Cache *cache, uint32_t address)
{
    stats_record(EVENT_CPU_READ);
    L1Line *line = sa2_access(cache, address);
    return line->data[address % BLOCK_SIZE];

}

void sa2_write(Cache *cache, uint32_t address, uint8_t value)
{
    stats_record(EVENT_CPU_WRITE);
    L1Line *line = sa2_access(cache, address);
    line->data[address % BLOCK_SIZE] = value;
    line->dirty = true;
    
}

void sa2_flush(Cache *cache)
{
    for (uint32_t i = 0; i < L1_LINES; i++) {
        L1Line *line = &cache->l1[i];
        if (line->valid && line->dirty) {
            uint32_t set = i / 2;
            uint32_t base = (line->tag * NUM_SETS + set) * BLOCK_SIZE;
            memory_write_block(base, line->data);
            line->dirty = false; 
        }
    }
}
