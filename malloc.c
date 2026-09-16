#include "malloc.h"
#include <stdio.h>
#include <string.h>

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

/* ---------- my_free ---------- */
void my_free(void *ptr)
{
    if (!ptr) return;

    block_t *blk = (block_t *)((char *)ptr - HEADER_SIZE);
    blk->free = 1;

    /* coalesce adjacent free blocks */
    block_t *cur = free_list;
    while (cur && cur->next) {
        if (cur->free && cur->next->free) {
            cur->size += HEADER_SIZE + cur->next->size;
            cur->next  = cur->next->next;
        } else {
            cur = cur->next;
        }
    }
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
