#include "data.h"
#include "system/data/RBTree.h"
#include "system/service/logger.h"

#include <system/driver/math.h>

#include <system/memory/StackBalancer.h>
#include <system/memory/stack_relocate.h>

// Some data inside BSS
uint64_t bss_filler[2048];

void fft(int64_t *target_array_real, int64_t *target_array_imaginary,
         uint64_t *src_array, uint64_t el_count);

void app_init() {
    uint64_t horizon = 50;
    log("Calculating fft for horizon of " << dec << horizon << " values");

    int64_t target_real[horizon];
    int64_t target_imaginary[horizon];
    // for (uint64_t i = 0; i < 100; i++) {
    fft(target_real, target_imaginary, random_number, horizon);
    // }

    log("Printing result");
    for (uint64_t i = 0; i < horizon; i++) {
        log("[" << dec << i << "]:\t " << dec << target_real[i] << " + " << dec
                << target_imaginary[i] << "i");
    }

    asm volatile("svc #0");
}

int64_t clevel = 0;

void fft(int64_t *target_array_real, int64_t *target_array_imaginary,
         uint64_t *src_array, uint64_t el_count) {
    // STACK_RECURSIVE_FUNC
    clevel++;
    // relocate_stack();
    if (el_count == 0) {
        clevel--;
        return;
    }
    if (el_count == 1) {
        target_array_real[0] = src_array[0];
        target_array_imaginary[0] = 0;
        clevel--;
        return;
    }

    uint64_t l_size = (el_count / 2) + (el_count % 2);
    uint64_t r_size = (el_count / 2);

    uint64_t l_src[l_size];
    uint64_t r_src[r_size];

    for (uint64_t i = 0; i < el_count; i++) {
        if (i % 2 == 0) {
            l_src[i / 2] = src_array[i];
        } else {
            r_src[i / 2] = src_array[i];
        }
    }

    int64_t l_target_real[l_size];
    int64_t l_target_imaginary[l_size];
    int64_t r_target_real[r_size];
    int64_t r_target_imaginary[r_size];

    fft(l_target_real, l_target_imaginary, l_src, l_size);
    if (clevel == 1) {
        log_info("Relocate");
        log_info("l_target_real at " << l_target_real);
        log_info("l_target_imaginary at " << l_target_imaginary);
        log_info("r_target_real at " << r_target_real);
        log_info("r_target_imaginary at " << r_target_imaginary);
        log_info("r_src at " << r_src);
        relocate_stack();
        log_info("l_target_real at " << l_target_real);
        log_info("l_target_imaginary at " << l_target_imaginary);
        log_info("r_target_real at " << r_target_real);
        log_info("r_target_imaginary at " << r_target_imaginary);
        log_info("r_src at " << r_src);
    }
    fft(r_target_real, r_target_imaginary, r_src, r_size);

    for (uint64_t i = 0; i < (el_count / 2); i++) {
        float exp_real __attribute__((aligned(8))) =
            Math::cos_s(-2 * Math::pi_s() * (((float)(i)) / (el_count)));
        float exp_im __attribute__((aligned(8))) =
            Math::sin_s(-2 * Math::pi_s() * (((float)(i)) / (el_count)));

        target_array_real[i] = (int32_t)(
            l_target_real[i] + (int32_t)(r_target_real[i] * exp_real -
                                         r_target_imaginary[i] * exp_im));
        target_array_imaginary[i] =
            (int32_t)(l_target_imaginary[i] +
                      (int32_t)(r_target_real[i] * exp_im +
                                r_target_imaginary[i] * exp_real));

        target_array_real[i + (el_count / 2)] = (int32_t)(
            l_target_real[i] - (int32_t)(r_target_real[i] * exp_real -
                                         r_target_imaginary[i] * exp_im));
        target_array_imaginary[i + (el_count / 2)] =
            (int32_t)(l_target_imaginary[i] -
                      (int32_t)(r_target_real[i] * exp_im +
                                r_target_imaginary[i] * exp_real));
    }
    clevel--;
}