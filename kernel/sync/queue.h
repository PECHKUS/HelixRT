// HelixRT - Message Queue API
 // FIFO message queues for inter-task communication.


#ifndef QUEUE_H
#define QUEUE_H

#include <stdint.h>
#include "../task.h"

// Message Queue Structure


typedef struct msg_queue {
    uint8_t *buffer;                  
    uint32_t msg_size;                 
    uint32_t capacity;                 
    volatile uint32_t head;            
    volatile uint32_t tail;             
    volatile uint32_t count;            
    task_tcb_t *send_wait_head;         
    task_tcb_t *send_wait_tail;
    task_tcb_t *recv_wait_head;        
    task_tcb_t *recv_wait_tail;
} msg_queue_t;

// Message Queue API

/*
 * queue_init - Initialize a message queue
 * 
 * @queue:    Pointer to queue structure
 * @buffer:   Pointer to message buffer (msg_size * capacity bytes)
 * @msg_size: Size of each message in bytes
 * @capacity: Maximum number of messages
 * 
 * Returns: KERNEL_OK or error code
 */
 
int queue_init(msg_queue_t *queue, void *buffer, uint32_t msg_size, uint32_t capacity);

/*
 * queue_send - Send a message to the queue (FIFO)
 * 
 * Copies message to queue. If queue is full, blocks until space available.
 * 
 * @queue:   Queue to send to
 * @msg:     Pointer to message data
 * @timeout: Timeout in ticks (0 = no wait, UINT32_MAX = infinite)
 * 
 * Returns: KERNEL_OK, KERNEL_ERR_TIMEOUT, or error code
 */
 
int queue_send(msg_queue_t *queue, const void *msg, uint32_t timeout);

/*
 * queue_send_front - Send a message to front of queue (LIFO behavior)
 * 
 * For urgent messages that should be processed first.
 * 
 * @queue:   Queue to send to
 * @msg:     Pointer to message data
 * @timeout: Timeout in ticks
 * 
 * Returns: KERNEL_OK, KERNEL_ERR_TIMEOUT, or error code
 */
 
 
int queue_send_front(msg_queue_t *queue, const void *msg, uint32_t timeout);

/*
 * queue_send_isr - Send a message from ISR context
 * 
 * Non-blocking. Returns error if queue is full.
 * 
 * @queue: Queue to send to
 * @msg:   Pointer to message data
 * 
 * Returns: KERNEL_OK or KERNEL_ERR_OVERFLOW
 */
 
 
int queue_send_isr(msg_queue_t *queue, const void *msg);

/*
 * queue_receive - Receive a message from the queue
 * 
 * Copies message from queue to buffer. If queue is empty, blocks.
 * 
 * @queue:   Queue to receive from
 * @msg:     Buffer to copy message into
 * @timeout: Timeout in ticks (0 = no wait, UINT32_MAX = infinite)
 * 
 * Returns: KERNEL_OK, KERNEL_ERR_TIMEOUT, or error code
 */
 
 
int queue_receive(msg_queue_t *queue, void *msg, uint32_t timeout);

/*
 * queue_peek - Peek at front message without removing it
 * 
 * @queue:   Queue to peek
 * @msg:     Buffer to copy message into
 * @timeout: Timeout in ticks
 * 
 * Returns: KERNEL_OK, KERNEL_ERR_TIMEOUT, or error code
 */
 
 
int queue_peek(msg_queue_t *queue, void *msg, uint32_t timeout);

/*
 * queue_get_count - Get number of messages in queue
 * 
 * @queue: Queue to query
 * 
 * Returns: Number of messages currently in queue
 */
 
 
uint32_t queue_get_count(msg_queue_t *queue);

/*
 * queue_get_space - Get available space in queue
 * 
 * @queue: Queue to query
 * 
 * Returns: Number of messages that can be sent without blocking
 */
 
 
uint32_t queue_get_space(msg_queue_t *queue);

/*
 * queue_is_empty - Check if queue is empty
 * 
 * @queue: Queue to query
 * 
 * Returns: 1 if empty, 0 if not
 */
 
static inline int queue_is_empty(msg_queue_t *queue)
{
    return queue->count == 0;
}

/*
 * queue_is_full - Check if queue is full
 * 
 * @queue: Queue to query
 * 
 * Returns: 1 if full, 0 if not
 */
 
static inline int queue_is_full(msg_queue_t *queue)
{
    return queue->count >= queue->capacity;
}

/*
 * queue_reset - Reset queue to empty state
 * 
 * Wakes all waiting tasks with error.
 * 
 * @queue: Queue to reset
 * 
 * Returns: KERNEL_OK or error code
 */
 
int queue_reset(msg_queue_t *queue);

//Static Queue Allocation
 
#define QUEUE_STATIC_DEFINE(name, type, size)                           \
    static uint8_t name##_buffer[sizeof(type) * (size)]                 \
        __attribute__((section(".msg_queues"), aligned(4)));            \
    static msg_queue_t name = {                                         \
        .buffer = name##_buffer,                                        \
        .msg_size = sizeof(type),                                       \
        .capacity = (size),                                             \
        .head = 0,                                                      \
        .tail = 0,                                                      \
        .count = 0,                                                     \
        .send_wait_head = NULL,                                         \
        .send_wait_tail = NULL,                                         \
        .recv_wait_head = NULL,                                         \
        .recv_wait_tail = NULL                                          \
    }

/*
 * Mailbox (Single-Message Queue) Helper
 * 
 * A mailbox is a queue with capacity=1, useful for signaling with data.
 */

#define MAILBOX_STATIC_DEFINE(name, type) QUEUE_STATIC_DEFINE(name, type, 1)

#endif // QUEUE_H 
