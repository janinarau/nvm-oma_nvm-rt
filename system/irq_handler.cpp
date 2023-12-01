#include <system/driver/GIC_Interrupts.h>
#include <system/memory/WriteMonitor.h>
#include <system/service/logger.h>
#include <system/syscall.h>

extern "C" void handle_irq_lowlevel_sync(uint64_t *saved_stack_base) {
    /**
     * Some synchronous exceptions might be handleable, like a permission abort
     * from the MMU
     */
    // The exception syndrome register
    uint64_t esr_el1 = 0;
    asm volatile("mrs %0, esr_el1" : "=r"(esr_el1));

    // Data Abort
    if ((esr_el1 >> 26) == 0x24) {
        if ((esr_el1 & 0b1100) == 0b1100) {
            if (WriteMonitor::instance.handle_data_permission_interrupt()) {
                // The interrupt could be handled by the Monitor
                return;
            }
        }
    }
    // SVC Call
    else if ((esr_el1 >> 26) == 0x15) {
        // Stop call
        if ((esr_el1 & 0xFFFF) == 0) {
            Syscall::instance.stop_system();
            return;
        }
        if ((esr_el1 & 0xFFFF) == 10) {
            Syscall::instance.shutdown_system();
            return;
        }
#ifdef STACK_BALANCIMG
        // Pause Relocation call
        if ((esr_el1 & 0xFFFF) == 1) {
            Syscall::instance.pause_relocation(saved_stack_base);
            return;
        }
        // Continue Relocation call
        if ((esr_el1 & 0xFFFF) == 2) {
            Syscall::instance.resume_relocation(saved_stack_base);
            return;
        }
#endif
    }

    log("Sync IRQ Occured, not handleable :(");
    log("ESR: " << hex << esr_el1);
    uint64_t far = 0;
    asm volatile("mrs %0, far_el1" : "=r"(far));
    log("FAR " << hex << far);
    uint64_t elr = 0;
    asm volatile("mrs %0, elr_el1" : "=r"(elr));
    log("ELR " << hex << elr);
    while (1)
        ;
}

extern "C" void handle_irq_lowlevel_irq() {
    log("IRQ Occured");
    while (1)
        ;
}

extern "C" void handle_irq_lowlevel_fiq(uint64_t *saved_stack_base) {
    // log("FIQ Occured");
    GIC_Interrupts::instance.trigger_irq(saved_stack_base);
}

extern "C" void handle_irq_lowlevel_error() {
    log("Error IRQ Occured");
    while (1)
        ;
}