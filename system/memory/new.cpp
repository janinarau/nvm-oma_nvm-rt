#include "new.h"

void *operator new(uint64_t size, void *placement) { return (void *)placement; }