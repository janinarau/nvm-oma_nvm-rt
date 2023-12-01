#include "MMU.h"
#include <system/service/logger.h>

MMU MMU::instance;

/**
 * Level 0 Table controls 512GB memory
 * =>1 Entry -- 512GB
 */
uint64_t level0_table[1] __attribute__((aligned(4096)));

/**
 * Level 1 Table controls 1GB memory each
 * =>6 Entry -- 6GB
 * =>40 Entry
 * The virtual stack is at 40GB
 * The DRAM begins at 2GB Boundary (0x80000000), with 4GB DRAM we need 6 GB VM
 * Spave in total
 */
uint64_t level1_table[50] __attribute__((aligned(4096)));

/**
 * Level 2 Table controls 2MB memory each
 * =>2048 Entry -- 4GB
 * L2 Table begins at 2GB Boundary
 * =>512 entries (1GB for stack)
 */
uint64_t level2_table[4096] __attribute__((aligned(4096)));
uint64_t level2_table_stack[512] __attribute__((aligned(4096)));

/**
 * Level 3 Table controls 4kb memory each
 * =>DRAM / 4096 Entry
 * L3 Table begins at 2GB Boundary
 */
uint64_t level3_table[DRAM_SIZE / 4096] __attribute__((aligned(4096)));
uint64_t level3_table_stack[STACK_SIZE / 4096] __attribute__((aligned(4096)));

void MMU::setup_pagetables() {
    /**
     * Some steps required:
     * 1: Set MMU Granularity to 4KB
     * 2: Define memory types
     * 3: Write descriptor entries
     */

    // Set granularity to 4KB
    uint64_t tcr = 0UL;
    tcr |= (0b001UL << 32);  // 36 Address Bits
    tcr |= (0b10UL << 30);   // 4KB Granule for TTBR1
    tcr |= (0b01UL << 26);   // Outer WB, allocate for TTBR1
    tcr |= (0b01UL << 24);   // Inner WB, allocate for TTBR1
    tcr |= (0b1UL << 23);    // Disable TTBR1

    tcr |= (0b01UL << 10);  // Outer WB, allocate for TTBR0
    tcr |= (0b01UL << 8);   // Inner WB, allocate for TTBR0
    tcr |= (0b00UL << 14);  // 4KB Granule for TTBR0
    asm volatile("msr tcr_el1, %0" ::"r"(tcr));

    // Setup memory types. Basically one for dram and one for everything else
    uint8_t normal_outer_inner_wb_transient = 0b01110111;
    uint8_t device_memory = 0b00000000;
    uint64_t mair = 0;
    asm volatile("mrs %0, mair_el1" : "=r"(mair));
    mair &= ~(0xFFFF);
    mair |= (device_memory);                         // ATTR0
    mair |= (normal_outer_inner_wb_transient << 8);  // ATTR1
    asm volatile("msr mair_el1, %0" ::"r"(mair));

    // Setup pagetables
    log("Setting UP Level 0 Table");
    // Begin with level 0
    level0_table[0] = (0b11) | (uint64_t)level1_table;

    log("Setting UP Level 1 Table");
    // Level 1 Table: Redirects to Table 2

    for (uint64_t i = 0; i < 6; i++) {
        // level2_table, redirecting to level2 table
        level1_table[i] = (0b11) | (((uint64_t)level2_table) + i * 512 * 8);
    }
    // The stack l1 table is 1 gb at 39GB
    level1_table[40] = (0b11) | (((uint64_t)level2_table_stack));

    log("Setting UP Level 2 Table");
    // First 2 GB of Level 2 Table are device memory blocks
    for (uint64_t i = 0; i < 1024; i++) {
        level2_table[i] = (0b01) | (i * 0x200000) | (0b100011000 << 2);
        if (i % 51 == 0) {
            OutputStream::instance << ".";
        }
    }
    // Level 2 Table: Completely redirecting to Level 3
    for (uint64_t i = 0; i < 2048; i++) {
        level2_table[1024 + i] =
            (0b11) | (((uint64_t)level3_table) + i * 512 * 8);
        if (i % 102 == 0) {
            OutputStream::instance << ".";
        }
    }
    OutputStream::instance << endl;
    // Level 2 Table Stack: Completely redirecting to Level 3 Stack
    for (uint64_t i = 0; i <= (STACK_SIZE / 2097152); i++) {
        level2_table_stack[i] =
            (0b11) | (((uint64_t)level3_table_stack) + i * 512 * 8);
    }

    log("Setting UP Level 3 Table");
    // Level 3 Table: Block entries from Offset 0x80000000UL
    for (uint64_t i = 0; i < (DRAM_SIZE / 4096); i++) {
        level3_table[i] =
            (0b11) | (0b111001001 << 2) | (0x80000000UL + (i * 0x1000));
        if (i % (DRAM_SIZE / (4096 * 10 * 4)) == 0) {
            OutputStream::instance << ".";
        }
    }
    OutputStream::instance << endl;
    for (uint64_t i = 0; i <= (STACK_SIZE / 4096); i++) {
        level3_table_stack[i] = (0b11) | (0b111001001 << 2) |
                                (TARGET_STACK_VADDRESS + (i * 0x1000));
    }

    // Finally set the ttbr0 register to level 0 table
    asm volatile("msr ttbr0_el1, %0" ::"r"(level0_table));
    asm volatile("isb");
}
void MMU::activate_mmu() {
    asm volatile(
        "isb;"
        "dsb ishst;"
        "tlbi VMALLE1;"
        "ic IALLU;"
        "dsb sy;"
        "isb;"
        "mrs x0, sctlr_el1;"
        "orr x0, x0, #0x1;"
        "msr sctlr_el1, x0;"
        "tlbi vmalle1;"
        "dsb sy;"
        "isb;" ::
            : "x0");
}
void MMU::activate_caches() {
    asm volatile(
        "mrs x0, sctlr_el1;"
        "orr x0, x0, #(0x1 << 2);"
        "orr x0, x0, #(0x1 << 12);"
        "msr sctlr_el1, x0;"
        "dsb sy;"
        "isb;" ::
            : "x0");
}
void MMU::clean_and_disable_caches() {
    asm volatile(
        // Deactivate Cache
        "mrs x0, sctlr_el1;"
        "bic x0,x0,#(0x1 << 2);"
        "bic x0,x0,#(0x1 << 12);"
        "msr sctlr_el1, x0;"
        // Invalidate Cache Content
        "mov x0, #0x0;"
        "msr csselr_el1, x0;"
        "mrs x4, ccsidr_el1;"
        "and x1, x4, #0x7;"
        "ldr x3, =0x7FFF;"
        "and x2,x3,x4,lsr #13;"
        "ldr x3, =0x3FF;"
        "and x3,x3,x4,lsr #3;"
        "clz w4,w3;"
        "mov x5, #0;"
        "way_loop:"
        "mov x6, #0;"
        "set_loop:"
        "lsl x7,x5,x4;"
        "orr x7,x0,x7;"
        "lsl x8,x6,x1;"
        "orr x7,x7,x8;"
        "dc cisw, x7;"
        "add x6,x6, #1;"
        "cmp x6,x2;"
        "ble set_loop;"
        "add x5, x5, #1;"
        "cmp x5, x3;"
        "ble way_loop;" ::
            : "x0", "x1", "x2", "x3", "x4", "x5", "x6", "x7", "x8");
}

void MMU::flush_tlb() {
    asm volatile(
        "dsb ishst;"
        "tlbi vmalle1;"
        "dsb ish;"
        "isb;");
}
void MMU::invalidate_tlb_entry(void *vm_page) {
    uintptr_t page = (uintptr_t)vm_page;
    page &= ~(0xfff);
    asm volatile(
        "mov x0, %0;"
        "dsb ishst;"
        "tlbi vaae1, x0;"
        "dsb ish;"
        "isb;" ::"r"(page)
        : "x0");
    // Does not work somehow :-(, maybe gem5
    flush_tlb();
}
void MMU::set_access_flag(void *vm_page, bool af) {
    // Check if it is a mapped dram page
    uintptr_t page = (uintptr_t)vm_page;
    if (page < 0x80000000UL) {
        log_error("Trying to set access flag for no dram page " << vm_page);
        while (1)
            ;
    }
    if (page >= 0x80000000UL + DRAM_SIZE) {
        log_error(
            "System does not manage this page (maybe extend the managed "
            "memory)");
        while (1)
            ;
    }

    uint64_t *l3_desc = &level3_table[(page - 0x80000000UL) / 0x1000];
    if (af) {
        *l3_desc |= (0b1 << 10);
    } else {
        *l3_desc &= ~(0b1 << 10);
    }
}
void MMU::set_access_permission(void *vm_page, uint64_t ap) {
    // Check if it is a mapped dram page
    uintptr_t page = (uintptr_t)vm_page;
    if (page < 0x80000000UL) {
        log_error("Trying to set access flag for no dram page " << vm_page);
        while (1)
            ;
    }
    if (page >= 0x80000000UL + DRAM_SIZE) {
        log_error(
            "System does not manage this page (maybe extend the managed "
            "memory)");
        while (1)
            ;
    }

    uint64_t *l3_desc = &level3_table[(page - 0x80000000UL) / 0x1000];
    *l3_desc &= ~(0b11 << 6);
    *l3_desc |= (ap & 0b11) << 6;
}
void *MMU::get_mapping(void *vm_page) {
    // Check if it is a mapped dram page
    uintptr_t page = (uintptr_t)vm_page;
    if (page < 0x80000000UL) {
        log_error("Trying to get the mapping for no dram page " << vm_page);
        while (1)
            ;
    }
    if (page >= 0x80000000UL + DRAM_SIZE) {
        log_error(
            "System does not manage this page (maybe extend the managed "
            "memory)");
        while (1)
            ;
    }

    uint64_t *l3_desc = &level3_table[(page - 0x80000000UL) / 0x1000];
    return (void *)((*l3_desc) & 0xFFFFFFFFF000);
}

void MMU::set_page_mapping(void *vm_page, void *phys_page) {
    // Check if it is a mapped dram page
    uintptr_t page = (uintptr_t)vm_page;
    if (page < 0x80000000UL) {
        log_error("Trying to get the mapping for no dram page " << vm_page);
        while (1)
            ;
    }
    if (page >= 0x80000000UL + DRAM_SIZE) {
        log_error(
            "System does not manage this page (maybe extend the managed "
            "memory)");
        while (1)
            ;
    }

    // log_info("Mapping" << hex << vm_page << " -> " << hex << phys_page);

    uint64_t *l3_desc = &level3_table[(page - 0x80000000UL) / 0x1000];
    (*l3_desc) &= ~0xFFFFFFFFF000;                         // Clear mapping
    (*l3_desc) |= ((uint64_t)phys_page) & 0xFFFFFFFFF000;  // Set mapping
}

void MMU::set_stack_page_mapping(void *vm_page, void *phys_page) {
    // Check if it is a mapped stack page
    uintptr_t page = (uintptr_t)vm_page;
    if (page < TARGET_STACK_VADDRESS) {
        log_error("Trying to get the mapping for no dram page " << vm_page);
        while (1)
            ;
    }
    if (page >= TARGET_STACK_VADDRESS + (STACK_SIZE / 2)) {
        log_error(
            "System does not manage this page (maybe extend the managed stack "
            "memory)");
        while (1)
            ;
    }

    // log_info("Mapping Stack" << hex << vm_page << " -> " << hex << phys_page);

    uint64_t *l3_desc =
        &level3_table_stack[(page - TARGET_STACK_VADDRESS) / 0x1000];
    // log_info("Prev mapped to " << ((void *)((*l3_desc) & 0xFFFFFFFFF000)));
    (*l3_desc) &= ~0xFFFFFFFFF000;                         // Clear mapping
    (*l3_desc) |= ((uint64_t)phys_page) & 0xFFFFFFFFF000;  // Set mapping
    uint64_t *l3_desc_shadow =
        &level3_table_stack[(page - TARGET_STACK_VADDRESS + (STACK_SIZE / 2)) /
                            0x1000];
    // log_info("Prev mapped to " << ((void *)((*l3_desc_shadow) &
    // 0xFFFFFFFFF000)));
    (*l3_desc_shadow) &= ~0xFFFFFFFFF000;  // Clear mapping
    (*l3_desc_shadow) |= ((uint64_t)phys_page) & 0xFFFFFFFFF000;  // Set mapping
}

void MMU::set_stack_access_permission(void *vm_page, uint64_t ap) {
    // Check if it is a mapped stack page
    uintptr_t page = (uintptr_t)vm_page;
    if (page < TARGET_STACK_VADDRESS) {
        log_error("Trying to get the mapping for no dram page " << vm_page);
        while (1)
            ;
    }
    if (page >= TARGET_STACK_VADDRESS + (STACK_SIZE / 2)) {
        log_error(
            "System does not manage this page (maybe extend the managed stack "
            "memory)");
        while (1)
            ;
    }

    uint64_t *l3_desc =
        &level3_table_stack[(page - TARGET_STACK_VADDRESS) / 0x1000];
    *l3_desc &= ~(0b11 << 6);
    *l3_desc |= (ap & 0b11) << 6;
    uint64_t *l3_desc_shadow =
        &level3_table_stack[(page - TARGET_STACK_VADDRESS + (STACK_SIZE / 2)) /
                            0x1000];
    *l3_desc_shadow &= ~(0b11 << 6);
    *l3_desc_shadow |= (ap & 0b11) << 6;
}

void *MMU::get_stack_mapping(void *vm_page) {
    // Check if it is a mapped dram page
    uintptr_t page = (uintptr_t)vm_page;
    if (page < TARGET_STACK_VADDRESS) {
        log_error("Trying to get the mapping for no dram page " << vm_page);
        while (1)
            ;
    }
    if (page >= TARGET_STACK_VADDRESS + STACK_SIZE) {
        log_error(
            "System does not manage this page (maybe extend the managed "
            "memory)");
        while (1)
            ;
    }

    uint64_t *l3_desc = &level3_table_stack[(page - TARGET_STACK_VADDRESS) / 0x1000];
    return (void *)((*l3_desc) & 0xFFFFFFFFF000);
}