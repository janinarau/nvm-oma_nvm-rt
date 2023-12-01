#include "driver/GIC.h"
#include "driver/GIC_Interrupts.h"
#include "driver/PMC.h"
#include "memory/MMU.h"
#include "memory/PageBalancer.h"
#include "memory/StackBalancer.h"
#include "memory/WriteMonitor.h"
#include "system/service/logger.h"

GIC400_Distributor gic_distributor;
GIC400_CPU gic_cpu;

extern void app_init();

/**
 * This function is called by the assmebler bootcode
 */

extern uint64_t __current_stack_base_ptr;
extern uint64_t __virtual_stack_begin;
extern uint64_t __virtual_stack_end;

extern "C" void init_system_c() {
    extern void (*__init_array_start)();
    extern void (*__init_array_end)();

    for (void (**actual)() = &__init_array_start; actual < &__init_array_end;
         actual++) {
        (**actual)();
    }
    log("Called init arrays, welcome!");

    // Determine the running exception level
    uint64_t current_el_value = 0;
    asm volatile("mrs %0, currentel" : "=r"(current_el_value));
    log("Running on Excpetion Level " << (current_el_value >> 2));

    gic_distributor.initialize();
    gic_cpu.initialize();
    gic_cpu.setPriotityMask(0xFF);
    GIC_Interrupts::instance.enable(&gic_cpu);

    // Enable CPU Interrupts
    uint64_t spsr_el1;
    asm volatile("mrs %0, spsr_el1" : "=r"(spsr_el1));
    spsr_el1 |= 0b1;  // Use current sp for exceptions
    asm volatile("msr spsr_el1, %0" ::"r"(spsr_el1));

    log("Clearing the BSS");
    extern unsigned long __NVMSYMBOL__APPLICATION_BSS_BEGIN;
    extern unsigned long __NVMSYMBOL__APPLICATION_BSS_END;
    unsigned long bss_size = ((uint64_t)(&__NVMSYMBOL__APPLICATION_BSS_END)) -
                             ((uint64_t)(&__NVMSYMBOL__APPLICATION_BSS_BEGIN));
    unsigned long *last_dot = &__NVMSYMBOL__APPLICATION_BSS_BEGIN;

    for (unsigned long *actual = &__NVMSYMBOL__APPLICATION_BSS_BEGIN;
         actual < &__NVMSYMBOL__APPLICATION_BSS_END; actual++) {
        *actual = 0;

        if ((uint64_t)actual - (uint64_t)last_dot > bss_size / 10) {
            last_dot = actual;
            OutputStream::instance << ".";
        }
    }
    OutputStream::instance << "\n";
    log("Cleared the BSS");

    asm volatile(
        "mov x1, #(0b11 << 20);"
        "msr cpacr_el1, x1;"
        "isb;" ::
            : "x1");
    log("Disabled floating point trapping");

    MMU::instance.clean_and_disable_caches();
    log("Cleaned and disabled caches");
    MMU::instance.setup_pagetables();
    log("Setup pagetables");
    // MMU::instance.activate_caches();
    // log("Caches Active");
    MMU::instance.activate_mmu();
    log("MMU Active");

    // Determine number of available performance counters
    uint64_t num_counter = PMC::instance.get_num_available_counters();
    log("Available performance counters: " << dec << num_counter);

    // Start the WriteMonitor
    extern void *__INTERNAL_CODE_BEGIN;
    extern void *__INTERNAL_CODE_END;
    extern void *__INTERNAL_DATA_BEGIN;
    extern void *__NVMSYMBOL__APPLICATION_TEXT_BEGIN;
    extern void *__NVMSYMBOL__APPLICATION_STACK_END;

    void *application_stack = (void *)&__NVMSYMBOL__APPLICATION_STACK_END;
#ifdef STACK_BALANCIMG
    extern void *__NVMSYMBOL__APPLICATION_STACK_BEGIN;

    uint64_t stack_size = ((uint64_t)(&__NVMSYMBOL__APPLICATION_STACK_END)) -
                          ((uint64_t)(&__NVMSYMBOL__APPLICATION_STACK_BEGIN));
    application_stack = StackBalancer::instance.setup_shadow_stack(
        application_stack, stack_size);

    __current_stack_base_ptr = (uint64_t)application_stack;
    __virtual_stack_end = (uint64_t)application_stack;
    __virtual_stack_begin = __virtual_stack_end - stack_size;
#endif

    /**
     * Setup the memory access permissions, such that el0 can access the full
     * code
     */

    for (uintptr_t app_page = (uintptr_t)&__INTERNAL_CODE_BEGIN;
         app_page < (uintptr_t)&__INTERNAL_CODE_END; app_page += 0x1000) {
        MMU::instance.set_access_permission(
            (void *)app_page, MMU::ACCESS_PERMISSION::RW_FROM_EL1);
    }
    for (uintptr_t app_page = (uintptr_t)&__INTERNAL_DATA_BEGIN;
         app_page < (uintptr_t)&__NVMSYMBOL__APPLICATION_TEXT_BEGIN;
         app_page += 0x1000) {
        MMU::instance.set_access_permission(
            (void *)app_page, MMU::ACCESS_PERMISSION::RW_FROM_EL1_EL0);
    }
    for (uintptr_t app_page = (uintptr_t)&__NVMSYMBOL__APPLICATION_TEXT_BEGIN;
         app_page < (uintptr_t)&__NVMSYMBOL__APPLICATION_STACK_END;
         app_page += 0x1000) {
#ifdef DO_MONITORING
        WriteMonitor::instance.add_page_to_observe((void *)app_page);
#endif
        MMU::instance.set_access_permission(
            (void *)app_page, MMU::ACCESS_PERMISSION::RW_FROM_EL1_EL0);
    }

#ifdef STACK_BALANCIMG
    for (uintptr_t app_page = (uintptr_t)TARGET_STACK_VADDRESS;
         app_page < (uintptr_t)(TARGET_STACK_VADDRESS + (STACK_SIZE / 2));
         app_page += 0x1000) {
#ifdef DO_MONITORING
        WriteMonitor::instance.add_stack_page_to_observe((void *)app_page);
#endif
        MMU::instance.set_stack_access_permission(
            (void *)app_page, MMU::ACCESS_PERMISSION::RW_FROM_EL1_EL0);
    }
#endif

    MMU::instance.flush_tlb();

    log("Calling target app (with swapped stack pointer on EL0) ["
        << application_stack << "]");

    asm volatile(
        "mov x0,%0;"
        "msr sp_el0, x0;" ::"r"(application_stack)
        : "x0");
    asm volatile("msr elr_el1, %0;" ::"r"(&app_init));
    uint64_t spsr = 0;
    asm volatile("mrs %0, spsr_el1" : "=r"(spsr));
    spsr &= ~(0b111);
    asm volatile("msr spsr_el1, %0" ::"r"(spsr));
    log("Handover to app");

#ifdef DO_MONITORING
    WriteMonitor::instance.initialize();
#endif
#ifdef DO_REBALANCING
    PageBalancer::instance.enable_balancing();
#endif
    asm volatile("eret");
    while (1)
        ;
}