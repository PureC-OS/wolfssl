/* PureC OS wolfSSL heap: malloc/free/realloc of the PureC libc. */

#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

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

/* Host unit test support (make test-wolfcrypt): entropy, test driver and the
 * certificate-manager hooks that live in the TLS sources this build omits. */

int purec_host_GenerateSeed(unsigned char *output, unsigned int sz)
{
    FILE *source = fopen("/dev/urandom", "rb");
    if (source == NULL) {
        return -1;
    }
    size_t got = fread(output, 1, sz, source);
    fclose(source);
    return got == sz ? 0 : -1;
}

void *GetCA(void *cm, unsigned char *hash)
{
    (void)cm;
    (void)hash;
    return NULL;
}

void *GetCAByName(void *cm, unsigned char *hash)
{
    (void)cm;
    (void)hash;
    return NULL;
}

int wolfcrypt_test(void *args);

int main(void)
{
    return wolfcrypt_test(NULL) == 0 ? 0 : 1;
}
