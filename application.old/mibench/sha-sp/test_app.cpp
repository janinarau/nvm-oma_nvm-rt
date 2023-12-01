#include <system/memory/RelocationSafePointer.h>
#include <system/memory/new.h>
#include "input_large.h"
#include "sha.h"

void app_init() {
    stack_allocate(SHA_INFO, 1, sha_info);

    syscall_delay_relocation();
    RelocationSafePointer<char> input_ptr(sha_input_large);

    sha_stream(sha_info, sha_input_large);
    sha_print(sha_info);

    asm volatile("svc #0");
}