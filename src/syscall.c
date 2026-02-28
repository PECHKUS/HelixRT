/*
 * HelixRT - User-Side System Call Wrappers
 *
 * These wrappers demonstrate the SVC interface while keeping the
 * application-facing kernel API unchanged.
 */

#include <stdint.h>
#include "../kernel/syscall.h"

int syscall_task_yield(void)
{
    register int r0 __asm("r0");
    __asm volatile ("svc %1" : "=r" (r0) : "I" (SVC_TASK_YIELD) : "memory");
    return r0;
}

int syscall_task_delay(uint32_t ticks)
{
    register uint32_t r0 __asm("r0") = ticks;
    __asm volatile ("svc %1" : "+r" (r0) : "I" (SVC_TASK_DELAY) : "memory");
    return (int)r0;
}

int syscall_task_suspend(void *tcb)
{
    register void *r0 __asm("r0") = tcb;
    __asm volatile ("svc %1" : "+r" (r0) : "I" (SVC_TASK_SUSPEND) : "memory");
    return (int)(uintptr_t)r0;
}

int syscall_task_resume(void *tcb)
{
    register void *r0 __asm("r0") = tcb;
    __asm volatile ("svc %1" : "+r" (r0) : "I" (SVC_TASK_RESUME) : "memory");
    return (int)(uintptr_t)r0;
}
