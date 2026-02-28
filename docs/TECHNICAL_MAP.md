# HelixRT Technical Map

This document is a practical map of what exists in this repository right now, how modules connect, and how to inspect each layer before flashing.

## 1. Directory-by-Directory Inventory

### `src/`
- `startup.c`
- Boot metadata (`.flash_config`, `.ivt`, `.boot_data`), vector table, reset flow, weak default handlers
- Critical early duties: watchdog disable, FPU enable, section relocation/zeroing, VTOR setup
- `main.c`
- Example board bring-up (GPIO LED) and two demo tasks (`blink`, `heartbeat`)
- `syscall.c`
- User-side SVC wrappers (`task_yield`, `task_delay`, suspend/resume)

### `kernel/`
- `kernel.h`
- Public kernel API and status codes
- `kernel.c`
- Kernel lifecycle, task creation/deletion/suspend/resume, SVC dispatcher, stack frame init
- `task.h`
- TCB definition, task state/reason enums, static task macros
- `scheduler.h`
- Internal scheduler API and context-switch globals
- `scheduler.c`
- Ready queues, blocked list, tick handling, unblock policies, preemption lock
- `context.s`
- PendSV context save/restore and SVC dispatch bridge
- `syscall.h`
- SVC number definitions
- `timer.h` / `timer.c`
- Tick-driven software timers

### `kernel/sync/`
- `critical.h` / `critical.c`
- Critical section primitives (PRIMASK + optional BASEPRI helpers)
- `semaphore.h` / `semaphore.c`
- Counting/binary semaphore implementation
- `mutex.h` / `mutex.c`
- Mutex with owner tracking, recursion option, priority inheritance
- `queue.h` / `queue.c`
- Ring-buffer message queues with blocking send/receive
- `event.h` / `event.c`
- Event flag groups with wait-any/wait-all semantics

### `hal/`
- `imxrt1062.h`
- Register-level MCU definitions and intrinsic helpers
- `clock.h`
- Core clock publication and selective gate enables
- `gpio.h`
- Inline GPIO control
- `uart.h`
- Inline polling UART init/read/write helpers

### `include/`
- `config.h`
- Central compile-time kernel configuration
- `helixrt.h`
- Single top-level include aggregating kernel, sync, timer, and HAL

### Top-level build/config files
- `Makefile`
- Toolchain flags, object list, build targets, flash target
- `linker.ld`
- Flash/ITCM/DTCM/OCRAM2 section ownership and linker symbols
- `LICENSE`

### `docs/`
- `ARCHITECTURE.md`
- Deep structural explanation
- `TECHNICAL_MAP.md`
- This operational map
- `EVOLUTION.md`
- Future architecture path to deadline-accurate and microkernel-oriented system

### Generated artifacts (`build/`)
- `*.elf`, `*.hex`, `*.bin`, `*.map`, `*.o`
- Build outputs; not source of truth for design

## 2. Module Connection Map

## 2.1 Control Path

1. ROM boot -> `startup.c` metadata and `Reset_Handler`
2. `main()` initializes board + kernel
3. `kernel_init()` initializes scheduler and idle task
4. `task_create()` registers runnable tasks
5. `kernel_start()` enables SysTick and launches scheduler
6. Runtime switching occurs via PendSV (`context.s`)

## 2.2 Data Path for Blocking APIs

1. Task calls primitive (`sem_take`, `mutex_lock`, `queue_receive`, `event_wait`)
2. Primitive checks local object state under critical section
3. If unavailable, primitive calls `scheduler_block_task(reason, object, timeout)`
4. Another task/ISR calls wake path (`sem_give`, `queue_send`, `event_set`, etc.)
5. Scheduler unblocks one/all matching tasks by `(reason, object)`
6. Unblocked task resumes and returns with result code

## 2.3 Time Path

1. SysTick fires at `CONFIG_TICK_RATE_HZ`
2. `scheduler_tick()` updates global tick and wakeups
3. Optional `timer_tick_isr()` processes software timer list
4. `kernel_tick_hook()` executes application hook

## 3. Why Each Part Exists

- `startup.c`
- Owns machine-level truth: boot metadata, memory init, vector routing
- `linker.ld`
- Turns memory architecture into enforceable placement contract
- Scheduler + `context.s`
- Separates policy (C) from low-level context mechanics (assembly)
- Sync primitives
- Reuse scheduler block/unblock core instead of per-object wait queues
- Static pools (`.task_stacks`, `.tcb_pool`)
- Deterministic allocation and failure behavior without heap reliance
- Thin HAL
- Keeps hardware operations explicit and testable at register level

## 4. Build and Verification Workflow

## 4.1 Clean Build

```bash
make clean
make -j4
```

## 4.2 Fast Structural Checks

```bash
arm-none-eabi-size build/helixrt.elf
arm-none-eabi-objdump -h build/helixrt.elf
arm-none-eabi-nm -n build/helixrt.elf | rg 'Reset_Handler|SVC_Handler|PendSV_Handler|SysTick_Handler'
```

## 4.3 Boot and Memory Integrity Checks

```bash
arm-none-eabi-objdump -s -j .flash_config build/helixrt.elf | head -n 20
arm-none-eabi-objdump -s -j .ivt build/helixrt.elf | head -n 20
arm-none-eabi-objdump -s -j .vectors build/helixrt.elf | head -n 20
rg '__data_start|__data_load|__bss_start|__stack_top|__task_stacks_start|__msg_queues_start' build/helixrt.map
```

## 4.4 Scheduler and IPC Symbol Presence

```bash
arm-none-eabi-nm -n build/helixrt.elf | rg 'scheduler_|task_create|task_delete|sem_|mutex_|queue_|event_'
```

## 4.5 Disassembly Inspection

```bash
arm-none-eabi-objdump -d -S build/helixrt.elf > build/helixrt.dis
rg -n 'Reset_Handler|PendSV_Handler|SVC_Handler|scheduler_tick|timer_tick_isr' build/helixrt.dis
```

## 5. Flash Workflow

```bash
make flash
```

Requires:
- Connected Teensy 4.1
- `teensy_loader_cli`
- Bootloader/program mode as needed

## 6. Configuration Knobs That Change Behavior

`include/config.h` directly influences:
- Tick period and CPU-clock assumptions
- Task count limits and stack defaults
- Preemption and round-robin behavior
- Priority inheritance policy
- Software timer/event-group inclusion
- Debug/hook/assert behavior

When changing these, always rebuild and re-run section/symbol checks because memory layout and timing behavior can shift.

## 7. Current Implementation Boundary

Implemented in code now:
- Boot metadata + reset path
- Priority scheduler + context switching
- Task lifecycle API
- Semaphore/mutex/queue/event primitives
- Software timers

Not yet implemented as full subsystem:
- MPU process/task isolation
- Capability-based service boundaries
- Tickless scheduling and formal WCET/response-time analysis
- Complete peripheral-driver suite (DMA/I2C/SPI/ADC integration)

## 8. Practical Traceability: Requirement -> File

- Boot integrity and image layout: `src/startup.c`, `linker.ld`
- Scheduling behavior and latency: `kernel/scheduler.c`, `kernel/context.s`
- Task state and stack model: `kernel/task.h`, `kernel/kernel.c`
- IPC correctness: `kernel/sync/*.c`
- ISR-time effects: `kernel/scheduler.c`, `kernel/timer.c`
- Hardware register behavior: `hal/imxrt1062.h`

Use this mapping when debugging to avoid searching unrelated modules.
