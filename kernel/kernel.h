/*
 * HelixRT - Kernel API
 * 
 * This header defines the complete kernel interface.
 * Stage 2+ implements these functions.
 */


#ifndef KERNEL_H
#define KERNEL_H

#include <stdint.h>
#include <stdbool.h>
#include "task.h"



// Kernel Return Codes
#define KERNEL_OK               0
#define KERNEL_ERR_PARAM        -1
#define KERNEL_ERR_NO_MEM       -2
#define KERNEL_ERR_TIMEOUT      -3
#define KERNEL_ERR_ISR          -4      // Called from ISR context 
#define KERNEL_ERR_STATE        -5      // Invalid state for operation
#define KERNEL_ERR_DELETED      -6      // Object was deleted 
#define KERNEL_ERR_OVERFLOW     -7      // Buffer/stack overflow 

// Kernel State


typedef enum {
    KERNEL_STATE_UNINIT = 0,
    KERNEL_STATE_INIT,
    KERNEL_STATE_RUNNING,
    KERNEL_STATE_STOPPED
} kernel_state_t;

// Kernel Configuration (from config.h or defaults)

#ifndef CONFIG_TICK_RATE_HZ
#define CONFIG_TICK_RATE_HZ     1000
#endif

#ifndef CONFIG_MAX_TASKS
#define CONFIG_MAX_TASKS        16
#endif

#ifndef CONFIG_MAX_PRIORITY
#define CONFIG_MAX_PRIORITY     32
#endif

#ifndef CONFIG_TIME_SLICE
#define CONFIG_TIME_SLICE       10
#endif

// Kernel Control API

/*
 * kernel_init - Initialize the kernel
 * 
 * Must be called before any other kernel function.
 * Sets up scheduler, idle task, and kernel data structures.
 * 
 * Returns: KERNEL_OK on success
 */
 
 
 
int kernel_init(void);

/*
 * kernel_start - Start the kernel scheduler
 * 
 * This function never returns. It starts the first ready task
 * and begins scheduling.
 * 
 * Prerequisites:
 *   - kernel_init() called
 *   - At least one task created
 */
 
 
void kernel_start(void) __attribute__((noreturn));

/*
 * kernel_get_state - Get current kernel state
 */
 
kernel_state_t kernel_get_state(void);

/*
 * kernel_get_tick - Get current system tick count
 */
 
uint32_t kernel_get_tick(void);

/*
 * kernel_get_tick_rate - Get tick rate in Hz
 */
 
static inline uint32_t kernel_get_tick_rate(void)
{
    return CONFIG_TICK_RATE_HZ;
}

// Task Management API

/*
 * task_create - Create a new task
 * 
 * @tcb:        Pointer to TCB (can be NULL for dynamic allocation)
 * @name:       Task name (for debugging)
 * @entry:      Task entry function
 * @arg:        Argument passed to entry function
 * @priority:   Task priority (0 = highest)
 * @stack:      Pointer to stack memory (can be NULL for dynamic)
 * @stack_size: Stack size in bytes
 * 
 * Returns: KERNEL_OK or error code
 */
 
 
int task_create(task_tcb_t *tcb,
                const char *name,
                void (*entry)(void *),
                void *arg,
                uint8_t priority,
                uint32_t *stack,
                uint32_t stack_size);

/*
 * task_delete - Delete a task
 * 
 * @tcb: Task to delete (NULL = current task)
 */
 
int task_delete(task_tcb_t *tcb);

/*
 * task_suspend - Suspend a task
 * 
 * @tcb: Task to suspend (NULL = current task)
 */
 
int task_suspend(task_tcb_t *tcb);

/*
 * task_resume - Resume a suspended task
 * 
 * @tcb: Task to resume
 */
 
int task_resume(task_tcb_t *tcb);

/*
 * task_yield - Yield CPU to another ready task of same priority
 */
 
void task_yield(void);

/*
 * task_delay - Delay current task for specified ticks
 * 
 * @ticks: Number of ticks to delay
 */
 
void task_delay(uint32_t ticks);

/*
 * task_delay_ms - Delay current task for specified milliseconds
 * 
 * @ms: Milliseconds to delay
 */
 
static inline void task_delay_ms(uint32_t ms)
{
    task_delay((ms * CONFIG_TICK_RATE_HZ) / 1000);
}

/*
 * task_get_current - Get pointer to current task TCB
 */
 
task_tcb_t *task_get_current(void);

/*
 * task_set_priority - Change task priority
 * 
 * @tcb:      Task to modify (NULL = current task)
 * @priority: New priority
 */
 
int task_set_priority(task_tcb_t *tcb, uint8_t priority);

//Scheduler Control API
 
/*
 * scheduler_lock - Disable task preemption
 * 
 * Prevents context switches. Can be nested.
 * Interrupts remain enabled.
 */
 
void scheduler_lock(void);

/*
 * scheduler_unlock - Re-enable task preemption
 * 
 * Decrements lock count. Context switch occurs when count reaches 0.
 */
 
void scheduler_unlock(void);

/*
 * scheduler_is_locked - Check if scheduler is locked
 */
bool scheduler_is_locked(void);

//Kernel Hooks (Weak - Override in Application)

/*
 * kernel_idle_hook - Called in idle task loop
 * 
 * Default implementation enters WFI (Wait For Interrupt)
 */
 
void kernel_idle_hook(void) __attribute__((weak));

/*
 * kernel_tick_hook - Called every system tick
 * 
 * Default is empty. Use for periodic application tasks.
 */
 
void kernel_tick_hook(void) __attribute__((weak));

/*
 * kernel_stack_overflow_hook - Called on stack overflow detection
 * 
 * @tcb: Task that overflowed
 */
 
void kernel_stack_overflow_hook(task_tcb_t *tcb) __attribute__((weak));

#endif // KERNEL_H 
