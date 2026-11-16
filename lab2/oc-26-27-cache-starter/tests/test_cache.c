#include <stdlib.h>
#include <string.h>
#include "cache.h"
#include "memory.h"
#include "check.h"
/* Fixtures deliberately bypass cache_init, so its bugs do not mask other tests.
 * These helpers construct test states; they do not implement cache accesses.
 */
static Cache fixture(CacheMode mode)
{
    Cache c = {0}; c.mode = mode; memory_init(); return c;
}
static void seed_l1(Cache *c, unsigned slot, uint32_t block, bool dirty, uint64_t age)
{
    L1Line *l = &c->l1[slot];
    l->valid = true; l->dirty = dirty;
    l->tag = block / (c->mode == MODE_SA2 ? L1_LINES / 2 : L1_LINES);
    l->last_used = age;
    for (unsigned i = 0; i < BLOCK_SIZE; ++i) l->data[i] = (uint8_t)(block * BLOCK_SIZE + i);
    if (age > c->clock) c->clock = age;
}
static void seed_victim(Cache *c, unsigned slot, uint32_t block, bool dirty, uint64_t age)
{
    VictimEntry *v = &c->victim[slot];
    v->valid = true; v->dirty = dirty; v->block_number = block; v->last_used = age;
    for (unsigned i = 0; i < BLOCK_SIZE; ++i) v->data[i] = (uint8_t)(block * BLOCK_SIZE + i);
    if (age > c->clock) c->clock = age;
}
static void part1_reads(void)
{
    Cache c = fixture(MODE_DM); c.clock = 123;
    for (unsigned i = 0; i < L1_LINES; ++i) seed_l1(&c, i, i, true, 7);
    for (unsigned i = 0; i < VICTIM_BLOCKS; ++i) seed_victim(&c, i, L1_LINES + i, true, 8);
    cache_init(&c, MODE_SA2);
    CHECK_EQ("supplied initialization mode", c.mode, MODE_SA2);
    CHECK_EQ("supplied initialization clock", c.clock, 0);
    bool cleared = true;
    for (unsigned i = 0; i < L1_LINES; ++i) {
        const L1Line *l = &c.l1[i];
        if (l->valid || l->dirty || l->tag || l->last_used) cleared = false;
        for (unsigned j = 0; j < BLOCK_SIZE; ++j) if (l->data[j]) cleared = false;
    }
    for (unsigned i = 0; i < VICTIM_BLOCKS; ++i) {
        const VictimEntry *v = &c.victim[i];
        if (v->valid || v->dirty || v->block_number || v->last_used) cleared = false;
        for (unsigned j = 0; j < BLOCK_SIZE; ++j) if (v->data[j]) cleared = false;
    }
    CHECK_EQ("supplied initialization metadata and arrays", cleared, 1);
    c = fixture(MODE_DM);
    CHECK_EQ("1 block offset", cache_read(&c, BLOCK_SIZE + 3), BLOCK_SIZE + 3);
    CHECK_EQ("1 line index", c.l1[1].valid, 1);
    CHECK_EQ("1 repeat", cache_read(&c, BLOCK_SIZE + 3), BLOCK_SIZE + 3);
    CHECK_EQ("1 same block other offset", cache_read(&c, BLOCK_SIZE + 5), BLOCK_SIZE + 5);
    CHECK_EQ("1 one fetch for three reads", memory_read_count(), 1);
    c = fixture(MODE_DM); seed_l1(&c, 0, 0, false, 1);
    CHECK_EQ("1 conflict data", cache_read(&c, L1_SIZE + 7), (uint8_t)(L1_SIZE + 7));
    CHECK_EQ("1 replacement tag", c.l1[0].tag, 1);
    CHECK_EQ("1 conflict fetch", memory_read_count(), 1);
    CHECK_EQ("1 clean eviction", memory_write_count(), 0);
    c = fixture(MODE_DM);
    CHECK_EQ("1 last address", cache_read(&c, MEMORY_SIZE - 1), 255);
    CHECK_EQ("1 independent line", cache_read(&c, 2), 2);
    CHECK_EQ("1 two fetches", memory_read_count(), 2);
}
static void part1_writes(void)
{
    Cache c = fixture(MODE_DM); seed_l1(&c, 0, 0, false, 1);
    cache_write(&c, 3, 0xA7);
    CHECK_EQ("1 hit byte", c.l1[0].data[3], 0xA7);
    CHECK_EQ("1 hit dirty", c.l1[0].dirty, 1);
    CHECK_EQ("1 no fetch on write hit", memory_read_count(), 0);
    CHECK_EQ("1 write-back delayed", memory_peek(3), 3);
    c = fixture(MODE_DM); cache_write(&c, BLOCK_SIZE + 3, 0xB9);
    CHECK_EQ("1 write allocation fetch", memory_read_count(), 1);
    CHECK_EQ("1 allocated line valid", c.l1[1].valid, 1);
    CHECK_EQ("1 modified byte", c.l1[1].data[3], 0xB9);
    CHECK_EQ("1 neighboring byte preserved", c.l1[1].data[4], BLOCK_SIZE + 4);
    c = fixture(MODE_DM); seed_l1(&c, 0, 0, true, 1); c.l1[0].data[0] = 0xAA;
    (void)cache_read(&c, L1_SIZE);
    CHECK_EQ("1 dirty eviction write", memory_write_count(), 1);
    CHECK_EQ("1 old address write-back", memory_peek(0), 0xAA);
    CHECK_EQ("1 new block clean", c.l1[0].dirty, 0);
    c = fixture(MODE_DM); seed_l1(&c, 0, 0, true, 1); c.l1[0].data[2] = 0xBB;
    seed_l1(&c, 1, 1, false, 2); cache_flush(&c);
    CHECK_EQ("1 flush dirty only", memory_write_count(), 1);
    CHECK_EQ("1 flush data", memory_peek(2), 0xBB);
    CHECK_EQ("1 flush validity", c.l1[0].valid, 1);
    CHECK_EQ("1 flush clears dirty", c.l1[0].dirty, 0);
    CHECK_EQ("1 flush preserves recency", c.l1[0].last_used, 1);
    cache_flush(&c); CHECK_EQ("1 repeated flush", memory_write_count(), 1);
}
static void part2(void)
{
    const uint32_t stride = L1_SIZE / 2;
    Cache c = fixture(MODE_SA2);
    (void)cache_read(&c, 0); (void)cache_read(&c, stride);
    CHECK_EQ("2 lowest invalid way first", c.l1[0].valid && c.l1[0].tag == 0, 1);
    CHECK_EQ("2 second way", c.l1[1].valid && c.l1[1].tag == 1, 1);
    CHECK_EQ("2 two fetches", memory_read_count(), 2);
    c = fixture(MODE_SA2); seed_l1(&c, 0, 0, false, 1);
    seed_l1(&c, 1, L1_LINES / 2, true, 2); c.l1[1].data[3] = 0xCC;
    CHECK_EQ("2 way one hit", cache_read(&c, stride + 3), 0xCC);
    CHECK_EQ("2 way one no fetch", memory_read_count(), 0);
    c = fixture(MODE_SA2); seed_l1(&c, 0, 0, false, 1);
    seed_l1(&c, 1, L1_LINES / 2, false, 2);
    (void)cache_read(&c, 0); (void)cache_read(&c, 2 * stride);
    CHECK_EQ("2 hit protects way zero", c.l1[0].tag, 0);
    CHECK_EQ("2 LRU replaces way one", c.l1[1].tag, 2);
    CHECK_EQ("2 only miss fetches", memory_read_count(), 1);
    c = fixture(MODE_SA2); seed_l1(&c, 0, 0, true, 1); c.l1[0].data[0] = 0xAA;
    seed_l1(&c, 1, L1_LINES / 2, true, 2); c.l1[1].data[0] = 0xBB;
    (void)cache_read(&c, 2 * stride);
    CHECK_EQ("2 dirty LRU write", memory_write_count(), 1);
    CHECK_EQ("2 reconstructed address", memory_peek(0), 0xAA);
    cache_flush(&c);
    CHECK_EQ("2 flush remaining dirty way", memory_write_count(), 2);
    CHECK_EQ("2 second block address", memory_peek(stride), 0xBB);
    CHECK_EQ("2 flush keeps both ways", c.l1[0].valid && c.l1[1].valid, 1);
}
static void part3(void)
{
    Cache c = fixture(MODE_VICTIM); seed_l1(&c, 0, 0, false, 1);
    (void)cache_read(&c, L1_SIZE);
    CHECK_EQ("3 invalid victim slot used", c.victim[0].valid, 1);
    CHECK_EQ("3 moved full block number", c.victim[0].block_number, 0);
    CHECK_EQ("3 requested block in L1", c.l1[0].tag, 1);
    CHECK_EQ("3 no write for clean move", memory_write_count(), 0);
    c = fixture(MODE_VICTIM); seed_l1(&c, 0, 0, true, 1); c.l1[0].data[0] = 0xAA;
    seed_victim(&c, 0, L1_LINES, true, 2); c.victim[0].data[0] = 0xBB;
    CHECK_EQ("3 victim hit data", cache_read(&c, L1_SIZE), 0xBB);
    CHECK_EQ("3 swap L1 tag", c.l1[0].tag, 1);
    CHECK_EQ("3 swap victim full block", c.victim[0].block_number, 0);
    CHECK_EQ("3 swap old L1 data", c.victim[0].data[0], 0xAA);
    CHECK_EQ("3 swap preserves dirty bits", c.victim[0].dirty && c.l1[0].dirty, 1);
    CHECK_EQ("3 swap no memory read", memory_read_count(), 0);
    CHECK_EQ("3 swap no memory write", memory_write_count(), 0);
    c = fixture(MODE_VICTIM); seed_l1(&c, 0, 0, false, 1);
    for (unsigned i = 0; i < VICTIM_BLOCKS; ++i)
        seed_victim(&c, i, (i + 1) * L1_LINES, i == 0, i + 2);
    c.victim[0].data[0] = 0xDD;
    (void)cache_read(&c, (VICTIM_BLOCKS + 1) * L1_SIZE);
    CHECK_EQ("3 LRU victim replaced", c.victim[0].block_number, 0);
    CHECK_EQ("3 dirty victim write", memory_write_count(), 1);
    CHECK_EQ("3 victim original address", memory_peek(L1_SIZE), 0xDD);
    CHECK_EQ("3 new entry most recent", c.victim[0].last_used > VICTIM_BLOCKS + 1, 1);
    c = fixture(MODE_VICTIM); seed_l1(&c, 0, 0, true, 1); c.l1[0].data[0] = 0xAA;
    seed_victim(&c, 0, L1_LINES, true, 2); c.victim[0].data[0] = 0xBB;
    cache_flush(&c);
    CHECK_EQ("3 flush both structures", memory_write_count(), 2);
    CHECK_EQ("3 L1 memory value", memory_peek(0), 0xAA);
    CHECK_EQ("3 victim memory value", memory_peek(L1_SIZE), 0xBB);
    CHECK_EQ("3 flush preserves validity", c.l1[0].valid && c.victim[0].valid, 1);
    CHECK_EQ("3 flush clears dirty bits", !c.l1[0].dirty && !c.victim[0].dirty, 1);
    CHECK_EQ("3 flush preserves victim age", c.victim[0].last_used, 2);
    cache_flush(&c); CHECK_EQ("3 repeated flush", memory_write_count(), 2);
}
int main(int argc, char **argv)
{
    if (argc != 2) return 2;
    switch (atoi(argv[1])) {
        case 1: part1_reads(); part1_writes(); break;
        case 2: part2(); break; case 3: part3(); break;
        default: return 2;
    }
    return finish("Functional tests");
}
