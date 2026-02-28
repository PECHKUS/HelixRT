# HelixRT

HelixRT is a custom embedded RTOS firmware project for **Teensy 4.1** (NXP i.MX RT1062, ARM Cortex-M7, 600 MHz target core clock).

It currently implements a **monolithic kernel** with:
- Preemptive **fixed-priority scheduling**
- Optional round-robin among equal priorities
- Static task/stack pools (no required heap)
- PendSV context switching + SysTick time base
- Core IPC/sync primitives (semaphore, mutex, queue, event flags)
- Software timers executed from tick context

## RTOS Type and Real-Time Profile

Current design profile:
- Kernel architecture: **Monolithic RTOS kernel**
- Scheduling model: **Priority-preemptive**, bitmap-ready-queue based
- Timing model: **Tick-driven** (`CONFIG_TICK_RATE_HZ`, default 1000)
- Real-time class today: **soft/firm-oriented baseline**, not yet hard real-time guaranteed

Why this matters:
- Deterministic behavior is already a design target.
- Hard real-time proof (bounded WCET + response-time guarantees) still requires additional architecture and verification work.

## What This Project Is For

HelixRT is useful for:
- Learning and validating RTOS internals on real Cortex-M7 hardware
- Experimenting with scheduler, context switch, and IPC design
- Building a custom firmware stack without vendor RTOS dependency
- Evolving toward stricter real-time behavior and cleaner kernel boundaries

## Repository Layout

- `src/`: startup, entry point, user syscall wrappers
- `kernel/`: core kernel, scheduler, timer, context switch assembly
- `kernel/sync/`: critical sections, semaphore, mutex, queue, event flags
- `hal/`: register definitions + minimal clock/GPIO/UART HAL
- `include/`: configuration and top-level include
- `docs/`: deep architecture and engineering docs
- `linker.ld`: memory and section placement
- `Makefile`: build, inspect, flash targets

## Toolchain and Prerequisites

Required tools:
- `arm-none-eabi-gcc`
- `arm-none-eabi-objdump`
- `arm-none-eabi-nm`
- `arm-none-eabi-size`
- `make`
- `teensy_loader_cli` (for `make flash`)

### Setup Commands by Host OS

Use the command set that matches your machine:

Ubuntu/Debian:
```bash
sudo apt update
sudo apt install -y gcc-arm-none-eabi binutils-arm-none-eabi make
```

Arch Linux:
```bash
sudo pacman -S --needed arm-none-eabi-gcc arm-none-eabi-binutils make
```

Fedora:
```bash
sudo dnf install -y arm-none-eabi-gcc-cs arm-none-eabi-binutils make
```

macOS (Homebrew):
```bash
brew install --cask gcc-arm-embedded
brew install make
```

Optional flash tool (if not already installed):
- Install `teensy_loader_cli` from your platform package manager or upstream build.

Verify tools:
```bash
arm-none-eabi-gcc --version
arm-none-eabi-objdump --version
make --version
```

## Build

```bash
make clean
make -j4
```

Generated artifacts:
- `build/helixrt.elf`
- `build/helixrt.hex`
- `build/helixrt.bin`
- `build/helixrt.map`

## Before Flash: Inspection Checklist and Commands

Run these checks before programming hardware.

### 1. Confirm vector table exists and is placed correctly
```bash
arm-none-eabi-objdump -s -j .vectors build/helixrt.elf | head -n 20
arm-none-eabi-objdump -h build/helixrt.elf | rg -n '\.vectors|\.flash_config|\.ivt'
```

### 2. Validate key exception handlers are linked
```bash
arm-none-eabi-nm -n build/helixrt.elf | rg 'Reset_Handler|HardFault_Handler|SVC_Handler|PendSV_Handler|SysTick_Handler'
```

### 3. Check section placement and memory footprint
```bash
arm-none-eabi-objdump -h build/helixrt.elf
arm-none-eabi-size build/helixrt.elf
```

### 4. Inspect startup, context switch, and syscall flow
```bash
arm-none-eabi-objdump -d -S build/helixrt.elf > build/helixrt.dis
rg -n 'Reset_Handler|PendSV_Handler|SVC_Handler|scheduler_tick' build/helixrt.dis
```

### 5. Verify linker symbols and memory region boundaries
```bash
rg '__vectors_start|__data_start|__data_load|__bss_start|__stack_top|__task_stacks_start|__tcb_pool_start' build/helixrt.map
```

### 6. Confirm expected task/scheduler symbols are present
```bash
arm-none-eabi-nm -n build/helixrt.elf | rg 'kernel_init|kernel_start|task_create|scheduler_add_task|scheduler_select_next_task'
```

## Flashing

```bash
make flash
```

This calls:
```bash
teensy_loader_cli --mcu=TEENSY41 -w -v build/helixrt.hex
```

Hardware notes:
- Board must be connected and bootloader-accessible.
- If flashing fails, press the Teensy program button and retry.

## Typical Bring-Up Flow

1. Build: `make clean && make -j4`
2. Run inspection checklist above
3. Flash: `make flash`
4. Observe runtime behavior (LED blink task + heartbeat task in `src/main.c`)
5. Iterate kernel and app logic

## Configuration Entry Point

Primary tunables are in `include/config.h`:
- Tick rate and core frequency assumptions
- Max tasks, priorities, stack sizes
- Preemption/round-robin settings
- Priority inheritance toggle
- Software timer and event-group feature toggles
- Hook enables and debug options

## Deep Documentation

- Architecture deep dive: `docs/ARCHITECTURE.md`
- File-by-file technical map: `docs/TECHNICAL_MAP.md`
- Evolution to deadline-accurate, microkernel-oriented, firm/hard RT: `docs/EVOLUTION.md`
