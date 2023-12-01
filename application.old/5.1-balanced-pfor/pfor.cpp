#include "pfor.h"

void pfor_compress(uint64_t *input_numbers, uint64_t input_size,
                   uint64_t *output_numbers, uint64_t *exception_list,
                   uint64_t *exception_count, uint64_t *first_exception) {
    uint64_t *last_exception_entry = first_exception;
    uint64_t last_exception_entry_offset = 0;
    uint64_t steps_since_exception = 0;

    *exception_count = 0;

    for (uint64_t i = 0; i < input_size; i++) {
        /**
         * The value is compressable and no exception
         * If the value is compressable, but the exception distance reached its
         * maximum, it has to be stored as an exception anyhow
         */
        if (input_numbers[i] <= MAX_STORE_VALUE &&
            steps_since_exception < MAX_STORE_VALUE) {
            // The target output
            output_numbers[i / (32 / COMPRESSION_SIZE)] &=
                ~(MAX_STORE_VALUE
                  << (COMPRESSION_SIZE * (i % (32 / COMPRESSION_SIZE))));
            output_numbers[i / (32 / COMPRESSION_SIZE)] |=
                (input_numbers[i]
                 << (COMPRESSION_SIZE * (i % (32 / COMPRESSION_SIZE))));
            steps_since_exception++;
        } else {
            // The current value is stored as an exception
            exception_list[(*exception_count)++] = input_numbers[i];
            *last_exception_entry &=
                ~(MAX_STORE_VALUE
                  << (COMPRESSION_SIZE * last_exception_entry_offset));
            *last_exception_entry |=
                (steps_since_exception
                 << (COMPRESSION_SIZE * last_exception_entry_offset));
            steps_since_exception = 0;
            last_exception_entry =
                output_numbers + (i / (32 / COMPRESSION_SIZE));
            last_exception_entry_offset = i % (32 / COMPRESSION_SIZE);
        }
    }

    *last_exception_entry &=
        ~(MAX_STORE_VALUE << (COMPRESSION_SIZE * last_exception_entry_offset));
    *last_exception_entry |=
        (steps_since_exception
         << (COMPRESSION_SIZE * last_exception_entry_offset));
}

uint64_t pfor_uncompress(uint64_t *input_array, uint64_t input_size,
                         uint64_t *exception_list, uint64_t first_execption,
                         uint64_t *uncompressed, uint64_t *exception_count_p) {
    uint64_t current_offset = 0;
    uint64_t exception_countdown = first_execption;
    uint64_t exception_count = 0;

    for (uint64_t i = 0; i < input_size; i++) {
        // Extract the value
        uint64_t decompressed = input_array[i / (32 / COMPRESSION_SIZE)];
        decompressed = decompressed >> (current_offset * COMPRESSION_SIZE);
        decompressed &= MAX_STORE_VALUE;

        current_offset = (current_offset + 1) % (32 / COMPRESSION_SIZE);

        if (exception_countdown == 0) {
            uncompressed[i] = exception_list[exception_count++];
            exception_countdown = decompressed;
        } else {
            exception_countdown--;
            uncompressed[i] = decompressed;
        }
    }

    if (exception_count_p != 0) {
        *exception_count_p = exception_count;
    }

    return exception_countdown;
}