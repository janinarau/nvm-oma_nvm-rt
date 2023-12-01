#ifndef SYSTEM_MEMORY_STACK_BALANCER
#define SYSTEM_MEMORY_STACK_BALANCER

#include <system/stdint.h>
#include "stack_relocate.h"

#define STACK_OUTER_LOOP StackBalancer::instance.outer_loop_automatic();
#define STACK_RECURSIVE_FUNC RecursiveGuard __r__;
//#define SHint StackBalancer::instance.hint_relocation();

#define SHint                            \
    if (!StackBalancer::instance.performed_since_last_irq) {     \
        StackBalancer::instance.performing_balane = true;        \
        StackBalancer::instance.performed_since_last_irq = true; \
        relocate_stack();                \
        StackBalancer::instance.relocation_count_syn++;          \
        StackBalancer::instance.performing_balane = false;       \
    }

class RecursiveGuard;

class StackBalancer {
    friend class RecursiveGuard;

   public:
    StackBalancer();

    static StackBalancer instance;

    void *setup_shadow_stack(void *phys_head, uint64_t size);

    void trigger_on_interrupt(uint64_t *saved_stack_base);

    void hint_relocation();

    void print_statistic();

    void outer_loop_automatic();

   private:
    void perform_irq_relocation(uint64_t *saved_stack_base);

   public:
    uint64_t relocation_count_syn = 0;
    uint64_t relocation_coint_irq = 0;

    bool performed_since_last_irq = false;
    bool performing_balane = false;
    bool performed_irq_version = false;

   public:
    bool paused_relocation = false;
    bool irq_while_paused = false;

   private:
    uint64_t outer_loop_balancing_ratio = 1;
    uint64_t outer_loop_cycle = 0;

    uint64_t recursion_depth_trigger = 1;
    uint64_t current_recursion_depth = 0;

    void recursive_automatic_call_begin();
    void recursive_automatic_call_end();
};

class RecursiveGuard {
   public:
    RecursiveGuard();
    ~RecursiveGuard();
};

#endif