#include "system/data/RBTree.h"
#include "system/service/logger.h"

#include <system/driver/math.h>
#include "stack_relocate.h"

// Some data inside BSS
uint64_t bss_filler[2048];

void var_p(uint64_t *v1, uint64_t *v2);

void app_init() {
    log_info("Creating local variables");
    volatile uint64_t test_arr[5] = {51, 52, 53, 54, 55};
    uint64_t t1 = 42;
    uint64_t t2 = 182600;

    volatile uint64_t *ptr = (volatile uint64_t *)&t1;
    log_info("PTR (" << (void *)ptr << ") val: " << dec << *ptr);

    uint64_t current_base;
    asm volatile(
        "adr x0, __current_stack_base_ptr;"
        "ldr %0,[x0]"
        : "=r"(current_base)::"x0");

    log_info("Current base pointer is at " << hex << current_base);

    relocate_stack();
    var_p(&t1, &t2);

    asm volatile(
        "adr x0, __current_stack_base_ptr;"
        "ldr %0,[x0]"
        : "=r"(current_base)::"x0");

    log_info("PTR (" << (void *)ptr << ") val: " << dec << *ptr);
    volatile uint64_t *sptr=ptr-2048;
    log_info("Shadow-PTR (" << (void *)sptr << ") val: " << dec << *sptr);

    log_info("Current base pointer is at " << hex << current_base);

    log_info("Jumping back and exit");

    log_info(dec << test_arr[2]);

    asm volatile("svc #0");
}

void var_p(uint64_t *v1, uint64_t *v2) {
    uint64_t sp;
    asm volatile("mov %0, sp" : "=r"(sp));

    log_info("Called with pointer at " << hex << sp);
    log_info("Variables are " << dec << *v1 << " and " << *v2);
}