// HelixRT - Semaphore API

// Counting and binary semaphores for task synchronization.
 

#ifndef SEMAPHORE_H
#define SEMAPHORE_H

#include <stdint.h>
#include "../task.h"

// Semaphore Structure

typedef struct semaphore {
    volatile int32_t count;        
    int32_t max_count;              
    task_tcb_t *wait_list_head;   
    task_tcb_t *wait_list_tail;     
} semaphore_t;

// Semaphore API

/*
 * sem_init - Initialize a semaphore
 * 
 * @sem:       Pointer to semaphore structure
 * @initial:   Initial count value
 * @max_count: Maximum count (0 = unbounded)
 * 
 * Returns: KERNEL_OK or error code
 */
 
int sem_init(semaphore_t *sem, int32_t initial, int32_t max_count);

/*
 * sem_take - Take (decrement) semaphore
 * 
 * If count > 0, decrements and returns immediately.
 * If count <= 0, blocks until semaphore available or timeout.
 * 
 * @sem:     Semaphore to take
 * @timeout: Timeout in ticks (0 = no wait, UINT32_MAX = infinite)
 * 
 * Returns: KERNEL_OK, KERNEL_ERR_TIMEOUT, or error code
 */
 
int sem_take(semaphore_t *sem, uint32_t timeout);

/*
 * sem_give - Give (increment) semaphore
 * 
 * Increments count. If tasks are waiting, wakes highest priority waiter.
 * 
 * @sem: Semaphore to give
 * 
 * Returns: KERNEL_OK or error code
 */
 
int sem_give(semaphore_t *sem);

/*
 * sem_give_isr - Give semaphore from ISR context
 * 
 * Same as sem_give but safe to call from interrupt handlers.
 * 
 * @sem: Semaphore to give
 * 
 * Returns: KERNEL_OK or error code
 */
 
int sem_give_isr(semaphore_t *sem);

/*
 * sem_get_count - Get current semaphore count
 * 
 * @sem: Semaphore to query
 * 
 * Returns: Current count value
 */
 
int32_t sem_get_count(semaphore_t *sem);

/*
 * sem_reset - Reset semaphore to initial state
 * 
 * Wakes all waiting tasks with error result.
 * 
 * @sem:       Semaphore to reset
 * @new_count: New count value
 * 
 * Returns: KERNEL_OK or error code
 */
 
int sem_reset(semaphore_t *sem, int32_t new_count);

// Binary Semaphore Helpers
 
// Initialize as binary semaphore (max count 
static inline int sem_init_binary(semaphore_t *sem, int32_t initial)
{
    return sem_init(sem, initial ? 1 : 0, 1);
}

// Static Semaphore Allocation
 
#define SEMAPHORE_STATIC_DEFINE(name, initial, max)     \
    static semaphore_t name = {                         \
        .count = (initial),                             \
        .max_count = (max),                             \
        .wait_list_head = NULL,                         \
        .wait_list_tail = NULL                          \
    }

#define SEMAPHORE_BINARY_DEFINE(name, initial)          \
    SEMAPHORE_STATIC_DEFINE(name, (initial) ? 1 : 0, 1)

#endif // SEMAPHORE_H 
