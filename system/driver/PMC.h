#ifndef SYSTEM_DRIVER_PMC
#define SYSTEM_DRIVER_PMC
#include <system/stdint.h>
class PMC {
   public:
    static PMC instance;

    uint64_t get_num_available_counters();
    bool get_clock_divider();
    void set_clock_divider(bool enabled);
    void reset_cycle_counter();
    void reset_all_event_counter();
    void set_counters_enabled(bool enabled);
    void set_event_counter_enabled(uint64_t counter_num, bool enabled);

    uint32_t read_event_counter(uint64_t counter_num);
    void write_event_counter(uint64_t counter_num, uint32_t value);

    void set_count_event(uint64_t counter_num, uint32_t event);
    void set_secure_el3_counting(uint64_t counter_num, bool enabled);
    void set_non_secure_el2_counting(uint64_t counter_num, bool enabled);
    void set_non_secure_el0_counting(uint64_t counter_num, bool enabled);
    void set_non_secure_el1_counting(uint64_t counter_num, bool enabled);
    void set_el0_counting(uint64_t counter_num, bool enabled);
    void set_el1_counting(uint64_t counter_num, bool enabled);

    void enable_overflow_interrupt(uint64_t counter_num, bool enabled);

    enum event_values { BUS_ACCESS_STORE = 0x61 , BUS_ACCESS=0x19, CPU_CYCLES=0x11};
};

#endif