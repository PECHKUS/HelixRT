/*
 * HelixRT - Critical Section Implementation
 * 
 * Most critical section functions are inline in critical.h.
 * This file contains any non-inline implementations.
 */

#include "critical.h"

/* 
 * All critical section functions are currently inline.
 * This file is a placeholder for any future non-inline implementations
 * such as debug wrappers or statistics collection.
 */

#ifdef CONFIG_CRITICAL_STATS

// Critical section statistics (optional) 
static volatile uint32_t critical_enter_count = 0;
static volatile uint32_t critical_max_depth = 0;
static volatile uint32_t critical_current_depth = 0;

uint32_t critical_enter_debug(void)
{
    uint32_t state = critical_enter();
    critical_enter_count++;
    critical_current_depth++;
    if (critical_current_depth > critical_max_depth) {
        critical_max_depth = critical_current_depth;
    }
    return state;
}

void critical_exit_debug(uint32_t state)
{
    critical_current_depth--;
    critical_exit(state);
}

void critical_get_stats(uint32_t *enter_count, uint32_t *max_depth)
{
    *enter_count = critical_enter_count;
    *max_depth = critical_max_depth;
}

#endif // CONFIG_CRITICAL_STATS
