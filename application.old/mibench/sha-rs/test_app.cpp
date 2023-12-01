#include "sha.h"
#include "input_large.h"

void app_init() {
    SHA_INFO sha_info;

    sha_stream(&sha_info, sha_input_large);
    sha_print(&sha_info);

    asm volatile("svc #0");
}