#include "GIC_Interrupts.h"
#include <system/memory/WriteMonitor.h>
#include <system/service/logger.h>

GIC_Interrupts GIC_Interrupts::instance;

void GIC_Interrupts::enable(GIC400_CPU *cpu_interface) {
    this->cpu_interface = cpu_interface;
}

void GIC_Interrupts::trigger_irq(uint64_t *saved_stack_base) {
    if (cpu_interface != 0) {
        uint16_t signalled_irq = cpu_interface->raedSignalledInterruptNumber();
        // log("Triggered IRQ" << dec << signalled_irq);
        switch (signalled_irq) {
            case 320:
                handle_irq_320(saved_stack_base);
                break;
        };
        cpu_interface->signalEndOfInterrupt(signalled_irq);
    }
}

void GIC_Interrupts::handle_irq_320(uint64_t *saved_stack_base) {
    // Look, which Performance counter caused the interrupt
    uint64_t pmovsclr = 0;
    asm volatile("mrs %0, pmovsclr_el0" : "=r"(pmovsclr));
    if ((pmovsclr & (0b1 << 0)) == (0b1 << 0)) {
        WriteMonitor::instance.handle_pmc_0_interrupt(saved_stack_base);
    }
    // Reset the overflow bits
    asm volatile("msr pmovsclr_el0, %0" ::"r"(pmovsclr));
}