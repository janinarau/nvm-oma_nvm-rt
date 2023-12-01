#include "simple_alloc.h"

char alloc_pool[SIMPLE_ALLOC_SIZE];
char *alloc_ptr = alloc_pool;

void *malloc(unsigned long size) {
    char *copy = alloc_ptr;
    alloc_ptr += size;
    if (alloc_ptr >= alloc_pool + SIMPLE_ALLOC_SIZE) {
        // Wraparound alloc
        alloc_ptr=alloc_pool + size;
        return alloc_pool;
    }
    return copy;
}

void free(void *mem) {}