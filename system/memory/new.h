#ifndef SYSTEM_MEMORY_NEW
#define SYSTEM_MEMORY_NEW

#include <system/memory/RelocationSafePointer.h>
#include <system/stdint.h>
#include <system/syscall_interface.h>

void *operator new(uint64_t size, void *placement);

#ifdef STACK_BALANCIMG

#define stack_allocate(type, size, name)           \
    syscall_delay_relocation();                    \
    char __auto_alloc_##name[sizeof(type) * size]; \
    RelocationSafePointer<type> name((type *)__auto_alloc_##name);

#endif

#endif