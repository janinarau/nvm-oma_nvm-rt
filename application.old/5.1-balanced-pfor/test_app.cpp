#include <system/service/logger.h>
#include "data.h"
#include "pfor.h"
#include <system/memory/StackBalancer.h>
#include <system/memory/stack_relocate.h>

#define BENCHMARK

#ifndef BENCHMARK
void simple_test();
void compress_array();
void decompress_and_check();
void decompress_incremental();
#endif
void benchmark();

void app_init() {
    // simple_test();
    // compress_array();
    // decompress_and_check();
    // decompress_incremental();
    benchmark();

    asm volatile("svc #0");
}

void benchmark() {
    /**
     * This benchmark decompresses pfor data in small packets and aggregatves
     * over them
     */
    uint64_t step_size = 40;
    uint64_t runs = 8000 / step_size;
    uint64_t compressed_size = step_size / 4;
    uint64_t repetitions = 10;

    uint32_t __attribute__((aligned(8))) overall_sum = 0;

    for (uint64_t rep = 0; rep < repetitions; rep++) {
        uint64_t next_exception = pfor_d_first_exception;
        uint64_t exception_offset = 0;
        for (uint64_t i = 0; i < runs; i++) {
            STACK_OUTER_LOOP
            // relocate_stack();
            uint64_t uncompressed[step_size];
            uint64_t number_exceptions = 0;
            next_exception = pfor_uncompress(
                pfor_d_compressed + (i * compressed_size), step_size,
                pfor_d_exception_list + exception_offset, next_exception,
                uncompressed, &number_exceptions);
            exception_offset += number_exceptions;

            // Loop over uncompressed data and aggregate
            for (uint64_t x = 0; x < step_size; x++) {
                if (uncompressed[x] != random_number[(step_size * i) + x]) {
                    log_error("Found mismatch");
                    while (1)
                        ;
                }
                overall_sum += uncompressed[x];
            }
        }
    }

    log_info("Uncompressed sum is " << dec << overall_sum);
}

#ifndef BENCHMARK
uint64_t full_compress_compressed[2000];
uint64_t full_compress_exception_list[8000];
void compress_array() {
    log_info("Beginning PFOR Compression");
    log_info("Compressing 8000 Numbers:");

    uint64_t first_exception;
    uint64_t exception_count;

    pfor_compress(random_number, 8000, full_compress_compressed,
                  full_compress_exception_list, &exception_count,
                  &first_exception);

    log_info("Compressed entire array: " << dec << exception_count
                                         << " Exceptions, first at " << dec
                                         << first_exception);

    OutputStream::instance << "\n";
    for (uint64_t i = 0; i < 2000; i++) {
        OutputStream::instance << dec << full_compress_compressed[i] << ",\n";
    }
    OutputStream::instance << "\n\n";
    for (uint64_t i = 0; i < exception_count; i++) {
        OutputStream::instance << dec << full_compress_exception_list[i]
                               << ",\n";
    }
}

uint64_t full_decompress_uncompressed[8000];

void decompress_and_check() {
    log_info("Beginning PFOR Compression");
    log_info("Decompressing 8000 Numbers:");

    pfor_uncompress(pfor_d_compressed, 8000, pfor_d_exception_list,
                    pfor_d_first_exception, full_decompress_uncompressed);

    for (uint64_t i = 0; i < 8000; i++) {
        if (full_decompress_uncompressed[i] != random_number[i]) {
            log_info("Found mismatch at " << dec << i << ": " << dec
                                          << full_decompress_uncompressed[i]
                                          << " != " << dec << random_number[i]);
        }
    }
}

void decompress_incremental() {
    log_info("Beginning PFOR Compression");
    log_info("Decompressing 8000 Numbers incremental (40 each):");
    uint64_t next_exception = pfor_d_first_exception;
    uint64_t exception_offset = 0;
    for (uint64_t i = 0; i < 200; i++) {
        uint64_t uncompressed[40];
        uint64_t number_exceptions = 0;
        next_exception =
            pfor_uncompress(pfor_d_compressed + (i * 10), 40,
                            pfor_d_exception_list + exception_offset,
                            next_exception, uncompressed, &number_exceptions);
        exception_offset += number_exceptions;

        for (uint64_t x = 0; x < 40; x++) {
            if (uncompressed[x] != random_number[(i * 40) + x]) {
                log_info("Found mismatch at " << dec << ((i * 40) + x) << ": "
                                              << dec << uncompressed[x]
                                              << " != " << dec
                                              << random_number[(i * 40) + x]);
            }
        }
    }
    log_info("No errors found");
}

void simple_test() {
    log_info("Beginning PFOR Compression");
    log_info("Compressing 16 Numbers:");
    for (uint64_t i = 0; i < 16; i++) {
        log_info(dec << random_number[i]);
    }

    uint64_t compressed[4];
    uint64_t exception_list[16];
    uint64_t first_exception;
    uint64_t exception_count;

    pfor_compress(random_number, 16, compressed, exception_list,
                  &exception_count, &first_exception);

    log_info("Compressed:");
    for (uint64_t i = 0; i < 4; i++) {
        log_info("[" << dec << i << "]:0 " << dec
                     << (((compressed[i]) & (255 << 0)) >> 0));
        log_info("[" << dec << i << "]:1 " << dec
                     << (((compressed[i]) & (255 << 8)) >> 8));
        log_info("[" << dec << i << "]:2 " << dec
                     << (((compressed[i]) & (255 << 16)) >> 16));
        log_info("[" << dec << i << "]:3 " << dec
                     << (((compressed[i]) & (255 << 24)) >> 24));
    }
    log_info("Exceptions:");
    for (uint64_t i = 0; i < exception_count; i++) {
        log_info("[" << dec << i << "] " << dec << exception_list[i]);
    }
    log_info("First exception at " << dec << first_exception);

    log_info("\n=====Decompressing=====\n");
    uint64_t decompressed[10];
    pfor_uncompress(compressed, 16, exception_list, first_exception,
                    decompressed);

    for (uint64_t i = 0; i < 16; i++) {
        log_info("[" << dec << i << "] " << dec << decompressed[i]);
    }
}
#endif