/* PureC OS wolfSSL user settings.
 * Included by wolfssl/wolfcrypt/settings.h when -DWOLFSSL_USER_SETTINGS is set.
 * Shared by the userspace library (libwolf.a) and the kernel module (wolf.ko).
 * The kernel build additionally defines WOLF_PUREC_KERNEL.
 */

#ifndef PUREC_WOLFSSL_USER_SETTINGS_H
#define PUREC_WOLFSSL_USER_SETTINGS_H

/* ------------------------------------------------------------------------- */
/* Platform                                                                  */
/* ------------------------------------------------------------------------- */

#define SIZEOF_LONG_LONG 8
#define WOLFSSL_GENERAL_ALIGNMENT 16

/* One wolfSSL call runs on one CPU thread. The kernel drivers call the
 * module from scheduler threads, never from interrupt context. */
#define SINGLE_THREADED

/* Kernel scheduler stacks are 16 KiB, so push big locals to the heap. */
#define WOLFSSL_SMALL_STACK

/* Pure C only: the kernel image is built with -mgeneral-regs-only and the
 * AES-NI/SP assembly paths would still emit MMX/AVX state transitions. */
#define WOLFSSL_NO_ASM

/* ------------------------------------------------------------------------- */
/* Math                                                                      */
/* ------------------------------------------------------------------------- */

/* SP math for every key size and curve (integer bignum engine of wolfSSL). */
#define WOLFSSL_SP_MATH_ALL
#ifdef WOLF_PUREC_KERNEL
#define WOLFSSL_SP_SMALL
#endif

/* ------------------------------------------------------------------------- */
/* Public key crypto                                                         */
/* ------------------------------------------------------------------------- */

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

/* ------------------------------------------------------------------------- */
/* Symmetric crypto                                                          */
/* ------------------------------------------------------------------------- */

#define HAVE_AES_CBC
#define HAVE_AESGCM
#define GCM_TABLE_4BIT
/* 802.11 CCMP is AES-CCM. */
#define HAVE_AESCCM
#define WOLFSSL_AES_COUNTER

#define NO_DES3

#define HAVE_CHACHA
#define HAVE_POLY1305
#define HAVE_ONE_TIME_AUTH

#define WOLFSSL_CMAC

/* ------------------------------------------------------------------------- */
/* Hashing                                                                   */
/* ------------------------------------------------------------------------- */

/* SHA-1 stays on: TLS 1.2 signatures and 802.11 HMAC-SHA1 both need it. */
#define USE_SLOW_SHA
#define WOLFSSL_SHA512
#define WOLFSSL_SHA384
#define NO_MD5
#define NO_MD4
#define NO_RC4

/* HKDF for TLS 1.3 and key derivation helpers. */
#define HAVE_HKDF

/* PBKDF2 (WPA2-PSK SAE etcetera) lives in pwdbased.c. */
#undef NO_PWDBASED

/* ------------------------------------------------------------------------- */
/* Memory                                                                    */
/* ------------------------------------------------------------------------- */

/* purec/memory_userspace.c or purec/memory_kernel.c provide these. */
#define XMALLOC_USER

/* ------------------------------------------------------------------------- */
/* RNG                                                                       */
/* ------------------------------------------------------------------------- */

#define HAVE_HASHDRBG
/* purec/seed_purec.c: RDRAND when available, RDTSC jitter otherwise. */
#define CUSTOM_RAND_GENERATE_SEED purec_GenerateSeed
int purec_GenerateSeed(unsigned char *output, unsigned int sz);

/* ------------------------------------------------------------------------- */
/* OS services                                                               */
/* ------------------------------------------------------------------------- */

#define NO_FILESYSTEM
#define WOLFSSL_NO_SOCK
#define NO_WRITEV
#define NO_MAIN_DRIVER
#define NO_CRYPT_BENCHMARK
#define WOLFSSL_NO_ASSERT_H
#define WOLFSSL_USER_LOG(message) do { (void)(message); } while (0)
/* types.h -> memory.h pulls <stdlib.h>; the kernel has none. */
#define NO_STDLIB_H
/* Mini-printer from purec/snprintf_purec.c (PureC libc has no stdio). */
#define XSNPRINTF purec_snprintf

/* STRING_USER/CTYPE_USER: PureC libc/kernel has no <string.h>/<ctype.h>.
 * Prototypes live here; implementations come from src/lib/ (kernel),
 * src/libc/ (ring 3) and purec/memory_*.c for the few holes. */
#define STRING_USER
#define CTYPE_USER

#include <stddef.h>

void *memcpy(void *dst, const void *src, size_t n);
void *memmove(void *dst, const void *src, size_t n);
void *memset(void *dst, int c, size_t n);
int memcmp(const void *a, const void *b, size_t n);
size_t strlen(const char *s);
size_t strnlen(const char *s, size_t cap);
char *strcpy(char *dst, const char *src);
char *strncpy(char *dst, const char *src, size_t n);
char *strcat(char *dst, const char *src);
char *strncat(char *dst, const char *src, size_t n);
int strcmp(const char *a, const char *b);
int strncmp(const char *a, const char *b, size_t n);
char *strstr(const char *haystack, const char *needle);
char *wolfSSL_strnstr(const char *haystack, const char *needle, size_t n);
int wc_strcasecmp(const char *a, const char *b);
int wc_strncasecmp(const char *a, const char *b, size_t n);
int tolower(int c);
int toupper(int c);

#define XMEMCPY(d, s, l) memcpy((d), (s), (l))
#define XMEMSET(b, c, l) memset((b), (c), (l))
#define XMEMCMP(s1, s2, n) memcmp((s1), (s2), (n))
#define XMEMMOVE(d, s, l) memmove((d), (s), (l))
#define XSTRLEN(s1) strlen((s1))
#define XSTRNCPY(s1, s2, n) strncpy((s1), (s2), (n))
#define XSTRSTR(s1, s2) strstr((s1), (s2))
#define XSTRNSTR(s1, s2, n) wolfSSL_strnstr((s1), (s2), (n))
#define XSTRNCMP(s1, s2, n) strncmp((s1), (s2), (n))
#define XSTRCMP(s1, s2) strcmp((s1), (s2))
#define XSTRNCAT(s1, s2, n) strncat((s1), (s2), (n))
#define XSTRCASECMP(s1, s2) wc_strcasecmp((s1), (s2))
#define XSTRNCASECMP(s1, s2, n) wc_strncasecmp((s1), (s2), (n))

/* wc_strsep / wc_strcasecmp / wc_strncasecmp from wc_port.c. */
#define USE_WOLF_STRSEP
#define USE_WOLF_STRCASECMP
#define USE_WOLF_STRNCASECMP

#define XTOUPPER(c) toupper((c))
#define XTOLOWER(c) tolower((c))

/* No RTC yet: certificate validity windows are skipped in phase 1.
 * Phase 2 (TLS over TCP) must swap this for an XTIME implementation boot
 * from SYS_CPU_INFO uptime. */
#define NO_ASN_TIME

/* ------------------------------------------------------------------------- */
/* Protocols                                                                 */
/* ------------------------------------------------------------------------- */

#define NO_PSK
#define NO_OLD_TLS
#define NO_SESSION_CACHE

/* ------------------------------------------------------------------------- */
/* Build hygiene                                                             */
/* ------------------------------------------------------------------------- */

#define WOLFSSL_IGNORE_FILE_WARN

#endif /* PUREC_WOLFSSL_USER_SETTINGS_H */
