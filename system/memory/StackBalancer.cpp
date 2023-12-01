#include "StackBalancer.h"
#include <system/service/logger.h>
#include "MMU.h"

StackBalancer StackBalancer::instance;

StackBalancer::StackBalancer() {}

void *StackBalancer::setup_shadow_stack(void *phys_head, uint64_t size) {
    uint64_t v_stack_begin = TARGET_STACK_VADDRESS;
    uint64_t phys_begin = ((uint64_t)(phys_head)) - size;
    for (uint64_t i = 0; i < (size / 4096); i++) {
        MMU::instance.set_stack_page_mapping((void *)v_stack_begin,
                                             (void *)phys_begin);
        // log_info("MAP " << hex << v_stack_begin << " -> " << hex <<
        // phys_begin);
        v_stack_begin += 4096;
        phys_begin += 4096;
    }
    return (void *)(TARGET_STACK_VADDRESS + STACK_SIZE);
}

void StackBalancer::trigger_on_interrupt(uint64_t *saved_stack_base) {
    if (paused_relocation) {
        irq_while_paused = true;
        return;
    }
    // log_info("Saved stack frame is at " << saved_stack_base);

    // log_info("X0\t" << hex << saved_stack_base[28]);
    // log_info("X1\t" << hex << saved_stack_base[29]);
    // log_info("X2\t" << hex << saved_stack_base[26]);
    // log_info("X3\t" << hex << saved_stack_base[27]);
    // log_info("X4\t" << hex << saved_stack_base[24]);
    // log_info("X5\t" << hex << saved_stack_base[25]);
    // log_info("X6\t" << hex << saved_stack_base[22]);
    // log_info("X7\t" << hex << saved_stack_base[23]);
    // log_info("X8\t" << hex << saved_stack_base[20]);
    // log_info("X9\t" << hex << saved_stack_base[21]);
    // log_info("X10\t" << hex << saved_stack_base[18]);
    // log_info("X11\t" << hex << saved_stack_base[19]);
    // log_info("X12\t" << hex << saved_stack_base[16]);
    // log_info("X13\t" << hex << saved_stack_base[17]);
    // log_info("X14\t" << hex << saved_stack_base[14]);
    // log_info("X15\t" << hex << saved_stack_base[15]);
    // log_info("X16\t" << hex << saved_stack_base[12]);
    // log_info("X17\t" << hex << saved_stack_base[13]);
    // log_info("X18\t" << hex << saved_stack_base[10]);
    // log_info("X19\t" << hex << saved_stack_base[11]);
    // log_info("X20\t" << hex << saved_stack_base[8]);
    // log_info("X21\t" << hex << saved_stack_base[9]);
    // log_info("X22\t" << hex << saved_stack_base[6]);
    // log_info("X23\t" << hex << saved_stack_base[7]);
    // log_info("X24\t" << hex << saved_stack_base[4]);
    // log_info("X25\t" << hex << saved_stack_base[5]);
    // log_info("X26\t" << hex << saved_stack_base[2]);
    // log_info("X27\t" << hex << saved_stack_base[3]);
    // log_info("X28\t" << hex << saved_stack_base[0]);
    // log_info("X29\t" << hex << saved_stack_base[30]);
    // log_info("X30\t" << hex << saved_stack_base[1]);

    // while (1)
    //     ;

    if (performing_balane) {
        return;
    }
    if (!performed_since_last_irq) {
        perform_irq_relocation(saved_stack_base);
        performed_irq_version = true;
        relocation_coint_irq++;

        if (current_recursion_depth > recursion_depth_trigger) {
            // log_info("On level " << dec << current_recursion_depth);
            recursion_depth_trigger++;
        }
    } else {
        performed_since_last_irq = false;
    }
    // log_info("[OLBR]: " << dec << outer_loop_balancing_ratio);
    // log_info("[RACR]: " << dec << recursion_depth_trigger);
    // extern uint64_t __current_stack_base_ptr;
    // log_info("[BBPTR]: " << hex << __current_stack_base_ptr);
}

void StackBalancer::hint_relocation() {
    if (!performed_since_last_irq) {
        performing_balane = true;
        performed_since_last_irq = true;
        performed_irq_version = false;
        relocate_stack();
        relocation_count_syn++;
        performing_balane = false;
    }
}

void StackBalancer::perform_irq_relocation(uint64_t *saved_stack_base) {
    /**
     * Basically do the same steps here, than in the synchronous version, just
     * this time in c :D
     */
    // Applications stack pointer
    uint64_t app_sp;
    asm volatile("mrs %0, sp_el0" : "=r"(app_sp));
    // log_info("App sp was at " << hex << app_sp);

    extern uint64_t __virtual_stack_begin;
    uint64_t __shadow_stack_begin = __virtual_stack_begin - REAL_STACK_SIZE;
    extern uint64_t __current_stack_base_ptr;

    // log_info("Stack base is at " << hex << __current_stack_base_ptr
    //                              << ", begin at " << hex
    //                              << __virtual_stack_begin << " and shadow at
    //                              "
    //                              << __shadow_stack_begin);

    bool will_wrap =
        __current_stack_base_ptr < (__virtual_stack_begin + RELOCATION_STEP);

    // Relocate stack pointer
    app_sp -= RELOCATION_STEP;

    // Copy the old stack
    for (uint64_t target = app_sp + RELOCATION_STEP;
         target < __current_stack_base_ptr; target += 8) {
        uint64_t lword = *((uint64_t *)(target));
#ifdef IN_MEMORY_POINTER_RELOCATION
        // Check if the word has to be relocated
        if (lword < __current_stack_base_ptr && lword > __shadow_stack_begin) {
            lword -= RELOCATION_STEP;
        }
        // Check if the word is in the shadow
        if (will_wrap && lword < __virtual_stack_begin &&
            lword >= __shadow_stack_begin) {
            lword += REAL_STACK_SIZE;
        }
#endif
        *((uint64_t *)(target - RELOCATION_STEP)) = lword;
    }

#ifdef IN_MEMORY_POINTER_RELOCATION
    // Relocate all the current applications registers
    for (uint64_t i = 0; i < 31; i++) {
        uint64_t lword = saved_stack_base[i];
        // Check if the word has to be relocated
        if (lword < __current_stack_base_ptr && lword > __shadow_stack_begin) {
            lword -= RELOCATION_STEP;
        }
        // Check if the word is in the shadow
        if (will_wrap && lword < __virtual_stack_begin &&
            lword >= __shadow_stack_begin) {
            lword += REAL_STACK_SIZE;
        }
        saved_stack_base[i] = lword;
    }
#else
    /**
     * Relocate at least X29, as it is the frame pointer
     */
    uint64_t lword = saved_stack_base[30];
    // Check if the word has to be relocated
    if (lword < __current_stack_base_ptr && lword > __shadow_stack_begin) {
        lword -= RELOCATION_STEP;
    }
    // Check if the word is in the shadow
    if (will_wrap && lword < __virtual_stack_begin &&
        lword >= __shadow_stack_begin) {
        lword += REAL_STACK_SIZE;
    }
    saved_stack_base[30] = lword;
#endif

    // Relocate the new stack base
    __current_stack_base_ptr -= RELOCATION_STEP;
    if (will_wrap) {
        __current_stack_base_ptr += REAL_STACK_SIZE;
    }

    if (will_wrap && app_sp < __virtual_stack_begin) {
        app_sp += REAL_STACK_SIZE;
    }

    // Write back the stack pointer
    asm volatile("msr sp_el0, %0" ::"r"(app_sp));
    // log_info("Done IRQ reloc");
}

void StackBalancer::outer_loop_automatic() {
    /**
     * Basic idea: the outer_loop_ratio determines the rate of calls of this
     * function have to be called to trigger a software hint. If the hint is not
     * successfull (performs no action), the ratio is multiplied by 2. If the
     * last relocation still was triggered by an irq, the ratio is reduced by 1
     */

#ifdef STACK_BALANCIMG
    if (outer_loop_cycle++ >= outer_loop_balancing_ratio) {
        if (performed_irq_version) {
            if (outer_loop_balancing_ratio > 1) {
                outer_loop_balancing_ratio--;
            }
        }

        if (performed_since_last_irq) {
            outer_loop_balancing_ratio *= 2;
        }

        hint_relocation();
        outer_loop_cycle = 0;

        // log_info("[OLBR]: " << dec << outer_loop_balancing_ratio);
    }
#endif
}

void StackBalancer::recursive_automatic_call_begin() {
// First increase recursion depth
#ifdef STACK_BALANCIMG
    current_recursion_depth++;
    // log_info("Now at level" << dec << current_recursion_depth);

    // if (current_recursion_depth <= recursion_depth_trigger) {
    //     log_info("Call level " << current_recursion_depth << "reached");
    // }

    // if (performed_irq_version) {
    //     if (current_recursion_depth > recursion_depth_trigger) {
    //         recursion_depth_trigger++;
    //     }
    // }

    if (current_recursion_depth == recursion_depth_trigger) {
        if (performed_since_last_irq && recursion_depth_trigger > 1) {
            recursion_depth_trigger--;
        }

        hint_relocation();
    }
#endif
}
void StackBalancer::recursive_automatic_call_end() {
// Reduce recursion depth
#ifdef STACK_BALANCIMG
    current_recursion_depth--;
#endif
}

void StackBalancer::print_statistic() {
    log("[Stack Balancer] Syn RLCs: " << dec << relocation_count_syn
                                      << " / IRQ RLCs: " << dec
                                      << relocation_coint_irq);
}

RecursiveGuard::RecursiveGuard() {
    StackBalancer::instance.recursive_automatic_call_begin();
}

RecursiveGuard::~RecursiveGuard() {
    StackBalancer::instance.recursive_automatic_call_end();
}