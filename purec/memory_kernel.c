/* PureC OS wolfSSL heap: oom slab + PMM pages in the kernel.
 *
 * oom_slab_alloc hands out 4KB-class blocks 8-byte aligned, and frees need
 * the allocation size back, so the bridge keeps a 32-byte header in front
 * of the user data and two mirror words behind it. The user data start is
 * rounded up to 16 bytes so wolfSSL sees 16-byte alignment.
 */

#include <stddef.h>
#include <stdint.h>

#include "../../src/mm/pmm.h"
#include "../../src/mm/oom/oom_slab.h"

#define WOLF_HDR_MAGIC 0x50554357ULL /* "PUCW" */
#define WOLF_HDR_SIZE 32
#define WOLF_DATA_OFFSET 128
#define WOLF_TAIL_SIZE 16
#define WOLF_SLAB_MAX 4096ULL

#define WOLF_KIND_SLAB 1
#define WOLF_KIND_PAGES 2

/* Slab path worst case: header 32B + alignment slack ~15B for the 16-byte
 * rounded data start + one 16B tail. Reserve 48 extra bytes over a fixed
 * 128-byte data offset. */
#define WOLF_SLAB_OVERHEAD (WOLF_DATA_OFFSET + 48)
#define WOLF_PAGE_OVERHEAD (WOLF_DATA_OFFSET + WOLF_TAIL_SIZE + WOLF_HDR_SIZE)

struct wolf_hdr {
    uint64_t kind;   /* WOLF_KIND_* */
    uint64_t count;  /* slab: raw block size; pages: page count */
    uint64_t req;    /* requested user bytes */
    uint64_t phys;   /* pages: physical address of first page */
};

struct wolf_data_tail {
    uint64_t hdr;    /* back pointer, at data - 16 */
    uint64_t magic;  /* WOLF_HDR_MAGIC, at data - 8 */
};

static uint64_t align_up16(uintptr_t value)
{
    return (value + 15) & ~15UL;
}

static uint8_t *wolf_data_start(uint8_t *base)
{
    return (uint8_t *)align_up16((uintptr_t)base + WOLF_DATA_OFFSET);
}

static void *wolf_finish(uint8_t *base)
{
    uint8_t *data = wolf_data_start(base);
    struct wolf_data_tail *tail = (struct wolf_data_tail *)
        (data - WOLF_TAIL_SIZE);

    tail->hdr = (uint64_t)(uintptr_t)base;
    tail->magic = WOLF_HDR_MAGIC;
    return data;
}

static struct wolf_hdr *wolf_from_user(void *user)
{
    struct wolf_data_tail *tail = (struct wolf_data_tail *)
        ((uint8_t *)user - WOLF_TAIL_SIZE);

    if (tail->magic != WOLF_HDR_MAGIC || tail->hdr == 0) {
        return NULL;
    }
    return (struct wolf_hdr *)(uintptr_t)tail->hdr;
}

void *XMALLOC(size_t n, void *heap, int type)
{
    (void)heap;
    (void)type;

    if (n == 0 || n + WOLF_PAGE_OVERHEAD < n) {
        return NULL;
    }
    if (n + WOLF_SLAB_OVERHEAD <= WOLF_SLAB_MAX) {
        uint64_t raw_size = WOLF_SLAB_OVERHEAD + (uint64_t)n;
        uint8_t *raw = oom_slab_alloc(raw_size);

        if (raw != NULL) {
            struct wolf_hdr *hdr = (struct wolf_hdr *)raw;

            hdr->kind = WOLF_KIND_SLAB;
            hdr->count = raw_size;
            hdr->req = (uint64_t)n;
            hdr->phys = 0;
            return wolf_finish(raw);
        }
        /* slab exhausted: fall through to whole pages */
    }
    {
        uint64_t pages = ((uint64_t)n + WOLF_PAGE_OVERHEAD + PMM_PAGE_SIZE - 1) /
                         PMM_PAGE_SIZE;
        uint64_t phys = pmm_allocate_contiguous(pages);

        if (phys == 0) {
            return NULL;
        }
        {
            uint8_t *base = pmm_physical_to_virtual(phys);
            struct wolf_hdr *hdr = (struct wolf_hdr *)base;

            hdr->kind = WOLF_KIND_PAGES;
            hdr->count = pages;
            hdr->req = (uint64_t)n;
            hdr->phys = phys;
            return wolf_finish(base);
        }
    }
}

static size_t wolf_user_size(struct wolf_hdr *hdr)
{
    if (hdr == NULL || hdr->req == 0) {
        return 0;
    }
    return (size_t)hdr->req;
}

void XFREE(void *p, void *heap, int type)
{
    struct wolf_hdr *hdr;

    (void)heap;
    (void)type;

    if (p == NULL) {
        return;
    }
    hdr = wolf_from_user(p);
    if (hdr == NULL) {
        return;
    }
    if (hdr->kind == WOLF_KIND_SLAB) {
        oom_slab_free(hdr, hdr->count);
    } else if (hdr->kind == WOLF_KIND_PAGES) {
        pmm_free_contiguous(hdr->phys, hdr->count);
    }
}

void *XREALLOC(void *p, size_t n, void *heap, int type)
{
    struct wolf_hdr *hdr;
    size_t old_size;
    void *fresh;
    size_t copy_size;

    (void)heap;
    (void)type;

    if (p == NULL) {
        return XMALLOC(n, heap, type);
    }
    if (n == 0) {
        XFREE(p, heap, type);
        return NULL;
    }
    hdr = wolf_from_user(p);
    old_size = wolf_user_size(hdr);
    fresh = XMALLOC(n, heap, type);
    if (fresh == NULL) {
        return NULL;
    }
    copy_size = old_size < n ? old_size : n;
    {
        uint8_t *out = (uint8_t *)fresh;
        const uint8_t *in = (const uint8_t *)p;

        for (size_t i = 0; i < copy_size; i++) {
            out[i] = in[i];
        }
    }
    XFREE(p, heap, type);
    return fresh;
}
