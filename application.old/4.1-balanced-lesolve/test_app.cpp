#include "system/data/RBTree.h"
#include "system/service/logger.h"

#include <system/driver/math.h>
#include "data.h"

#include <system/memory/StackBalancer.h>
#include <system/memory/stack_relocate.h>

// #define HINTING
#define SOFTONLY

// Some data inside BSS
uint64_t bss_filler[1];

float system_a[RANGE][RANGE];
float right_a[RANGE];

void print_system(float **system, float *right, uint64_t range);

void solve_system(float **system, float *right, uint64_t range);

#ifdef SOFTONLY
uint64_t reloc_count = 0;
#endif

void app_init() {
    float *system_ptr[RANGE];
    for (uint64_t i = 0; i < RANGE; i++) {
        system_ptr[i] = system_a[i];
    }

    // print_system(system_ptr, right, RANGE);

    for (uint64_t outer_runs = 0; outer_runs < 50; outer_runs++) {
        for (uint64_t i = 0; i < RANGE; i++) {
            for (uint64_t j = 0; j < RANGE; j++) {
                system_a[i][j] = system[i][j];
            }
            right_a[i] = right[i];
        }
        solve_system(system_ptr, right_a, RANGE);
    }

#ifdef SOFTONLY
    log_info("Performed " << dec << reloc_count << " relocations so");
#endif

    // print_system(system_ptr, right, RANGE);
    asm volatile("svc #0");
}

void substract_lines(float **system, float *right, uint64_t target,
                     uint64_t src, float *fac, uint64_t range) {
    float *target_line = system[target];
    float *src_line = system[src];
    for (uint64_t i = 0; i < range; i++) {
        float tmp __attribute__((aligned(8))) = src_line[i];
        tmp *= (*fac);
        target_line[i] -= tmp;
    }
    float tmp __attribute__((aligned(8))) = right[src];
    tmp *= (*fac);
    right[target] -= tmp;
}

void swap_lines(float **system, float *right, uint64_t target, uint64_t src,
                uint64_t range) {
    float *target_line = system[target];
    float *src_line = system[src];
    for (uint64_t i = 0; i < range; i++) {
        float acc __attribute__((aligned(8))) = target_line[i];
        target_line[i] = src_line[i];
        src_line[i] = acc;
    }
    float acc __attribute__((aligned(8))) = right[target];
    right[target] = right[src];
    right[src] = acc;
}

void solve_system(float **system, float *right, uint64_t range) {
    // Outer loop over range
    for (uint64_t range_reduce = 0; range_reduce < range; range_reduce++) {
        // First check if line can be used to reduce
        if (system[range_reduce][range_reduce] == 0) {
            for (uint64_t left_over = range_reduce + 1; left_over < range;
                 left_over++) {
                if (system[left_over][range_reduce] != 0) {
                    swap_lines(system, right, range_reduce, left_over, range);
                    break;
                }
            }
            if (system[range_reduce][range_reduce] == 0) {
                log_error("System is not solvable");
                return;
            }
        }

        // Now eliminate the current column everywhere
        for (uint64_t left_over = range_reduce + 1; left_over < range;
             left_over++) {
#ifdef HINTING
            STACK_OUTER_LOOP
#endif
#ifdef SOFTONLY
            if (left_over % 30 == 0) {
                relocate_stack();
                reloc_count++;
            }
#endif
            float reduce_fac __attribute__((aligned(8))) =
                system[left_over][range_reduce];
            reduce_fac /= system[range_reduce][range_reduce];
            substract_lines(system, right, left_over, range_reduce, &reduce_fac,
                            range);
        }
    }

    // Now insert solved variables and eliminate them all
    for (int64_t left_line = range - 1; left_line >= 0; left_line--) {
        // Insert variables first
        for (int64_t i = left_line + 1; i < (int64_t)range; i++) {
#ifdef HINTING
            STACK_OUTER_LOOP
#endif
#ifdef SOFTONLY
            if (left_line % 30 == 0) {
                relocate_stack();
                reloc_count++;
            }
#endif
            float tmp __attribute__((aligned(8))) = system[left_line][i];
            tmp *= right[i];
            right[left_line] -= tmp;
            system[left_line][i] = 0;
        }

        right[left_line] /= system[left_line][left_line];
        system[left_line][left_line] = 1;
    }
}

void print_system(float **system, float *right, uint64_t range) {
    log("Printing equation system:");
    for (uint64_t y = 0; y < range; y++) {
        for (uint64_t x = 0; x < range; x++) {
            if (system[y][x] > 0.00001 || system[y][x] < -0.00001) {
                if (system[y][x] == 1) {
                } else if (system[y][x] == -1) {
                    OutputStream::instance << "-";
                } else {
                    OutputStream::instance << system[y][x] << " ";
                }
                OutputStream::instance << "x" << dec << x << "\t";
            }
        }
        OutputStream::instance << "=\t" << dec << right[y] << endl;
    }
}