// HelixRT - Event Group Implementation
 

#include <stdint.h>
#include <stddef.h>
#include "../../include/config.h"
#include "event.h"
#include "../kernel.h"
#include "../scheduler.h"
#include "../task.h"
#include "critical.h"

static int event_match(uint32_t current, uint32_t bits, uint8_t wait_all)
{
    if (wait_all) {
        return ((current & bits) == bits);
    }
    return ((current & bits) != 0U);
}

int event_init(event_group_t *eg)
{
    if (eg == NULL) {
        return KERNEL_ERR_PARAM;
    }
    eg->flags = 0;
    return KERNEL_OK;
}

int event_set(event_group_t *eg, uint32_t bits)
{
    uint32_t irq_state;

    if (eg == NULL) {
        return KERNEL_ERR_PARAM;
    }

    irq_state = critical_enter();
    eg->flags |= bits;
    
    /*
     * Wake all waiters; each task re-checks its own condition and either
     * consumes bits or blocks again.
     */
     
    (void)scheduler_unblock_all(BLOCK_EVENT, eg, KERNEL_OK);
    critical_exit(irq_state);
    return KERNEL_OK;
}

int event_clear(event_group_t *eg, uint32_t bits)
{
    if (eg == NULL) {
        return KERNEL_ERR_PARAM;
    }
    eg->flags &= ~bits;
    return KERNEL_OK;
}

uint32_t event_get(event_group_t *eg)
{
    if (eg == NULL) {
        return 0;
    }
    return eg->flags;
}

uint32_t event_wait(event_group_t *eg, uint32_t bits, uint8_t wait_all, uint32_t timeout)
{
    task_tcb_t *self;
    uint32_t matched;
    int res;
    uint32_t clear_on_exit;

    if (eg == NULL || bits == 0U) {
        return 0;
    }
    if (is_isr_context()) {
        return 0;
    }

    clear_on_exit = bits & EVENT_CLEAR_ON_EXIT;
    bits &= ~EVENT_CLEAR_ON_EXIT;

    self = task_get_current();
    if (self == NULL) {
        return 0;
    }

    self->event_wait_bits = bits;
    self->event_wait_all = wait_all ? 1U : 0U;

    while (1) {
        uint32_t irq_state = critical_enter();
        matched = eg->flags & bits;
        if (event_match(eg->flags, bits, wait_all)) {
            if (clear_on_exit != 0U) {
                eg->flags &= ~matched;
            }
            critical_exit(irq_state);
            return matched;
        }
        critical_exit(irq_state);

        if (timeout == TIMEOUT_NONE) {
            return 0;
        }

        res = scheduler_block_task(BLOCK_EVENT, eg, timeout);
        if (res != KERNEL_OK) {
            return 0;
        }
    }
}
