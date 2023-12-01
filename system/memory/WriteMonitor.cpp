#include "WriteMonitor.h"
#include <system/service/logger.h>

#include <system/driver/GIC.h>
#include <system/driver/PMC.h>
#include <system/memory/MMU.h>
#include <system/memory/PageBalancer.h>
#include <system/memory/StackBalancer.h>

extern GIC400_Distributor gic_distributor;

WriteMonitor WriteMonitor::instance;

void WriteMonitor::initialize() {
    // Set all observed pages not writable
    // set_all_observed_pages(true);

    // Initialize the performance counter
    PMC::instance.set_counters_enabled(true);
    PMC::instance.set_event_counter_enabled(0, true);
    PMC::instance.set_count_event(0, PMC::BUS_ACCESS_STORE);
    PMC::instance.set_non_secure_el0_counting(0, true);
    PMC::instance.enable_overflow_interrupt(0, true);
    gic_distributor.maskInterrupt(320, false);
    PMC::instance.write_event_counter(0, UINT32_MAX - MONITORING_RESOLUTION);
}
void WriteMonitor::terminate() {
    gic_distributor.maskInterrupt(320, true);
    set_all_observed_pages(false);
    PMC::instance.enable_overflow_interrupt(0, false);
    PMC::instance.set_event_counter_enabled(0, false);
}
void WriteMonitor::plot_results() {
    for (uint64_t i = 0; i < MONITOR_CAPACITY; i++) {
        OutputStream::instance << hex << (SYSTEM_OFFSET + i * 0x1000) << " "
                               << dec << write_count[i] << endl;
    }
#ifdef STACK_BALANCIMG
    OutputStream::instance << "===STACK===" << endl;
    for (uint64_t i = 0; i < REAL_STACK_SIZE / 0x1000; i++) {
        OutputStream::instance << hex << (TARGET_STACK_VADDRESS + i * 0x1000)
                               << " " << dec << stack_write_count[i] << endl;
    }
#endif
}
void WriteMonitor::add_page_to_observe(void *vm_page) {
    if (observerd_vm_pages_count >= MONITOR_CAPACITY) {
        log_error("Cannot monitor more than "
                  << MONITOR_CAPACITY
                  << " Pages. Please increase the capacity or remove pages");
        while (1)
            ;
    }
    vm_pages_to_observe[observerd_vm_pages_count++] = vm_page;
}

void WriteMonitor::add_stack_page_to_observe(void *vm_page) {
    if (oberved_stack_pages_count >= MONITOR_CAPACITY) {
        log_error(
            "Cannot monitor more than "
            << MONITOR_CAPACITY
            << " Stack Pages. Please increase the capacity or remove pages");
        while (1)
            ;
    }
    stack_vm_pages_to_observe[oberved_stack_pages_count++] = vm_page;
}

void WriteMonitor::set_notify_threshold(uint64_t notify_threshold) {
    this->notify_threshold = notify_threshold;
}

void WriteMonitor::set_all_observed_pages(bool generate_interrupt_on_write) {
    for (uint64_t i = 0; i < observerd_vm_pages_count; i++) {
        if (generate_interrupt_on_write) {
            MMU::instance.set_access_permission(
                vm_pages_to_observe[i], MMU::ACCESS_PERMISSION::R_FROM_EL1_EL0);
        } else
            MMU::instance.set_access_permission(
                vm_pages_to_observe[i],
                MMU::ACCESS_PERMISSION::RW_FROM_EL1_EL0);
    }
    for (uint64_t i = 0; i < oberved_stack_pages_count; i++) {
        if (generate_interrupt_on_write) {
            MMU::instance.set_stack_access_permission(
                stack_vm_pages_to_observe[i],
                MMU::ACCESS_PERMISSION::R_FROM_EL1_EL0);
        } else
            MMU::instance.set_stack_access_permission(
                stack_vm_pages_to_observe[i],
                MMU::ACCESS_PERMISSION::RW_FROM_EL1_EL0);
    }
    MMU::instance.flush_tlb();
}

bool WriteMonitor::handle_data_permission_interrupt() {
    PMC::instance.enable_overflow_interrupt(0, false);
    // Read the fault address
    uint64_t far_el1 = 0;
    asm volatile("mrs %0, far_el1" : "=r"(far_el1));

    // log("Data Permission fault at " << hex << far_el1);
    uintptr_t fault_page = far_el1 & ~(0xFFF);
    uint64_t fault_page_number =
        ((uint64_t)(fault_page)-SYSTEM_OFFSET) / 0x1000;
#ifdef STACK_BALANCIMG
    uint64_t stack_fault_page_number;
    if (fault_page >= TARGET_STACK_VADDRESS) {
        stack_fault_page_number =
            ((uint64_t)(fault_page)-TARGET_STACK_VADDRESS) / 0x1000;
        if (stack_fault_page_number >= (REAL_STACK_SIZE / 0x1000)) {
            stack_fault_page_number -= (REAL_STACK_SIZE / 0x1000);
        }
        stack_write_count[stack_fault_page_number]++;
    } else
#endif
    {
        write_count[fault_page_number]++;
    }

    // MMU::instance.set_access_permission(
    //     (void *)fault_page, MMU::ACCESS_PERMISSION::RW_FROM_EL1_EL0);
    set_all_observed_pages(false);
    MMU::instance.invalidate_tlb_entry((void *)fault_page);

#ifdef STACK_BALANCIMG
    if (fault_page >= TARGET_STACK_VADDRESS &&
        stack_write_count[stack_fault_page_number] >= notify_threshold) {
        if (fault_page >= (TARGET_STACK_VADDRESS) + REAL_STACK_SIZE) {
            fault_page -= REAL_STACK_SIZE;
        }
        stack_write_count[stack_fault_page_number] = 0;
    } else
#endif
        if (write_count[fault_page_number] >= notify_threshold) {
        write_count[fault_page_number] = 0;
    } else {
        PMC::instance.write_event_counter(0,
                                          UINT32_MAX - MONITORING_RESOLUTION);
        PMC::instance.enable_overflow_interrupt(0, true);
        return true;
    }

    PageBalancer::instance.trigger_rebalance((void *)(fault_page & ~0xFFF));
    PMC::instance.write_event_counter(0, UINT32_MAX - MONITORING_RESOLUTION);
    PMC::instance.enable_overflow_interrupt(0, true);

    return true;
}

void WriteMonitor::handle_pmc_0_interrupt(uint64_t *saved_stack_base) {
// log("PMC 0 overflowed");
// Simply reset all acces permissions to generate interrupts
#ifdef STACK_BALANCIMG
    StackBalancer::instance.trigger_on_interrupt(saved_stack_base);
#endif

    set_all_observed_pages(true);
    PMC::instance.write_event_counter(0, UINT32_MAX - MONITORING_RESOLUTION);
}