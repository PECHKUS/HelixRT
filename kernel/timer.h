// HelixRT - Software Timer API


#ifndef TIMER_H
#define TIMER_H

#include <stdint.h>

typedef void (*timer_callback_t)(void *arg);

typedef struct sw_timer {
    uint32_t period_ticks;
    uint32_t remaining_ticks;
    timer_callback_t callback;
    void *arg;
    uint8_t periodic;
    uint8_t active;
    struct sw_timer *next;
} sw_timer_t;

int timer_create(sw_timer_t *timer, timer_callback_t cb, void *arg);
int timer_start(sw_timer_t *timer, uint32_t period_ticks, uint8_t periodic);
int timer_stop(sw_timer_t *timer);
int timer_is_active(sw_timer_t *timer);

// Called from SysTick context
void timer_tick_isr(void);

#endif // TIMER_H 
