#pragma once
#include <system/stdint.h>
#include <system/service/logger.h>

int32_t psci_call(uint32_t function, uint64_t arg1, uint64_t arg2){
    int32_t result_value=0;

    log_info("Doing psci calls");

    asm volatile(
        "mov x0, %1;"
        "mov x1, %2;"
        "mov x2, %3;"
        "dsb sy;"
        "dmb sy;"
        "isb;"
        "smc #0;"
        "mov %0, x0;"
        : "=r"(result_value)
        : "r"(function), "r"(arg1), "r"(arg2)
        : "x0", "x1", "x2"
    );

    log_info("Done psci calls");

    return result_value;
}