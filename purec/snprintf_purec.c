/* Minimal vsnprintf/snprintf for the PureC OS wolfSSL userspace build.
 *
 * The PureC libc has no stdio, but wolfASN formatting (asn.c and friends)
 * relies on snprintf. This covers the format verbs wolfSSL actually emits:
 * %d %i %u %x %X %s %c %p %% with 'l'/'z'/'h' length modifiers, zero
 * padding, star width and precision.
 *
 * Standard snprintf semantics: at most size-1 characters are stored, the
 * buffer is NUL terminated, and the return value is the length the full
 * output would have had. The prototype lives in purec_snprintf.h which is
 * force-included (-include) in the userspace wolfSSL build.
 */

#include <stddef.h>
#include <stdarg.h>
#include <stdint.h>

static size_t emit_char(char *out, size_t cap, size_t pos, char c)
{
    if (pos < cap) {
        out[pos] = c;
    }
    return pos + 1;
}

static size_t emit_pad(char *out, size_t cap, size_t pos, char c, int count)
{
    while (count-- > 0) {
        pos = emit_char(out, cap, pos, c);
    }
    return pos;
}

static size_t emit_string(char *out, size_t cap, size_t pos, const char *s,
                          int width, int precision, int left)
{
    size_t len = 0;

    if (s != NULL) {
        while (s[len] != '\0' &&
               (precision < 0 || (int)len < precision)) {
            len++;
        }
    } else {
        s = "null";
        len = 4;
        if (precision >= 0 && (int)len > precision) {
            len = (size_t)precision;
        }
    }
    if ((int)len < width && !left) {
        pos = emit_pad(out, cap, pos, ' ', width - (int)len);
    }
    for (size_t i = 0; i < len; i++) {
        pos = emit_char(out, cap, pos, s[i]);
    }
    if ((int)len < width && left) {
        pos = emit_pad(out, cap, pos, ' ', width - (int)len);
    }
    return pos;
}

static size_t emit_unsigned(char *out, size_t cap, size_t pos,
                            uint64_t value, unsigned base, int upper,
                            int width, char pad, int negative)
{
    char digits[24];
    int n = 0;
    const char digit_depot[16] = {'0','1','2','3','4','5','6','7',
                                  '8','9','a','b','c','d','e','f'};

    if (value == 0) {
        digits[n++] = '0';
    }
    while (value != 0) {
        digits[n++] = digit_depot[value % base];
        value /= base;
    }
    if (n > 0 && base == 16 && upper) {
        for (int i = 0; i < n; i++) {
            char d = digits[i];

            if (d >= 'a' && d <= 'f') {
                digits[i] = (char)(d - 'a' + 'A');
            }
        }
    }
    if (negative) {
        pos = emit_char(out, cap, pos, '-');
        if (width > 0) {
            width--;
        }
    }
    if (n < width) {
        pos = emit_pad(out, cap, pos, pad, width - n);
    }
    while (n > 0) {
        pos = emit_char(out, cap, pos, digits[--n]);
    }
    return pos;
}

static size_t emit_signed(char *out, size_t cap, size_t pos, int64_t value,
                          int width, char pad)
{
    uint64_t magnitude;
    int negative = value < 0;

    if (negative) {
        magnitude = (uint64_t)(-(value + 1)) + 1;
    } else {
        magnitude = (uint64_t)value;
    }
    return emit_unsigned(out, cap, pos, magnitude, 10, 0, width, pad,
                         negative);
}

int purec_vsnprintf(char *out, size_t size, const char *fmt, va_list ap)
{
    size_t cap = size > 0 ? size - 1 : 0;
    size_t pos = 0;

    if (fmt == NULL) {
        fmt = "";
    }
    while (*fmt != '\0') {
        char c = *fmt++;

        if (c != '%') {
            pos = emit_char(out, cap, pos, c);
            continue;
        }
        if (*fmt == '%') {
            fmt++;
            pos = emit_char(out, cap, pos, '%');
            continue;
        }
        {
            int width = 0;
            char pad = ' ';
            int left = 0;
            int long_mod = 0;
            int precision = -1;

            if (*fmt == '-') {
                left = 1;
                fmt++;
            }
            if (*fmt == '0') {
                pad = '0';
                fmt++;
            }
            if (*fmt == '*') {
                width = va_arg(ap, int);
                if (width < 0) {
                    left = 1;
                    width = -width;
                }
                fmt++;
            } else {
                while (*fmt >= '0' && *fmt <= '9') {
                    width = width * 10 + (*fmt - '0');
                    fmt++;
                }
            }
            if (*fmt == '.') {
                precision = 0;
                fmt++;
                if (*fmt == '*') {
                    precision = va_arg(ap, int);
                    fmt++;
                } else {
                    while (*fmt >= '0' && *fmt <= '9') {
                        precision = precision * 10 + (*fmt - '0');
                        fmt++;
                    }
                }
            }
            while (*fmt == 'l' || *fmt == 'z' || *fmt == 'h') {
                if (*fmt == 'l') {
                    long_mod++;
                }
                fmt++;
            }
            switch (*fmt) {
            case 'd':
            case 'i': {
                int64_t v;

                if (long_mod >= 2) {
                    v = va_arg(ap, long long);
                } else if (long_mod == 1) {
                    v = va_arg(ap, long);
                } else {
                    v = va_arg(ap, int);
                }
                pos = emit_signed(out, cap, pos, v, width, pad);
                break;
            }
            case 'u': {
                uint64_t v;

                if (long_mod >= 2) {
                    v = va_arg(ap, unsigned long long);
                } else if (long_mod == 1) {
                    v = va_arg(ap, unsigned long);
                } else {
                    v = va_arg(ap, unsigned int);
                }
                pos = emit_unsigned(out, cap, pos, v, 10, 0, width, pad, 0);
                break;
            }
            case 'x':
            case 'X': {
                uint64_t v;

                if (long_mod >= 2) {
                    v = va_arg(ap, unsigned long long);
                } else if (long_mod == 1) {
                    v = va_arg(ap, unsigned long);
                } else {
                    v = va_arg(ap, unsigned int);
                }
                pos = emit_unsigned(out, cap, pos, v, 16, *fmt == 'X',
                                    width, pad, 0);
                break;
            }
            case 'p': {
                void *ptr = va_arg(ap, void *);

                pos = emit_string(out, cap, pos, "0x", width, -1, left);
                pos = emit_unsigned(out, cap, pos, (uint64_t)(uintptr_t)ptr,
                                    16, 0, width, pad, 0);
                break;
            }
            case 'c': {
                int ch = va_arg(ap, int);

                if (width > 1 && !left) {
                    pos = emit_pad(out, cap, pos, ' ', width - 1);
                }
                pos = emit_char(out, cap, pos, (char)ch);
                if (width > 1 && left) {
                    pos = emit_pad(out, cap, pos, ' ', width - 1);
                }
                break;
            }
            case 's': {
                const char *s = va_arg(ap, const char *);

                pos = emit_string(out, cap, pos, s, width, precision, left);
                break;
            }
            default:
                pos = emit_char(out, cap, pos, '%');
                if (*fmt != '\0') {
                    pos = emit_char(out, cap, pos, *fmt);
                }
                break;
            }
            if (*fmt != '\0') {
                fmt++;
            }
        }
    }
    if (size > 0) {
        out[pos < cap ? pos : cap] = '\0';
    }
    return (int)pos;
}

int purec_snprintf(char *out, size_t size, const char *fmt, ...)
{
    va_list ap;
    int ret;

    va_start(ap, fmt);
    ret = purec_vsnprintf(out, size, fmt, ap);
    va_end(ap);
    return ret;
}
