#include <system/service/logger.h>
#include "sort_data.h"
#include <system/driver/math.h>
#include "system/data/RBTree.h"

void quick_sort(uint8_t *begin, uint8_t *end) {
    if (begin + 2 >= end) {
        return;
    }
    uint8_t *pivot_element = end - 1;

    uint8_t *li = begin;
    uint8_t *ri = end - 1;

    while (li < ri) {
        while (li < end - 2 && *li < *pivot_element) li++;
        while (ri > begin && *ri >= *pivot_element) ri--;

        if (li < ri) {
            uint8_t buffer = *li;
            *li = *ri;
            *ri = buffer;
        }
    }

    if (*li > *pivot_element) {
        uint8_t buffer = *li;
        *li = *pivot_element;
        *pivot_element = buffer;
    }

    quick_sort(begin, li);
    quick_sort(li + 1, end);
}

void test_quick_sort(){
    unsigned int sort_size = 5000;
    log("Starting to sort " << dec << sort_size << " numbers");

    quick_sort(random_number_sort, random_number_sort + sort_size);
}