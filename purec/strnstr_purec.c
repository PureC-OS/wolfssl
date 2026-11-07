#include <stddef.h>

char *wolfSSL_strnstr(const char *haystack, const char *needle, size_t n)
{
    if (!*needle)
        return (char *)haystack;
    for (; n && *haystack; haystack++, n--)
    {
        size_t i = 0;
        while (i < n && needle[i] && haystack[i] == needle[i])
            i++;
        if (!needle[i])
            return (char *)haystack;
        if (i == n)
            return 0;
    }
    return 0;
}
