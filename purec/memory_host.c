/* PureC OS wolfSSL heap: malloc/free/realloc of the PureC libc. */

#include <stddef.h>
#include <stdlib.h>

int purec_host_GenerateSeed(unsigned char *output, unsigned int sz);

void *XMALLOC(size_t n, void *heap, int type)
{
    (void)heap;
    (void)type;
    return malloc(n);
}

void *XREALLOC(void *p, size_t n, void *heap, int type)
{
    (void)heap;
    (void)type;
    return realloc(p, n);
}

void XFREE(void *p, void *heap, int type)
{
    (void)heap;
    (void)type;
    if (p != NULL) {
        free(p);
    }
}
