/*
 * HelixRT - Linker Script for Teensy 4.1 (IMXRT1062)
 * 
 * Memory Layout:
 *   - Flash (QSPI): 8MB at 0x60000000 (Execute-In-Place)
 *   - ITCM: 512KB at 0x00000000 (Instruction Tightly Coupled Memory)
 *   - DTCM: 512KB at 0x20000000 (Data Tightly Coupled Memory) 
 *   - OCRAM2: 512KB at 0x20200000 (On-Chip RAM)
 *
 * Note: Default Teensy 4.1 configuration uses 512KB ITCM + 512KB DTCM
 *       OCRAM2 is the dedicated 512KB block
 */

MEMORY
{
    /* Flash configuration block must be at 0x60000000 */
    FLASH_CONFIG (rx) : ORIGIN = 0x60000000, LENGTH = 512
    
    /* IVT and Boot Data */
    FLASH_IVT (rx)    : ORIGIN = 0x60000400, LENGTH = 4K - 512
    
    /* Main flash for code */
    FLASH (rx)        : ORIGIN = 0x60001000, LENGTH = 8M - 4K
    
    /* Tightly Coupled Memory for fast code */
    ITCM (rwx)        : ORIGIN = 0x00000000, LENGTH = 512K
    
    /* Tightly Coupled Memory for fast data */
    DTCM (rwx)        : ORIGIN = 0x20000000, LENGTH = 512K
    
    /* On-Chip RAM (dedicated 512KB block) */
    OCRAM2 (rwx)      : ORIGIN = 0x20200000, LENGTH = 512K
}

/* Entry point */
ENTRY(Reset_Handler)

/* Stack configuration */
_stack_size = 8K;
_heap_size = 64K;

SECTIONS
{
    /* Flash Configuration Block - Required for boot */
    .flash_config : ALIGN(4)
    {
        FILL(0xFF)
        KEEP(*(.flash_config))
        . = ORIGIN(FLASH_CONFIG) + LENGTH(FLASH_CONFIG);
    } > FLASH_CONFIG

    /* Image Vector Table and Boot Data */
    .ivt : ALIGN(4)
    {
        KEEP(*(.ivt))
        KEEP(*(.boot_data))
    } > FLASH_IVT

    /* Vector table - placed at start of flash text region */
    .vectors : ALIGN(4)
    {
        __vectors_start = .;
        KEEP(*(.vectors))
        __vectors_end = .;
    } > FLASH

    /* Code section */
    .text : ALIGN(4)
    {
        __text_start = .;
        *(.text*)
        *(.rodata*)
        
        /* C++ constructors/destructors */
        . = ALIGN(4);
        __init_array_start = .;
        KEEP(*(.init_array*))
        __init_array_end = .;
        
        . = ALIGN(4);
        __fini_array_start = .;
        KEEP(*(.fini_array*))
        __fini_array_end = .;
        
        . = ALIGN(4);
        __text_end = .;
    } > FLASH

    /* ARM exception unwinding (needed for C++) */
    .ARM.extab : ALIGN(4)
    {
        *(.ARM.extab*)
    } > FLASH

    .ARM.exidx : ALIGN(4)
    {
        __exidx_start = .;
        *(.ARM.exidx*)
        __exidx_end = .;
    } > FLASH

    /* Fast code - copied to ITCM at startup */
    .fast_code : ALIGN(4)
    {
        __fast_code_start = .;
        *(.fast_code*)
        . = ALIGN(4);
        __fast_code_end = .;
    } > ITCM AT > FLASH
    __fast_code_load = LOADADDR(.fast_code);

    /* Initialized data - copied from flash to DTCM */
    .data : ALIGN(4)
    {
        __data_start = .;
        *(.data*)
        . = ALIGN(4);
        __data_end = .;
    } > DTCM AT > FLASH
    __data_load = LOADADDR(.data);

    /* Uninitialized data - zeroed at startup */
    .bss (NOLOAD) : ALIGN(4)
    {
        __bss_start = .;
        *(.bss*)
        *(COMMON)
        . = ALIGN(4);
        __bss_end = .;
    } > DTCM

    /* Heap - grows upward */
    .heap (NOLOAD) : ALIGN(8)
    {
        __heap_start = .;
        . += _heap_size;
        __heap_end = .;
    } > DTCM

    /* Stack - placed at end of DTCM */
    .stack (NOLOAD) : ALIGN(8)
    {
        __stack_start = .;
        . += _stack_size;
        __stack_end = .;
        __stack_top = .;
    } > DTCM

    
    
    /* Task stacks pool - separate from kernel stack for isolation */
    .task_stacks (NOLOAD) : ALIGN(8)
    {
        __task_stacks_start = .;
        *(.task_stacks*)
        . = ALIGN(8);
        __task_stacks_end = .;
    } > OCRAM2
    
    /* Task Control Blocks pool */
    .tcb_pool (NOLOAD) : ALIGN(4)
    {
        __tcb_pool_start = .;
        *(.tcb_pool*)
        . = ALIGN(4);
        __tcb_pool_end = .;
    } > OCRAM2
    
    
    /* Message queue buffers */
    .msg_queues (NOLOAD) : ALIGN(4)
    {
        __msg_queues_start = .;
        *(.msg_queues*)
        . = ALIGN(4);
        __msg_queues_end = .;
    } > OCRAM2
    
    /* Synchronization objects (semaphores, mutexes) - in DTCM for fast access */
    .sync_objects (NOLOAD) : ALIGN(4)
    {
        __sync_objects_start = .;
        *(.sync_objects*)
        . = ALIGN(4);
        __sync_objects_end = .;
    } > DTCM
    
   
    
    /* DMA descriptors - must be 32-byte aligned for IMXRT DMA */
    .dma_descriptors (NOLOAD) : ALIGN(32)
    {
        __dma_desc_start = .;
        *(.dma_descriptors*)
        . = ALIGN(32);
        __dma_desc_end = .;
    } > OCRAM2
    
    /* DMA buffers - 32-byte aligned for cache line coherency */
    .dma_buffers (NOLOAD) : ALIGN(32)
    {
        __dma_buffers_start = .;
        *(.dma_buffers*)
        . = ALIGN(32);
        __dma_buffers_end = .;
    } > OCRAM2
    
    /* General OCRAM2 usage (remaining space) */
    .ocram2 (NOLOAD) : ALIGN(4)
    {
        __ocram2_start = .;
        *(.ocram2*)
        __ocram2_end = .;
    } > OCRAM2

    /* Symbols for startup code */
    _estack = __stack_top;
    _sdata = __data_start;
    _edata = __data_end;
    _sbss = __bss_start;
    _ebss = __bss_end;
    _sidata = __data_load;
}
