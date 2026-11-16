/* Functional tests deliberately do not link student stats.c. */
#include "stats.h"
void stats_record(Event event) { (void)event; }
void stats_reset(void) { }
Stats stats_get(void) { Stats zero = {0}; return zero; }
