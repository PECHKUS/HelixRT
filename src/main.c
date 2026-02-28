/*
 * HelixRT - Main Entry Point
 *
 * This file demonstrates the intended architecture:
 * - HAL handles board clocks/GPIO setup
 * - Kernel handles timing/scheduling
 * - Tasks own application behavior
 */

#include <stdint.h>
#include <stddef.h>
#include "../include/helixrt.h"

#define LED_PIN 3U

TASK_STATIC_DEFINE(blink, 1024);
TASK_STATIC_DEFINE(heartbeat, 1024);

static void board_gpio_init(void)
{
    // Teensy 4.1 onboard LED: GPIO_B0_03 -> GPIO2_IO03 (ALT5)
    (*(volatile uint32_t *)0x401F814C) = 5U;
    (*(volatile uint32_t *)0x401F833C) =
        IOMUXC_PAD_DSE(6) | IOMUXC_PAD_SPEED(2) | IOMUXC_PAD_SRE;

    gpio_set_mode(GPIO2, LED_PIN, GPIO_MODE_OUTPUT);
    gpio_clear(GPIO2, LED_PIN);
}

static void blink_task(void *arg)
{
    (void)arg;
    while (1) {
        gpio_toggle(GPIO2, LED_PIN);
        task_delay_ms(500);
    }
}

static void heartbeat_task(void *arg)
{
    (void)arg;
    while (1) {
        /*
         * This periodic yield-only task keeps a second runnable task active,
         * which makes round-robin/preemption behavior observable.
         */
         
        task_delay_ms(100);
    }
}

int main(void)
{
    clock_init();
    board_gpio_init();

    if (kernel_init() != KERNEL_OK) {
        while (1) { __WFI(); }
    }

    (void)task_create(TASK_STATIC_TCB(blink),
                      "blink",
                      blink_task,
                      NULL,
                      2,
                      TASK_STATIC_STACK(blink),
                      TASK_STATIC_STACK_SIZE(blink));

    (void)task_create(TASK_STATIC_TCB(heartbeat),
                      "hb",
                      heartbeat_task,
                      NULL,
                      3,
                      TASK_STATIC_STACK(heartbeat),
                      TASK_STATIC_STACK_SIZE(heartbeat));

    kernel_start();
}
