//HelixRT - Message Queue Implementation
 

#include <stdint.h>
#include <stddef.h>
#include "../../include/config.h"
#include "queue.h"
#include "../kernel.h"
#include "../scheduler.h"
#include "critical.h"

static void mem_copy(uint8_t *dst, const uint8_t *src, uint32_t len)
{
    while (len-- > 0U) {
        *dst++ = *src++;
    }
}

static uint8_t *queue_slot_ptr(msg_queue_t *queue, uint32_t idx)
{
    return &queue->buffer[idx * queue->msg_size];
}

static void queue_push_back(msg_queue_t *queue, const void *msg)
{
    mem_copy(queue_slot_ptr(queue, queue->head), (const uint8_t *)msg, queue->msg_size);
    queue->head = (queue->head + 1U) % queue->capacity;
    queue->count++;
}

static void queue_push_front(msg_queue_t *queue, const void *msg)
{
    if (queue->tail == 0U) {
        queue->tail = queue->capacity - 1U;
    } else {
        queue->tail--;
    }
    mem_copy(queue_slot_ptr(queue, queue->tail), (const uint8_t *)msg, queue->msg_size);
    queue->count++;
}

static void queue_pop(msg_queue_t *queue, void *msg)
{
    mem_copy((uint8_t *)msg, queue_slot_ptr(queue, queue->tail), queue->msg_size);
    queue->tail = (queue->tail + 1U) % queue->capacity;
    queue->count--;
}

int queue_init(msg_queue_t *queue, void *buffer, uint32_t msg_size, uint32_t capacity)
{
    if (queue == NULL || buffer == NULL || msg_size == 0U || capacity == 0U) {
        return KERNEL_ERR_PARAM;
    }

    queue->buffer = (uint8_t *)buffer;
    queue->msg_size = msg_size;
    queue->capacity = capacity;
    queue->head = 0;
    queue->tail = 0;
    queue->count = 0;
    queue->send_wait_head = NULL;
    queue->send_wait_tail = NULL;
    queue->recv_wait_head = NULL;
    queue->recv_wait_tail = NULL;
    return KERNEL_OK;
}

int queue_send(msg_queue_t *queue, const void *msg, uint32_t timeout)
{
    uint32_t irq_state;
    int res;

    if (queue == NULL || msg == NULL) {
        return KERNEL_ERR_PARAM;
    }

    while (1) {
        irq_state = critical_enter();
        if (queue->count < queue->capacity) {
            queue_push_back(queue, msg);
            (void)scheduler_unblock_one(BLOCK_QUEUE_RECV, queue, KERNEL_OK);
            critical_exit(irq_state);
            return KERNEL_OK;
        }
        critical_exit(irq_state);

        if (timeout == TIMEOUT_NONE) {
            return KERNEL_ERR_TIMEOUT;
        }
        if (is_isr_context()) {
            return KERNEL_ERR_ISR;
        }

        res = scheduler_block_task(BLOCK_QUEUE_SEND, queue, timeout);
        if (res != KERNEL_OK) {
            return res;
        }
    }
}

int queue_send_front(msg_queue_t *queue, const void *msg, uint32_t timeout)
{
    uint32_t irq_state;
    int res;

    if (queue == NULL || msg == NULL) {
        return KERNEL_ERR_PARAM;
    }

    while (1) {
        irq_state = critical_enter();
        if (queue->count < queue->capacity) {
            queue_push_front(queue, msg);
            (void)scheduler_unblock_one(BLOCK_QUEUE_RECV, queue, KERNEL_OK);
            critical_exit(irq_state);
            return KERNEL_OK;
        }
        critical_exit(irq_state);

        if (timeout == TIMEOUT_NONE) {
            return KERNEL_ERR_TIMEOUT;
        }
        if (is_isr_context()) {
            return KERNEL_ERR_ISR;
        }

        res = scheduler_block_task(BLOCK_QUEUE_SEND, queue, timeout);
        if (res != KERNEL_OK) {
            return res;
        }
    }
}

int queue_send_isr(msg_queue_t *queue, const void *msg)
{
    uint32_t irq_state;

    if (queue == NULL || msg == NULL) {
        return KERNEL_ERR_PARAM;
    }

    irq_state = critical_enter();
    if (queue->count >= queue->capacity) {
        critical_exit(irq_state);
        return KERNEL_ERR_OVERFLOW;
    }

    queue_push_back(queue, msg);
    (void)scheduler_unblock_one(BLOCK_QUEUE_RECV, queue, KERNEL_OK);
    critical_exit(irq_state);
    return KERNEL_OK;
}

int queue_receive(msg_queue_t *queue, void *msg, uint32_t timeout)
{
    uint32_t irq_state;
    int res;

    if (queue == NULL || msg == NULL) {
        return KERNEL_ERR_PARAM;
    }

    while (1) {
        irq_state = critical_enter();
        if (queue->count > 0U) {
            queue_pop(queue, msg);
            (void)scheduler_unblock_one(BLOCK_QUEUE_SEND, queue, KERNEL_OK);
            critical_exit(irq_state);
            return KERNEL_OK;
        }
        critical_exit(irq_state);

        if (timeout == TIMEOUT_NONE) {
            return KERNEL_ERR_TIMEOUT;
        }
        if (is_isr_context()) {
            return KERNEL_ERR_ISR;
        }

        res = scheduler_block_task(BLOCK_QUEUE_RECV, queue, timeout);
        if (res != KERNEL_OK) {
            return res;
        }
    }
}

int queue_peek(msg_queue_t *queue, void *msg, uint32_t timeout)
{
    uint32_t irq_state;
    int res;

    if (queue == NULL || msg == NULL) {
        return KERNEL_ERR_PARAM;
    }

    while (1) {
        irq_state = critical_enter();
        if (queue->count > 0U) {
            mem_copy((uint8_t *)msg, queue_slot_ptr(queue, queue->tail), queue->msg_size);
            critical_exit(irq_state);
            return KERNEL_OK;
        }
        critical_exit(irq_state);

        if (timeout == TIMEOUT_NONE) {
            return KERNEL_ERR_TIMEOUT;
        }
        if (is_isr_context()) {
            return KERNEL_ERR_ISR;
        }

        res = scheduler_block_task(BLOCK_QUEUE_RECV, queue, timeout);
        if (res != KERNEL_OK) {
            return res;
        }
    }
}

uint32_t queue_get_count(msg_queue_t *queue)
{
    if (queue == NULL) {
        return 0;
    }
    return queue->count;
}

uint32_t queue_get_space(msg_queue_t *queue)
{
    if (queue == NULL || queue->capacity < queue->count) {
        return 0;
    }
    return queue->capacity - queue->count;
}

int queue_reset(msg_queue_t *queue)
{
    if (queue == NULL) {
        return KERNEL_ERR_PARAM;
    }

    queue->head = 0;
    queue->tail = 0;
    queue->count = 0;
    (void)scheduler_unblock_all(BLOCK_QUEUE_SEND, queue, KERNEL_ERR_STATE);
    (void)scheduler_unblock_all(BLOCK_QUEUE_RECV, queue, KERNEL_ERR_STATE);
    return KERNEL_OK;
}
