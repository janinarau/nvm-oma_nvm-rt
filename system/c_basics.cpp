extern "C" void *memset(void *str, int c, unsigned long n) {
    unsigned char *set = (unsigned char *)str;
    for (unsigned long i = 0; i < n; i++) {
        *set = ((unsigned char)c);
        set++;
    }
    return str;
}

extern "C" void *memcpy(void *destination, const void *source,
                        unsigned long num) {
    for (unsigned long i = 0; i < num; i++) {
        ((char *)destination)[i] = ((char *)source)[i];
    }
    return destination;
}