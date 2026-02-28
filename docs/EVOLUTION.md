# HelixRT Evolution Plan: Toward Deadline-Accurate, Firm/Hard RT, and Microkernel Direction

This document explains how the current HelixRT baseline can evolve into a more deadline-accurate system and, over time, a microkernel-oriented architecture suitable for firm/hard real-time classes.

## 1. Current Baseline (What Exists Today)

Current kernel characteristics:
- Monolithic privileged kernel
- Tick-based fixed-priority preemptive scheduling
- Blocking primitives through centralized scheduler
- Static pool allocation for tasks/stacks
- ISR-driven timer callbacks

This is a baseline for deterministic behavior, but it is not yet a proven hard real-time kernel.

## 2. What "Deadline-Accurate" Requires

To claim stronger deadline behavior, HelixRT needs all of the following:
- Defined task model: period, deadline, WCET, criticality class
- Bounded interrupt disable regions
- Bounded scheduler path length per event
- Bounded IPC operations under worst contention
- Time-source precision and jitter characterization
- Measurement + analysis loop (not only code structure)

## 3. Roadmap Phase 1: Strengthen Monolithic Kernel Determinism

Phase goal: make current architecture more predictable before structural split.

Work items:
- Add cycle-accurate trace hooks in scheduler/context paths
- Measure max time in:
- PendSV switch
- SysTick processing
- semaphore/mutex/queue hot and contended paths
- Replace global blocked-list scans with indexed wait structures by object/reason
- Introduce bounded critical-section policy and assert on excessive hold times
- Move timer callback execution out of SysTick ISR into a dedicated timer service task
- Add static response-time analysis inputs to each task descriptor

Expected result:
- Stronger firm real-time behavior and quantified latency budgets.

## 4. Roadmap Phase 2: Scheduling for Deadlines

Phase goal: improve deadline success under mixed workloads.

Candidate scheduling upgrades:
- Keep fixed-priority for simple control loops
- Add optional RM/DM assignment helpers (rate/deadline monotonic)
- Add optional EDF scheduler class for dynamic-deadline workloads
- Use admission control for periodic task sets

Required supporting changes:
- Task metadata extension: period/deadline/WCET
- Overrun detection and handling policy
- Per-task execution budget accounting

Expected result:
- Better deadline adherence and explicit overload behavior.

## 5. Roadmap Phase 3: Toward Hard/Firm Real-Time Grades

Define real-time grades in project policy:
- Soft RT profile: occasional miss acceptable
- Firm RT profile: late result is discarded, miss rate bounded
- Hard RT profile: misses treated as system-level failure

Engineering controls required for hard profile:
- Stable clock/timer calibration and drift budget
- Worst-case interrupt storm testing
- Priority inversion proofs (including nested lock cases)
- WCET estimates for all kernel services used in critical paths
- Deterministic fault handling and safe degraded mode

Verification strategy:
- Repeatable stress tests on target hardware
- Trace-based latency histograms + worst-case captures
- Requirement-to-test traceability matrix in CI artifacts

## 6. Microkernel-Oriented Evolution Path

HelixRT is currently monolithic. A practical migration path is incremental, not a full rewrite.

## 6.1 Introduce Service Boundaries First

Start by defining internal service interfaces while still in one binary:
- Scheduler service
- IPC service
- Timer service
- Device service facade

Use message-based internal APIs between these modules even before protection separation.

## 6.2 Add Protection and Fault Containment

Next steps:
- Enable MPU regions for user tasks vs kernel memory
- Move selected drivers/services to unprivileged tasks
- Route requests through syscall/message interfaces

## 6.3 Convert Selected Subsystems to User-Space Servers

Recommended order:
1. Logging/diagnostics server
2. Timer service server
3. Device drivers with clear request/response model
4. Filesystem/network (if added later)

Kernel then shrinks to microkernel core responsibilities:
- Scheduling
- IPC transport
- Interrupt routing primitives
- Memory/protection control

## 6.4 Real-Time Impact of Microkernel Move

Benefits:
- Better fault isolation
- Cleaner reasoning about trust boundaries
- More testable subsystems

Costs and risks:
- IPC overhead on critical paths
- More context switches
- More complex priority management across servers

Mitigation:
- Zero-copy/message-buffer pools
- Priority-aware IPC and priority inheritance across endpoints
- Keep hardest real-time loops in kernel or tightly controlled privileged services

## 7. Architectural Recommendations by Priority

Highest priority (next practical steps):
1. Instrumentation and latency measurements
2. Timer-callback deferral to timer task
3. Faster blocked-task indexing and wake logic
4. Task model extension with period/deadline/budget metadata

Medium priority:
1. Admission control for periodic workloads
2. Optional EDF scheduling mode
3. MPU partitioning groundwork

Long-term:
1. Service decomposition and message-oriented internals
2. Progressive microkernel conversion of non-critical subsystems
3. Hard RT certification-oriented evidence workflow

## 8. Definition of Done for "Hard-RT-Candidate" Milestone

A realistic milestone should require:
- Documented timing contracts for each critical task
- Proven upper bounds for context-switch and IPC latency on target board
- Measured interrupt latency under load
- Priority inversion bounded with test evidence
- Deterministic overload and fault policy
- Traceable tests that run continuously in CI and on hardware-in-loop

Without this evidence, architecture claims remain aspirational.

## 9. How This Connects to the Current Code

Immediate code-level insertion points:
- Scheduler timing probes: `kernel/scheduler.c`
- Context-switch probes: `kernel/context.s`
- Timer-service shift: `kernel/timer.c` + new timer task in `kernel/kernel.c`
- Task model extension: `kernel/task.h` + creation APIs in `kernel/kernel.c`
- Admission/sched policy modules: new files under `kernel/`
- MPU and privilege policy: startup + exception/syscall path (`src/startup.c`, `kernel/context.s`)

This keeps evolution aligned with existing structure instead of replacing it abruptly.
