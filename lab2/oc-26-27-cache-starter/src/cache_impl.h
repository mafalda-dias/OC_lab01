/* Supplied module interfaces; do not modify. */
#ifndef CACHE_IMPL_H
#define CACHE_IMPL_H
#include "cache.h"
/* Each module operates only on the mode selected by the supplied dispatcher.
 * Initialize through cache_init before use. Keep helpers static in each file.
 */
uint8_t dm_read(Cache *cache, uint32_t address);
void dm_write(Cache *cache, uint32_t address, uint8_t value);
void dm_flush(Cache *cache);
uint8_t sa2_read(Cache *cache, uint32_t address);
void sa2_write(Cache *cache, uint32_t address, uint8_t value);
void sa2_flush(Cache *cache);
uint8_t victim_read(Cache *cache, uint32_t address);
void victim_write(Cache *cache, uint32_t address, uint8_t value);
void victim_flush(Cache *cache);
#endif
