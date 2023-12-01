#ifndef SYSTEM_DRIVER_PL011
#define SYSTEM_DRIVER_PL011

#define PL011_BASE 0x1c090000

class PL011 {
   public:
    static PL011 instance;
    void output_char(char c);
};

#endif