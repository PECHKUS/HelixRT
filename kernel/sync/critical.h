// HelixRT - Critical Section API
 
// Interrupt-safe critical sections using PRIMASK
 

#ifndef CRITICAL_H
#define CRITICAL_H

#include <stdint.h>

/*
 * Critical Section API
 * 
 * Critical sections disable interrupts to protect shared data.
 * They are nestable - the original interrupt state is saved and restored.
 * 
 * Usage:
 *   uint32_t state = critical_enter();
 *   // ... protected code ...
 *   critical_exit(state);
 * 
 * WARNING: Keep critical sections as short as possible.
 *          Long critical sections increase interrupt latency.
 */
 

/*
 * critical_enter - Enter critical section (disable interrupts)
 * 
 * Saves the current PRIMASK value and disables interrupts.
 * Can be nested - each enter must have a matching exit.
 * 
 * Returns: Previous interrupt state (for critical_exit)
 */
 
 
static inline uint32_t critical_enter(void)
{
    uint32_t primask;
    __asm volatile (
        "mrs %0, primask    \n"     // Save current PRIMASK 
        "cpsid i            \n"     // Disable interrupts 
        : "=r" (primask)
        :
        : "memory"
    );
    return primask;
}

/*
 * critical_exit - Exit critical section (restore interrupts)
 * 
 * Restores the PRIMASK value that was saved by critical_enter.
 * Only re-enables interrupts if they were enabled before critical_enter.
 * 
 * @state: Value returned by critical_enter()
 */
 
 
static inline void critical_exit(uint32_t state)
{
    __asm volatile (
        "msr primask, %0    \n"     //Restore PRIMASK 
        :
        : "r" (state)
        : "memory"
    );
}

/* 
 * BASEPRI-Based Critical Sections (Alternative)
 * 
 * These allow high-priority interrupts to still run while blocking
 * lower-priority interrupts. Useful for real-time requirements
 * 
 * BASEPRI of 0 = all interrupts enabled
 * BASEPRI of N = interrupts with priority >= N are disabled
 */


/*
 * critical_enter_basepri - Enter critical section using BASEPRI
 * 
 * @max_priority: Maximum priority to disable (lower number = higher priority)
 * 
 * Returns: Previous BASEPRI value
 */
 
 
static inline uint32_t critical_enter_basepri(uint32_t max_priority)
{
    uint32_t basepri;
    __asm volatile (
        "mrs %0, basepri    \n"
        "msr basepri, %1    \n"
        : "=r" (basepri)
        : "r" (max_priority << 4)   // Priority is in upper 4 bits 
        : "memory"
    );
    return basepri;
}


/*
 * critical_exit_basepri - Exit BASEPRI-based critical section
 * 
 * @state: Value returned by critical_enter_basepri()
 */
 
static inline void critical_exit_basepri(uint32_t state)
{
    __asm volatile (
        "msr basepri, %0    \n"
        :
        : "r" (state)
        : "memory"
    );
}

// Interrupt State Query


/*
 * is_irq_disabled - Check if interrupts are disabled
 * 
 * Returns: 1 if interrupts disabled, 0 if enabled
 */
 
 
static inline int is_irq_disabled(void)
{
    uint32_t primask;
    __asm volatile ("mrs %0, primask" : "=r" (primask));
    return primask & 1;
}

/*
 * is_isr_context - Check if currently in ISR (handler mode)
 * 
 * Returns: 1 if in ISR, 0 if in thread mode
 */
 
static inline int is_isr_context(void)
{
    uint32_t ipsr;
    __asm volatile ("mrs %0, ipsr" : "=r" (ipsr));
    return ipsr != 0;
}

//Convenience Macros
 
// Critical section with automatic scope 
#define CRITICAL_SECTION_BEGIN  uint32_t __critical_state = critical_enter()
#define CRITICAL_SECTION_END    critical_exit(__critical_state)

// Alternative with block scope 
#define CRITICAL_SECTION(code)          \
    do {                                \
        uint32_t __cs = critical_enter(); \
        { code; }                       \
        critical_exit(__cs);            \
    } while (0)

#endif // CRITICAL_H 
