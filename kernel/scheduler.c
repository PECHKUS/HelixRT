/*
 * HelixRT - Scheduler Implementation
 *
 * Priority scheduler with O(1) bitmap lookup and round-robin among
 * equal-priority tasks.
 */

#include <stdint.h>
#include <stddef.h>
#include "../include/config.h"
#include "scheduler.h"
#include "kernel.h"
#include "timer.h"
#include "sync/critical.h"

task_tcb_t *current_task = NULL;
task_tcb_t *next_task = NULL;

static scheduler_t g_sched;
static volatile uint32_t g_tick_count = 0;
static task_tcb_t *g_blocked_head = NULL;

static void ready_insert_tail(task_tcb_t *tcb)
{
    task_tcb_t *head;
    uint8_t prio = tcb->priority;

    tcb->next = NULL;
    tcb->prev = NULL;

    head = g_sched.ready_list[prio];
    if (head == NULL) {
        g_sched.ready_list[prio] = tcb;
        bitmap_set(&g_sched.priority_bitmap, prio);
        return;
    }

    while (head->next != NULL) {
        head = head->next;
    }
    head->next = tcb;
    tcb->prev = head;
}

static void blocked_insert(task_tcb_t *tcb)
{
    tcb->next = g_blocked_head;
    tcb->prev = NULL;
    if (g_blocked_head != NULL) {
        g_blocked_head->prev = tcb;
    }
    g_blocked_head = tcb;
}

static void blocked_remove(task_tcb_t *tcb)
{
    if (tcb->prev != NULL) {
        tcb->prev->next = tcb->next;
    } else if (g_blocked_head == tcb) {
        g_blocked_head = tcb->next;
    }
    if (tcb->next != NULL) {
        tcb->next->prev = tcb->prev;
    }
    tcb->next = NULL;
    tcb->prev = NULL;
}

void scheduler_init(void)
{
    uint32_t i;
    g_sched.priority_bitmap = 0;
    g_sched.current = NULL;
    g_sched.lock_count = 0;
    g_sched.reschedule_pending = false;
    for (i = 0; i < CONFIG_MAX_PRIORITY; i++) {
        g_sched.ready_list[i] = NULL;
    }
    g_tick_count = 0;
    g_blocked_head = NULL;
    current_task = NULL;
    next_task = NULL;
}

void scheduler_start(void)
{
    task_tcb_t *first;

    first = scheduler_get_next();
    if (first == NULL) {
        while (1) { __asm volatile ("wfi"); }
    }
    

    /*
     * First task starts via normal C call path.
     * Later switches use PendSV save/restore and EXC_RETURN.
     */
     
    current_task = first;
    g_sched.current = first;
    first->state = TASK_STATE_RUNNING;
    first->time_slice = CONFIG_TIME_SLICE;
    first->sp = first->stack_top;

    __asm volatile (
        "msr psp, %0      \n"
        "mrs r1, control  \n"
        "orr r1, r1, #2   \n"
        "msr control, r1  \n"
        "isb              \n"
        :
        : "r" (first->sp)
        : "r1", "memory"
    );

    first->entry(first->arg);
    (void)task_delete(NULL);
    while (1) { __asm volatile ("wfi"); }
}

void scheduler_add_task(task_tcb_t *tcb)
{
    uint32_t irq_state = critical_enter();

    tcb->state = TASK_STATE_READY;
    tcb->time_slice = CONFIG_TIME_SLICE;
    ready_insert_tail(tcb);

    if (current_task != NULL && tcb->priority < current_task->priority) {
        scheduler_trigger_switch();
    }

    critical_exit(irq_state);
}

void scheduler_remove_task(task_tcb_t *tcb)
{
    task_tcb_t *head;
    uint8_t prio;
    uint32_t irq_state = critical_enter();

    if (tcb == NULL) {
        critical_exit(irq_state);
        return;
    }

    prio = tcb->priority;
    head = g_sched.ready_list[prio];

    if (head == tcb) {
        g_sched.ready_list[prio] = tcb->next;
    }
    if (tcb->prev != NULL) {
        tcb->prev->next = tcb->next;
    }
    if (tcb->next != NULL) {
        tcb->next->prev = tcb->prev;
    }

    if (g_sched.ready_list[prio] == NULL) {
        bitmap_clear(&g_sched.priority_bitmap, prio);
    }

    tcb->next = NULL;
    tcb->prev = NULL;

    critical_exit(irq_state);
}

void scheduler_set_priority(task_tcb_t *tcb, uint8_t new_priority)
{
    if (tcb == NULL || new_priority >= CONFIG_MAX_PRIORITY) {
        return;
    }

    if (tcb->state == TASK_STATE_READY || tcb->state == TASK_STATE_RUNNING) {
        scheduler_remove_task(tcb);
        tcb->priority = new_priority;
        scheduler_add_task(tcb);
    } else {
        tcb->priority = new_priority;
    }
}

void scheduler_yield(void)
{
    uint8_t prio;
    task_tcb_t *head;
    uint32_t irq_state = critical_enter();

    if (current_task == NULL) {
        critical_exit(irq_state);
        return;
    }

    if (g_sched.lock_count > 0U) {
        g_sched.reschedule_pending = true;
        critical_exit(irq_state);
        return;
    }

    prio = current_task->priority;
    head = g_sched.ready_list[prio];

    // Round-robin: move head to tail when peers exist at same priority
    if (head != NULL && head->next != NULL && head == current_task) {
        g_sched.ready_list[prio] = head->next;
        g_sched.ready_list[prio]->prev = NULL;

        head->next = NULL;
        head->prev = NULL;
        ready_insert_tail(head);
    }

    scheduler_trigger_switch();
    critical_exit(irq_state);
}

void scheduler_tick(void)
{
    task_tcb_t *iter;
    task_tcb_t *next_iter;
    uint8_t highest_prio;
    uint32_t irq_state = critical_enter();

    g_tick_count++;

    iter = g_blocked_head;
    while (iter != NULL) {
        bool wake = false;
        next_iter = iter->next;

        if (iter->block_reason == BLOCK_DELAY) {
            if (g_tick_count >= iter->wake_tick) {
                wake = true;
            }
        } else if (iter->block_timeout != 0U && iter->block_timeout != UINT32_MAX) {
            if (g_tick_count >= iter->wake_tick) {
                iter->block_result = KERNEL_ERR_TIMEOUT;
                wake = true;
            }
        }

        if (wake) {
            blocked_remove(iter);
            iter->block_reason = BLOCK_NONE;
            iter->block_object = NULL;
            iter->block_timeout = 0;
            iter->state = TASK_STATE_READY;
            ready_insert_tail(iter);
        }

        iter = next_iter;
    }

    if (current_task != NULL) {
        if (current_task->time_slice > 0U) {
            current_task->time_slice--;
        }
        if (CONFIG_ROUND_ROBIN && current_task->time_slice == 0U) {
            current_task->time_slice = CONFIG_TIME_SLICE;
            scheduler_yield();
        }
    }

    highest_prio = bitmap_find_highest(g_sched.priority_bitmap);
    if (current_task != NULL && highest_prio < current_task->priority) {
        scheduler_trigger_switch();
    }

    critical_exit(irq_state);
}

int scheduler_block_task(block_reason_t reason, void *object, uint32_t timeout)
{
    task_tcb_t *self;
    uint32_t irq_state = critical_enter();

    if (current_task == NULL) {
        critical_exit(irq_state);
        return KERNEL_ERR_STATE;
    }

    self = current_task;
    scheduler_remove_task(current_task);
    current_task->state = TASK_STATE_BLOCKED;
    current_task->block_reason = reason;
    current_task->block_object = object;
    current_task->block_timeout = timeout;
    current_task->block_result = KERNEL_OK;
    if (timeout == 0U) {
        current_task->wake_tick = g_tick_count + 1U;
    } else if (timeout == UINT32_MAX) {
        current_task->wake_tick = UINT32_MAX;
    } else {
        current_task->wake_tick = g_tick_count + timeout;
    }

    blocked_insert(current_task);
    scheduler_trigger_switch();
    critical_exit(irq_state);


    /*
     * Force a switch point; this function resumes only when 'self'
     * is scheduled again and can then return the unblock result.
     */
     
    scheduler_yield();
    return self->block_result;
}

void scheduler_unblock_task(task_tcb_t *tcb, int result)
{
    uint32_t irq_state = critical_enter();

    if (tcb == NULL || tcb->state != TASK_STATE_BLOCKED) {
        critical_exit(irq_state);
        return;
    }

    blocked_remove(tcb);
    tcb->state = TASK_STATE_READY;
    tcb->block_reason = BLOCK_NONE;
    tcb->block_result = result;
    tcb->block_object = NULL;
    tcb->block_timeout = 0;
    ready_insert_tail(tcb);

    if (current_task != NULL && tcb->priority < current_task->priority) {
        scheduler_trigger_switch();
    }

    critical_exit(irq_state);
}

bool scheduler_unblock_one(block_reason_t reason, void *object, int result)
{
    task_tcb_t *iter;
    task_tcb_t *best = NULL;
    uint32_t irq_state = critical_enter();

    iter = g_blocked_head;
    while (iter != NULL) {
        if (iter->block_reason == reason && iter->block_object == object) {
            if (best == NULL || iter->priority < best->priority) {
                best = iter;
            }
        }
        iter = iter->next;
    }

    if (best != NULL) {
        blocked_remove(best);
        best->state = TASK_STATE_READY;
        best->block_reason = BLOCK_NONE;
        best->block_result = result;
        best->block_object = NULL;
        best->block_timeout = 0;
        ready_insert_tail(best);
        if (current_task != NULL && best->priority < current_task->priority) {
            scheduler_trigger_switch();
        }
        critical_exit(irq_state);
        return true;
    }

    critical_exit(irq_state);
    return false;
}

uint32_t scheduler_unblock_all(block_reason_t reason, void *object, int result)
{
    task_tcb_t *iter;
    task_tcb_t *next_iter;
    uint32_t unblocked = 0;
    uint32_t irq_state = critical_enter();

    iter = g_blocked_head;
    while (iter != NULL) {
        next_iter = iter->next;
        if (iter->block_reason == reason && iter->block_object == object) {
            blocked_remove(iter);
            iter->state = TASK_STATE_READY;
            iter->block_reason = BLOCK_NONE;
            iter->block_result = result;
            iter->block_object = NULL;
            iter->block_timeout = 0;
            ready_insert_tail(iter);
            unblocked++;
        }
        iter = next_iter;
    }

    if (unblocked > 0U) {
        scheduler_trigger_switch();
    }

    critical_exit(irq_state);
    return unblocked;
}

task_tcb_t *scheduler_get_current(void)
{
    return current_task;
}

task_tcb_t *scheduler_get_next(void)
{
    uint8_t highest_prio = bitmap_find_highest(g_sched.priority_bitmap);
    if (highest_prio >= CONFIG_MAX_PRIORITY) {
        return NULL;
    }
    return g_sched.ready_list[highest_prio];
}

task_tcb_t *scheduler_select_next_task(void)
{
    next_task = scheduler_get_next();
    g_sched.current = next_task;
    return next_task;
}

uint32_t scheduler_get_tick_count(void)
{
    return g_tick_count;
}

void scheduler_lock(void)
{
    uint32_t irq_state = critical_enter();
    g_sched.lock_count++;
    critical_exit(irq_state);
}

void scheduler_unlock(void)
{
    uint32_t irq_state = critical_enter();

    if (g_sched.lock_count > 0U) {
        g_sched.lock_count--;
    }
    if (g_sched.lock_count == 0U && g_sched.reschedule_pending) {
        g_sched.reschedule_pending = false;
        scheduler_trigger_switch();
    }

    critical_exit(irq_state);
}

bool scheduler_is_locked(void)
{
    return g_sched.lock_count > 0U;
}

// SysTick is owned by the scheduler when kernel is running 
void SysTick_Handler(void)
{
    scheduler_tick();
#if CONFIG_SW_TIMERS
    timer_tick_isr();
#endif
    kernel_tick_hook();
}
