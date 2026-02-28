# HelixRT Architecture Deep Dive

## 1. System Overview

HelixRT is a bare-metal RTOS firmware stack for i.MX RT1062 (Teensy 4.1) built around explicit control of:
- Boot chain and memory layout
- Scheduling and context switch mechanics
- Synchronization behavior under contention
- Interrupt-driven time base

Current architecture is **monolithic kernel in privileged mode** with C + assembly integration.

Core layers:
1. Application tasks (`src/main.c`)
2. Public API (`include/helixrt.h`, `kernel/*.h`)
3. Kernel runtime (`kernel/kernel.c`, `kernel/scheduler.c`, `kernel/context.s`)
4. HAL/register facade (`hal/*.h`)
5. MCU hardware (Cortex-M7 + i.MX RT1062 peripherals)

## 2. Boot Chain and Early Runtime

Boot sequence is anchored in `src/startup.c`:
1. Flash Config Block (`.flash_config`) and IVT/BootData (`.ivt`, `.boot_data`) satisfy i.MX RT ROM expectations.
2. Vector table (`.vectors`) defines core exceptions and external IRQ defaults.
3. `Reset_Handler` immediately disables watchdog sources (`WDOG1/WDOG2/RTWDOG`) and enables FPU CP10/CP11.
4. Startup copies:
- `.data` from flash load address to DTCM runtime address
- `.fast_code` from flash load address to ITCM runtime address
5. Startup zeros `.bss`.
6. `SCB_VTOR` is pointed at runtime vector table.
7. Control transfers to `main()`.

Rationale:
- Watchdog handling is done before any long init path to avoid early reset loops.
- Explicit data/code relocation keeps runtime behavior deterministic and inspectable.

## 3. Memory Architecture and Linker Contract

`linker.ld` defines memory regions:
- `FLASH_CONFIG`: `0x60000000`, 512B
- `FLASH_IVT`: `0x60000400`, IVT/boot metadata block
- `FLASH`: `0x60001000` onward, main XIP image
- `ITCM`: `0x00000000`, fast code runtime
- `DTCM`: `0x20000000`, fast data/kernel runtime
- `OCRAM2`: `0x20200000`, task stacks/pools/queues and future DMA buffers

Visual memory map (address-oriented):

```text
┌─────────────────────────────────────────────────────────────────────────────┐
│                         IMXRT1062 MEMORY LAYOUT                            │
├─────────────────────────────────────────────────────────────────────────────┤
│ FLASH (QSPI) @ 0x60000000, 8MB                                             │
│ 0x60000000  .flash_config   (512B, FCB for ROM boot)                       │
│ 0x60000400  .ivt            (Image Vector Table)                           │
│ 0x60000420  .boot_data      (Boot Data)                                    │
│ 0x60001000  .vectors        (runtime vector table)                         │
│ 0x60001000+ .text/.rodata/.ARM.* (XIP code and constants)                  │
│            load images for .fast_code and .data are also stored in FLASH   │
│ 0x607FFFFF  end of FLASH region                                             │
├─────────────────────────────────────────────────────────────────────────────┤
│ ITCM @ 0x00000000, 512KB                                                    │
│ 0x00000000  .fast_code      (runtime fast code, copied from FLASH)         │
│ 0x0007FFFF  end of ITCM                                                     │
├─────────────────────────────────────────────────────────────────────────────┤
│ DTCM @ 0x20000000, 512KB                                                    │
│ 0x20000000  .data           (initialized data, copied from FLASH)          │
│            .bss            (zero-initialized data)                         │
│            .sync_objects   (reserved section for sync placement)           │
│            .heap           (configured `_heap_size`, default 64KB)         │
│            .stack          (configured `_stack_size`, default 8KB, MSP)    │
│ 0x2007FFFF  end of DTCM                                                     │
├─────────────────────────────────────────────────────────────────────────────┤
│ OCRAM2 @ 0x20200000, 512KB                                                  │
│ 0x20200000  .task_stacks    (task stack pool, static stacks)               │
│            .tcb_pool       (task control block pool)                       │
│            .msg_queues     (message queue buffers)                         │
│            .dma_descriptors/.dma_buffers (reserved for DMA evolution)      │
│            .ocram2         (general OCRAM2 placement)                      │
│ 0x2027FFFF  end of OCRAM2                                                   │
└─────────────────────────────────────────────────────────────────────────────┘
```

Section location and symbol relationship summary:

| Section | Run Region | Load Region | Key Symbols |
|---|---|---|---|
| `.flash_config` | FLASH_CONFIG | FLASH_CONFIG | N/A |
| `.ivt` / `.boot_data` | FLASH_IVT | FLASH_IVT | N/A |
| `.vectors` | FLASH | FLASH | `__vectors_start`, `__vectors_end` |
| `.text`, `.rodata`, `.ARM.*` | FLASH | FLASH | `__text_start`, `__text_end` |
| `.fast_code` | ITCM | FLASH | `__fast_code_start`, `__fast_code_end`, `__fast_code_load` |
| `.data` | DTCM | FLASH | `__data_start`, `__data_end`, `__data_load` |
| `.bss` | DTCM | NOLOAD | `__bss_start`, `__bss_end` |
| `.heap` | DTCM | NOLOAD | `__heap_start`, `__heap_end` |
| `.stack` | DTCM | NOLOAD | `__stack_start`, `__stack_end`, `__stack_top` |
| `.task_stacks`, `.tcb_pool`, `.msg_queues`, `.dma_*`, `.ocram2` | OCRAM2 | NOLOAD | section-specific `__*_start/__*_end` |

Key section contracts:
- `.vectors` anchored in flash and referenced by `SCB_VTOR`
- `.fast_code` executes from ITCM after startup copy
- `.task_stacks`, `.tcb_pool`, `.msg_queues` are explicit pools for deterministic allocation

Why this split:
- ITCM/DTCM reduce latency for critical kernel code/data.
- OCRAM2 isolates larger pools from kernel-fast data path.
- Named linker symbols allow binary-level validation before flashing.

## 4. Kernel Lifecycle and Control Path

`kernel/kernel.c` implements lifecycle:
- `kernel_init()`:
- Initializes scheduler state
- Sets PendSV lowest and SysTick near-lowest priority via `SCB_SHPR3`
- Creates idle task
- `kernel_start()`:
- Configures SysTick reload from `SystemCoreClock / CONFIG_TICK_RATE_HZ`
- Transitions state to running
- Enters scheduler start path

Kernel state machine (`kernel_state_t`):
- `UNINIT -> INIT -> RUNNING` (with `STOPPED` reserved)

Design choice:
- Startup and scheduler ownership are separated: startup handles machine init, kernel handles runtime policy.

## 5. Task Model

Task metadata lives in `task_tcb_t` (`kernel/task.h`):
- `sp` at offset 0 (assembly context-switch requirement)
- scheduling fields (`priority`, `state`, queue links)
- blocking fields (`block_reason`, `block_object`, timeout/result)
- stack boundaries and bookkeeping

Task creation path (`task_create`):
1. Validate entry/priority/stack parameters
2. Allocate static TCB/stack from pools when caller passes NULL
3. Fill stack with known pattern
4. Build initial exception frame via `task_init_stack`
5. Add task to scheduler ready queue

Why static pools:
- Avoid heap nondeterminism and fragmentation in baseline kernel.
- Keep resource-failure mode explicit (`KERNEL_ERR_NO_MEM`).

## 6. Scheduler Design

`scheduler.c` implements:
- Priority-ready lists (`ready_list[CONFIG_MAX_PRIORITY]`)
- Priority bitmap for O(1)-class highest-priority lookup
- Blocked list with reason/object matching
- Tick accounting and timeout wakeups

Scheduling policy:
- Fixed-priority preemptive core
- Optional round-robin for same priority (`CONFIG_ROUND_ROBIN` + `time_slice`)

Critical operations:
- `scheduler_add_task`: inserts task and can trigger preemption
- `scheduler_block_task`: removes current task, marks blocked reason/object/timeout, yields
- `scheduler_unblock_one/all`: wake policies used by sync primitives
- `SysTick_Handler`: tick update, timer tick hook, kernel tick hook

Tradeoff:
- Global blocked-list scan is simple and maintainable but scales linearly with blocked task count.

## 7. Context Switching and Privileged Calls

Assembly entry points in `kernel/context.s`:
- `PendSV_Handler`:
- Saves outgoing software frame (`r4-r11`, `lr`) to PSP stack
- Calls `scheduler_select_next_task`
- Restores incoming software frame and PSP
- Returns via exception return path

- `SVC_Handler`:
- Detects active stack pointer (MSP/PSP)
- Decodes SVC immediate from instruction stream
- Passes args to `svc_dispatch`
- Writes return value back to stacked `r0`

`svc_dispatch` is implemented in `kernel/kernel.c` and maps SVC numbers to task APIs.

Why PendSV + SVC split:
- PendSV gives predictable low-priority context switch point.
- SVC provides controlled gateway for privileged kernel services.

## 8. Synchronization and IPC

### 8.1 Critical Sections
`kernel/sync/critical.h` uses PRIMASK-based enter/exit and optional BASEPRI helpers.

Purpose:
- Protect scheduler/object structures during short atomic regions.

### 8.2 Semaphores
`semaphore.c`:
- Counting semaphore with optional max count
- Blocking take via `scheduler_block_task(BLOCK_SEMAPHORE, sem, timeout)`
- ISR-safe give path

### 8.3 Mutexes
`mutex.c`:
- Ownership tracking
- Optional recursive locking
- Priority inheritance (`CONFIG_PRIORITY_INHERITANCE`)

### 8.4 Message Queues
`queue.c`:
- Fixed-size ring buffer (`head`, `tail`, `count`)
- Blocking send/receive with object-specific wakeups
- ISR non-blocking send path

### 8.5 Event Groups
`event.c`:
- Bit-flag wait semantics (ANY/ALL)
- Optional clear-on-exit flag protocol
- Wait implemented as block/recheck loop

Connection model:
- All sync primitives converge to scheduler block/unblock APIs.
- Reason/object tags unify waiting semantics across primitives.

## 9. Software Timers

`kernel/timer.c` provides a tick-driven software timer list:
- Create/start/stop APIs
- Periodic and one-shot behavior
- Executed in SysTick ISR context

Important behavior:
- Timer callbacks run from interrupt context and must be short, non-blocking, and allocation-free.

## 10. HAL Boundary

HAL headers in `hal/` are thin and register-centric:
- `imxrt1062.h`: register maps, bit masks, utility intrinsics
- `clock.h`: baseline clock gate setup and core frequency publication
- `gpio.h`: inline pin IO helpers
- `uart.h`: minimal polling-mode LPUART helpers

Why thin HAL:
- Keeps deterministic visibility into hardware access patterns.
- Avoids opaque vendor runtime dependencies.

## 11. End-to-End Runtime Flow

Nominal runtime path:
1. `Reset_Handler` initializes machine + memory.
2. `main()` initializes clock/board and kernel.
3. Tasks are created and enqueued.
4. `kernel_start()` enables SysTick and starts first task.
5. Tasks run, block on delay/IPC/events, and resume.
6. PendSV performs context switches.
7. Sync objects wake blocked tasks via reason/object matching.

## 12. Constraints and Current Risks

Observed constraints in current codebase:
- Timer callbacks in ISR context can increase jitter if heavy.
- Blocked wait lists are global scans, which can increase latency under high blocked-task counts.
- No MPU-enforced isolation yet; all kernel/tasks are in shared protection domain.
- Hard real-time guarantees are not yet formally established.

## 13. Where To Read Next

- `docs/TECHNICAL_MAP.md` for file-by-file mapping and command-level inspection flow
- `docs/EVOLUTION.md` for microkernel and hard/firm RT evolution strategy
