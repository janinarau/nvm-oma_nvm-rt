#ifndef SYSTEM_MEMORY_WRITE_MONITOR
#define SYSTEM_MEMORY_WRITE_MONITOR

#include <system/stdint.h>

/**
 * The Write Monitor uses some mechanisms to approximate the write count per
 * page. First a performance counter is used to trigger the monitor every nth
 * write. Second the Access Permission Mechanism of the MMU is used to generate
 * interrupts on write accesses.
 */
class WriteMonitor {
   public:
    static WriteMonitor instance;

    void initialize();
    void terminate();
    void plot_results();
    void add_page_to_observe(void* vm_page);
    void add_stack_page_to_observe(void* vm_page);
    void set_notify_threshold(uint64_t notify_threshold);

    bool handle_data_permission_interrupt();

    void handle_pmc_0_interrupt(uint64_t* saved_stack_base);

   private:
    uint64_t write_count[MONITOR_CAPACITY];
    uint64_t stack_write_count[REAL_STACK_SIZE / 0x1000];
    void* vm_pages_to_observe[MONITOR_CAPACITY];
    void* stack_vm_pages_to_observe[MONITOR_CAPACITY];
    uint64_t observerd_vm_pages_count = 0;
    uint64_t oberved_stack_pages_count = 0;

    uint64_t notify_threshold = UINT64_MAX;

    // Helper functions
    void set_all_observed_pages(bool generate_interrupt_on_write);
};

#endif