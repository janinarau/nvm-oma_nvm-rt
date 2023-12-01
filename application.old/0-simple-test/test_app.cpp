#include "system/service/logger.h"

#define TEST_SIZE 100

volatile uint64_t test_array[TEST_SIZE];

void app_init() {
    for (unsigned int i = 0; i < TEST_SIZE * 10; i++) {
        test_array[i % TEST_SIZE]++;
    }
}