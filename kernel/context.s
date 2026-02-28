/*
 * HelixRT - Context Switch Assembly (Cortex-M7)
 *
 * PendSV:
 * - Save outgoing task callee-saved context to PSP stack
 * - Ask scheduler for next task
 * - Restore incoming task context and exception-return
 *
 * SVC:
 * - Decode SVC immediate
 * - Dispatch into C (svc_dispatch)
 * - Write return value back to stacked r0
 */
 
 

    .syntax unified
    .cpu cortex-m7
    .fpu fpv5-d16
    .thumb

    .extern current_task
    .extern scheduler_select_next_task
    .extern svc_dispatch

    .global PendSV_Handler
    .global SVC_Handler
    .global task_start_first

    .text
    .align 4

PendSV_Handler:
    cpsid   i

    mrs     r0, psp
    ldr     r3, =current_task
    ldr     r2, [r3]
    cbz     r2, 1f

    // Save r4-r11 and EXC_RETURN (LR) for outgoing task
    stmdb   r0!, {r4-r11, lr}
    str     r0, [r2]

1:
    bl      scheduler_select_next_task
    str     r0, [r3]
    cbz     r0, 2f

    // Restore incoming task software frame and PSP
    ldr     r1, [r0]
    ldmia   r1!, {r4-r11, lr}
    msr     psp, r1

2:
    cpsie   i
    bx      lr

SVC_Handler:
    // r0 points to stacked exception frame (MSP or PSP)
    tst     lr, #4
    ite     eq
    mrseq   r0, msp
    mrsne   r0, psp

    // Decode immediate from SVC instruction at stacked PC - 2
    ldr     r1, [r0, #24]       // stacked PC 
    ldrb    r1, [r1, #-2]       // SVC number 

    // Pass stacked r0-r2 as svc args 0..2
    ldr     r2, [r0, #0]
    ldr     r3, [r0, #4]
    ldr     r12, [r0, #8]

    push    {r0, lr}
    mov     r0, r1
    mov     r1, r2
    mov     r2, r3
    mov     r3, r12
    bl      svc_dispatch
    pop     {r1, lr}

    // Return value -> stacked r0 
    str     r0, [r1, #0]
    bx      lr



/*
 * Legacy bootstrap entry kept for API compatibility.
 * Scheduler starts first task from C path in scheduler_start().
 */
 
 
task_start_first:
    bx      lr

    .end
