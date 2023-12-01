#include "syscall_interface.h"

void syscall_exit() { asm volatile("svc #0"); }

void syscall_shutdown() { asm volatile("svc #10"); }

#ifdef STACK_BALANCIMG

#include <system/memory/StackBalancer.h>

void syscall_delay_relocation() {
    // asm volatile("svc #1");
    StackBalancer::instance.paused_relocation = true;
}
void syscall_continue_relocatuion() {
    if (StackBalancer::instance.irq_while_paused) {
        asm volatile("svc #2");
    } else {
        StackBalancer::instance.paused_relocation = false;
    }
}
#endif