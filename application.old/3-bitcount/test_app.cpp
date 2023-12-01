#include "data.h"
#include "system/data/RBTree.h"
#include "system/service/logger.h"

#include <system/driver/math.h>

// Some data inside BSS
uint64_t bss_filler[2048];

uint64_t bitcount(uint8_t *data, uint64_t el_count);

void app_init() {
    uint64_t number = 4000;
    log("Calculating bitcount for " << dec << number << " values");

    uint64_t count = bitcount(random_number, number);

    log("Counted " << count << " bits");

    asm volatile("svc #0");
}

uint64_t bitcount(uint8_t *data, uint64_t el_count) {
    uint64_t result = 0;
    for (uint64_t i = 0; i < el_count; i++) {
        result += (data[i] & (0b1 << 0)) != 0;
        result += (data[i] & (0b1 << 1)) != 0;
        result += (data[i] & (0b1 << 2)) != 0;
        result += (data[i] & (0b1 << 3)) != 0;
        result += (data[i] & (0b1 << 4)) != 0;
        result += (data[i] & (0b1 << 5)) != 0;
        result += (data[i] & (0b1 << 6)) != 0;
        result += (data[i] & (0b1 << 7)) != 0;
    }
    return result;
}