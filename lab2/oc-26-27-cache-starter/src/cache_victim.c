/* STUDENT FILE: Part 3 - Direct-mapped L1 plus victim cache.
 * Implement only this mode here; cache.c supplies initialization/dispatch.
 * Keep helpers static and in this file. Do not call another student module.
 * Record CPU and lookup events through stats_record(). Memory transfers
 * record their own events automatically; never record those twice.
 */
#include "cache_impl.h"
#include "memory.h"
#include "stats.h"

static VictimEntry *victim_choose_slot(Cache *cache)
{
    /* Empty slot first, otherwise the LRU one. */
    for (uint32_t i = 0; i < VICTIM_BLOCKS; i++) {
        if (!cache->victim[i].valid) {
            return &cache->victim[i];
        }
    }

    /* Smallest last_used = used longest ago */
    VictimEntry *lru = &cache->victim[0];
    for (uint32_t i = 0; i < VICTIM_BLOCKS; i++) {
        if (cache->victim[i].last_used < lru->last_used) 
            lru = &cache->victim[i];
    }
    return lru;
}

static L1Line *victim_access(Cache *cache, uint32_t address) 
{
    uint32_t block_number = address / BLOCK_SIZE;
    uint32_t index = block_number % L1_LINES;
    uint32_t tag = block_number / L1_LINES;
    L1Line *line = &cache->l1[index];

    /* L1 hit: no need to check the victim buffer */
    if (line->valid && line->tag == tag) {
        stats_record(EVENT_L1_HIT);
        return line;
    }
    
    stats_record(EVENT_L1_MISS);

    /* L1 miss: look for the block in the victim buffer */
    VictimEntry *hit = 0;
    for (uint32_t i = 0; i < VICTIM_BLOCKS; i++) {
        if (cache->victim[i].valid && cache->victim[i].block_number == block_number) {
            hit = &cache->victim[i];
        }
    }

    if (hit) {
        stats_record(EVENT_VICTIM_HIT);
        if (line->valid) {
            /* Swap the L1 block and the victim block, no memory access */
            for (uint32_t i = 0; i < BLOCK_SIZE; i++) {
                uint8_t tmp = line->data[i];
                line->data[i] = hit->data[i];
                hit->data[i] = tmp;
            }

            bool d = line->dirty;
            line->dirty = hit->dirty;
            hit->dirty = d;

            /* Old L1 block now lives in this victim slot */
            hit->block_number = line->tag * L1_LINES + index;
            /* Entering the victim buffer makes it most recent */
            hit->last_used = ++cache->clock;
        
        } else {
            /* L1 line was empty: just move it up (exclusive). */
            for (uint32_t i = 0; i < BLOCK_SIZE; i++)
                line->data[i] = hit->data[i];
            line->dirty = hit->dirty;
            hit->valid = false;
        }
        line->valid = true;
        line->tag = tag;
        return line;
    }

    stats_record(EVENT_VICTIM_MISS);
    /* Victim miss: old L1 block goes to the victim buffer */
    if (line->valid) {
        VictimEntry *slot = victim_choose_slot(cache);
        /* Evicted victim block is dirty: write it back first */
        if (slot->valid && slot->dirty)
            memory_write_block(slot->block_number * BLOCK_SIZE, slot->data);

        for (uint32_t i = 0; i < BLOCK_SIZE; i++)
            slot->data[i] = line->data[i];
        slot->block_number = line->tag * L1_LINES + index;
        slot->dirty = line->dirty;
        slot->valid = true;
        slot->last_used = ++cache->clock;
    }

    /* Fetch the requested block from memory into L1 */
    memory_read_block(block_number * BLOCK_SIZE, line->data);
    line->valid = true;
    line->tag = tag;
    line->dirty = false;
    return line;
}

uint8_t victim_read(Cache *cache, uint32_t address)
{
    stats_record(EVENT_CPU_READ);
    L1Line *line = victim_access(cache, address);
    return line->data[address % BLOCK_SIZE];
    
}

void victim_write(Cache *cache, uint32_t address, uint8_t value)
{
    stats_record(EVENT_CPU_WRITE);
    L1Line *line = victim_access(cache, address);
    line->data[address % BLOCK_SIZE] = value;
    line->dirty = true;

}

void victim_flush(Cache *cache)
{
    /* Flush L1 first */
    for (uint32_t i = 0; i < L1_LINES; i++) {  
        L1Line *line = &cache->l1[i];
        if (line->valid && line->dirty) {
            uint32_t base = (line->tag * L1_LINES + i) * BLOCK_SIZE;
            memory_write_block(base, line->data);
            line->dirty = false;
        }
    }

    /* Then the victim buffer */
    for (uint32_t i = 0; i < VICTIM_BLOCKS; i++) {
        VictimEntry *v = &cache->victim[i];
        if (v->valid && v->dirty) {
            memory_write_block(v->block_number * BLOCK_SIZE, v->data);
            v->dirty = false;
        }
    }
}
