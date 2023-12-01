/* NIST Secure Hash Algorithm */
/* heavily modified by Uwe Hollerbach uh@alumni.caltech edu */
/* from Peter C. Gutmann's implementation as found in */
/* Applied Cryptography by Bruce Schneier */

/* NIST's proposed modification to SHA of 7/11/94 may be */
/* activated by defining USE_MODIFIED_SHA */

// #include <stdlib.h>
// #include <stdio.h>
// #include <string.h>
#include "sha.h"
#include <system/service/logger.h>
#include <system/stdint.h>

extern "C" void memcpy_safe(RelocationSafePointer<BYTE> destination,
                            RelocationSafePointer<BYTE> source, size_t num) {
    for (unsigned long i = 0; i < num; i++) {
        destination[i] = destination[i];
    }
}
extern "C" void memset_safe(RelocationSafePointer<BYTE> str, long c,
                            unsigned long n) {
    for (unsigned long i = 0; i < n; i++) {
        str[i] = c & 0xFF;
    }
}

/* SHA f()-functions */

#define f1(x, y, z) ((x & y) | (~x & z))
#define f2(x, y, z) (x ^ y ^ z)
#define f3(x, y, z) ((x & y) | (x & z) | (y & z))
#define f4(x, y, z) (x ^ y ^ z)

/* SHA constants */

#define CONST1 0x5a827999L
#define CONST2 0x6ed9eba1L
#define CONST3 0x8f1bbcdcL
#define CONST4 0xca62c1d6L

/* 32-bit rotate */

#define ROT32(x, n) ((x << n) | (x >> (32 - n)))

#define FUNC(n, i)                                            \
    temp = ROT32(A, 5) + f##n(B, C, D) + E + W[i] + CONST##n; \
    E = D;                                                    \
    D = C;                                                    \
    C = ROT32(B, 30);                                         \
    B = A;                                                    \
    A = temp

/* do SHA transformation */

static void sha_transform(SHA_INFO_P sha_info) {
    long i;
    LONG temp, A, B, C, D, E, W[80];

    for (i = 0; i < 16; ++i) {
        W[i] = ((SHA_INFO)(*sha_info)).data[i];
    }
    for (i = 16; i < 80; ++i) {
        W[i] = W[i - 3] ^ W[i - 8] ^ W[i - 14] ^ W[i - 16];
#ifdef USE_MODIFIED_SHA
        W[i] = ROT32(W[i], 1);
#endif /* USE_MODIFIED_SHA */
    }
    A = ((SHA_INFO)(*sha_info)).digest[0];
    B = ((SHA_INFO)(*sha_info)).digest[1];
    C = ((SHA_INFO)(*sha_info)).digest[2];
    D = ((SHA_INFO)(*sha_info)).digest[3];
    E = ((SHA_INFO)(*sha_info)).digest[4];
#ifdef UNROLL_LOOPS
    FUNC(1, 0);
    FUNC(1, 1);
    FUNC(1, 2);
    FUNC(1, 3);
    FUNC(1, 4);
    FUNC(1, 5);
    FUNC(1, 6);
    FUNC(1, 7);
    FUNC(1, 8);
    FUNC(1, 9);
    FUNC(1, 10);
    FUNC(1, 11);
    FUNC(1, 12);
    FUNC(1, 13);
    FUNC(1, 14);
    FUNC(1, 15);
    FUNC(1, 16);
    FUNC(1, 17);
    FUNC(1, 18);
    FUNC(1, 19);

    FUNC(2, 20);
    FUNC(2, 21);
    FUNC(2, 22);
    FUNC(2, 23);
    FUNC(2, 24);
    FUNC(2, 25);
    FUNC(2, 26);
    FUNC(2, 27);
    FUNC(2, 28);
    FUNC(2, 29);
    FUNC(2, 30);
    FUNC(2, 31);
    FUNC(2, 32);
    FUNC(2, 33);
    FUNC(2, 34);
    FUNC(2, 35);
    FUNC(2, 36);
    FUNC(2, 37);
    FUNC(2, 38);
    FUNC(2, 39);

    FUNC(3, 40);
    FUNC(3, 41);
    FUNC(3, 42);
    FUNC(3, 43);
    FUNC(3, 44);
    FUNC(3, 45);
    FUNC(3, 46);
    FUNC(3, 47);
    FUNC(3, 48);
    FUNC(3, 49);
    FUNC(3, 50);
    FUNC(3, 51);
    FUNC(3, 52);
    FUNC(3, 53);
    FUNC(3, 54);
    FUNC(3, 55);
    FUNC(3, 56);
    FUNC(3, 57);
    FUNC(3, 58);
    FUNC(3, 59);

    FUNC(4, 60);
    FUNC(4, 61);
    FUNC(4, 62);
    FUNC(4, 63);
    FUNC(4, 64);
    FUNC(4, 65);
    FUNC(4, 66);
    FUNC(4, 67);
    FUNC(4, 68);
    FUNC(4, 69);
    FUNC(4, 70);
    FUNC(4, 71);
    FUNC(4, 72);
    FUNC(4, 73);
    FUNC(4, 74);
    FUNC(4, 75);
    FUNC(4, 76);
    FUNC(4, 77);
    FUNC(4, 78);
    FUNC(4, 79);
#else  /* !UNROLL_LOOPS */
    for (i = 0; i < 20; ++i) {
        FUNC(1, i);
    }
    for (i = 20; i < 40; ++i) {
        FUNC(2, i);
    }
    for (i = 40; i < 60; ++i) {
        FUNC(3, i);
    }
    for (i = 60; i < 80; ++i) {
        FUNC(4, i);
    }
#endif /* !UNROLL_LOOPS */
    SHA_INFO tmp = *sha_info;
    tmp.digest[0] += A;
    tmp.digest[1] += B;
    tmp.digest[2] += C;
    tmp.digest[3] += D;
    tmp.digest[4] += E;
    *sha_info = tmp;
}

#ifdef LITTLE_ENDIAN

/* change endianness of data */

static void byte_reverse(RelocationSafePtr<LONG> buffer, long count) {
    long i;
    unsigned long ct;
    unsigned long cp = *buffer;
    // BYTE ct[4], *cp;

    count /= sizeof(LONG);
    // cp = (BYTE *)buffer;
    for (i = 0; i < count; ++i) {
        ct &= ~(0xFFFFFFFFUL);
        ct |= (cp & 0xFFFFFFFFUL);
        // ct[0] = cp[0];
        // ct[1] = cp[1];
        // ct[2] = cp[2];
        // ct[3] = cp[3];
        cp &= ~(0xFFFFFFFFUL);
        cp |= ((ct & 0xFF) << 24) cp |= ((ct & 0xFF00) << 8) cp |=
            ((ct & 0xFF0000) >> 8) cp |= ((ct & 0xFF000000) >> 24)
            // cp[0] = ct[3];
            // cp[1] = ct[2];
            // cp[2] = ct[1];
            // cp[3] = ct[0];
            buffer++;
        cp = *buffer;
        // cp += sizeof(LONG);
    }
}

#endif /* LITTLE_ENDIAN */

/* initialize the SHA digest */

void sha_init(SHA_INFO_P sha_info) {
    SHA_INFO tmp = *sha_info;
    tmp.digest[0] = 0x67452301L;
    tmp.digest[1] = 0xefcdab89L;
    tmp.digest[2] = 0x98badcfeL;
    tmp.digest[3] = 0x10325476L;
    tmp.digest[4] = 0xc3d2e1f0L;
    tmp.count_lo = 0L;
    tmp.count_hi = 0L;
    *sha_info = tmp;
}

/* update the SHA digest */

void sha_update(SHA_INFO_P sha_info, RelocationSafePointer<BYTE> buffer,
                long count) {
    SHA_INFO tmp = *sha_info;
    if ((tmp.count_lo + ((LONG)count << 3)) < tmp.count_lo) {
        ++tmp.count_hi;
    }
    tmp.count_lo += (LONG)count << 3;
    tmp.count_hi += (LONG)count >> 29;
    syscall_delay_relocation();
    RelocationSafePointer<BYTE> data_ptr((unsigned char *)(&tmp.data));
    while (count >= SHA_BLOCKSIZE) {
        memcpy_safe(data_ptr, buffer, SHA_BLOCKSIZE);
#ifdef LITTLE_ENDIAN
        byte_reverse((RelocationSafePointer<LONG>)data_ptr, SHA_BLOCKSIZE);
#endif /* LITTLE_ENDIAN */
        *sha_info = tmp;
        sha_transform(sha_info);
        tmp = *sha_info;
        buffer += SHA_BLOCKSIZE;
        count -= SHA_BLOCKSIZE;
    }
    memcpy_safe(data_ptr, buffer, count);
    *sha_info = tmp;
}

/* finish computing the SHA digest */

void sha_final(SHA_INFO_P sha_info) {
    SHA_INFO tmp = *sha_info;
    long count;
    LONG lo_bit_count, hi_bit_count;

    lo_bit_count = tmp.count_lo;
    hi_bit_count = tmp.count_hi;
    count = (long)((lo_bit_count >> 3) & 0x3f);
    ((BYTE *)tmp.data)[count++] = 0x80;
    syscall_delay_relocation();
    RelocationSafePointer<BYTE> data_ptr((unsigned char *)(&tmp.data));
    if (count > 56) {
        memset_safe(data_ptr + count, 0, 64 - count);
#ifdef LITTLE_ENDIAN
        byte_reverse((RelocationSafePointer<LONG>)data_ptr, SHA_BLOCKSIZE);
#endif /* LITTLE_ENDIAN */
        *sha_info = tmp;
        sha_transform(sha_info);
        tmp = *sha_info;
        memset_safe(data_ptr, 0, 56);
    } else {
        memset_safe(data_ptr + count, 0, 56 - count);
    }
#ifdef LITTLE_ENDIAN
    byte_reverse((RelocationSafePointer<LONG>)data_ptr, SHA_BLOCKSIZE);
#endif /* LITTLE_ENDIAN */
    tmp.data[14] = hi_bit_count;
    tmp.data[15] = lo_bit_count;
    *sha_info = tmp;
    sha_transform(sha_info);
}

/* compute the SHA digest of a FILE stream */

#define BLOCK_SIZE 8192

void sha_stream(SHA_INFO_P sha_info, RelocationSafePointer<char> data_input) {
    int i;
    stack_allocate(BYTE, BLOCK_SIZE, data);
    // BYTE data[BLOCK_SIZE];

    RelocationSafePointer<char> read_ptr = data_input;

    sha_init(sha_info);
    while (*read_ptr != 0) {
        i = 0;
        for (unsigned long x = 0; x < BLOCK_SIZE; x++) {
            if (*read_ptr != 0) {
                data[x] = *read_ptr;
                i++;
            } else {
                break;
            }
            read_ptr++;
        }
        sha_update(sha_info, data, i);
    }
    sha_final(sha_info);
}

/* print a SHA digest */

void sha_print(SHA_INFO_P sha_info) {
    SHA_INFO tmp = *sha_info;
    log_info("sha result: " << hex << tmp.digest[0] << hex << tmp.digest[1]
                            << hex << tmp.digest[2] << hex << tmp.digest[3]
                            << hex << tmp.digest[4]);
}
