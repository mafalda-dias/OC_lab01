#ifndef MEMORY_H
#define MEMORY_H
#include <stdint.h>
#include "config.h"
/* Supplied. Only these block functions transfer simulated data.
 * base must be aligned to BLOCK_SIZE and within memory.
 * Each call records exactly one MEMORY_READ or MEMORY_WRITE event.
 */
void memory_init(void);
void memory_read_block(uint32_t base, uint8_t data[BLOCK_SIZE]);
void memory_write_block(uint32_t base, const uint8_t data[BLOCK_SIZE]);
/* Supplied test-only observers; no simulated events or elapsed time.
 * Audit counts measure actual function calls independently of stats.c.
 * They are reset by memory_init, NOT by stats_reset.
 * Do not use these observers in your cache implementation.
 */
uint8_t memory_peek(uint32_t address);
uint64_t memory_read_count(void);
uint64_t memory_write_count(void);
#endif
