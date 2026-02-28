// HelixRT - GPIO HAL for IMXRT1062
 

#ifndef GPIO_H
#define GPIO_H

#include "imxrt1062.h"

// GPIO pin modes 
typedef enum {
    GPIO_MODE_INPUT = 0,
    GPIO_MODE_OUTPUT = 1
} gpio_mode_t;

// GPIO pull configuration 
typedef enum {
    GPIO_PULL_NONE = 0,
    GPIO_PULL_DOWN = 1,
    GPIO_PULL_UP = 2
} gpio_pull_t;

// Initialize a GPIO pin 
static inline void gpio_set_mode(GPIO_Type *gpio, uint32_t pin, gpio_mode_t mode)
{
    if (mode == GPIO_MODE_OUTPUT) {
        gpio->GDIR |= (1 << pin);
    } else {
        gpio->GDIR &= ~(1 << pin);
    }
}

// Set a GPIO pin high 
static inline void gpio_set(GPIO_Type *gpio, uint32_t pin)
{
    gpio->DR_SET = (1 << pin);
}

// Set a GPIO pin low 
static inline void gpio_clear(GPIO_Type *gpio, uint32_t pin)
{
    gpio->DR_CLEAR = (1 << pin);
}

// Toggle a GPIO pin
static inline void gpio_toggle(GPIO_Type *gpio, uint32_t pin)
{
    gpio->DR_TOGGLE = (1 << pin);
}

// Read a GPIO pin 
static inline uint32_t gpio_read(GPIO_Type *gpio, uint32_t pin)
{
    return (gpio->PSR >> pin) & 1;
}

// Write to a GPIO pin 
static inline void gpio_write(GPIO_Type *gpio, uint32_t pin, uint32_t value)
{
    if (value) {
        gpio->DR_SET = (1 << pin);
    } else {
        gpio->DR_CLEAR = (1 << pin);
    }
}

#endif // GPIO_H 
