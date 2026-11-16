#include <assert.h>
#include <string.h>
#include "memory.h"
#include "stats.h"
static uint8_t bytes[MEMORY_SIZE];
static uint64_t read_count, write_count;
void memory_init(void)
{
    for (uint32_t a = 0; a < MEMORY_SIZE; ++a) bytes[a] = (uint8_t)a;
    read_count = write_count = 0;
}
void memory_read_block(uint32_t base, uint8_t data[BLOCK_SIZE])
{
    assert(base % BLOCK_SIZE == 0 && base <= MEMORY_SIZE - BLOCK_SIZE);
    memcpy(data, bytes + base, BLOCK_SIZE);
    ++read_count;
    stats_record(EVENT_MEMORY_READ);
}
void memory_write_block(uint32_t base, const uint8_t data[BLOCK_SIZE])
{
    assert(base % BLOCK_SIZE == 0 && base <= MEMORY_SIZE - BLOCK_SIZE);
    memcpy(bytes + base, data, BLOCK_SIZE);
    ++write_count;
    stats_record(EVENT_MEMORY_WRITE);
}
uint8_t memory_peek(uint32_t address)
{
    assert(address < MEMORY_SIZE);
    return bytes[address];
}
uint64_t memory_read_count(void) { return read_count; }
uint64_t memory_write_count(void) { return write_count; }
