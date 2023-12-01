ENTRY(system_entry)

SECTIONS
{
    . = 0x80000000;
    __NVMSYMBOL_SYSTEM_MEMORY_OFFSET = .;

    __NVMSYMBOL__DRIVER_BEGIN = .;
    __bootcode : {
        build/system/bootcode.o(*)
    }

    . = ALIGN(0x1000);

    __INTERNAL_CODE_BEGIN = .;
    __system_tools_exec : {
        build/system/driver/PL011.o(.text)
        build/system/service/OutputStream.o(.text)
        build/system/driver/GIC.o(.text)
        build/system/driver/PMC.o(.text)

        build/system/driver/GIC_Interrupts.o(.text)

        build/system/memory/MMU.o(.text)
        build/system/memory/WriteMonitor.o(.text)
        build/system/memory/PageBalancer.o(.text)
        build/system/memory/StackBalancer.o(.text)

        build/system/interrupt.o(.text)
        build/system/irq_handler.o(.text)

        build/system/init_system.o(.text)
        build/system/syscall.o(.text)

        build/system/data/RBTree.o(*)

        build/system/driver/math.o(.text)

        build/system/memory/stack_relocate.o(.text)

        build/system/m5/m5op_arm_A64.o(.text)
    }
    __INTERNAL_CODE_END = .;
    
    . = ALIGN(0x1000);

    __INTERNAL_DATA_BEGIN = .;

    __system_tools_data : {
        build/system/driver/PL011.o(.data)
        build/system/driver/PL011.o(.bss)
        build/system/service/OutputStream.o(.data)
        build/system/service/OutputStream.o(.bss)
        build/system/driver/GIC.o(.data)
        build/system/driver/GIC.o(.bss)
        build/system/driver/PMC.o(.data)
        build/system/driver/PMC.o(.bss)

        build/system/driver/GIC_Interrupts.o(.data)
        build/system/driver/GIC_Interrupts.o(.bss)

        build/system/memory/MMU.o(.data)
        build/system/memory/MMU.o(.bss)
        build/system/memory/WriteMonitor.o(.data)
        build/system/memory/WriteMonitor.o(.bss)
        build/system/memory/PageBalancer.o(.data)
        build/system/memory/PageBalancer.o(.bss)
        build/system/memory/StackBalancer.o(.data)
        build/system/memory/StackBalancer.o(.bss)

        build/system/m5/m5op_arm_A64.o(.data)
        build/system/m5/m5op_arm_A64.o(.bss)

        build/system/interrupt.o(*)
        build/system/irq_handler.o(*)

        build/system/init_system.o(*)
        build/system/syscall.o(*)

        build/system/data/RBTree.o(*)

        build/system/driver/math.o(*)

        build/system/memory/stack_relocate.o(*)
    }
    __NVMSYMBOL__DRIVER_END = .;

    __NVMSYMBOL_BOOTSTACK_BEGIN =.;
    .bootstack : {
        . = ALIGN(0x8);
        . += 0x1000; /* 1MB */
        __bootstack_top = .;
    }
    __NVMSYMBOL_BOOTSTACK_END =.;

    __NVMSYMBOL_BENCHDATA_BEGIN =.;
    .ro_bench_dat :{
        *(ro_bench_dat)
    }
    __NVMSYMBOL_BENCHDATA_END =.;

    . = ALIGN(0x1000);

    __NVMSYMBOL_SPARE_VM_PAGE_BEGIN = .;
    . += 0x1000;
    __NVMSYMBOL_SPARE_VM_PAGE_END = .;

    __NVMSYMBOL__APPLICATION_INIT_FINI_BEGIN = .;
    .init_array     :
    {
       __init_array_start = .;
       KEEP (*(SORT(.init_array.*)))
       KEEP (*(.init_array))
       KEEP (*(".ctors"))
       KEEP (*(".ctor"))
       __init_array_end = .;
    }

    .fini_array     :
    {
       __fini_array_start = .;
       KEEP (*(SORT(.fini_array.*)))
       KEEP (*(.fini_array))
       KEEP (*(".dtors"))
       KEEP (*(".dtor"))
       __fini_array_end = .;
    }
    __NVMSYMBOL__APPLICATION_INIT_FINI_END = .;

    . = ALIGN(0x1000);

    __NVMSYMBOL__APPLICATION_TEXT_BEGIN = .;
    .text : {
        *(.text)
    }
    __NVMSYMBOL__APPLICATION_TEXT_END = .;

    . = ALIGN(0x1000);

    __NVMSYMBOL__APPLICATION_DATA_BEGIN = .;
    .data : {
        *(.data)
        *(.rodata)
    }
    __NVMSYMBOL__APPLICATION_DATA_END = .;

    .note : {
        *(".note.*")
        *(.eh_frame)
    }

    . += 0x8;

    . = ALIGN(0x1000);

    __NVMSYMBOL__APPLICATION_BSS_BEGIN = .;
    .bss : {
        *(.bss)
        . += 1024;
    }
    __NVMSYMBOL__APPLICATION_BSS_END = .;
    
    . = ALIGN(0x1000);

    __NVMSYMBOL__APPLICATION_STACK_BEGIN = .;
    .stack : {
        . = ALIGN(0x8);
        . += 0x8000; /* 1MB */
        __stack_top = .;
    }
    __NVMSYMBOL__APPLICATION_STACK_END = .;

    PROVIDE (end = .);
}