#ifndef PUREC_WOLFSSL_USER_SETTINGS_H
#define PUREC_WOLFSSL_USER_SETTINGS_H

#include <stdlib.h>
#include <string.h>
#include <time.h>

#define SIZEOF_LONG_LONG 8
#define WOLFSSL_GENERAL_ALIGNMENT 16
#define SINGLE_THREADED
#define WOLFSSL_SMALL_STACK
#define WOLFSSL_NO_ASM

#define WOLFSSL_SP_MATH_ALL

#define NO_DH
#define NO_DSA

#define WC_RSA_BLINDING
#define WC_RSA_PSS

#undef HAVE_ECC
#define HAVE_ECC
#define ECC_USER_CURVES
#undef NO_ECC256
#define ECC_SHAMIR
#define ECC_TIMING_RESISTANT

#define HAVE_AES_CBC
#define HAVE_AESGCM
#define GCM_TABLE_4BIT
#define HAVE_AESCCM
#define WOLFSSL_AES_COUNTER

#define NO_DES3

#define HAVE_CHACHA
#define HAVE_POLY1305
#define HAVE_ONE_TIME_AUTH

#define WOLFSSL_CMAC

#define USE_SLOW_SHA
#define WOLFSSL_SHA512
#define WOLFSSL_SHA384
#define NO_MD5
#define NO_MD4
#define NO_RC4

#define HAVE_HKDF
#undef NO_PWDBASED

#define NO_WOLFSSL_MEMORY
#define XMALLOC(s, h, t) ((void)(h), (void)(t), malloc((size_t)(s)))
#define XFREE(p, h, t) \
    do { void *xp_ = (p); if (xp_) free(xp_); } while (0)
#define XREALLOC(p, n, h, t) ((void)(h), (void)(t), realloc((p), (size_t)(n)))

#define HAVE_HASHDRBG
#define CUSTOM_RAND_GENERATE_SEED purec_host_GenerateSeed

#define NO_FILESYSTEM
#define NO_WRITEV
#define NO_MAIN_DRIVER
#define NO_ASN_TIME

#define NO_PSK
#define NO_OLD_TLS
#define NO_SESSION_CACHE

#define WOLFSSL_IGNORE_FILE_WARN

#endif /* PUREC_WOLFSSL_USER_SETTINGS_H */
