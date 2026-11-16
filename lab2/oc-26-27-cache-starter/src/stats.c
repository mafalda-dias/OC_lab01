/* STUDENT FILE: Part 4. Do not print to stdout. */
#include "config.h"
#include "stats.h"

static Stats counters;

void stats_reset(void)
{
    /* TODO: reset every performance counter and simulated time. */
}

void stats_record(Event event)
{
    /* TODO: account for exactly one event using the costs in config.h. */

}

Stats stats_get(void)
{
    /* Supplied accessor. The student code above maintains counters. */
    return counters;
}
