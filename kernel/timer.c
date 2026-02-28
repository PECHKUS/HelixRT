// HelixRT - Software Timer Implementation


#include <stdint.h>
#include <stddef.h>
#include "../include/config.h"
#include "timer.h"
#include "kernel.h"
#include "sync/critical.h"

static sw_timer_t *g_timer_list = NULL;

static void timer_list_remove(sw_timer_t *timer)
{
    sw_timer_t *iter = g_timer_list;
    sw_timer_t *prev = NULL;

    while (iter != NULL) {
        if (iter == timer) {
            if (prev == NULL) {
                g_timer_list = iter->next;
            } else {
                prev->next = iter->next;
            }
            iter->next = NULL;
            iter->active = 0;
            return;
        }
        prev = iter;
        iter = iter->next;
    }
}

static void timer_list_add(sw_timer_t *timer)
{
    timer->next = g_timer_list;
    g_timer_list = timer;
    timer->active = 1;
}

int timer_create(sw_timer_t *timer, timer_callback_t cb, void *arg)
{
    if (timer == NULL || cb == NULL) {
        return KERNEL_ERR_PARAM;
    }

    timer->period_ticks = 0;
    timer->remaining_ticks = 0;
    timer->callback = cb;
    timer->arg = arg;
    timer->periodic = 0;
    timer->active = 0;
    timer->next = NULL;
    return KERNEL_OK;
}

int timer_start(sw_timer_t *timer, uint32_t period_ticks, uint8_t periodic)
{
    uint32_t irq_state;

    if (timer == NULL || timer->callback == NULL || period_ticks == 0U) {
        return KERNEL_ERR_PARAM;
    }

    irq_state = critical_enter();
    if (timer->active) {
        timer_list_remove(timer);
    }
    timer->period_ticks = period_ticks;
    timer->remaining_ticks = period_ticks;
    timer->periodic = periodic ? 1U : 0U;
    timer_list_add(timer);
    critical_exit(irq_state);

    return KERNEL_OK;
}

int timer_stop(sw_timer_t *timer)
{
    uint32_t irq_state;

    if (timer == NULL) {
        return KERNEL_ERR_PARAM;
    }

    irq_state = critical_enter();
    if (timer->active) {
        timer_list_remove(timer);
    }
    critical_exit(irq_state);

    return KERNEL_OK;
}

int timer_is_active(sw_timer_t *timer)
{
    if (timer == NULL) {
        return 0;
    }
    return timer->active ? 1 : 0;
}

void timer_tick_isr(void)
{
#if CONFIG_SW_TIMERS
    sw_timer_t *iter = g_timer_list;
    sw_timer_t *next;

    while (iter != NULL) {
        next = iter->next;
        if (iter->remaining_ticks > 0U) {
            iter->remaining_ticks--;
        }

        if (iter->remaining_ticks == 0U) {
            timer_callback_t cb = iter->callback;
            void *arg = iter->arg;

            if (iter->periodic) {
                iter->remaining_ticks = iter->period_ticks;
            } else {
                timer_list_remove(iter);
            }

            /*
             * Callback executes in SysTick context.
             * Keep callbacks short and non-blocking.
             */
             
             
            cb(arg);
        }
        iter = next;
    }
#endif
}
