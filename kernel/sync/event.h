// HelixRT - Event Group API


#ifndef EVENT_H
#define EVENT_H

#include <stdint.h>

typedef struct event_group {
    volatile uint32_t flags;
} event_group_t;

#define EVENT_WAIT_ANY          0U
#define EVENT_WAIT_ALL          1U
#define EVENT_CLEAR_ON_EXIT     (1UL << 31)

int event_init(event_group_t *eg);
int event_set(event_group_t *eg, uint32_t bits);
int event_clear(event_group_t *eg, uint32_t bits);
uint32_t event_get(event_group_t *eg);
uint32_t event_wait(event_group_t *eg, uint32_t bits, uint8_t wait_all, uint32_t timeout);

#endif // EVENT_H 
