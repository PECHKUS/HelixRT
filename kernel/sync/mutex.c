// HelixRT - Mutex Implementation


#include <stdint.h>
#include <stddef.h>
#include "../../include/config.h"
#include "mutex.h"
#include "../kernel.h"
#include "../scheduler.h"
#include "critical.h"

int mutex_init(mutex_t *mtx, uint8_t flags)
{
    if (mtx == NULL) {
        return KERNEL_ERR_PARAM;
    }

    mtx->locked = 0;
    mtx->owner = NULL;
    mtx->owner_base_priority = 0;
    mtx->recursive_count = 0;
    mtx->flags = flags;
    mtx->wait_list_head = NULL;
    mtx->wait_list_tail = NULL;
    return KERNEL_OK;
}

int mutex_trylock(mutex_t *mtx)
{
    task_tcb_t *self;
    uint32_t irq_state;

    if (mtx == NULL) {
        return KERNEL_ERR_PARAM;
    }
    if (is_isr_context()) {
        return KERNEL_ERR_ISR;
    }

    self = task_get_current();
    if (self == NULL) {
        return KERNEL_ERR_STATE;
    }

    irq_state = critical_enter();

    if (!mtx->locked) {
        mtx->locked = 1;
        mtx->owner = self;
        mtx->owner_base_priority = self->priority;
        mtx->recursive_count = 1;
        critical_exit(irq_state);
        return KERNEL_OK;
    }

    if (mtx->owner == self && (mtx->flags & MUTEX_FLAG_RECURSIVE)) {
        if (mtx->recursive_count < 0xFFU) {
            mtx->recursive_count++;
        }
        critical_exit(irq_state);
        return KERNEL_OK;
    }

    critical_exit(irq_state);
    return KERNEL_ERR_TIMEOUT;
}

int mutex_lock(mutex_t *mtx, uint32_t timeout)
{
    int res;
    task_tcb_t *self;

    if (mtx == NULL) {
        return KERNEL_ERR_PARAM;
    }
    if (is_isr_context()) {
        return KERNEL_ERR_ISR;
    }

    self = task_get_current();
    if (self == NULL) {
        return KERNEL_ERR_STATE;
    }

    while (1) {
        res = mutex_trylock(mtx);
        if (res == KERNEL_OK) {
            return KERNEL_OK;
        }

        if (timeout == TIMEOUT_NONE) {
            return KERNEL_ERR_TIMEOUT;
        }

#if CONFIG_PRIORITY_INHERITANCE
        if (mtx->owner != NULL && mtx->owner->priority > self->priority) {
            scheduler_set_priority(mtx->owner, self->priority);
        }
#endif

        res = scheduler_block_task(BLOCK_MUTEX, mtx, timeout);
        if (res != KERNEL_OK) {
            return res;
        }
    }
}

int mutex_unlock(mutex_t *mtx)
{
    task_tcb_t *self;
    uint32_t irq_state;

    if (mtx == NULL) {
        return KERNEL_ERR_PARAM;
    }
    if (is_isr_context()) {
        return KERNEL_ERR_ISR;
    }

    self = task_get_current();
    if (self == NULL || mtx->owner != self) {
        return KERNEL_ERR_STATE;
    }

    irq_state = critical_enter();

    if (mtx->recursive_count > 1U) {
        mtx->recursive_count--;
        critical_exit(irq_state);
        return KERNEL_OK;
    }

#if CONFIG_PRIORITY_INHERITANCE
    if (self->priority != self->base_priority) {
        scheduler_set_priority(self, self->base_priority);
    }
#endif

    mtx->locked = 0;
    mtx->owner = NULL;
    mtx->recursive_count = 0;

    (void)scheduler_unblock_one(BLOCK_MUTEX, mtx, KERNEL_OK);

    critical_exit(irq_state);
    return KERNEL_OK;
}

task_tcb_t *mutex_get_owner(mutex_t *mtx)
{
    if (mtx == NULL) {
        return NULL;
    }
    return mtx->owner;
}
