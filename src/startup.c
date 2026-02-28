/*
 * HelixRT - Startup Code for Teensy 4.1 (IMXRT1062)
 * 
 * This file contains:
 *   - Flash Configuration Block (FCB) for QSPI boot
 *   - Image Vector Table (IVT) and Boot Data
 *   - Cortex-M7 Vector Table
 *   - Reset Handler with memory initialization
 *   - Default exception/interrupt handlers
 */

#include <stdint.h>
#include "../hal/imxrt1062.h"

// Forward declaration for boot_data (defined below) 
extern const uint32_t boot_data[4];

// External Symbols from Linker Script

extern uint32_t __vectors_start;
extern uint32_t __vectors_end;
extern uint32_t __text_start;
extern uint32_t __text_end;
extern uint32_t __fast_code_start;
extern uint32_t __fast_code_end;
extern uint32_t __fast_code_load;
extern uint32_t __data_start;
extern uint32_t __data_end;
extern uint32_t __data_load;
extern uint32_t __bss_start;
extern uint32_t __bss_end;
extern uint32_t __stack_top;
extern uint32_t __heap_start;
extern uint32_t __heap_end;

// Function Prototypes
 
void Reset_Handler(void);
void NMI_Handler(void);
void HardFault_Handler(void);
void MemManage_Handler(void);
void BusFault_Handler(void);
void UsageFault_Handler(void);
void SVC_Handler(void);
void DebugMon_Handler(void);
void PendSV_Handler(void);
void SysTick_Handler(void);
void Default_Handler(void);

extern int main(void);

/* 
 * Flash Configuration Block (FCB) for QSPI Boot
 * 
 * Located at 0x60000000 (FLASH_CONFIG region)
 * Configures the FlexSPI controller for the external QSPI flash
*/

__attribute__((section(".flash_config"), used))
const uint32_t flash_config[128] = {
    // 0x000: Tag 'FCFB' 
    0x42464346,
    // 0x004: Version
    0x56010400,
    // 0x008: Reserved 
    0,
    // 0x00C: readSampleClkSrc, seqEnable, reserved 
    0x00030301,
    // 0x010: Reserved 
    0,
    // 0x014: Reserved 
    0,
    // 0x018: Reserved 
    0,
    // 0x01C: Reserved 
    0,
    
    // 0x020: Reserved 
    0,
    // 0x024: Reserved 
    0,
    //0x028: Reserved 
    0,
    // 0x02C: Reserved 
    0,
    // 0x030: Reserved
    0,
    // 0x034: Reserved 
    0,
    // 0x038: Reserved 
    0,
    // 0x03C: Reserved 
    0,
    
    // 0x040: Reserved 
    0,
    // 0x044: deviceModeCfgEnable, deviceModeSeq, deviceModeArg 
    0,
    // 0x048: Reserved 
    0,
    // 0x04C: configCmdEnable, configCmdSeqs 
    0,
    // 0x050: Reserved 
    0,
    // 0x054: Reserved 
    0,
    // 0x058: Reserved 
    0,
    // 0x05C: Reserved 
    0,
    
    // 0x060: configCmdArgs 
    0,
    // 0x064: Reserved 
    0,
    //0x068: Reserved 
    0,
    // 0x06C: Reserved 
    0,
    // 0x070: controllerMiscOption 
    0,
    // 0x074: deviceType=SerialNOR, sflashPadType=QuadPads, serialClkFreq=100MHz 
    0x00010100,
    // 0x078: lutCustomSeqEnable, reserved 
    0,
    // 0x07C: Reserved 
    0,
    
    /// 0x080: sflashA1Size (8MB) 
    0x00800000,
    // 0x084: sflashA2Size 
    0,
    // 0x088: sflashB1Size 
    0,
    // 0x08C: sflashB2Size 
    0,
    // 0x090: csPadSettingOverride 
    0,
    // 0x094: sclkPadSettingOverride
    0,
    // 0x098: dataPadSettingOverride 
    0,
    // 0x09C: dqsPadSettingOverride
    0,
    
    // 0x0A0: timeoutInMs 
    0,
    // 0x0A4: commandInterval 
    0,
    // 0x0A8: dataValidTime
    0,
    /* 0x0AC: busyOffset, busyBitPolarity */
    0,
    
    /* 0x0B0-0x0BF: LUT[0] - Read (CMD_SDR + ADDR_SDR + READ_SDR) */
    /* Sequence for Quad Read command 0xEB */
    0x0A1804EB,  /* CMD_SDR(0xEB), ADDR_SDR(24-bit) */
    0x26043206,  /* DUMMY_SDR(6 cycles), READ_SDR */
    0x00000000,
    0x00000000,
    
    /* 0x0C0-0x0CF: LUT[1] - Read Status */
    0x24040405,  /* CMD_SDR(0x05), READ_SDR */
    0x00000000,
    0x00000000,
    0x00000000,
    
    /* 0x0D0-0x0DF: LUT[2] - Reserved */
    0x00000000,
    0x00000000,
    0x00000000,
    0x00000000,
    
    /* 0x0E0-0x0EF: LUT[3] - Write Enable */
    0x00000406,  /* CMD_SDR(0x06) */
    0x00000000,
    0x00000000,
    0x00000000,
    
    /* 0x0F0-0x0FF: LUT[4] - Reserved */
    0x00000000,
    0x00000000,
    0x00000000,
    0x00000000,
    
    /* 0x100-0x10F: LUT[5] - Erase Sector (4KB) */
    0x08180420,  /* CMD_SDR(0x20), ADDR_SDR(24-bit) */
    0x00000000,
    0x00000000,
    0x00000000,
    
    /* 0x110-0x11F: LUT[6] - Reserved */
    0x00000000,
    0x00000000,
    0x00000000,
    0x00000000,
    
    /* 0x120-0x12F: LUT[7] - Reserved */
    0x00000000,
    0x00000000,
    0x00000000,
    0x00000000,
    
    /* 0x130-0x13F: LUT[8] - Reserved */
    0x00000000,
    0x00000000,
    0x00000000,
    0x00000000,
    
    /* 0x140-0x14F: LUT[9] - Page Program */
    0x08180402,  /* CMD_SDR(0x02), ADDR_SDR(24-bit) */
    0x00002004,  /* WRITE_SDR */
    0x00000000,
    0x00000000,
    
    /* 0x150-0x15F: LUT[10] - Reserved */
    0x00000000,
    0x00000000,
    0x00000000,
    0x00000000,
    
    /* 0x160-0x16F: LUT[11] - Chip Erase */
    0x00000460,  /* CMD_SDR(0x60) */
    0x00000000,
    0x00000000,
    0x00000000,
    
    /* 0x170-0x17F: Reserved */
    0x00000000,
    0x00000000,
    0x00000000,
    0x00000000,
    
    /* 0x180-0x1BF: Reserved */
    0, 0, 0, 0,
    0, 0, 0, 0,
    0, 0, 0, 0,
    0, 0, 0, 0,
    
    /* 0x1C0-0x1FF: pageSize, sectorSize, ipCmdSerialClkFreq, reserved */
    0x00000100,  /* pageSize = 256 bytes */
    0x00001000,  /* sectorSize = 4KB */
    0x00000001,  /* Serial clock frequency for IP command */
    0x00000000,  /* Reserved */
    
    0, 0, 0, 0,
    0, 0, 0, 0,
    0, 0, 0, 0,
};

/* ============================================================================
 * Image Vector Table (IVT)
 * 
 * Located at 0x60001000 (start of FLASH region after config)
 * Required by i.MX RT boot ROM
 * ============================================================================ */

/* IVT is at 0x60000400, Boot Data follows at 0x60000420 */
#define IVT_ADDRESS         0x60000400
#define BOOT_DATA_ADDRESS   0x60000420
#define VECTORS_ADDRESS     0x60001000

__attribute__((section(".ivt"), used))
const uint32_t image_vector_table[8] = {
    0x402000D1,                     /* IVT Header: Tag=0xD1, Length=0x0020, Version=0x40 */
    VECTORS_ADDRESS,                /* Entry point (address of vector table) */
    0,                              /* Reserved */
    0,                              /* DCD (Device Configuration Data) - not used */
    BOOT_DATA_ADDRESS,              /* Boot Data pointer */
    IVT_ADDRESS,                    /* Self pointer (IVT absolute address) */
    0,                              /* CSF (Command Sequence File) - not used for unsigned */
    0,                              /* Reserved */
};

/* ============================================================================
 * Boot Data Structure
 * 
 * Immediately follows IVT
 * Tells boot ROM where to find the application image
 * ============================================================================ */

__attribute__((section(".boot_data"), used))
const uint32_t boot_data[4] = {
    0x60000000,     /* Start address of image (QSPI flash base) */
    0x00100000,     /* Size of image (1MB - adjust as needed) */
    0,              /* Plugin flag (0 = not a plugin) */
    0,              /* Reserved */
};

/* ============================================================================
 * Cortex-M7 Vector Table
 * 
 * First entry is initial stack pointer
 * Second entry is reset handler
 * Followed by exception and interrupt handlers
 * ============================================================================ */

__attribute__((section(".vectors"), used))
void (* const vector_table[])(void) = {
    /* Initial Stack Pointer */
    (void (*)(void))(&__stack_top),
    
    /* Cortex-M7 Core Exceptions */
    Reset_Handler,          /* Reset Handler */
    NMI_Handler,            /* NMI Handler */
    HardFault_Handler,      /* Hard Fault Handler */
    MemManage_Handler,      /* MPU Fault Handler */
    BusFault_Handler,       /* Bus Fault Handler */
    UsageFault_Handler,     /* Usage Fault Handler */
    0,                      /* Reserved */
    0,                      /* Reserved */
    0,                      /* Reserved */
    0,                      /* Reserved */
    SVC_Handler,            /* SVCall Handler */
    DebugMon_Handler,       /* Debug Monitor Handler */
    0,                      /* Reserved */
    PendSV_Handler,         /* PendSV Handler */
    SysTick_Handler,        /* SysTick Handler */
    
    /* IMXRT1062 External Interrupts (IRQ 0-159) */
    Default_Handler,        /* 0: DMA channel 0 */
    Default_Handler,        /* 1: DMA channel 1 */
    Default_Handler,        /* 2: DMA channel 2 */
    Default_Handler,        /* 3: DMA channel 3 */
    Default_Handler,        /* 4: DMA channel 4 */
    Default_Handler,        /* 5: DMA channel 5 */
    Default_Handler,        /* 6: DMA channel 6 */
    Default_Handler,        /* 7: DMA channel 7 */
    Default_Handler,        /* 8: DMA channel 8 */
    Default_Handler,        /* 9: DMA channel 9 */
    Default_Handler,        /* 10: DMA channel 10 */
    Default_Handler,        /* 11: DMA channel 11 */
    Default_Handler,        /* 12: DMA channel 12 */
    Default_Handler,        /* 13: DMA channel 13 */
    Default_Handler,        /* 14: DMA channel 14 */
    Default_Handler,        /* 15: DMA channel 15 */
    Default_Handler,        /* 16: DMA error */
    Default_Handler,        /* 17: CTI trigger outputs */
    Default_Handler,        /* 18: Reserved */
    Default_Handler,        /* 19: Reserved */
    Default_Handler,        /* 20: LPUART1 */
    Default_Handler,        /* 21: LPUART2 */
    Default_Handler,        /* 22: LPUART3 */
    Default_Handler,        /* 23: LPUART4 */
    Default_Handler,        /* 24: LPUART5 */
    Default_Handler,        /* 25: LPUART6 */
    Default_Handler,        /* 26: LPUART7 */
    Default_Handler,        /* 27: LPUART8 */
    Default_Handler,        /* 28: LPI2C1 */
    Default_Handler,        /* 29: LPI2C2 */
    Default_Handler,        /* 30: LPI2C3 */
    Default_Handler,        /* 31: LPI2C4 */
    Default_Handler,        /* 32: LPSPI1 */
    Default_Handler,        /* 33: LPSPI2 */
    Default_Handler,        /* 34: LPSPI3 */
    Default_Handler,        /* 35: LPSPI4 */
    Default_Handler,        /* 36: CAN1 */
    Default_Handler,        /* 37: CAN2 */
    Default_Handler,        /* 38: FlexRAM */
    Default_Handler,        /* 39: KPP */
    Default_Handler,        /* 40: TSC_DIG */
    Default_Handler,        /* 41: GPR_IRQ */
    Default_Handler,        /* 42: LCDIF */
    Default_Handler,        /* 43: CSI */
    Default_Handler,        /* 44: PXP */
    Default_Handler,        /* 45: WDOG2 */
    Default_Handler,        /* 46: SNVS_HP_WRAPPER */
    Default_Handler,        /* 47: SNVS_HP_WRAPPER_TZ */
    Default_Handler,        /* 48: SNVS_LP_WRAPPER */
    Default_Handler,        /* 49: CSU */
    Default_Handler,        /* 50: DCP */
    Default_Handler,        /* 51: DCP_VMI */
    Default_Handler,        /* 52: Reserved */
    Default_Handler,        /* 53: TRNG */
    Default_Handler,        /* 54: SJC_IRQ */
    Default_Handler,        /* 55: BEE */
    Default_Handler,        /* 56: SAI1 */
    Default_Handler,        /* 57: SAI2 */
    Default_Handler,        /* 58: SAI3_RX */
    Default_Handler,        /* 59: SAI3_TX */
    Default_Handler,        /* 60: SPDIF */
    Default_Handler,        /* 61: PMU_EVENT */
    Default_Handler,        /* 62: Reserved */
    Default_Handler,        /* 63: Temperature Monitor */
    Default_Handler,        /* 64: Temperature Monitor Low */
    Default_Handler,        /* 65: Temperature Monitor High */
    Default_Handler,        /* 66: Temperature Monitor Panic */
    Default_Handler,        /* 67: USB PHY1 */
    Default_Handler,        /* 68: USB PHY2 */
    Default_Handler,        /* 69: ADC1 */
    Default_Handler,        /* 70: ADC2 */
    Default_Handler,        /* 71: DCDC */
    Default_Handler,        /* 72: Reserved */
    Default_Handler,        /* 73: Reserved */
    Default_Handler,        /* 74: GPIO1_INT0 */
    Default_Handler,        /* 75: GPIO1_INT1 */
    Default_Handler,        /* 76: GPIO1_INT2 */
    Default_Handler,        /* 77: GPIO1_INT3 */
    Default_Handler,        /* 78: GPIO1_INT4 */
    Default_Handler,        /* 79: GPIO1_INT5 */
    Default_Handler,        /* 80: GPIO1_INT6 */
    Default_Handler,        /* 81: GPIO1_INT7 */
    Default_Handler,        /* 82: GPIO1_Combined_0_15 */
    Default_Handler,        /* 83: GPIO1_Combined_16_31 */
    Default_Handler,        /* 84: GPIO2_Combined_0_15 */
    Default_Handler,        /* 85: GPIO2_Combined_16_31 */
    Default_Handler,        /* 86: GPIO3_Combined_0_15 */
    Default_Handler,        /* 87: GPIO3_Combined_16_31 */
    Default_Handler,        /* 88: GPIO4_Combined_0_15 */
    Default_Handler,        /* 89: GPIO4_Combined_16_31 */
    Default_Handler,        /* 90: GPIO5_Combined_0_15 */
    Default_Handler,        /* 91: GPIO5_Combined_16_31 */
    Default_Handler,        /* 92: FlexIO1 */
    Default_Handler,        /* 93: FlexIO2 */
    Default_Handler,        /* 94: WDOG1 */
    Default_Handler,        /* 95: RTWDOG */
    Default_Handler,        /* 96: EWM */
    Default_Handler,        /* 97: CCM_1 */
    Default_Handler,        /* 98: CCM_2 */
    Default_Handler,        /* 99: GPC */
    Default_Handler,        /* 100: SRC */
    Default_Handler,        /* 101: Reserved */
    Default_Handler,        /* 102: GPT1 */
    Default_Handler,        /* 103: GPT2 */
    Default_Handler,        /* 104: PWM1_0 */
    Default_Handler,        /* 105: PWM1_1 */
    Default_Handler,        /* 106: PWM1_2 */
    Default_Handler,        /* 107: PWM1_3 */
    Default_Handler,        /* 108: PWM1_FAULT */
    Default_Handler,        /* 109: FlexSPI2 */
    Default_Handler,        /* 110: FlexSPI */
    Default_Handler,        /* 111: SEMC */
    Default_Handler,        /* 112: USDHC1 */
    Default_Handler,        /* 113: USDHC2 */
    Default_Handler,        /* 114: USB_OTG2 */
    Default_Handler,        /* 115: USB_OTG1 */
    Default_Handler,        /* 116: ENET */
    Default_Handler,        /* 117: ENET_1588_Timer */
    Default_Handler,        /* 118: XBAR1_IRQ_0_1 */
    Default_Handler,        /* 119: XBAR1_IRQ_2_3 */
    Default_Handler,        /* 120: ADC_ETC_IRQ0 */
    Default_Handler,        /* 121: ADC_ETC_IRQ1 */
    Default_Handler,        /* 122: ADC_ETC_IRQ2 */
    Default_Handler,        /* 123: ADC_ETC_ERROR_IRQ */
    Default_Handler,        /* 124: PIT */
    Default_Handler,        /* 125: ACMP1 */
    Default_Handler,        /* 126: ACMP2 */
    Default_Handler,        /* 127: ACMP3 */
    Default_Handler,        /* 128: ACMP4 */
    Default_Handler,        /* 129: Reserved */
    Default_Handler,        /* 130: Reserved */
    Default_Handler,        /* 131: ENC1 */
    Default_Handler,        /* 132: ENC2 */
    Default_Handler,        /* 133: ENC3 */
    Default_Handler,        /* 134: ENC4 */
    Default_Handler,        /* 135: TMR1 */
    Default_Handler,        /* 136: TMR2 */
    Default_Handler,        /* 137: TMR3 */
    Default_Handler,        /* 138: TMR4 */
    Default_Handler,        /* 139: PWM2_0 */
    Default_Handler,        /* 140: PWM2_1 */
    Default_Handler,        /* 141: PWM2_2 */
    Default_Handler,        /* 142: PWM2_3 */
    Default_Handler,        /* 143: PWM2_FAULT */
    Default_Handler,        /* 144: PWM3_0 */
    Default_Handler,        /* 145: PWM3_1 */
    Default_Handler,        /* 146: PWM3_2 */
    Default_Handler,        /* 147: PWM3_3 */
    Default_Handler,        /* 148: PWM3_FAULT */
    Default_Handler,        /* 149: PWM4_0 */
    Default_Handler,        /* 150: PWM4_1 */
    Default_Handler,        /* 151: PWM4_2 */
    Default_Handler,        /* 152: PWM4_3 */
    Default_Handler,        /* 153: PWM4_FAULT */
    Default_Handler,        /* 154: ENET2 */
    Default_Handler,        /* 155: ENET2_1588_Timer */
    Default_Handler,        /* 156: CAN3 */
    Default_Handler,        /* 157: Reserved */
    Default_Handler,        /* 158: FlexIO3 */
    Default_Handler,        /* 159: GPIO6_7_8_9 */
};

/* ============================================================================
 * Reset Handler
 * 
 * Called on system reset. Initializes memory regions and calls main()
 * ============================================================================ */

void Reset_Handler(void)
{
    uint32_t *src, *dst;
    
    /*
     * CRITICAL: Disable watchdogs IMMEDIATELY - before any other code runs.
     * The IMXRT1062 boots with RTWDOG enabled by default and it will
     * cause a reset/hard fault if not disabled quickly.
     */
    
    /*
     * Disable WDOG1 and WDOG2 (Legacy Watchdogs)
     * These are typically NOT enabled by boot ROM on Teensy 4.1,
     * but we disable them anyway for safety.
     * Note: WDE bit can only be written once after reset.
     */
    WDOG1->WCR &= ~WDOG_WCR_WDE;
    WDOG2->WCR &= ~WDOG_WCR_WDE;
    
    /*
     * Disable RTWDOG (Real-Time Watchdog) - THIS IS THE CRITICAL ONE
     * 
     * The RTWDOG is enabled by default after reset on IMXRT1062.
     * It MUST be disabled within the first few hundred milliseconds
     * or it will trigger a system reset.
     * 
     * Unlock sequence for RTWDOG:
     * 1. Check if CMD32EN is set (determines unlock key format)
     * 2. Write unlock key to CNT register
     * 3. Wait for ULK bit in CS to indicate unlock complete
     * 4. Write new CS value with EN=0 and UPDATE=1
     * 
     * The unlock key is 0xD928C520 for 32-bit mode.
     * For 16-bit mode, write 0xC520 then 0xD928 to CNT.
     */
    if ((RTWDOG->CS & RTWDOG_CS_CMD32EN) != 0U) {
        /* 32-bit command mode - single write */
        RTWDOG->CNT = 0xD928C520UL;
    } else {
        /* 16-bit command mode - two sequential writes */
        *((volatile uint16_t *)&RTWDOG->CNT) = 0xC520U;
        *((volatile uint16_t *)&RTWDOG->CNT) = 0xD928U;
    }
    
    /* 
     * Wait for unlock - the ULK bit in CS indicates unlock is complete.
     * This typically happens within a few bus cycles.
     */
    while ((RTWDOG->CS & RTWDOG_CS_ULK) == 0U) {
        /* Spin - must not take too long or watchdog fires */
    }
    
    /*
     * Now disable the RTWDOG:
     * - Clear EN bit to disable
     * - Set UPDATE bit to allow future reconfiguration
     * - Write TOVAL to maximum to give us time even if disable fails
     */
    RTWDOG->TOVAL = 0xFFFFU;
    RTWDOG->CS = (RTWDOG->CS & ~RTWDOG_CS_EN) | RTWDOG_CS_UPDATE;
    
    /*
     * Enable FPU (Cortex-M7 has FPU)
     * Set CP10 and CP11 to Full Access (0b11 each)
     * This must be done early, before any floating-point code runs.
     */
    #define SCB_CPACR   (*(volatile uint32_t *)0xE000ED88UL)
    SCB_CPACR |= (0xFUL << 20);  /* Enable CP10 and CP11 */
    __DSB();
    __ISB();
    
    /* Copy .data section from flash to RAM */
    src = &__data_load;
    dst = &__data_start;
    while (dst < &__data_end) {
        *dst++ = *src++;
    }
    
    /* Copy .fast_code section from flash to ITCM */
    src = &__fast_code_load;
    dst = &__fast_code_start;
    while (dst < &__fast_code_end) {
        *dst++ = *src++;
    }
    
    /* Zero out .bss section */
    dst = &__bss_start;
    while (dst < &__bss_end) {
        *dst++ = 0;
    }
    
    /* Set vector table offset */
    SCB_VTOR = (uint32_t)&__vectors_start;
    __DSB();
    __ISB();
    
    /* Call main */
    main();
    
    /* If main returns, hang */
    while (1) {
        __WFI();
    }
}

/* ============================================================================
 * Default Exception Handlers
 * 
 * Weak aliases allow user code to override
 * ============================================================================ */

__attribute__((weak)) void NMI_Handler(void)
{
    while (1);
}

__attribute__((weak)) void HardFault_Handler(void)
{
    while (1);
}

__attribute__((weak)) void MemManage_Handler(void)
{
    while (1);
}

__attribute__((weak)) void BusFault_Handler(void)
{
    while (1);
}

__attribute__((weak)) void UsageFault_Handler(void)
{
    while (1);
}

__attribute__((weak)) void SVC_Handler(void)
{
    while (1);
}

__attribute__((weak)) void DebugMon_Handler(void)
{
    while (1);
}

__attribute__((weak)) void PendSV_Handler(void)
{
    while (1);
}

__attribute__((weak)) void SysTick_Handler(void)
{
    /* Empty - override in user code */
}

__attribute__((weak)) void Default_Handler(void)
{
    while (1);
}
