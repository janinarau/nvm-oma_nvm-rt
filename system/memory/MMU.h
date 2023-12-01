#ifndef SYSTEM_MEMORY_MMU
#define SYSTEM_MEMORY_MMU

#include <system/stdint.h>

class MMU {
   public:
    static MMU instance;

    void setup_pagetables();
    void activate_mmu();
    void activate_caches();
    void clean_and_disable_caches();

    /**
     * This will flush the entire TLB. This might cause hughe performance
     * impact, so better invalidate selective lines, if needed
     */
    void flush_tlb();
    /**
     * This will only uinvalidate TLB entries for the specific Virtual address
     */
    void invalidate_tlb_entry(void *vm_page);
    void set_access_flag(void *vm_page, bool af);
    enum ACCESS_PERMISSION {
        RW_FROM_EL1 = 0b00,
        RW_FROM_EL1_EL0 = 0b01,
        R_FROM_EL1 = 0b10,
        R_FROM_EL1_EL0 = 0b11
    };
    void set_access_permission(void *vm_page, uint64_t ap);
    void *get_mapping(void *vm_page);

    void set_page_mapping(void *vm_page, void *phys_page);
    /**
     * This function sets mapping according to shadow the stack
     */
    void set_stack_page_mapping(void *vm_page, void *phys_page);
    void set_stack_access_permission(void *vm_page, uint64_t ap);
    void *get_stack_mapping(void *vm_page);
};

#endif