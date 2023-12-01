#ifndef SYSTEM_DRIVER_GIC
#define SYSTEM_DRIVER_GIC

#include <system/stdint.h>

class GIC400_Distributor {
   private:
    uint32_t itLineNumber;

    volatile uint32_t *soc_gic_distributor = (volatile uint32_t *)0x2c001000;

    volatile uint32_t *gicd_ctlr = soc_gic_distributor + 0x0;
    volatile uint32_t *gicd_typer = soc_gic_distributor + 0x1;
    volatile uint32_t *gicd_iidr = soc_gic_distributor + 0x2;
    volatile uint32_t *gicd_igroup = soc_gic_distributor + 0x20;
    volatile uint32_t *gicd_isenabler = soc_gic_distributor + 0x40;
    volatile uint32_t *gicd_icenabler = soc_gic_distributor + 0x60;
    volatile uint32_t *gicd_isactiver = soc_gic_distributor + 0xc0;
    volatile uint32_t *gicd_icactiver = soc_gic_distributor + 0xe0;
    volatile uint32_t *gicd_ipriority = soc_gic_distributor + 0x100;
    volatile uint32_t *gicd_itarget = soc_gic_distributor + 0x200;
    volatile uint32_t *gicd_icfgr = soc_gic_distributor + 0x4300;

   public:
    GIC400_Distributor();

    void initialize();

    void enable();

    void maskInterrupt(uint32_t number, bool mask);
    bool getInterruptMask(uint32_t number);
    bool interruprActive(uint32_t number);
    void setInterruptPriority(uint32_t number, uint8_t priority);
    void setInterruptTargetCPUs(uint32_t number, uint8_t cpu_bitmask);
};

#define ICC_IAR0_EL1		S3_0_C12_C8_0
#define ICC_IAR1_EL1		S3_0_C12_C12_0
#define ICC_EOIR0_EL1		S3_0_C12_C8_1
#define ICC_EOIR1_EL1		S3_0_C12_C12_1
#define ICC_HPPIR0_EL1		S3_0_C12_C8_2
#define ICC_HPPIR1_EL1		S3_0_C12_C12_2
#define ICC_BPR0_EL1		S3_0_C12_C8_3
#define ICC_BPR1_EL1		S3_0_C12_C12_3
#define ICC_DIR_EL1		S3_0_C12_C11_1
#define ICC_PMR_EL1		S3_0_C4_C6_0
#define ICC_RPR_EL1		S3_0_C12_C11_3
#define ICC_CTLR_EL1		S3_0_C12_C12_4
#define ICC_CTLR_EL3		S3_6_C12_C12_4
#define ICC_SRE_EL1		S3_0_C12_C12_5
#define ICC_SRE_EL2		S3_4_C12_C9_5
#define ICC_SRE_EL3		S3_6_C12_C12_5
#define ICC_IGRPEN0_EL1		S3_0_C12_C12_6
#define ICC_IGRPEN1_EL1		S3_0_C12_C12_7
#define ICC_IGRPEN1_EL3		S3_6_C12_C12_7
#define ICC_SEIEN_EL1		S3_0_C12_C13_0
#define ICC_SGI0R_EL1		S3_0_C12_C11_7
#define ICC_SGI1R_EL1		S3_0_C12_C11_5
#define ICC_ASGI1R_EL1		S3_0_C12_C11_6

class GIC400_CPU {
   private:

   public:
    GIC400_CPU();

    void initialize();
    uint16_t raedSignalledInterruptNumber();
    void setPriotityMask(uint8_t highest_prio);

    void signalEndOfInterrupt(uint16_t irq_number);
};

#endif