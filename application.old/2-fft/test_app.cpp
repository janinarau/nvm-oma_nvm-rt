#include "data.h"
#include "system/data/RBTree.h"
#include "system/service/logger.h"

#include <system/driver/math.h>

// Some data inside BSS
uint64_t bss_filler[2048];

void fft(int8_t *target_array_real, int8_t *target_array_imaginary,
         uint8_t *src_array, uint64_t el_count);

void app_init() {
    uint64_t horizon = 500;
    log("Calculating fft for horizon of " << dec << horizon << " values");

    int8_t target_real[horizon];
    int8_t target_imaginary[horizon];
    // for (uint64_t i = 0; i < 100; i++) {
        fft(target_real, target_imaginary, random_number, horizon);
    // }

    // log("Printing result");
    // for (uint64_t i = 0; i < horizon; i++) {
    //     log("[" << dec << i << "]:\t " << dec << target_real[i] << " + " <<
    //     dec
    //             << target_imaginary[i] << "i");
    // }

    asm volatile("svc #0");
}

void fft(int8_t *target_array_real, int8_t *target_array_imaginary,
         uint8_t *src_array, uint64_t el_count) {
    if (el_count == 0) {
        return;
    }
    if (el_count == 1) {
        target_array_real[0] = src_array[0];
        target_array_imaginary[0] = 0;
        return;
    }

    uint64_t l_size = el_count / 2;
    uint64_t r_size = (el_count / 2) + (el_count % 2);

    uint8_t l_src[l_size];
    uint8_t r_src[r_size];

    for (uint64_t i = 0; i < el_count; i++) {
        if (i % 2 == 0) {
            l_src[i / 2] = src_array[i];
        } else {
            r_src[i / 2] = src_array[i];
        }
    }

    int8_t l_target_real[l_size];
    int8_t l_target_imaginary[l_size];
    int8_t r_target_real[r_size];
    int8_t r_target_imaginary[r_size];

    fft(l_target_real, l_target_imaginary, l_src, l_size);
    fft(r_target_real, r_target_imaginary, r_src, r_size);

    for (uint64_t i = 0; i < (el_count / 2); i++) {
        double exp_real =
            Math::cos(-2 * Math::pi() * (((double)(i)) / (el_count)));
        double exp_im =
            Math::sin(-2 * Math::pi() * (((double)(i)) / (el_count)));

        target_array_real[i] =
            l_target_real[i] +
            (r_target_real[i] * exp_real - r_target_imaginary[i] * exp_im);
        target_array_imaginary[i] =
            l_target_imaginary[i] +
            (r_target_real[i] * exp_im + r_target_imaginary[i] * exp_real);

        target_array_real[i + (el_count / 2)] =
            l_target_real[i] -
            (r_target_real[i] * exp_real - r_target_imaginary[i] * exp_im);
        target_array_imaginary[i + (el_count / 2)] =
            l_target_imaginary[i] -
            (r_target_real[i] * exp_im + r_target_imaginary[i] * exp_real);
    }
}