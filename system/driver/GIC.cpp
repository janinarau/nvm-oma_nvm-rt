#include "GIC.h"
#include <system/service/logger.h>

GIC400_Distributor::GIC400_Distributor() {}

void GIC400_Distributor::initialize() {
    soc_gic_distributor = (volatile uint32_t *)0x2c000000;

    gicd_ctlr = soc_gic_distributor + 0x0;
    gicd_typer = soc_gic_distributor + 0x1;
    gicd_iidr = soc_gic_distributor + 0x2;
    gicd_igroup = soc_gic_distributor + 0x20;
    gicd_isenabler = soc_gic_distributor + 0x40;
    gicd_icenabler = soc_gic_distributor + 0x60;
    gicd_ipriority = soc_gic_distributor + 0x100;
    gicd_itarget = soc_gic_distributor + 0x200;
    gicd_icfgr = soc_gic_distributor + 0x300;

    itLineNumber = (*gicd_typer) & 0xF;

    log("GIC Implements " << dec << (int)(itLineNumber + 1) << " Lines");

    log("GICD ID: " << hex << *gicd_iidr);

    // Set all interrupt prioritys to 0
    for (uint32_t i = 0; i < 8 * (itLineNumber + 1); i++) {
        gicd_ipriority[i] = 0x0;
    }

    // For every interrupt, set CPU0 as target cpu
    for (uint32_t i = 8; i < 8 * (itLineNumber + 1); i++) {
        // gicd_itarget[i] = 0x01010101;
        // gicd_itarget[i] = 0x0;
    }

    // Allocate Interrupts to group 0
    for (uint32_t i = 0; i <= itLineNumber; i++) {
        gicd_igroup[i] = 0x0;
    }

    // DeActivate all interrupts
    for (uint32_t i = 0; i <= itLineNumber; i++) {
        gicd_icenabler[i] = 0xFFFFFFFF;
    }

    // Set Trigger mode to levek
    for (uint32_t i = 0; i < 2 * (itLineNumber + 1); i++) {
        // gicd_icfgr[i] = 0xAAAAAAAA;
        gicd_icfgr[i] = 0x0;
    }

    // Enable the Distributor
    enable();
}

void GIC400_Distributor::enable() { *gicd_ctlr |= 0x1; }

void GIC400_Distributor::maskInterrupt(uint32_t number, bool mask) {
    // Determine register
    uint8_t reg_num = number / 32;
    if (reg_num > itLineNumber) {
        // This interrupt number does not exist
        return;
    }

    // Determine register offset
    uint8_t reg_offset = number % 32;

    if (mask) {
        // Disable the interrupt
        *(gicd_icenabler + reg_num) |= (0b1 << reg_offset);
    } else {
        // Enable the interrupt
        *(gicd_isenabler + reg_num) |= (0b1 << reg_offset);
    }
}
bool GIC400_Distributor::interruprActive(uint32_t number){
    // Determine register
    uint8_t reg_num = number / 32;
    if (reg_num > itLineNumber) {
        // This interrupt number does not exist
        return false;
    }

    // Determine register offset
    uint8_t reg_offset = number % 32;

    return (*(gicd_isactiver + reg_num)) & (0b1 << reg_offset);
    
}
bool GIC400_Distributor::getInterruptMask(uint32_t number) {
    // Determine register
    uint8_t reg_num = number / 32;
    if (reg_num > itLineNumber) {
        // This interrupt number does not exist
        return true;
    }

    // Determine register offset
    uint8_t reg_offset = number % 32;

    // 1 means interrupt is enables, 0 means not
    return !(((*(gicd_isenabler + reg_num)) & (0b1 << reg_offset)) >>
             reg_offset);
}
void GIC400_Distributor::setInterruptPriority(uint32_t number,
                                              uint8_t priority) {
    // Determine register
    uint8_t reg_num = number / 4;
    if (reg_num >= 8 * (itLineNumber + 1)) {
        // This interrupt number does not exist
        return;
    }
    // Determine register offset
    uint8_t reg_offset = (number % 4) * 8;

    // First clear the priority field, then write the value
    *(gicd_ipriority + reg_num) &= ~(0xFF << reg_offset);
    *(gicd_ipriority + reg_num) |= (priority << reg_offset);
}
void GIC400_Distributor::setInterruptTargetCPUs(uint32_t number,
                                                uint8_t cpu_bitmask) {
    // Determine register
    uint8_t reg_num = number / 4;
    if (reg_num >= 8 * (itLineNumber + 1)) {
        // This interrupt number does not exist
        return;
    }
    // Determine register offset
    uint8_t reg_offset = (number % 4) * 8;

    // First clear the priority field, then write the value
    *(gicd_itarget + reg_num) &= ~(0xFF << reg_offset);
    *(gicd_itarget + reg_num) |= (cpu_bitmask << reg_offset);
}

GIC400_CPU::GIC400_CPU() {}

void GIC400_CPU::initialize() {
    // Emable system register interface [ICC_SRE_EL1]
    uint32_t reg_value = 0;

    asm volatile("mrs %0, S3_0_C12_C12_5" : "=r"(reg_value));
    reg_value |= 0b1;
    asm volatile("msr S3_0_C12_C12_5, %0" ::"r"(reg_value));

    // Enable Group 0 Interrupts [ICC_IGRPEN0_EL1]

    asm volatile("mrs %0, S3_0_C12_C12_6" : "=r"(reg_value));
    reg_value |= 0b1;
    asm volatile("msr S3_0_C12_C12_6, %0" ::"r"(reg_value));

    log("GICC online");
}

uint16_t GIC400_CPU::raedSignalledInterruptNumber() {
    uint32_t reg_value = 0;
    // [ICC_IAR0_EL1]
    asm volatile("mrs %0, S3_0_C12_C8_0" : "=r"(reg_value));
    return reg_value & 0xFFFFFFF;
}

void GIC400_CPU::signalEndOfInterrupt(uint16_t irq_number) {
    uint32_t reg_value = 0;
    //[ICC_EOIR0_EL1]
    // asm volatile("mrs %0, S3_0_C12_C8_1" : "=r"(reg_value));
    reg_value |= irq_number;
    asm volatile("msr S3_0_C12_C8_1, %0" ::"r"(reg_value));
}

void GIC400_CPU::setPriotityMask(uint8_t highest_prio){
    uint32_t reg_value = 0;
    //[ICC_PMR_EL1]
    asm volatile("mrs %0, S3_0_C4_C6_0" : "=r"(reg_value));
    reg_value |= highest_prio;
    asm volatile("msr S3_0_C4_C6_0, %0" ::"r"(reg_value));
}