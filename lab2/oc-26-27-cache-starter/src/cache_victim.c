/* STUDENT FILE: Part 3 - Direct-mapped L1 plus victim cache.
 * Implement only this mode here; cache.c supplies initialization/dispatch.
 * Keep helpers static and in this file. Do not call another student module.
 * Record CPU and lookup events through stats_record(). Memory transfers
 * record their own events automatically; never record those twice.
 */
#include "cache_impl.h"
#include "memory.h"
#include "stats.h"

uint8_t victim_read(Cache *cache, uint32_t address)
{
    /* TODO: read one byte using this mode's mapping/replacement rules.
     * Emit one CPU_READ event and the appropriate lookup events.
     */

    return 0;
}

void victim_write(Cache *cache, uint32_t address, uint8_t value)
{
    /* TODO: write-back/write-allocate for this mode.
     * Emit one CPU_WRITE event and the appropriate lookup events.
     */

}

void victim_flush(Cache *cache)
{
    /* TODO: flush this mode without invalidation or recency changes.
     * Emit no CPU/lookup events. Memory writes record their own events.
     */

}
