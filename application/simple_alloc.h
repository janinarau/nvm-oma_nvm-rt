#pragma once

#define SIMPLE_ALLOC_SIZE 16384

#define NULL 0

void *malloc(unsigned long size);

void free(void *mem);