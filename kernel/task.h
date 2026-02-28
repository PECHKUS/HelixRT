/*
 * HelixRT - Task Control Block Definition
 * 
 * Defines the TCB structure and task-related types.
 * This structure is the foundation for task management.
 */


#ifndef TASK_H
#define TASK_H

#include <stdint.h>

// Task Configuration
 
#define TASK_NAME_MAX           16
#define TASK_STACK_MIN          256
#define TASK_STACK_GUARD        0xDEADBEEF
#define TASK_STACK_FILL         0xCDCDCDCD

// Task State

typedef enum {
    TASK_STATE_READY     = 0,   // Ready to run 
    TASK_STATE_RUNNING   = 1,   // Currently running 
    TASK_STATE_BLOCKED   = 2,   // Waiting for resource/event 
    TASK_STATE_SUSPENDED = 3,   // Manually suspended 
    TASK_STATE_DELETED   = 4,   // Marked for deletion 
} task_state_t;

// Block Reason (why task is blocked)

typedef enum {
    BLOCK_NONE          = 0,
    BLOCK_DELAY         = 1,    // task_delay() 
    BLOCK_SEMAPHORE     = 2,    // Waiting for semaphore
    BLOCK_MUTEX         = 3,    // Waiting for mutex 
    BLOCK_QUEUE_SEND    = 4,    // Queue full, waiting to send 
    BLOCK_QUEUE_RECV    = 5,    // Queue empty, waiting to receive 
    BLOCK_EVENT         = 6,    // Waiting for event flags 
} block_reason_t;

/* 
 * Task Control Block (TCB)
 * 
 * IMPORTANT: The 'sp' field MUST be at offset 0 for assembly access.
 * Do not reorder the first field.
 */

typedef struct task_tcb {
    //  OFFSET 0: Stack Pointer 
    // Assembly context switch accesses this at offset 0 
    
    uint32_t *sp;
    
    // Scheduler Queue Links
    struct task_tcb *next;         
    struct task_tcb *prev;          
    
    //Task Identity 
    uint32_t id;                  
    char name[TASK_NAME_MAX];       
    void (*entry)(void *);         
    void *arg;                     
    
    // Scheduling Parameters
    uint8_t priority;               
    uint8_t base_priority; 
    task_state_t state;       
    uint8_t flags;                 
    
    // Stack Information 
    uint32_t *stack_base;           
    uint32_t *stack_top;            
    uint32_t stack_size;          
    
    // Timing 
    uint32_t delay_ticks;          
    uint32_t time_slice;            
    uint32_t wake_tick;             
    
    //Blocking 
    block_reason_t block_reason;    
    void *block_object;             
    uint32_t block_timeout;         
    int block_result;               
    
    //Statistics (Optional) 
#ifdef CONFIG_TASK_STATS
    uint32_t run_count;             
    uint32_t total_ticks;          
    uint32_t max_stack_used;        
#endif
    
    //Event Waiting 
    uint32_t event_wait_bits;      
    uint8_t event_wait_all;        
    
} task_tcb_t;

//Task Flags
 
#define TASK_FLAG_STATIC        (1 << 0)    // TCB is statically allocated 
#define TASK_FLAG_STATIC_STACK  (1 << 1)    // Stack is statically allocated 
#define TASK_FLAG_PRIVILEGED    (1 << 2)    // Runs in privileged mode 
#define TASK_FLAG_FPU           (1 << 3)    // Uses FPU (for context save) 

/* 
 * Stack Frame Structures
 * 
 * Hardware automatically saves some registers on exception entry.
 * Software (PendSV) saves the rest.
 */



// Registers saved by hardware on exception entry (in this order on stack) 
typedef struct {
    uint32_t r0;
    uint32_t r1;
    uint32_t r2;
    uint32_t r3;
    uint32_t r12;
    uint32_t lr;       
    uint32_t pc;        
    uint32_t xpsr;      
} hw_stack_frame_t;

// Registers saved by software in PendSV handler 
typedef struct {
    uint32_t r4;
    uint32_t r5;
    uint32_t r6;
    uint32_t r7;
    uint32_t r8;
    uint32_t r9;
    uint32_t r10;
    uint32_t r11;
    uint32_t exc_return;    
} sw_stack_frame_t;

// Extended frame with FPU registers (if FPU context switching enabled) 
typedef struct {
    uint32_t s16;
    uint32_t s17;
    uint32_t s18;
    uint32_t s19;
    uint32_t s20;
    uint32_t s21;
    uint32_t s22;
    uint32_t s23;
    uint32_t s24;
    uint32_t s25;
    uint32_t s26;
    uint32_t s27;
    uint32_t s28;
    uint32_t s29;
    uint32_t s30;
    uint32_t s31;
} fpu_stack_frame_t;

//EXC_RETURN Values (for Cortex-M7)

#define EXC_RETURN_HANDLER_MSP      0xFFFFFFF1  
#define EXC_RETURN_THREAD_MSP       0xFFFFFFF9  
#define EXC_RETURN_THREAD_PSP       0xFFFFFFFD  

// With FPU (bit 4 indicates FPU context)
#define EXC_RETURN_THREAD_PSP_FPU   0xFFFFFFED  

//Task Stack Initialization Helper

/*
 * task_init_stack - Initialize a task's stack for first run
 * 
 * Sets up the stack so that when PendSV does a context restore,
 * the task starts executing at its entry point.
 * 
 * @stack_top:  Top of stack memory
 * @entry:      Task entry function
 * @arg:        Argument to pass to entry function
 * @exit_func:  Function to call if task returns (usually task_delete)
 * 
 * Returns: Initial stack pointer value (for TCB)
 */
 
 
uint32_t *task_init_stack(uint32_t *stack_top,
                          void (*entry)(void *),
                          void *arg,
                          void (*exit_func)(void));

// Static Task/Stack Allocation Macros

// Declare a static task with its stack 
#define TASK_STATIC_DEFINE(name, stack_size_bytes)                      \
    static task_tcb_t name##_tcb;                                       \
    static uint32_t name##_stack[(stack_size_bytes) / sizeof(uint32_t)] \
        __attribute__((section(".task_stacks"), aligned(8)))

// Get TCB pointer for static task /
#define TASK_STATIC_TCB(name)   (&name##_tcb)

// Get stack pointer and size for static task
#define TASK_STATIC_STACK(name) (name##_stack)
#define TASK_STATIC_STACK_SIZE(name) (sizeof(name##_stack))

#endif // TASK_H 
