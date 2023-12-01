#ifndef SYSTEM_SYSCALL
#define SYSTEM_SYSCALL

#include "stdint.h"

class Syscall {
   public:
    static Syscall instance;

    void stop_system();
    void shutdown_system();

#ifdef STACK_BALANCIMG
    void pause_relocation(uint64_t *saved_stack_base);
    void resume_relocation(uint64_t *saved_stack_base);
#endif
};

#endif