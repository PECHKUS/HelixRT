/*
 * HelixRT - Kernel Core
 *
 * Stage 2 foundation:
 * - Kernel lifecycle (init/start/state/tick)
 * - Static task/stack allocation
 * - Task management API glue over scheduler
 * - SVC dispatcher for privileged services
 */

#include <stdint.h>
#include <stddef.h>
#include "../include/config.h"
#include "kernel.h"
#include "scheduler.h"
#include "sync/critical.h"
#include "syscall.h"
#include "../hal/imxrt1062.h"

// Exposed for HAL/clock users   
volatile uint32_t SystemCoreClock = CONFIG_CPU_CLOCK_HZ;

// Kernel lifecycle state
static volatile kernel_state_t g_kernel_state = KERNEL_STATE_UNINIT;

// Deterministic static pools (no heap dependency)
static task_tcb_t g_task_pool[CONFIG_MAX_TASKS]
    __attribute__((section(".tcb_pool")));
static uint8_t g_task_slot_used[CONFIG_MAX_TASKS];
static uint32_t g_stack_pool[CONFIG_MAX_TASKS][CONFIG_DEFAULT_STACK_SIZE / sizeof(uint32_t)]
    __attribute__((section(".task_stacks"), aligned(8)));
static uint8_t g_stack_slot_used[CONFIG_MAX_TASKS];
static uint32_t g_next_task_id = 1;

// Idle task is always present to keep scheduler runnable
static task_tcb_t g_idle_tcb;
static uint32_t g_idle_stack[CONFIG_IDLE_STACK_SIZE / sizeof(uint32_t)]
    __attribute__((section(".task_stacks"), aligned(8)));

static void task_exit_trampoline(void);
static void idle_task(void *arg);

static int alloc_slot(uint8_t *bitmap, uint32_t count)
{
    uint32_t i;
    for (i = 0; i < count; i++) {
        if (bitmap[i] == 0) {
            bitmap[i] = 1;
            return (int)i;
        }
    }
    return -1;
}

static void free_slot(uint8_t *bitmap, int idx)
{
    if (idx >= 0) {
        bitmap[idx] = 0;
    }
}

static void copy_task_name(char *dst, const char *src)
{
    uint32_t i = 0;
    if (src == NULL) {
        src = "task";
    }

    while (i < (TASK_NAME_MAX - 1U) && src[i] != '\0') {
        dst[i] = src[i];
        i++;
    }
    dst[i] = '\0';
}

int kernel_init(void)
{
    if (g_kernel_state != KERNEL_STATE_UNINIT) {
        return KERNEL_ERR_STATE;
    }

    scheduler_init();

    // PendSV lowest, SysTick just above it for deterministic preemption
    SCB_SHPR3 = (SCB_SHPR3 & 0x0000FFFFUL) | (0xFFUL << 16) | (0xFEUL << 24);

    if (task_create(&g_idle_tcb,
                    "idle",
                    idle_task,
                    NULL,
                    (uint8_t)(CONFIG_MAX_PRIORITY - 1U),
                    g_idle_stack,
                    sizeof(g_idle_stack)) != KERNEL_OK) {
        return KERNEL_ERR_STATE;
    }

    g_kernel_state = KERNEL_STATE_INIT;
    return KERNEL_OK;
}

void kernel_start(void)
{
    uint32_t reload;

    if (g_kernel_state != KERNEL_STATE_INIT) {
        while (1) { __WFI(); }
    }

    reload = (SystemCoreClock / CONFIG_TICK_RATE_HZ) - 1U;
    SYSTICK_RVR = reload;
    SYSTICK_CVR = 0;
    SYSTICK_CSR = SYSTICK_CSR_ENABLE | SYSTICK_CSR_TICKINT | SYSTICK_CSR_CLKSOURCE;

    g_kernel_state = KERNEL_STATE_RUNNING;
    scheduler_start();
}

kernel_state_t kernel_get_state(void)
{
    return g_kernel_state;
}

uint32_t kernel_get_tick(void)
{
    return scheduler_get_tick_count();
}

int task_create(task_tcb_t *tcb,
                const char *name,
                void (*entry)(void *),
                void *arg,
                uint8_t priority,
                uint32_t *stack,
                uint32_t stack_size)
{
    int tcb_slot = -1;
    int stack_slot = -1;
    uint32_t *stack_top;

    if (entry == NULL || priority >= CONFIG_MAX_PRIORITY) {
        return KERNEL_ERR_PARAM;
    }

    if (stack_size < CONFIG_MIN_STACK_SIZE || (stack_size & 0x7U) != 0U) {
        return KERNEL_ERR_PARAM;
    }

    if (tcb == NULL) {
        tcb_slot = alloc_slot(g_task_slot_used, CONFIG_MAX_TASKS);
        if (tcb_slot < 0) {
            return KERNEL_ERR_NO_MEM;
        }
        tcb = &g_task_pool[tcb_slot];
    }

    if (stack == NULL) {
        if (stack_size > CONFIG_DEFAULT_STACK_SIZE) {
            if (tcb_slot >= 0) {
                free_slot(g_task_slot_used, tcb_slot);
            }
            return KERNEL_ERR_NO_MEM;
        }

        stack_slot = alloc_slot(g_stack_slot_used, CONFIG_MAX_TASKS);
        if (stack_slot < 0) {
            if (tcb_slot >= 0) {
                free_slot(g_task_slot_used, tcb_slot);
            }
            return KERNEL_ERR_NO_MEM;
        }
        stack = g_stack_pool[stack_slot];
        stack_size = CONFIG_DEFAULT_STACK_SIZE;
    }

    // Fill stack with a known pattern for post-mortem usage checks
    {
        uint32_t words = stack_size / sizeof(uint32_t);
        uint32_t i;
        for (i = 0; i < words; i++) {
            stack[i] = TASK_STACK_FILL;
        }
    }

    stack_top = (uint32_t *)(((uintptr_t)(stack + (stack_size / sizeof(uint32_t)))) & ~((uintptr_t)0x7U));

    tcb->id = g_next_task_id++;
    tcb->entry = entry;
    tcb->arg = arg;
    copy_task_name(tcb->name, name);
    tcb->priority = priority;
    tcb->base_priority = priority;
    tcb->state = TASK_STATE_READY;
    tcb->flags = 0;
    tcb->next = NULL;
    tcb->prev = NULL;
    tcb->stack_base = stack;
    tcb->stack_top = stack_top;
    tcb->stack_size = stack_size;
    tcb->delay_ticks = 0;
    tcb->time_slice = CONFIG_TIME_SLICE;
    tcb->wake_tick = 0;
    tcb->block_reason = BLOCK_NONE;
    tcb->block_object = NULL;
    tcb->block_timeout = 0;
    tcb->block_result = KERNEL_OK;
    tcb->event_wait_bits = 0;
    tcb->event_wait_all = 0;
#ifdef CONFIG_TASK_STATS
    tcb->run_count = 0;
    tcb->total_ticks = 0;
    tcb->max_stack_used = 0;
#endif

    tcb->sp = task_init_stack(stack_top, entry, arg, task_exit_trampoline);
    scheduler_add_task(tcb);

    return KERNEL_OK;
}

int task_delete(task_tcb_t *tcb)
{
    uint32_t irq_state;

    if (tcb == NULL) {
        tcb = task_get_current();
    }
    if (tcb == NULL) {
        return KERNEL_ERR_PARAM;
    }

    irq_state = critical_enter();
    scheduler_remove_task(tcb);
    tcb->state = TASK_STATE_DELETED;
    critical_exit(irq_state);

    if (tcb == task_get_current()) {
        scheduler_yield();
        while (1) { __WFI(); }
    }
    return KERNEL_OK;
}

int task_suspend(task_tcb_t *tcb)
{
    uint32_t irq_state;

    if (tcb == NULL) {
        tcb = task_get_current();
    }
    if (tcb == NULL) {
        return KERNEL_ERR_PARAM;
    }

    irq_state = critical_enter();
    scheduler_remove_task(tcb);
    tcb->state = TASK_STATE_SUSPENDED;
    critical_exit(irq_state);

    if (tcb == task_get_current()) {
        scheduler_yield();
    }
    return KERNEL_OK;
}

int task_resume(task_tcb_t *tcb)
{
    if (tcb == NULL || tcb->state != TASK_STATE_SUSPENDED) {
        return KERNEL_ERR_PARAM;
    }

    tcb->state = TASK_STATE_READY;
    scheduler_add_task(tcb);
    return KERNEL_OK;
}

void task_yield(void)
{
    scheduler_yield();
}

void task_delay(uint32_t ticks)
{
    if (ticks == 0U) {
        scheduler_yield();
        return;
    }

    (void)scheduler_block_task(BLOCK_DELAY, NULL, ticks);
}

task_tcb_t *task_get_current(void)
{
    return scheduler_get_current();
}

int task_set_priority(task_tcb_t *tcb, uint8_t priority)
{
    if (priority >= CONFIG_MAX_PRIORITY) {
        return KERNEL_ERR_PARAM;
    }
    if (tcb == NULL) {
        tcb = task_get_current();
    }
    if (tcb == NULL) {
        return KERNEL_ERR_PARAM;
    }

    scheduler_set_priority(tcb, priority);
    return KERNEL_OK;
}

// Weak defaults let applications add behavior without touching kernel internals
void kernel_idle_hook(void)
{
    __WFI();
}

void kernel_tick_hook(void)
{
}

void kernel_stack_overflow_hook(task_tcb_t *tcb)
{
    (void)tcb;
    while (1) { __WFI(); }
}

void kernel_assert_failed(const char *file, int line)
{
    (void)file;
    (void)line;
    while (1) { __WFI(); }
}



/*
 * SVC dispatcher called by SVC_Handler assembly.
 * Arguments map to stacked r0-r2 and return value goes back to stacked r0.
 */
 
 
int svc_dispatch(uint32_t svc_num, uint32_t arg0, uint32_t arg1, uint32_t arg2)
{
    switch (svc_num) {
    case SVC_TASK_YIELD:
        task_yield();
        return KERNEL_OK;
    case SVC_TASK_DELAY:
        task_delay(arg0);
        return KERNEL_OK;
    case SVC_TASK_DELETE:
        return task_delete((task_tcb_t *)arg0);
    case SVC_TASK_SUSPEND:
        return task_suspend((task_tcb_t *)arg0);
    case SVC_TASK_RESUME:
        return task_resume((task_tcb_t *)arg0);
    case SVC_TASK_PRIORITY:
        return task_set_priority((task_tcb_t *)arg0, (uint8_t)arg1);
    case SVC_TASK_CREATE:
        return task_create((task_tcb_t *)arg0, NULL, (void (*)(void *))arg1, (void *)arg2,
                           (uint8_t)(CONFIG_MAX_PRIORITY - 2U), NULL, CONFIG_DEFAULT_STACK_SIZE);
    default:
        return KERNEL_ERR_PARAM;
    }
}

uint32_t *task_init_stack(uint32_t *stack_top,
                          void (*entry)(void *),
                          void *arg,
                          void (*exit_func)(void))
{
    uint32_t *sp = stack_top;

    // Hardware exception frame (restored on EXC_RETURN)
    *(--sp) = 0x01000000UL;           // xPSR (Thumb bit set) 
    *(--sp) = (uint32_t)entry;        //PC 
    *(--sp) = (uint32_t)exit_func;    // LR 
    *(--sp) = 0;                      // R12 
    *(--sp) = 0;                      // R3 
    *(--sp) = 0;                      // R2 
    *(--sp) = 0;                      // R1 
    *(--sp) = (uint32_t)arg;          // R0 

    // Software frame saved/restored by PendSV
    *(--sp) = EXC_RETURN_THREAD_PSP;  // LR/EXC_RETURN for bx lr in PendSV 
    *(--sp) = 0;                      // R11 
    *(--sp) = 0;                      // R10 
    *(--sp) = 0;                      // R9
    *(--sp) = 0;                      // R8 
    *(--sp) = 0;                      // R7 
    *(--sp) = 0;                      // R6 
    *(--sp) = 0;                      // R5 
    *(--sp) = 0;                      // R4 

    return sp;
}

static void task_exit_trampoline(void)
{
    (void)task_delete(NULL);
    while (1) { __WFI(); }
}

static void idle_task(void *arg)
{
    (void)arg;
    while (1) {
        kernel_idle_hook();
    }
}
