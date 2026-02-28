// HelixRT - Kernel Configuration

// Modify these values to tune the RTOS for your application.

#ifndef HELIXRT_CONFIG_H
#define HELIXRT_CONFIG_H

// Core Timing

// System tick rate in Hz (1000 = 1ms tick) 
#define CONFIG_TICK_RATE_HZ             1000

// CPU clock frequency (set by clock init) 
#define CONFIG_CPU_CLOCK_HZ             600000000UL

// Task Configuration
 
// Maximum number of concurrent tasks 
#define CONFIG_MAX_TASKS                16

// Maximum priority levels (0 = highest, MAX-1 = lowest) 
#define CONFIG_MAX_PRIORITY             32

// Default stack size for new tasks (bytes) 
#define CONFIG_DEFAULT_STACK_SIZE       1024

// Minimum stack size allowed (bytes) 
#define CONFIG_MIN_STACK_SIZE           256

// Idle task stack size (bytes) 
#define CONFIG_IDLE_STACK_SIZE          256

// Task name maximum length 
#define CONFIG_TASK_NAME_MAX            16

// Scheduling

// Enable round-robin scheduling for same-priority tasks 
#define CONFIG_ROUND_ROBIN              1

// Time slice for round-robin (in ticks) 
#define CONFIG_TIME_SLICE               10

// Enable preemption (1 = preemptive, 0 = cooperative) 
#define CONFIG_PREEMPTIVE               1

// Synchronization

// Enable priority inheritance for mutexes 
#define CONFIG_PRIORITY_INHERITANCE     1

// Maximum semaphore count (0 = unlimited) 
#define CONFIG_SEM_MAX_COUNT            0

// Memory

// Enable dynamic memory allocation (heap) 
#define CONFIG_DYNAMIC_ALLOC            0

// Heap size in bytes (if dynamic alloc enabled)
#define CONFIG_HEAP_SIZE                (64 * 1024)

// Debugging & Safety

// Enable stack overflow checking 
#define CONFIG_STACK_CHECK              1

// Stack guard word for overflow detection 
#define CONFIG_STACK_GUARD_WORD         0xDEADBEEF

// Enable kernel assertions 
#define CONFIG_ASSERT                   1

// Enable task runtime statistics 
#define CONFIG_TASK_STATS               1

// Software Timers

// Enable software timers 
#define CONFIG_SW_TIMERS                1

// Maximum number of software timers 
#define CONFIG_MAX_SW_TIMERS            8

// Software timer task priority 
#define CONFIG_TIMER_TASK_PRIORITY      1

// Software timer task stack size 
#define CONFIG_TIMER_STACK_SIZE         512

// Event Groups

// Enable event groups/flags 
#define CONFIG_EVENT_GROUPS             1

// Number of bits in event group (max 24 on Cortex-M) 
#define CONFIG_EVENT_BITS               24

// Debug UART

//Enable kernel debug output
#define CONFIG_DEBUG_UART               1

// UART peripheral for debug output 
#define CONFIG_DEBUG_UART_PORT          LPUART1

/* Debug UART baud rate */
#define CONFIG_DEBUG_BAUD               115200

//Hooks

//Enable idle hook 
#define CONFIG_IDLE_HOOK                1

// Enable tick hook 
#define CONFIG_TICK_HOOK                1

// Enable stack overflow hook 
#define CONFIG_STACK_OVERFLOW_HOOK      1

//ISR Stack

// Separate stack for ISR handling (MSP) 
#define CONFIG_ISR_STACK_SIZE           (2 * 1024)

// Assertion Macro

#if CONFIG_ASSERT
    extern void kernel_assert_failed(const char *file, int line);
    #define KERNEL_ASSERT(expr) \
        do { if (!(expr)) kernel_assert_failed(__FILE__, __LINE__); } while(0)
#else
    #define KERNEL_ASSERT(expr) ((void)0)
#endif

//Timeout Constants

#define TIMEOUT_NONE        0           // No waiting, return immediately 
#define TIMEOUT_FOREVER     UINT32_MAX  // Wait indefinitely 

// Convert milliseconds to ticks 
#define MS_TO_TICKS(ms)     (((ms) * CONFIG_TICK_RATE_HZ) / 1000)

// Convert ticks to milliseconds 
#define TICKS_TO_MS(ticks)  (((ticks) * 1000) / CONFIG_TICK_RATE_HZ)

#endif // HELIXRT_CONFIG_H 
