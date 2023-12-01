#include "PMC.h"
#include <system/service/logger.h>

PMC PMC::instance;

uint64_t PMC::get_num_available_counters() {
    uint64_t hdcr_value = 5;
    asm volatile("mrs %0, pmcr_el0" : "=r"(hdcr_value));
    return (hdcr_value & (0b11111 << 11)) >> 11;
}
bool PMC::get_clock_divider() {
    uint64_t hdcr_value = 5;
    asm volatile("mrs %0, pmcr_el0" : "=r"(hdcr_value));
    return (hdcr_value & (0b1 << 3)) >> 3;
}
void PMC::set_clock_divider(bool enabled) {
    uint64_t hdcr_value = 5;
    asm volatile("mrs %0, pmcr_el0" : "=r"(hdcr_value));
    hdcr_value &= ~(0b1 << 3);
    hdcr_value |= (enabled << 3);
    asm volatile("msr pmcr_el0, %0" ::"r"(hdcr_value));
}
void PMC::reset_cycle_counter() {
    uint64_t hdcr_value = 5;
    asm volatile("mrs %0, pmcr_el0" : "=r"(hdcr_value));
    hdcr_value |= (0b1 << 2);
    asm volatile("msr pmcr_el0, %0" ::"r"(hdcr_value));
}
void PMC::reset_all_event_counter() {
    uint64_t hdcr_value = 5;
    asm volatile("mrs %0, pmcr_el0" : "=r"(hdcr_value));
    hdcr_value |= (0b1 << 1);
    asm volatile("msr pmcr_el0, %0" ::"r"(hdcr_value));
}
void PMC::set_counters_enabled(bool enabled) {
    uint64_t hdcr_value = 5;
    asm volatile("mrs %0, pmcr_el0" : "=r"(hdcr_value));
    hdcr_value &= ~(0b1 << 0);
    hdcr_value |= (enabled << 0);
    asm volatile("msr pmcr_el0, %0" ::"r"(hdcr_value));
}
void PMC::set_event_counter_enabled(uint64_t counter_num, bool enabled) {
    uint64_t pmcntenset = 5;
    asm volatile("mrs %0, pmcntenset_el0" : "=r"(pmcntenset));
    pmcntenset &= ~(0b1 << counter_num);
    pmcntenset |= (enabled << counter_num);
    asm volatile("msr pmcntenset_el0, %0" ::"r"(pmcntenset));
}

#define EXPAND_BRANCH_NUMBER(number, command, extra_params) \
    switch (number) {                                       \
        case 0: {                                           \
            command(extra_params, 0);                       \
        } break;                                            \
        case 1: {                                           \
            command(extra_params, 1);                       \
        } break;                                            \
        case 2: {                                           \
            command(extra_params, 2);                       \
        } break;                                            \
        case 3: {                                           \
            command(extra_params, 3);                       \
        } break;                                            \
        case 4: {                                           \
            command(extra_params, 4);                       \
        } break;                                            \
        case 5: {                                           \
            command(extra_params, 5);                       \
        } break;                                            \
        case 6: {                                           \
            command(extra_params, 6);                       \
        } break;                                            \
        case 7: {                                           \
            command(extra_params, 7);                       \
        } break;                                            \
        case 8: {                                           \
            command(extra_params, 8);                       \
        } break;                                            \
        case 9: {                                           \
            command(extra_params, 9);                       \
        } break;                                            \
        case 10: {                                          \
            command(extra_params, 10);                      \
        } break;                                            \
        case 11: {                                          \
            command(extra_params, 11);                      \
        } break;                                            \
        case 12: {                                          \
            command(extra_params, 12);                      \
        } break;                                            \
        case 13: {                                          \
            command(extra_params, 13);                      \
        } break;                                            \
        case 14: {                                          \
            command(extra_params, 14);                      \
        } break;                                            \
        case 15: {                                          \
            command(extra_params, 15);                      \
        } break;                                            \
        case 16: {                                          \
            command(extra_params, 16);                      \
        } break;                                            \
        case 17: {                                          \
            command(extra_params, 17);                      \
        } break;                                            \
        case 18: {                                          \
            command(extra_params, 18);                      \
        } break;                                            \
        case 19: {                                          \
            command(extra_params, 19);                      \
        } break;                                            \
        case 20: {                                          \
            command(extra_params, 20);                      \
        } break;                                            \
        case 21: {                                          \
            command(extra_params, 21);                      \
        } break;                                            \
        case 22: {                                          \
            command(extra_params, 22);                      \
        } break;                                            \
        case 23: {                                          \
            command(extra_params, 23);                      \
        } break;                                            \
        case 24: {                                          \
            command(extra_params, 24);                      \
        } break;                                            \
        case 25: {                                          \
            command(extra_params, 25);                      \
        } break;                                            \
        case 26: {                                          \
            command(extra_params, 26);                      \
        } break;                                            \
        case 27: {                                          \
            command(extra_params, 27);                      \
        } break;                                            \
        case 28: {                                          \
            command(extra_params, 28);                      \
        } break;                                            \
        case 29: {                                          \
            command(extra_params, 29);                      \
        } break;                                            \
        case 30: {                                          \
            command(extra_params, 30);                      \
        } break;                                            \
    }

#define READ_AND_RETURN_EVENT_COUNTER_N(useless, counternumber)             \
    uint32_t counter = 42;                                                  \
    asm volatile("mrs %0, pmevcntr" #counternumber "_el0" : "=r"(counter)); \
    return counter;
#define WRITE_EVENT_COUNTER_N(value, counternumber)                        \
    asm volatile("msr pmevcntr" #counternumber "_el0, %0" : : "r"(value)); \
    return;
#define SET_PMEVTYPER_BIT(offset, counternumber)                               \
    uint32_t pmevtyper = 42;                                                   \
    asm volatile("mrs %0, pmevtyper" #counternumber "_el0" : "=r"(pmevtyper)); \
    pmevtyper |= (0b1 << offset);                                              \
    asm volatile("msr pmevtyper" #counternumber "_el0, %0" ::"r"(pmevtyper));  \
    return;
#define UNSET_PMEVTYPER_BIT(offset, counternumber)                             \
    uint32_t pmevtyper = 42;                                                   \
    asm volatile("mrs %0, pmevtyper" #counternumber "_el0" : "=r"(pmevtyper)); \
    pmevtyper &= ~(0b1 << offset);                                             \
    asm volatile("msr pmevtyper" #counternumber "_el0, %0" ::"r"(pmevtyper));  \
    return;
#define SET_PMEVTYPER_EVENT(event, counternumber)                              \
    uint32_t pmevtyper = 42;                                                   \
    asm volatile("mrs %0, pmevtyper" #counternumber "_el0" : "=r"(pmevtyper)); \
    pmevtyper &= ~(0b1111111111);                                              \
    pmevtyper |= (event & 0b1111111111);                                       \
    asm volatile("msr pmevtyper" #counternumber "_el0, %0" ::"r"(pmevtyper));  \
    return;

uint32_t PMC::read_event_counter(uint64_t counter_num) {
    EXPAND_BRANCH_NUMBER(counter_num, READ_AND_RETURN_EVENT_COUNTER_N, useless);
    log_error("Cannot access counter number " << counter_num);
    return 0;
}
void PMC::write_event_counter(uint64_t counter_num, uint32_t value) {
    EXPAND_BRANCH_NUMBER(counter_num, WRITE_EVENT_COUNTER_N, value);
    log_error("Cannot access counter number " << counter_num);
}

void PMC::set_count_event(uint64_t counter_num, uint32_t event) {
    EXPAND_BRANCH_NUMBER(counter_num, SET_PMEVTYPER_EVENT, event);
    log_error("Cannot access counter number " << counter_num);
}
void PMC::set_secure_el3_counting(uint64_t counter_num, bool enabled) {
    if (!enabled) {
        EXPAND_BRANCH_NUMBER(counter_num, SET_PMEVTYPER_BIT, 26);
    } else {
        EXPAND_BRANCH_NUMBER(counter_num, UNSET_PMEVTYPER_BIT, 26);
    }
    log_error("Cannot access counter number " << counter_num);
}
void PMC::set_non_secure_el2_counting(uint64_t counter_num, bool enabled) {
    if (!enabled) {
        EXPAND_BRANCH_NUMBER(counter_num, SET_PMEVTYPER_BIT, 27);
    } else {
        EXPAND_BRANCH_NUMBER(counter_num, UNSET_PMEVTYPER_BIT, 27);
    }
    log_error("Cannot access counter number " << counter_num);
}
void PMC::set_non_secure_el0_counting(uint64_t counter_num, bool enabled) {
    if (!enabled) {
        EXPAND_BRANCH_NUMBER(counter_num, SET_PMEVTYPER_BIT, 28);
    } else {
        EXPAND_BRANCH_NUMBER(counter_num, UNSET_PMEVTYPER_BIT, 28);
    }
    log_error("Cannot access counter number " << counter_num);
}
void PMC::set_non_secure_el1_counting(uint64_t counter_num, bool enabled) {
    if (!enabled) {
        EXPAND_BRANCH_NUMBER(counter_num, SET_PMEVTYPER_BIT, 29);
    } else {
        EXPAND_BRANCH_NUMBER(counter_num, UNSET_PMEVTYPER_BIT, 29);
    }
    log_error("Cannot access counter number " << counter_num);
}
void PMC::set_el0_counting(uint64_t counter_num, bool enabled) {
    if (!enabled) {
        EXPAND_BRANCH_NUMBER(counter_num, SET_PMEVTYPER_BIT, 30);
    } else {
        EXPAND_BRANCH_NUMBER(counter_num, UNSET_PMEVTYPER_BIT, 30);
    }
    log_error("Cannot access counter number " << counter_num);
}
void PMC::set_el1_counting(uint64_t counter_num, bool enabled) {
    if (!enabled) {
        EXPAND_BRANCH_NUMBER(counter_num, SET_PMEVTYPER_BIT, 31);
    } else {
        EXPAND_BRANCH_NUMBER(counter_num, UNSET_PMEVTYPER_BIT, 31);
    }
    log_error("Cannot access counter number " << counter_num);
}

void PMC::enable_overflow_interrupt(uint64_t counter_num, bool enabled) {
    if (!enabled) {
        uint64_t pmintenclr = 5;
        asm volatile("mrs %0, pmintenclr_el1" : "=r"(pmintenclr));
        pmintenclr |= (0b1 << counter_num);
        asm volatile("msr pmintenclr_el1, %0" ::"r"(pmintenclr));
    } else {
        uint64_t pmintenset = 5;
        asm volatile("mrs %0, pmintenset_el1" : "=r"(pmintenset));
        pmintenset |= (0b1 << counter_num);
        asm volatile("msr pmintenset_el1, %0" ::"r"(pmintenset));
    }
}