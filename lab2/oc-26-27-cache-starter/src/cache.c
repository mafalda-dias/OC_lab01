/* SUPPLIED: common initialization and mode dispatch. Do not modify. */
#include <assert.h>
#include <string.h>
#include "cache_impl.h"

void cache_init(Cache *cache, CacheMode mode)
{
    assert(mode == MODE_DM || mode == MODE_SA2 || mode == MODE_VICTIM);
    memset(cache, 0, sizeof *cache);
    cache->mode = mode;
}

uint8_t cache_read(Cache *cache, uint32_t address)
{
    switch (cache->mode) {
        case MODE_DM: return dm_read(cache, address);
        case MODE_SA2: return sa2_read(cache, address);
        case MODE_VICTIM: return victim_read(cache, address);
    }
    assert(0 && "invalid cache mode");
    return 0;
}

void cache_write(Cache *cache, uint32_t address, uint8_t value)
{
    switch (cache->mode) {
        case MODE_DM: dm_write(cache, address, value); return;
        case MODE_SA2: sa2_write(cache, address, value); return;
        case MODE_VICTIM: victim_write(cache, address, value); return;
    }
    assert(0 && "invalid cache mode");
}

void cache_flush(Cache *cache)
{
    switch (cache->mode) {
        case MODE_DM: dm_flush(cache); return;
        case MODE_SA2: sa2_flush(cache); return;
        case MODE_VICTIM: victim_flush(cache); return;
    }
    assert(0 && "invalid cache mode");
}
