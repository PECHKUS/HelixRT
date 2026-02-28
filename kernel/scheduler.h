/*
 * HelixRT - Scheduler Interface
 * 
 * Priority-based preemptive scheduler with round-robin
 * for tasks of equal priority.
 */
 

#ifndef SCHEDULER_H
#define SCHEDULER_H

#include <stdint.h>
#include <stdbool.h>
#include "task.h"

// Scheduler Configuration

#ifndef CONFIG_MAX_PRIORITY
#define CONFIG_MAX_PRIORITY     32
#endif

#ifndef CONFIG_TIME_SLICE
#define CONFIG_TIME_SLICE       10
#endif

#ifndef CONFIG_ROUND_ROBIN
#define CONFIG_ROUND_ROBIN      1
#endif

/* 
 * Ready Queue Structure
 * 
 * Uses a bitmap for O(1) highest-priority lookup and
 * doubly-linked lists for each priority level.
 */


typedef struct {
    /* Bitmap: bit N set = priority N has ready tasks */
    uint32_t priority_bitmap;
    
    /* Head of ready list for each priority */
    task_tcb_t *ready_list[CONFIG_MAX_PRIORITY];
    
    /* Currently running task */
    task_tcb_t *current;
    
    /* Lock nesting count (>0 = scheduler locked) */
    uint32_t lock_count;
    
    /* Pending reschedule flag (set when locked) */
    bool reschedule_pending;
    
} scheduler_t;

// Scheduler API (Internal - used by kernel)

/*
 * scheduler_init - Initialize the scheduler
 */
 
void scheduler_init(void);

/*
 * scheduler_start - Start the scheduler (runs first task)
 * 
 * This function never returns.
 */
 
void scheduler_start(void) __attribute__((noreturn));

/*
 * scheduler_add_task - Add task to ready queue
 * 
 * @tcb: Task to add
 */
 
void scheduler_add_task(task_tcb_t *tcb);

/*
 * scheduler_remove_task - Remove task from ready queue
 * 
 * @tcb: Task to remove
 */
 
void scheduler_remove_task(task_tcb_t *tcb);

/*
 * scheduler_set_priority - Update task priority in scheduler
 * 
 * @tcb:          Task to update
 * @new_priority: New priority level
 */
 
void scheduler_set_priority(task_tcb_t *tcb, uint8_t new_priority);

/*
 * scheduler_yield - Voluntarily give up CPU
 * 
 * Moves current task to end of its priority queue and reschedules.
 */
 
void scheduler_yield(void);

/*
 * scheduler_tick - Called on every system tick
 * 
 * Handles:
 *   - Decrementing delay counters
 *   - Waking delayed tasks
 *   - Time slice management
 *   - Triggering context switch if needed
 */
 
void scheduler_tick(void);

/*
 * scheduler_block_task - Block current task
 * 
 * @reason:  Why task is blocking
 * @object:  Object blocking on (can be NULL)
 * @timeout: Timeout in ticks (0 = infinite)
 * 
 * Returns: Block result (KERNEL_OK, KERNEL_ERR_TIMEOUT, etc.)
 */
int scheduler_block_task(block_reason_t reason, void *object, uint32_t timeout);

/*
 * scheduler_unblock_task - Unblock a blocked task
 * 
 * @tcb:    Task to unblock
 * @result: Result to return from blocking call
 */
 
void scheduler_unblock_task(task_tcb_t *tcb, int result);

/*
 * scheduler_unblock_one - Wake highest-priority task blocked on object/reason
 *
 * @reason: Block reason to match
 * @object: Object pointer used in scheduler_block_task()
 * @result: Result code for resumed task
 *
 * Returns: true if a task was unblocked
 */
 
bool scheduler_unblock_one(block_reason_t reason, void *object, int result);

/*
 * scheduler_unblock_all - Wake all tasks blocked on object/reason
 *
 * Returns: Number of tasks unblocked
 */
 
uint32_t scheduler_unblock_all(block_reason_t reason, void *object, int result);

/*
 * scheduler_get_current - Get currently running task
 */
 
task_tcb_t *scheduler_get_current(void);

/*
 * scheduler_get_next - Get highest priority ready task
 * 
 * Does not modify any state, just returns the task that
 * would run next.
 */
 
task_tcb_t *scheduler_get_next(void);

/*
 * scheduler_select_next_task - Called by PendSV/context code
 *
 * Returns the task that should run after the switch point.
 */
 
task_tcb_t *scheduler_select_next_task(void);

/*
 * scheduler_get_tick_count - Global scheduler tick
 */
 
uint32_t scheduler_get_tick_count(void);

/*
 * scheduler_lock/unlock/is_locked - Preemption control
 */
 
void scheduler_lock(void);
void scheduler_unlock(void);
bool scheduler_is_locked(void);

// Priority Bitmap Helpers

/*
 * Find highest priority (lowest bit set) in bitmap
 * Uses CLZ (Count Leading Zeros) instruction for O(1) lookup
 */
 
static inline uint8_t bitmap_find_highest(uint32_t bitmap)
{
    if (bitmap == 0) return CONFIG_MAX_PRIORITY;
    return (uint8_t)__builtin_ctz(bitmap);  // Lowest set bit is highest priority 
}

/*
 * Set bit in bitmap
 */
 
static inline void bitmap_set(uint32_t *bitmap, uint8_t bit)
{
    *bitmap |= (1UL << bit);
}

/*
 * Clear bit in bitmap
 */
 
static inline void bitmap_clear(uint32_t *bitmap, uint8_t bit)
{
    *bitmap &= ~(1UL << bit);
}

/*
 * Test bit in bitmap
 */
 
static inline bool bitmap_test(uint32_t bitmap, uint8_t bit)
{
    return (bitmap & (1UL << bit)) != 0;
}



//Context Switch Interface (Implemented in Assembly)

/*
 * Trigger PendSV exception for context switch
 */
 
static inline void scheduler_trigger_switch(void)
{
    // Set PENDSVSET bit in ICSR 
    *((volatile uint32_t *)0xE000ED04) |= (1UL << 28);
}

/*
 * Start first task (called once from scheduler_start)
 * Implemented in context.s
 */
 
extern void task_start_first(void);

//Global Scheduler Variables (defined in scheduler.c)
 
//These are accessed by assembly code 
extern task_tcb_t *current_task;    // Currently running task 
extern task_tcb_t *next_task;       // Task to switch to 

#endif // SCHEDULER_H 
