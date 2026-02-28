//HelixRT - Clock Configuration HAL for IMXRT1062

#ifndef CLOCK_H
#define CLOCK_H

#include "imxrt1062.h"

// Clock frequencies (default after boot ROM) 
#define CLOCK_ARM_PLL_FREQ      1200000000UL    // 1.2 GHz 
#define CLOCK_SYS_PLL_FREQ      528000000UL     // 528 MHz 
#define CLOCK_USB_PLL_FREQ      480000000UL     // 480 MHz 
#define CLOCK_CORE_FREQ         600000000UL     // 600 MHz (ARM PLL / 2) 
#define CLOCK_AHB_FREQ          600000000UL     // 600 MHz 
#define CLOCK_IPG_FREQ          150000000UL     // 150 MHz 
#define CLOCK_OSC_FREQ          24000000UL      // 24 MHz crystal 

// External clock reference 
extern volatile uint32_t SystemCoreClock;

/*
  Initialize system clocks to default configuration.
 
  Boot ROM already configures a valid 600 MHz baseline on Teensy 4.1.
  This function explicitly enables gate clocks needed by early firmware
  and publishes the expected core frequency.
 */
static inline void clock_init(void)
{
    SystemCoreClock = CLOCK_CORE_FREQ;

    // Enable clock gates used by Stage-1/2 bring-up paths
    CCM_CCGR0 |= (CCM_CCGR_ON << 30); // GPIO2
    CCM_CCGR1 |= (CCM_CCGR_ON << 30); // GPIO3 
    CCM_CCGR6 |= (CCM_CCGR_ON << 24); // LPUART2 
}

// Get current core clock frequency 
static inline uint32_t clock_get_core_freq(void)
{
    return SystemCoreClock;
}

// Enable clock for a peripheral 
static inline void clock_enable_gpio3(void)
{
    CCM_CCGR1 |= (CCM_CCGR_ON << 30);
}

static inline void clock_enable_lpuart2(void)
{
    CCM_CCGR6 |= (CCM_CCGR_ON << 24);
}

static inline void clock_enable_lpuart3(void)
{
    CCM_CCGR1 |= (CCM_CCGR_ON << 28);
}

static inline void clock_enable_lpuart4(void)
{
    CCM_CCGR1 |= (CCM_CCGR_ON << 12);
}

static inline void clock_enable_lpuart5(void)
{
    CCM_CCGR2 |= (CCM_CCGR_ON << 24);
}

static inline void clock_enable_gpt2(void)
{
    CCM_CCGR2 |= (CCM_CCGR_ON << 20);
}

static inline void clock_enable_gpt3(void)
{
    CCM_CCGR1 |= (CCM_CCGR_ON << 24);
}

#endif // CLOCK_H 
