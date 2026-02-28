// HelixRT - Semaphore Implementation
 

#include <stdint.h>
#include <stddef.h>
#include "../../include/config.h"
#include "semaphore.h"
#include "../kernel.h"
#include "../scheduler.h"
#include "critical.h"

int sem_init(semaphore_t *sem, int32_t initial, int32_t max_count)
{
    if (sem == NULL || initial < 0 || (max_count > 0 && initial > max_count)) {
        return KERNEL_ERR_PARAM;
    }

    sem->count = initial;
    sem->max_count = max_count;
    sem->wait_list_head = NULL;
    sem->wait_list_tail = NULL;
    return KERNEL_OK;
}

int sem_take(semaphore_t *sem, uint32_t timeout)
{
    int res;
    uint32_t irq_state;

    if (sem == NULL) {
        return KERNEL_ERR_PARAM;
    }

    while (1) {
        irq_state = critical_enter();
        if (sem->count > 0) {
            sem->count--;
            critical_exit(irq_state);
            return KERNEL_OK;
        }
        critical_exit(irq_state);

        if (timeout == TIMEOUT_NONE) {
            return KERNEL_ERR_TIMEOUT;
        }
        if (is_isr_context()) {
            return KERNEL_ERR_ISR;
        }

        res = scheduler_block_task(BLOCK_SEMAPHORE, sem, timeout);
        if (res != KERNEL_OK) {
            return res;
        }
    }
}

int sem_give(semaphore_t *sem)
{
    uint32_t irq_state;

    if (sem == NULL) {
        return KERNEL_ERR_PARAM;
    }

    irq_state = critical_enter();

    if (!scheduler_unblock_one(BLOCK_SEMAPHORE, sem, KERNEL_OK)) {
        if (sem->max_count > 0 && sem->count >= sem->max_count) {
            critical_exit(irq_state);
            return KERNEL_ERR_OVERFLOW;
        }
        sem->count++;
    }

    critical_exit(irq_state);
    return KERNEL_OK;
}

int sem_give_isr(semaphore_t *sem)
{
    if (sem == NULL) {
        return KERNEL_ERR_PARAM;
    }

    if (!scheduler_unblock_one(BLOCK_SEMAPHORE, sem, KERNEL_OK)) {
        if (sem->max_count > 0 && sem->count >= sem->max_count) {
            return KERNEL_ERR_OVERFLOW;
        }
        sem->count++;
    }
    return KERNEL_OK;
}

int32_t sem_get_count(semaphore_t *sem)
{
    if (sem == NULL) {
        return 0;
    }
    return sem->count;
}

int sem_reset(semaphore_t *sem, int32_t new_count)
{
    if (sem == NULL || new_count < 0 || (sem->max_count > 0 && new_count > sem->max_count)) {
        return KERNEL_ERR_PARAM;
    }

    sem->count = new_count;
    (void)scheduler_unblock_all(BLOCK_SEMAPHORE, sem, KERNEL_ERR_STATE);
    return KERNEL_OK;
}
