#ifndef SYSTEM_SERVICE_OUTPUT_STREAM
#define SYSTEM_SERVICE_OUTPUT_STREAM

#include "system/stdint.h"

class OutputStream {
   public:
    static OutputStream instance;
    OutputStream(void (*)(char c));

   private:
    void (*print_func)(char c);
    char* plot_int(int64_t number, char* array);
    char* plot_int(uint64_t number, char* array);

   public:
    uint64_t base = 10;

    OutputStream& operator<<(const char* string);
    OutputStream& operator<<(char* string);
    OutputStream& operator<<(uint8_t number);
    OutputStream& operator<<(uint16_t number);
    OutputStream& operator<<(uint32_t number);
    OutputStream& operator<<(uint64_t number);
    OutputStream& operator<<(int8_t number);
    OutputStream& operator<<(int16_t number);
    OutputStream& operator<<(int32_t number);
    OutputStream& operator<<(int64_t number);

    OutputStream& operator<<(double number);

    OutputStream& operator<<(void *ptr);

    OutputStream& operator<<(OutputStream& (*manipulator)(OutputStream& other));
};

OutputStream& dec(OutputStream& other);
OutputStream& bin(OutputStream& other);
OutputStream& hex(OutputStream& other);
OutputStream& endl(OutputStream& other);

#endif