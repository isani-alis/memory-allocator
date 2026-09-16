#include "malloc.h"
#include <stdio.h>
#include <string.h>
#include <stdint.h>

#define HEAP_SIZE  (1024 * 64)   /* 64 KB static heap */
#define ALIGN       8

/* ---------- internal block header ---------- */
typedef struct block {
    size_t       size;   /* usable payload bytes */
    int          free;   /* 1 = free, 0 = in use */
    struct block *next;  /* next block in free-list / heap walk */
} block_t;

#define HEADER_SIZE  (sizeof(block_t))

/* ---------- heap storage ---------- */
static char   heap[HEAP_SIZE];
static block_t *free_list = NULL;   /* head of the free-list */

/* ---------- helpers ---------- */
static size_t align_up(size_t n)
{
    return (n + ALIGN - 1) & ~(ALIGN - 1);
}

static void init_heap(void)
{
    free_list = (block_t *)heap;
    free_list->size = HEAP_SIZE - HEADER_SIZE;
    free_list->free = 1;
    free_list->next = NULL;
}

/* ---------- my_malloc ---------- */
void *my_malloc(size_t size)
{
    if (size == 0) return NULL;

    if (free_list == NULL) init_heap();

    size = align_up(size);

    block_t *cur = free_list;
    while (cur) {
        if (cur->free && cur->size >= size) {
            /* split block if remainder is large enough */
            if (cur->size >= size + HEADER_SIZE + ALIGN) {
                block_t *split = (block_t *)((char *)cur + HEADER_SIZE + size);
                split->size = cur->size - size - HEADER_SIZE;
                split->free = 1;
                split->next = cur->next;

                cur->size = size;
                cur->next = split;
            }
            cur->free = 0;
            return (char *)cur + HEADER_SIZE;
        }
        cur = cur->next;
    }
    return NULL;   /* out of memory */
}

/* ---------- coalesce ---------- */
/* Single forward pass merging every run of adjacent free blocks.
 * Because blocks are kept in address order (split inserts in place,
 * free never reorders), physical adjacency == list adjacency, so one
 * pass is sufficient to fully defragment the free space. */
static void coalesce(void)
{
    block_t *cur = free_list;
    while (cur && cur->next) {
        if (cur->free && cur->next->free) {
            cur->size += HEADER_SIZE + cur->next->size;
            cur->next  = cur->next->next;   /* absorb next, re-check same cur */
        } else {
            cur = cur->next;
        }
    }
}

/* ---------- my_free ---------- */
void my_free(void *ptr)
{
    if (!ptr) return;

    /* recover the header that sits immediately before the payload */
    block_t *blk = (block_t *)((char *)ptr - HEADER_SIZE);
    blk->free = 1;

    coalesce();
}

/* ---------- my_calloc ---------- */
void *my_calloc(size_t nmemb, size_t size)
{
    if (nmemb == 0 || size == 0) return NULL;

    /* overflow-safe multiply */
    if (nmemb > SIZE_MAX / size) return NULL;

    size_t total = nmemb * size;
    void *p = my_malloc(total);
    if (p) memset(p, 0, total);
    return p;
}

/* ---------- my_realloc ---------- */
void *my_realloc(void *ptr, size_t size)
{
    if (ptr == NULL)  return my_malloc(size);   /* realloc(NULL, n) == malloc(n) */
    if (size == 0)  { my_free(ptr); return NULL; }

    block_t *blk = (block_t *)((char *)ptr - HEADER_SIZE);

    /* shrinking or same size: keep the block as-is */
    if (blk->size >= align_up(size))
        return ptr;

    /* growing: allocate new, copy old payload, free old */
    void *np = my_malloc(size);
    if (!np) return NULL;               /* original block left intact */
    memcpy(np, ptr, blk->size);
    my_free(ptr);
    return np;
}

/* ---------- heap_dump ---------- */
void heap_dump(void)
{
    if (free_list == NULL) {
        printf("[heap_dump] heap not initialised\n");
        return;
    }
    printf("\n=== heap_dump ===\n");
    block_t *cur = free_list;
    int idx = 0;
    while (cur) {
        printf("  [%d] addr=%p  size=%-6zu  %s\n",
               idx++,
               (void *)((char *)cur + HEADER_SIZE),
               cur->size,
               cur->free ? "FREE" : "USED");
        cur = cur->next;
    }
    printf("=================\n\n");
}
