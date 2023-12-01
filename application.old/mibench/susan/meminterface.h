#pragma once

extern "C" void *memcpy(void *destination, const void *source,
                        unsigned long num);

extern "C" void *memset(void *str, int c, unsigned long n);

void *malloc(unsigned long size);
void free(void *mem);