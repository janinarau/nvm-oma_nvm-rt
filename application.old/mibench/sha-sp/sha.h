#ifndef SHA_H
#define SHA_H

#include <system/memory/RelocationSafePointer.h>
#include <system/memory/new.h>

/* NIST Secure Hash Algorithm */
/* heavily modified from Peter C. Gutmann's implementation */

/* Useful defines & typedefs */

typedef unsigned char BYTE;
typedef unsigned long LONG;

#define SHA_BLOCKSIZE		64
#define SHA_DIGESTSIZE		20

typedef struct {
    LONG digest[5];		/* message digest */
    LONG count_lo, count_hi;	/* 64-bit bit count */
    LONG data[16];		/* SHA data buffer */
} SHA_INFO;

typedef RelocationSafePointer<SHA_INFO> SHA_INFO_P;

void sha_init(SHA_INFO_P);
void sha_update(SHA_INFO_P, RelocationSafePointer<BYTE>, long);
void sha_final(SHA_INFO_P);

void sha_stream(SHA_INFO_P, RelocationSafePointer<char> data_input);
void sha_print(SHA_INFO_P);

#endif /* SHA_H */
