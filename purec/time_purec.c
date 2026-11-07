#include <stdint.h>

struct purec_time_info {
    int64_t unix_seconds;
    uint32_t millis;
    int32_t tz_minutes;
};

long purec_wolf_time(long *timer)
{
    struct purec_time_info info = {0, 0, 0};
    int64_t result;
    __asm__ volatile("int $0x80"
                     : "=a"(result)
                     : "a"((uint64_t)319), "b"((uint64_t)(uintptr_t)&info),
                       "c"((uint64_t)0), "d"((uint64_t)0)
                     : "r10", "r8", "memory");
    long now = result < 0 ? 0 : (long)info.unix_seconds;
    if (timer)
        *timer = now;
    return now;
}
