// HelixRT - UART HAL for IMXRT1062 (LPUART)


#ifndef UART_H
#define UART_H

#include "imxrt1062.h"

// UART configuration 
typedef struct {
    uint32_t baudrate;
    uint8_t data_bits;      // 7, 8, or 9 
    uint8_t stop_bits;      // 1 or 2 
    uint8_t parity;         // 0=none, 1=odd, 2=even 
} uart_config_t;

// Default configuration: 115200 8N1 
#define UART_CONFIG_DEFAULT { \
    .baudrate = 115200, \
    .data_bits = 8, \
    .stop_bits = 1, \
    .parity = 0 \
}



/* Initialize UART with given configuration
 * Assumes clock is already enabled and pins are configured
 * 
 * uart: pointer to LPUART peripheral (LPUART1-8)
 * config: UART configuration
 * clock_freq: input clock frequency to LPUART
 */
 
 
static inline void uart_init(LPUART_Type *uart, const uart_config_t *config, uint32_t clock_freq)
{
    // Reset UART 
    uart->GLOBAL |= (1 << 1);  // Software Reset 
    uart->GLOBAL &= ~(1 << 1);
    
    
    
    /* Calculate baud rate divisor
     * Baud = clock / (SBR * (OSR + 1))
     * Using OSR = 15 (16x oversampling)
     * SBR = clock / (baud * 16)
     */
     
     
    uint32_t osr = 15;  // 16x oversampling 
    uint32_t sbr = clock_freq / (config->baudrate * (osr + 1));
    
    // Configure baud rate 
    uart->BAUD = LPUART_BAUD_SBR(sbr) | LPUART_BAUD_OSR(osr);
    
    // Configure control register 
    uint32_t ctrl = 0;
    
    // Data bits (M bit for 9-bit mode) 
    if (config->data_bits == 9) {
        ctrl |= LPUART_CTRL_M;
    }
    
    // Parity 
    if (config->parity != 0) {
        ctrl |= LPUART_CTRL_PE;
        if (config->parity == 1) {
            ctrl |= LPUART_CTRL_PT;  // Odd parity 
        }
    }
    
    // Enable TX and RX 
    ctrl |= LPUART_CTRL_TE | LPUART_CTRL_RE;
    
    uart->CTRL = ctrl;
    
    // Configure stop bits
    if (config->stop_bits == 2) {
        uart->BAUD |= LPUART_BAUD_SBNS;
    }
}

// Send a single byte 
static inline void uart_putc(LPUART_Type *uart, uint8_t c)
{
    // Wait for transmit buffer empty 
    while (!(uart->STAT & LPUART_STAT_TDRE));
    uart->DATA = c;
}

// Receive a single byte (blocking) 
static inline uint8_t uart_getc(LPUART_Type *uart)
{
    // Wait for receive data ready 
    while (!(uart->STAT & LPUART_STAT_RDRF));
    return (uint8_t)uart->DATA;
}

// Check if data is available to read 
static inline int uart_available(LPUART_Type *uart)
{
    return (uart->STAT & LPUART_STAT_RDRF) ? 1 : 0;
}

// Check if transmit buffer is empty 
static inline int uart_tx_ready(LPUART_Type *uart)
{
    return (uart->STAT & LPUART_STAT_TDRE) ? 1 : 0;
}

// Send a string 
static inline void uart_puts(LPUART_Type *uart, const char *s)
{
    while (*s) {
        uart_putc(uart, *s++);
    }
}

// Send data buffer 
static inline void uart_write(LPUART_Type *uart, const uint8_t *data, uint32_t len)
{
    while (len--) {
        uart_putc(uart, *data++);
    }
}

#endif // UART_H 
