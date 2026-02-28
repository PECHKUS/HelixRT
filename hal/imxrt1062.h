//HelixRT - IMXRT1062 Register Definitions
//Teensy 4.1 (Cortex-M7 @ 600MHz)


#ifndef IMXRT1062_H
#define IMXRT1062_H

#include <stdint.h>

//Core Cortex-M7 Registers


//System Control Block (SCB)
#define SCB_BASE            0xE000ED00UL
#define SCB_CPUID           (*(volatile uint32_t *)(SCB_BASE + 0x00))
#define SCB_ICSR            (*(volatile uint32_t *)(SCB_BASE + 0x04))
#define SCB_VTOR            (*(volatile uint32_t *)(SCB_BASE + 0x08))
#define SCB_AIRCR           (*(volatile uint32_t *)(SCB_BASE + 0x0C))
#define SCB_SCR             (*(volatile uint32_t *)(SCB_BASE + 0x10))
#define SCB_CCR             (*(volatile uint32_t *)(SCB_BASE + 0x14))
#define SCB_SHPR1           (*(volatile uint32_t *)(SCB_BASE + 0x18))
#define SCB_SHPR2           (*(volatile uint32_t *)(SCB_BASE + 0x1C))
#define SCB_SHPR3           (*(volatile uint32_t *)(SCB_BASE + 0x20))

// AIRCR bits 
#define SCB_AIRCR_VECTKEY       (0x05FA << 16)
#define SCB_AIRCR_SYSRESETREQ   (1 << 2)

// SysTick 
#define SYSTICK_BASE        0xE000E010UL
#define SYSTICK_CSR         (*(volatile uint32_t *)(SYSTICK_BASE + 0x00))
#define SYSTICK_RVR         (*(volatile uint32_t *)(SYSTICK_BASE + 0x04))
#define SYSTICK_CVR         (*(volatile uint32_t *)(SYSTICK_BASE + 0x08))
#define SYSTICK_CALIB       (*(volatile uint32_t *)(SYSTICK_BASE + 0x0C))

#define SYSTICK_CSR_ENABLE      (1 << 0)
#define SYSTICK_CSR_TICKINT     (1 << 1)
#define SYSTICK_CSR_CLKSOURCE   (1 << 2)
#define SYSTICK_CSR_COUNTFLAG   (1 << 16)

// NVIC 
#define NVIC_BASE           0xE000E100UL
#define NVIC_ISER(n)        (*(volatile uint32_t *)(NVIC_BASE + 0x000 + (n)*4))
#define NVIC_ICER(n)        (*(volatile uint32_t *)(NVIC_BASE + 0x080 + (n)*4))
#define NVIC_ISPR(n)        (*(volatile uint32_t *)(NVIC_BASE + 0x100 + (n)*4))
#define NVIC_ICPR(n)        (*(volatile uint32_t *)(NVIC_BASE + 0x180 + (n)*4))
#define NVIC_IABR(n)        (*(volatile uint32_t *)(NVIC_BASE + 0x200 + (n)*4))
#define NVIC_IPR(n)         (*(volatile uint8_t  *)(NVIC_BASE + 0x300 + (n)))

// Clock Control Module (CCM)
 
#define CCM_BASE            0x400FC000UL

// CCM General Purpose Register 
#define CCM_CSCMR1          (*(volatile uint32_t *)(CCM_BASE + 0x1C))
#define CCM_CSCMR2          (*(volatile uint32_t *)(CCM_BASE + 0x20))
#define CCM_CSCDR1          (*(volatile uint32_t *)(CCM_BASE + 0x24))

// CCM Clock Dividers 
#define CCM_CACRR           (*(volatile uint32_t *)(CCM_BASE + 0x10))
#define CCM_CBCDR           (*(volatile uint32_t *)(CCM_BASE + 0x14))
#define CCM_CBCMR           (*(volatile uint32_t *)(CCM_BASE + 0x18))

// CCM Clock Gating Registers
#define CCM_CCGR0           (*(volatile uint32_t *)(CCM_BASE + 0x68))
#define CCM_CCGR1           (*(volatile uint32_t *)(CCM_BASE + 0x6C))
#define CCM_CCGR2           (*(volatile uint32_t *)(CCM_BASE + 0x70))
#define CCM_CCGR3           (*(volatile uint32_t *)(CCM_BASE + 0x74))
#define CCM_CCGR4           (*(volatile uint32_t *)(CCM_BASE + 0x78))
#define CCM_CCGR5           (*(volatile uint32_t *)(CCM_BASE + 0x7C))
#define CCM_CCGR6           (*(volatile uint32_t *)(CCM_BASE + 0x80))

// Clock gating values 
#define CCM_CCGR_OFF        0x0
#define CCM_CCGR_ON_RUN     0x1
#define CCM_CCGR_ON         0x3

//CCM Analog (PLL Control)
 
#define CCM_ANALOG_BASE     0x400D8000UL

// ARM PLL (PLL1) - 1.2GHz 
#define CCM_ANALOG_PLL_ARM          (*(volatile uint32_t *)(CCM_ANALOG_BASE + 0x00))
#define CCM_ANALOG_PLL_ARM_SET      (*(volatile uint32_t *)(CCM_ANALOG_BASE + 0x04))
#define CCM_ANALOG_PLL_ARM_CLR      (*(volatile uint32_t *)(CCM_ANALOG_BASE + 0x08))

// System PLL (PLL2) - 528MHz 
#define CCM_ANALOG_PLL_SYS          (*(volatile uint32_t *)(CCM_ANALOG_BASE + 0x30))
#define CCM_ANALOG_PLL_SYS_SET      (*(volatile uint32_t *)(CCM_ANALOG_BASE + 0x34))
#define CCM_ANALOG_PLL_SYS_CLR      (*(volatile uint32_t *)(CCM_ANALOG_BASE + 0x38))

// USB1 PLL (PLL3) - 480MHz 
#define CCM_ANALOG_PLL_USB1         (*(volatile uint32_t *)(CCM_ANALOG_BASE + 0x10))
#define CCM_ANALOG_PLL_USB1_SET     (*(volatile uint32_t *)(CCM_ANALOG_BASE + 0x14))
#define CCM_ANALOG_PLL_USB1_CLR     (*(volatile uint32_t *)(CCM_ANALOG_BASE + 0x18))

// USB2 PLL (PLL7) - 480MHz 
#define CCM_ANALOG_PLL_USB2         (*(volatile uint32_t *)(CCM_ANALOG_BASE + 0x20))
#define CCM_ANALOG_PLL_USB2_SET     (*(volatile uint32_t *)(CCM_ANALOG_BASE + 0x24))
#define CCM_ANALOG_PLL_USB2_CLR     (*(volatile uint32_t *)(CCM_ANALOG_BASE + 0x28))

// Audio PLL (PLL4)/
#define CCM_ANALOG_PLL_AUDIO        (*(volatile uint32_t *)(CCM_ANALOG_BASE + 0x70))

// Video PLL (PLL5) 
#define CCM_ANALOG_PLL_VIDEO        (*(volatile uint32_t *)(CCM_ANALOG_BASE + 0xA0))

// ENET PLL (PLL6) 
#define CCM_ANALOG_PLL_ENET         (*(volatile uint32_t *)(CCM_ANALOG_BASE + 0xE0))

// PLL Control Bits
#define CCM_ANALOG_PLL_LOCK         (1 << 31)
#define CCM_ANALOG_PLL_BYPASS       (1 << 16)
#define CCM_ANALOG_PLL_ENABLE       (1 << 13)
#define CCM_ANALOG_PLL_POWERDOWN    (1 << 12)

// Misc registers 
#define CCM_ANALOG_MISC0            (*(volatile uint32_t *)(CCM_ANALOG_BASE + 0x150))
#define CCM_ANALOG_MISC1            (*(volatile uint32_t *)(CCM_ANALOG_BASE + 0x160))
#define CCM_ANALOG_MISC2            (*(volatile uint32_t *)(CCM_ANALOG_BASE + 0x170))

// DCDC Regulator

#define DCDC_BASE           0x40080000UL
#define DCDC_REG0           (*(volatile uint32_t *)(DCDC_BASE + 0x00))
#define DCDC_REG1           (*(volatile uint32_t *)(DCDC_BASE + 0x04))
#define DCDC_REG2           (*(volatile uint32_t *)(DCDC_BASE + 0x08))
#define DCDC_REG3           (*(volatile uint32_t *)(DCDC_BASE + 0x0C))

//IOMUXC - Pin Mux Control

#define IOMUXC_BASE         0x401F8000UL

// GPIO Pad Mux Registers (select alternate function) 
#define IOMUXC_SW_MUX_CTL_PAD_GPIO_AD_B0_00  (*(volatile uint32_t *)(IOMUXC_BASE + 0x0DC))
#define IOMUXC_SW_MUX_CTL_PAD_GPIO_AD_B0_01  (*(volatile uint32_t *)(IOMUXC_BASE + 0x0E0))
#define IOMUXC_SW_MUX_CTL_PAD_GPIO_AD_B0_02  (*(volatile uint32_t *)(IOMUXC_BASE + 0x0E4))
#define IOMUXC_SW_MUX_CTL_PAD_GPIO_AD_B0_03  (*(volatile uint32_t *)(IOMUXC_BASE + 0x0E8))
#define IOMUXC_SW_MUX_CTL_PAD_GPIO_AD_B0_12  (*(volatile uint32_t *)(IOMUXC_BASE + 0x108))
#define IOMUXC_SW_MUX_CTL_PAD_GPIO_AD_B0_13  (*(volatile uint32_t *)(IOMUXC_BASE + 0x10C))

// GPIO Pad Control Registers (drive strength, pull-up/down, etc.) 
#define IOMUXC_SW_PAD_CTL_PAD_GPIO_AD_B0_00  (*(volatile uint32_t *)(IOMUXC_BASE + 0x2CC))
#define IOMUXC_SW_PAD_CTL_PAD_GPIO_AD_B0_01  (*(volatile uint32_t *)(IOMUXC_BASE + 0x2D0))
#define IOMUXC_SW_PAD_CTL_PAD_GPIO_AD_B0_02  (*(volatile uint32_t *)(IOMUXC_BASE + 0x2D4))
#define IOMUXC_SW_PAD_CTL_PAD_GPIO_AD_B0_03  (*(volatile uint32_t *)(IOMUXC_BASE + 0x2D8))
#define IOMUXC_SW_PAD_CTL_PAD_GPIO_AD_B0_12  (*(volatile uint32_t *)(IOMUXC_BASE + 0x2F8))
#define IOMUXC_SW_PAD_CTL_PAD_GPIO_AD_B0_13  (*(volatile uint32_t *)(IOMUXC_BASE + 0x2FC))

// Pad config bits 
#define IOMUXC_PAD_SRE              (1 << 0)   // Slew Rate: 0=Slow, 1=Fast 
#define IOMUXC_PAD_DSE(n)           ((n) << 3) // Drive Strength: 0=disabled, 1-7
#define IOMUXC_PAD_SPEED(n)         ((n) << 6) // Speed: 0=50MHz, 1=100MHz, 2=150MHz, 3=200MHz 
#define IOMUXC_PAD_ODE              (1 << 11)  // Open Drain Enable 
#define IOMUXC_PAD_PKE              (1 << 12)  // Pull/Keep Enable 
#define IOMUXC_PAD_PUE              (1 << 13)  // Pull/Keep Select: 0=Keep, 1=Pull 
#define IOMUXC_PAD_PUS(n)           ((n) << 14)// Pull Up/Down: 0=100K down, 1=47K up, 2=100K up, 3=22K up 
#define IOMUXC_PAD_HYS              (1 << 16)  // Hysteresis Enable

// GPIO
 
#define GPIO1_BASE          0x401B8000UL
#define GPIO2_BASE          0x401BC000UL
#define GPIO3_BASE          0x401C0000UL
#define GPIO4_BASE          0x401C4000UL
#define GPIO5_BASE          0x400C0000UL
#define GPIO6_BASE          0x42000000UL
#define GPIO7_BASE          0x42004000UL
#define GPIO8_BASE          0x42008000UL
#define GPIO9_BASE          0x4200C000UL

typedef struct {
    volatile uint32_t DR;           // Data Register 
    volatile uint32_t GDIR;         // Direction Register 
    volatile uint32_t PSR;          // Pad Status Register 
    volatile uint32_t ICR1;         // Interrupt Config Register 1
    volatile uint32_t ICR2;         // Interrupt Config Register 2
    volatile uint32_t IMR;          // Interrupt Mask Register 
    volatile uint32_t ISR;          // Interrupt Status Register
    volatile uint32_t EDGE_SEL;     // Edge Select Registe
    uint32_t RESERVED[25];
    volatile uint32_t DR_SET;       // Data Set R
    volatile uint32_t DR_CLEAR;     // Data Clear Register 
    volatile uint32_t DR_TOGGLE;    // Data Toggle Register 
} GPIO_Type;

#define GPIO1               ((GPIO_Type *)GPIO1_BASE)
#define GPIO2               ((GPIO_Type *)GPIO2_BASE)
#define GPIO3               ((GPIO_Type *)GPIO3_BASE)
#define GPIO4               ((GPIO_Type *)GPIO4_BASE)
#define GPIO5               ((GPIO_Type *)GPIO5_BASE)
#define GPIO6               ((GPIO_Type *)GPIO6_BASE)
#define GPIO7               ((GPIO_Type *)GPIO7_BASE)
#define GPIO8               ((GPIO_Type *)GPIO8_BASE)
#define GPIO9               ((GPIO_Type *)GPIO9_BASE)

//LPUART (Low Power UART)
 
#define LPUART1_BASE        0x40184000UL
#define LPUART2_BASE        0x40188000UL
#define LPUART3_BASE        0x4018C000UL
#define LPUART4_BASE        0x40190000UL
#define LPUART5_BASE        0x40194000UL
#define LPUART6_BASE        0x40198000UL
#define LPUART7_BASE        0x4019C000UL
#define LPUART8_BASE        0x401A0000UL

typedef struct {
    volatile uint32_t VERID;        // Version ID Register*/
    volatile uint32_t PARAM;        // Parameter Register 
    volatile uint32_t GLOBAL;       // Global Register 
    volatile uint32_t PINCFG;       // Pin Configuration Register 
    volatile uint32_t BAUD;         // Baud Rate Register 
    volatile uint32_t STAT;         // Status Register 
    volatile uint32_t CTRL;         // Control Register 
    volatile uint32_t DATA;         // Data Register 
    volatile uint32_t MATCH;        // Match Address Register 
    volatile uint32_t MODIR;        //Modem IrDA Register 
    volatile uint32_t FIFO;         // FIFO Register
    volatile uint32_t WATER;        // Watermark Register
} LPUART_Type;

#define LPUART1             ((LPUART_Type *)LPUART1_BASE)
#define LPUART2             ((LPUART_Type *)LPUART2_BASE)
#define LPUART3             ((LPUART_Type *)LPUART3_BASE)
#define LPUART4             ((LPUART_Type *)LPUART4_BASE)
#define LPUART5             ((LPUART_Type *)LPUART5_BASE)
#define LPUART6             ((LPUART_Type *)LPUART6_BASE)
#define LPUART7             ((LPUART_Type *)LPUART7_BASE)
#define LPUART8             ((LPUART_Type *)LPUART8_BASE)

// LPUART BAUD register bits 
#define LPUART_BAUD_SBR(n)          ((n) & 0x1FFF)
#define LPUART_BAUD_SBNS            (1 << 13)   // Stop Bit Number Select 
#define LPUART_BAUD_RXEDGIE         (1 << 14)
#define LPUART_BAUD_LBKDIE          (1 << 15)
#define LPUART_BAUD_RESYNCDIS       (1 << 16)
#define LPUART_BAUD_BOTHEDGE        (1 << 17)
#define LPUART_BAUD_MATCFG(n)       ((n) << 18)
#define LPUART_BAUD_RDMAE           (1 << 21)
#define LPUART_BAUD_TDMAE           (1 << 23)
#define LPUART_BAUD_OSR(n)          (((n) & 0x1F) << 24)
#define LPUART_BAUD_M10             (1 << 29)
#define LPUART_BAUD_MAEN2           (1 << 30)
#define LPUART_BAUD_MAEN1           (1 << 31)

//LPUART STAT register bits /
#define LPUART_STAT_MA2F            (1 << 14)
#define LPUART_STAT_MA1F            (1 << 15)
#define LPUART_STAT_PF              (1 << 16)
#define LPUART_STAT_FE              (1 << 17)
#define LPUART_STAT_NF              (1 << 18)
#define LPUART_STAT_OR              (1 << 19)
#define LPUART_STAT_IDLE            (1 << 20)
#define LPUART_STAT_RDRF            (1 << 21)   // Receive Data Register Full 
#define LPUART_STAT_TC              (1 << 22)   // Transmission Complete
#define LPUART_STAT_TDRE            (1 << 23)   // Transmit Data Register Empty 
#define LPUART_STAT_RAF             (1 << 24)
#define LPUART_STAT_LBKDE           (1 << 25)
#define LPUART_STAT_BRK13           (1 << 26)
#define LPUART_STAT_RWUID           (1 << 27)
#define LPUART_STAT_RXINV           (1 << 28)
#define LPUART_STAT_MSBF            (1 << 29)
#define LPUART_STAT_RXEDGIF         (1 << 30)
#define LPUART_STAT_LBKDIF          (1 << 31)

//LPUART CTRL register bits /
#define LPUART_CTRL_PT              (1 << 0)    // Parity Type
#define LPUART_CTRL_PE              (1 << 1)    // Parity Enable 
#define LPUART_CTRL_ILT             (1 << 2)
#define LPUART_CTRL_WAKE            (1 << 3)
#define LPUART_CTRL_M               (1 << 4)    // 9-bit Mode 
#define LPUART_CTRL_RSRC            (1 << 5)
#define LPUART_CTRL_DOZEEN          (1 << 6)
#define LPUART_CTRL_LOOPS           (1 << 7)
#define LPUART_CTRL_IDLECFG(n)      ((n) << 8)
#define LPUART_CTRL_M7              (1 << 11)
#define LPUART_CTRL_MA2IE           (1 << 14)
#define LPUART_CTRL_MA1IE           (1 << 15)
#define LPUART_CTRL_SBK             (1 << 16)
#define LPUART_CTRL_RWU             (1 << 17)
#define LPUART_CTRL_RE              (1 << 18)   // Receiver Enable
#define LPUART_CTRL_TE              (1 << 19)   //Transmitter Enable 
#define LPUART_CTRL_ILIE            (1 << 20)
#define LPUART_CTRL_RIE             (1 << 21)   // Receive Interrupt Enable 
#define LPUART_CTRL_TCIE            (1 << 22)
#define LPUART_CTRL_TIE             (1 << 23)   // Transmit Interrupt Enable 
#define LPUART_CTRL_PEIE            (1 << 24)
#define LPUART_CTRL_FEIE            (1 << 25)
#define LPUART_CTRL_NEIE            (1 << 26)
#define LPUART_CTRL_ORIE            (1 << 27)
#define LPUART_CTRL_TXINV           (1 << 28)
#define LPUART_CTRL_TXDIR           (1 << 29)
#define LPUART_CTRL_R9T8            (1 << 30)
#define LPUART_CTRL_R8T9            (1 << 31)

// LPUART FIFO register bits 
#define LPUART_FIFO_RXFIFOSIZE(n)   (((n) >> 0) & 0x7)
#define LPUART_FIFO_RXFE            (1 << 3)    // Receive FIFO Enable 
#define LPUART_FIFO_TXFIFOSIZE(n)   (((n) >> 4) & 0x7)
#define LPUART_FIFO_TXFE            (1 << 7)    // Transmit FIFO Enable
#define LPUART_FIFO_RXUFE           (1 << 8)
#define LPUART_FIFO_TXOFE           (1 << 9)
#define LPUART_FIFO_RXIDEN(n)       ((n) << 10)
#define LPUART_FIFO_RXFLUSH         (1 << 14)
#define LPUART_FIFO_TXFLUSH         (1 << 15)
#define LPUART_FIFO_RXUF            (1 << 16)
#define LPUART_FIFO_TXOF            (1 << 17)
#define LPUART_FIFO_RXEMPT          (1 << 22)
#define LPUART_FIFO_TXEMPT          (1 << 23)

//FlexSPI (QSPI Flash Controller)
 
#define FLEXSPI_BASE        0x402A8000UL
#define FLEXSPI2_BASE       0x402A4000UL

//GPT (General Purpose Timer)

#define GPT1_BASE           0x401EC000UL
#define GPT2_BASE           0x401F0000UL

typedef struct {
    volatile uint32_t CR;           //Control Register 
    volatile uint32_t PR;           // Prescaler Register 
    volatile uint32_t SR;           // Status Register 
    volatile uint32_t IR;           // Interrupt Register 
    volatile uint32_t OCR1;         // Output Compare Register 1 
    volatile uint32_t OCR2;         //Output Compare Register 2 
    volatile uint32_t OCR3;         // Output Compare Register 3 
    volatile uint32_t ICR1;         // Input Capture Register 1 
    volatile uint32_t ICR2;         // Input Capture Register 2 
    volatile uint32_t CNT;          // Counter Register 
} GPT_Type;

#define GPT1                ((GPT_Type *)GPT1_BASE)
#define GPT2                ((GPT_Type *)GPT2_BASE)

// GPT Control Register bits 
#define GPT_CR_EN               (1 << 0)
#define GPT_CR_ENMOD            (1 << 1)
#define GPT_CR_DBGEN            (1 << 2)
#define GPT_CR_WAITEN           (1 << 3)
#define GPT_CR_DOZEEN           (1 << 4)
#define GPT_CR_STOPEN           (1 << 5)
#define GPT_CR_CLKSRC(n)        ((n) << 6)
#define GPT_CR_FRR              (1 << 9)
#define GPT_CR_EN_24M           (1 << 10)
#define GPT_CR_SWR              (1 << 15)
#define GPT_CR_IM1(n)           ((n) << 16)
#define GPT_CR_IM2(n)           ((n) << 18)
#define GPT_CR_OM1(n)           ((n) << 20)
#define GPT_CR_OM2(n)           ((n) << 23)
#define GPT_CR_OM3(n)           ((n) << 26)
#define GPT_CR_FO1              (1 << 29)
#define GPT_CR_FO2              (1 << 30)
#define GPT_CR_FO3              (1 << 31)

// Watchdog (WDOG)

#define WDOG1_BASE          0x400B8000UL
#define WDOG2_BASE          0x400D0000UL

typedef struct {
    volatile uint16_t WCR;          // Watchdog Control Register 
    volatile uint16_t WSR;          // Watchdog Service Register 
    volatile uint16_t WRSR;         // Watchdog Reset Status Register 
    volatile uint16_t WICR;         // Watchdog Interrupt Control Register 
    volatile uint16_t WMCR;         // Watchdog Misc Control Register 
} WDOG_Type;

#define WDOG1               ((WDOG_Type *)WDOG1_BASE)
#define WDOG2               ((WDOG_Type *)WDOG2_BASE)

// WDOG Control Register bits 
#define WDOG_WCR_WDZST          (1 << 0)
#define WDOG_WCR_WDBG           (1 << 1)
#define WDOG_WCR_WDE            (1 << 2)
#define WDOG_WCR_WDT            (1 << 3)
#define WDOG_WCR_SRS            (1 << 4)
#define WDOG_WCR_WDA            (1 << 5)
#define WDOG_WCR_SRE            (1 << 6)
#define WDOG_WCR_WDW            (1 << 7)
#define WDOG_WCR_WT(n)          ((n) << 8)

// WDOG unlock/service values
#define WDOG_UNLOCK_SEQ1        0xC520
#define WDOG_UNLOCK_SEQ2        0xD928
#define WDOG_REFRESH_SEQ1       0xA602
#define WDOG_REFRESH_SEQ2       0xB480

//RTWDOG (Real-Time Watchdog)
#define RTWDOG_BASE         0x400BC000UL

typedef struct {
    volatile uint32_t CS;           // Control and Status Register 
    volatile uint32_t CNT;          //Counter Register
    volatile uint32_t TOVAL;        // Timeout Value Register 
    volatile uint32_t WIN;          // Window Register 
} RTWDOG_Type;

#define RTWDOG              ((RTWDOG_Type *)RTWDOG_BASE)

#define RTWDOG_CS_STOP          (1 << 0)
#define RTWDOG_CS_WAIT          (1 << 1)
#define RTWDOG_CS_DBG           (1 << 2)
#define RTWDOG_CS_TST(n)        ((n) << 3)
#define RTWDOG_CS_UPDATE        (1 << 5)
#define RTWDOG_CS_INT           (1 << 6)
#define RTWDOG_CS_EN            (1 << 7)
#define RTWDOG_CS_CLK(n)        ((n) << 8)
#define RTWDOG_CS_RCS           (1 << 10)
#define RTWDOG_CS_ULK           (1 << 11)
#define RTWDOG_CS_PRES          (1 << 12)
#define RTWDOG_CS_CMD32EN       (1 << 13)
#define RTWDOG_CS_FLG           (1 << 14)
#define RTWDOG_CS_WIN           (1 << 15)

#define RTWDOG_UPDATE_KEY       0xD928C520
#define RTWDOG_REFRESH_KEY      0xB480A602

//Interrupt Numbers (IRQn)

typedef enum {
    // Cortex-M7 Internal Interrupts 
    NonMaskableInt_IRQn     = -14,
    HardFault_IRQn          = -13,
    MemoryManagement_IRQn   = -12,
    BusFault_IRQn           = -11,
    UsageFault_IRQn         = -10,
    SVCall_IRQn             = -5,
    DebugMonitor_IRQn       = -4,
    PendSV_IRQn             = -2,
    SysTick_IRQn            = -1,

    // IMXRT1062 External Interrupts /
    DMA0_IRQn               = 0,
    DMA1_IRQn               = 1,
    DMA2_IRQn               = 2,
    DMA3_IRQn               = 3,
    DMA4_IRQn               = 4,
    DMA5_IRQn               = 5,
    DMA6_IRQn               = 6,
    DMA7_IRQn               = 7,
    DMA8_IRQn               = 8,
    DMA9_IRQn               = 9,
    DMA10_IRQn              = 10,
    DMA11_IRQn              = 11,
    DMA12_IRQn              = 12,
    DMA13_IRQn              = 13,
    DMA14_IRQn              = 14,
    DMA15_IRQn              = 15,
    DMA_ERROR_IRQn          = 16,
    // more interrupts /
    LPUART1_IRQn            = 20,
    LPUART2_IRQn            = 21,
    LPUART3_IRQn            = 22,
    LPUART4_IRQn            = 23,
    LPUART5_IRQn            = 24,
    LPUART6_IRQn            = 25,
    LPUART7_IRQn            = 26,
    LPUART8_IRQn            = 27,
   
    GPT1_IRQn               = 100,
    GPT2_IRQn               = 101,
 
    GPIO1_Combined_0_15_IRQn    = 80,
    GPIO1_Combined_16_31_IRQn   = 81,
    GPIO2_Combined_0_15_IRQn    = 82,
    GPIO2_Combined_16_31_IRQn   = 83,
    GPIO3_Combined_0_15_IRQn    = 84,
    GPIO3_Combined_16_31_IRQn   = 85,
    GPIO4_Combined_0_15_IRQn    = 86,
    GPIO4_Combined_16_31_IRQn   = 87,
    GPIO5_Combined_0_15_IRQn    = 88,
    GPIO5_Combined_16_31_IRQn   = 89,
} IRQn_Type;

// Utility Macros

#define __IO    volatile
#define __I     volatile const
#define __O     volatile

// Memory barrier intrinsics 
#define __DSB() __asm volatile ("dsb 0xF" ::: "memory")
#define __ISB() __asm volatile ("isb 0xF" ::: "memory")
#define __DMB() __asm volatile ("dmb 0xF" ::: "memory")

//Interrupt control 
#define __disable_irq() __asm volatile ("cpsid i" ::: "memory")
#define __enable_irq()  __asm volatile ("cpsie i" ::: "memory")

static inline uint32_t __get_PRIMASK(void) {
    uint32_t result;
    __asm volatile ("mrs %0, primask" : "=r" (result));
    return result;
}

static inline void __set_PRIMASK(uint32_t value) {
    __asm volatile ("msr primask, %0" :: "r" (value) : "memory");
}

static inline uint32_t __get_MSP(void) {
    uint32_t result;
    __asm volatile ("mrs %0, msp" : "=r" (result));
    return result;
}

static inline void __set_MSP(uint32_t value) {
    __asm volatile ("msr msp, %0" :: "r" (value) : "memory");
}

static inline void __WFI(void) {
    __asm volatile ("wfi");
}

static inline void __WFE(void) {
    __asm volatile ("wfe");
}

static inline void __NOP(void) {
    __asm volatile ("nop");
}

//NVIC functions 
static inline void NVIC_EnableIRQ(IRQn_Type IRQn) {
    if ((int32_t)IRQn >= 0) {
        NVIC_ISER(IRQn >> 5) = (1UL << (IRQn & 0x1F));
    }
}

static inline void NVIC_DisableIRQ(IRQn_Type IRQn) {
    if ((int32_t)IRQn >= 0) {
        NVIC_ICER(IRQn >> 5) = (1UL << (IRQn & 0x1F));
    }
}

static inline void NVIC_SetPriority(IRQn_Type IRQn, uint32_t priority) {
    if ((int32_t)IRQn >= 0) {
        NVIC_IPR(IRQn) = (uint8_t)((priority << 4) & 0xFF);
    } else {
        //Core system handlers use SCB->SHPR 
    }
}

#endif // IMXRT1062_H 
