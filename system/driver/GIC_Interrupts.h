#ifndef SYSTEM_DRIVER_GIC_INTERRUPTS
#define SYSTEM_DRIVER_GIC_INTERRUPTS

#include "GIC.h"

class GIC_Interrupts {
   public:
    static GIC_Interrupts instance;

    void enable(GIC400_CPU *cpu_interface);

    void trigger_irq(uint64_t *saved_stack_base);

    void handle_irq_320(uint64_t *saved_stack_base);

   private:
    GIC400_CPU *cpu_interface = 0;
};

#endif