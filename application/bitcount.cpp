#include <system/service/logger.h>
#include "bitcount_data.h"

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

void test_bitcount(){
    uint64_t number = 4000;
    log("Calculating bitcount for " << dec << number << " values");

    uint64_t count = bitcount(random_number_bitcount, number);

    log("Counted " << count << " bits");
}