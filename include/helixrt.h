// HelixRT - Main User Include

// Include this single header to access all RTOS functionality.

#ifndef HELIXRT_H
#define HELIXRT_H

// Version 
#define HELIXRT_VERSION_MAJOR   0
#define HELIXRT_VERSION_MINOR   1
#define HELIXRT_VERSION_PATCH   0
#define HELIXRT_VERSION_STRING  "0.1.0"

// Configuration 
#include "config.h"

// Kernel API 
#include "../kernel/kernel.h"
#include "../kernel/task.h"
#include "../kernel/scheduler.h"

// Synchronization Primitives 
#include "../kernel/sync/critical.h"
#include "../kernel/sync/semaphore.h"
#include "../kernel/sync/mutex.h"
#include "../kernel/sync/queue.h"
#include "../kernel/sync/event.h"
#include "../kernel/timer.h"

// HAL 
#include "../hal/imxrt1062.h"
#include "../hal/clock.h"
#include "../hal/gpio.h"
#include "../hal/uart.h"

#endif // HELIXRT_H 
