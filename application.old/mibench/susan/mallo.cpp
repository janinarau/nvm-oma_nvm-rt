char mem_pool[16777216] __attribute__ ((section("ro_bench_dat")));

void *alloc_start = (void *)mem_pool;

#include <system/service/logger.h>

void *malloc(unsigned long size) {
    void *cpy = alloc_start;
    alloc_start = (void *)(((unsigned long)alloc_start) + size);
    if(alloc_start > (mem_pool+sizeof(mem_pool))){
        log_error("Not enough memory to allocate");
        while(1);
    }
    return cpy;
}

void free(void *memory) {}