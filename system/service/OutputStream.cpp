#include "OutputStream.h"
#include "system/driver/PL011.h"

void uart_output_func(char c) { PL011::instance.output_char(c); }

OutputStream OutputStream::instance(uart_output_func);

OutputStream::OutputStream(void (*p_func)(char c)) { print_func = p_func; }

char* OutputStream::plot_int(int64_t number, char* array) {
    bool negative = number < 0;
    if (number < 0) {
        number *= -1;
    }
    char* c = plot_int((uint64_t)number, array);
    if (base == 10 && negative) {
        c--;
        *c = '-';
    }
    return c;
}
char* OutputStream::plot_int(uint64_t number, char* array) {
    char* last = array + 64;
    *last = '0';
    if (number > 0) {
        last++;
    }
    while (number > 0) {
        last--;
        switch (number % base) {
            case 0:
                *last = '0';
                break;
            case 1:
                *last = '1';
                break;
            case 2:
                *last = '2';
                break;
            case 3:
                *last = '3';
                break;
            case 4:
                *last = '4';
                break;
            case 5:
                *last = '5';
                break;
            case 6:
                *last = '6';
                break;
            case 7:
                *last = '7';
                break;
            case 8:
                *last = '8';
                break;
            case 9:
                *last = '9';
                break;
            case 10:
                *last = 'a';
                break;
            case 11:
                *last = 'b';
                break;
            case 12:
                *last = 'c';
                break;
            case 13:
                *last = 'd';
                break;
            case 14:
                *last = 'e';
                break;
            case 15:
                *last = 'f';
                break;
        };
        number /= base;
    }
    return last;
}

OutputStream& OutputStream::operator<<(const char* string) {
    char* c = (char*)string;
    return *this << c;
}
OutputStream& OutputStream::operator<<(char* string) {
    while (*string != 0) {
        print_func(*string++);
    }
    return *this;
}
OutputStream& OutputStream::operator<<(uint8_t number) {
    return *this << (uint64_t)number;
}
OutputStream& OutputStream::operator<<(uint16_t number) {
    return *this << (uint64_t)number;
}
OutputStream& OutputStream::operator<<(uint32_t number) {
    return *this << (uint64_t)number;
}
OutputStream& OutputStream::operator<<(uint64_t number) {
    char array[66];
    array[65] = 0;
    return *this << plot_int(number, array);
}
OutputStream& OutputStream::operator<<(int8_t number) {
    return *this << (int64_t)number;
}
OutputStream& OutputStream::operator<<(int16_t number) {
    return *this << (int64_t)number;
}
OutputStream& OutputStream::operator<<(int32_t number) {
    return *this << (int64_t)number;
}
OutputStream& OutputStream::operator<<(int64_t number) {
    char array[66];
    array[65] = 0;
    return *this << plot_int(number, array);
}

OutputStream& OutputStream::operator<<(double number) {
    bool negative = false;
    if (number < 0) {
        negative = true;
        number *= -1;
    }

    uint64_t rounded = (uint64_t)number;
    uint64_t ko = (uint64_t)(number * 10000);
    ko -= rounded * 10000;

    if (negative) {
        *this << "-";
    }
    return *this << dec << rounded << "." << dec << ko;
}

OutputStream& OutputStream::operator<<(void* ptr) {
    return *this << hex << ((uintptr_t)ptr);
}

OutputStream& OutputStream::operator<<(
    OutputStream& (*manipulator)(OutputStream& other)) {
    return manipulator(*this);
}

OutputStream& dec(OutputStream& other) {
    other.base = 10;
    return other;
}
OutputStream& bin(OutputStream& other) {
    other.base = 2;
    return other << "0b";
}
OutputStream& hex(OutputStream& other) {
    other.base = 16;
    return other << "0x";
}
OutputStream& endl(OutputStream& other) { return other << "\n"; }