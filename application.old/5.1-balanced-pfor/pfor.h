#ifndef ALGORITHM_PFOR
#define ALGORITHM_PFOR

#include <system/stdint.h>

/**
 * How much bits, each compressed word has
 */
#define COMPRESSION_SIZE 8UL
#define MAX_STORE_VALUE 255UL

void pfor_compress(uint64_t *input_numbers, uint64_t input_size,
                   uint64_t *output_numbers, uint64_t *exception_list,
                   uint64_t *exception_count, uint64_t *first_exception);

uint64_t pfor_uncompress(uint64_t *input_array, uint64_t input_size,
                         uint64_t *exception_list, uint64_t first_execption,
                         uint64_t *uncompressed, uint64_t *exception_count_p = 0);

#endif