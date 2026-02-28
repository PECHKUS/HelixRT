// HelixRT - Mutex API
// Mutual exclusion with priority inheritance to prevent priority inversion.
 

#ifndef MUTEX_H
#define MUTEX_H

#include <stdint.h>
#include "../task.h"

//Mutex Structure


typedef struct mutex {
    volatile uint8_t locked;          
    task_tcb_t *owner;                
    uint8_t owner_base_priority;      
    uint8_t recursive_count;           
    uint8_t flags;                    
    task_tcb_t *wait_list_head;        
    task_tcb_t *wait_list_tail;        
} mutex_t;

// Mutex flags 
#define MUTEX_FLAG_RECURSIVE    (1 << 0)    //Allow recursive locking 

// Mutex API

/*
 * mutex_init - Initialize a mutex
 * 
 * @mtx:   Pointer to mutex structure
 * @flags: Mutex flags (e.g., MUTEX_FLAG_RECURSIVE)
 * 
 * Returns: KERNEL_OK or error code
 */
 
 
int mutex_init(mutex_t *mtx, uint8_t flags);

/*
 * mutex_lock - Lock the mutex
 * 
 * If unlocked, locks and returns immediately.
 * If locked by another task, blocks and may raise owner's priority.
 * If locked by same task and recursive, increments count.
 * 
 * @mtx:     Mutex to lock
 * @timeout: Timeout in ticks (0 = no wait, UINT32_MAX = infinite)
 * 
 * Returns: KERNEL_OK, KERNEL_ERR_TIMEOUT, or error code
 */
 
 
int mutex_lock(mutex_t *mtx, uint32_t timeout);

/*
 * mutex_trylock - Try to lock mutex without blocking
 * 
 * @mtx: Mutex to lock
 * 
 * Returns: KERNEL_OK if locked, KERNEL_ERR_TIMEOUT if already locked
 */
 
 
int mutex_trylock(mutex_t *mtx);

/*
 * mutex_unlock - Unlock the mutex
 * 
 * Must be called by the owner. Restores owner's priority if it was
 * raised due to priority inheritance.
 * 
 * @mtx: Mutex to unlock
 * 
 * Returns: KERNEL_OK or error code
 */
 
int mutex_unlock(mutex_t *mtx);

/*
 * mutex_get_owner - Get mutex owner
 * 
 * @mtx: Mutex to query
 * 
 * Returns: Pointer to owner TCB, or NULL if unlocked
 */
 
task_tcb_t *mutex_get_owner(mutex_t *mtx);

/*
 * mutex_is_locked - Check if mutex is locked
 * 
 * @mtx: Mutex to query
 * 
 * Returns: 1 if locked, 0 if unlocked
 */
 
static inline int mutex_is_locked(mutex_t *mtx)
{
    return mtx->locked;
}

/* 
 * Priority Inheritance Implementation Notes
 * 
 * When a high-priority task blocks on a mutex held by a lower-priority task:
 * 
 * 1. mutex_lock() detects the priority inversion
 * 2. Owner's priority is temporarily raised to blocker's priority
 * 3. Owner is moved to higher priority queue in scheduler
 * 4. When mutex_unlock() is called:
 *    a. Owner's priority is restored to base_priority
 *    b. Highest priority waiter is woken
 *    c. Scheduler is invoked if woken task has higher priority
 * 
 * This prevents unbounded priority inversion.
 */

//Static Mutex Allocation

#define MUTEX_STATIC_DEFINE(name)                       \
    static mutex_t name = {                             \
        .locked = 0,                                    \
        .owner = NULL,                                  \
        .owner_base_priority = 0,                       \
        .recursive_count = 0,                           \
        .flags = 0,                                     \
        .wait_list_head = NULL,                         \
        .wait_list_tail = NULL                          \
    }

#define MUTEX_RECURSIVE_DEFINE(name)                    \
    static mutex_t name = {                             \
        .locked = 0,                                    \
        .owner = NULL,                                  \
        .owner_base_priority = 0,                       \
        .recursive_count = 0,                           \
        .flags = MUTEX_FLAG_RECURSIVE,                  \
        .wait_list_head = NULL,                         \
        .wait_list_tail = NULL                          \
    }

#endif //MUTEX_H
