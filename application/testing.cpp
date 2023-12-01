#include <system/service/logger.h>

void testDetermineWriteEnergy(){
    volatile long value = 0x0;
    for (long i = 0x0; i < 0x40; i++){ //0x40 = 64
        value = (value << 1) + 0x1;
        //log(" Value: 0x" << value << " Pointer: " << (void*)&value);
    }
}

void testDetermineWritingTime(){
    volatile long long value = 0x0;
    log(" Value: 0x" << (long) value << " Pointer: " << (void*)&value);
    value = 0xffffff;
    log(" Value: 0x" << (long) value << " Pointer: " << (void*)&value);
    value = 0xffffffffffffffff;
    log(" Value: 0x" << (long) value << " Pointer: " << (void*)&value);
}