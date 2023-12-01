#include "PL011.h"

PL011 PL011::instance;

void PL011::output_char(char c){
    volatile unsigned char *UARTDR=(volatile unsigned char *)(PL011_BASE + 0x0);
    volatile unsigned char *UARTFR=(volatile unsigned char *)(PL011_BASE + 0x18);

    while(*UARTFR & (0b1 << 5));
    *UARTDR = c;
}