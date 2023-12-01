#include <system/driver/math.h>
#include <system/memory/StackBalancer.h>
#include "data.h"
#include "system/data/RBTree.h"
#include "system/service/logger.h"
#include <system/memory/stack_relocate.h>

// Some data inside BSS
uint64_t bss_filler[2048];

// #define HINTING
#define MANUAL_BALANCING

uint64_t bitcount(uint8_t *data, uint64_t el_count);
void do_bitcount();

extern uint64_t __current_stack_base_ptr;

#ifdef MANUAL_BALANCING
uint64_t so_reloc_count = 0;
#endif

void app_init() {
    for (uint64_t i = 0; i < 200; i++) do_bitcount();
#ifdef MANUAL_BALANCING
    log("Performed " << dec << so_reloc_count << " relocations so");
#endif
    asm volatile("svc #0");
}

void do_bitcount() {
    uint64_t result = 0;
    uint64_t el_count = 8000;
    for (uint64_t i = 0; i < el_count; i++) {
#ifdef HINTING
        STACK_OUTER_LOOP
#endif
#ifdef MANUAL_BALANCING
        if (i % 307 == 0) {
            relocate_stack();
            so_reloc_count++;
        }
#endif
        result += (random_number[i] & (0b1 << 0)) != 0;
        result += (random_number[i] & (0b1 << 1)) != 0;
        result += (random_number[i] & (0b1 << 2)) != 0;
        result += (random_number[i] & (0b1 << 3)) != 0;
        result += (random_number[i] & (0b1 << 4)) != 0;
        result += (random_number[i] & (0b1 << 5)) != 0;
        result += (random_number[i] & (0b1 << 6)) != 0;
        result += (random_number[i] & (0b1 << 7)) != 0;
    }
    // log_info("Counted " << dec << result << " Bits");
}