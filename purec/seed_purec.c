/* PureC OS wolfSSL seed source: RDRAND when the CPU offers it, RDTSC
 * jitter folding otherwise. Used by both the userspace library and the
 * kernel module; ring 3 may execute RDRAND/RDTSC without traps.
 *
 * RDTSC alone is entropy-weak: it feeds the hash-DRBG which still mixes
 * per-boot state, but picking up real hardware entropy first is expected.
 */

#include <stdint.h>
#include <stddef.h>

#define RDRAND_RETRY_MAX 20

static inline void purec_cpuid(uint32_t leaf, uint32_t *a, uint32_t *b,
                               uint32_t *c, uint32_t *d)
{
    __asm__ volatile("cpuid"
                     : "=a"(*a), "=b"(*b), "=c"(*c), "=d"(*d)
                     : "a"(leaf), "c"(0));
}

static inline void purec_rdtsc_scramble(uint64_t *state)
{
    uint32_t lo, hi;
    uint64_t tsc;

    __asm__ volatile("rdtsc" : "=a"(lo), "=d"(hi));
    tsc = ((uint64_t)hi << 32) | lo;
    *state ^= tsc + (*state << 7) + (*state >> 3);
}

static int purec_rdrand_u64(uint64_t *out)
{
    for (int retry = 0; retry < RDRAND_RETRY_MAX; retry++) {
        unsigned char ok;

        __asm__ volatile("rdrand %0; setc %1"
                         : "=r"(*out), "=q"(ok)
                         :
                         : "cc");
        if (ok) {
            return 1;
        }
    }
    return 0;
}

static int purec_rdrand_available(void)
{
    uint32_t a, b, c, d;

    purec_cpuid(1, &a, &b, &c, &d);
    return (c >> 30) & 1U;
}

static void purec_fill_tsc(unsigned char *output, unsigned int sz)
{
    uint64_t state = 0x9E3779B97F4A7C15ULL;

    while (sz > 0) {
        uint64_t value;
        unsigned int take;

        purec_rdtsc_scramble(&state);
        value = state;
        take = sz < 8 ? sz : 8;
        for (unsigned int i = 0; i < take; i++) {
            output[i] = (unsigned char)(value >> (i * 8));
        }
        output += take;
        sz -= take;
    }
}

static void purec_fill_rdrand(unsigned char *output, unsigned int sz)
{
    unsigned int filled = 0;

    while (filled < sz) {
        uint64_t value;
        unsigned int take = sz - filled < 8 ? sz - filled : 8;

        if (!purec_rdrand_u64(&value)) {
            /* RDRAND exhausted its retries; finish with the mixer. */
            purec_fill_tsc(output + filled, sz - filled);
            return;
        }
        for (unsigned int i = 0; i < take; i++) {
            output[filled + i] = (unsigned char)(value >> (i * 8));
        }
        filled += take;
    }
}

static void purec_fill(unsigned char *output, unsigned int sz)
{
    if (output == NULL || sz == 0) {
        return;
    }
    if (purec_rdrand_available()) {
        purec_fill_rdrand(output, sz);
    } else {
        purec_fill_tsc(output, sz);
    }
}

int purec_GenerateSeed(unsigned char *output, unsigned int sz)
{
    purec_fill(output, sz);
    return 0;
}
