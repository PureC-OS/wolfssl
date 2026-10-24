/* PureC OS wolfSSL heap: libc malloc/free/realloc in ring 3. */

#include <stddef.h>

extern void *malloc(size_t);
extern void *realloc(void *, size_t);
extern void free(void *);

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
