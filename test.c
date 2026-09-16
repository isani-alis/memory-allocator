#include "malloc.h"
#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <stdint.h>

#define ALIGN 8

int main(void)
{
    printf("--- basic allocation ---\n");
    int *a = my_malloc(sizeof(int) * 4);
    assert(a != NULL);
    a[0] = 10; a[1] = 20; a[2] = 30; a[3] = 40;
    printf("a = %d %d %d %d\n", a[0], a[1], a[2], a[3]);

    char *b = my_malloc(32);
    assert(b != NULL);
    strcpy(b, "hello allocator");
    printf("b = %s\n", b);

    heap_dump();

    printf("--- alignment check ---\n");
    /* every payload pointer must be 8-byte aligned */
    assert(((uintptr_t)a % ALIGN) == 0);
    assert(((uintptr_t)b % ALIGN) == 0);
    printf("payloads 8-byte aligned OK\n");

    printf("--- free and coalesce ---\n");
    my_free(a);
    my_free(b);
    heap_dump();

    printf("--- reuse after free ---\n");
    double *c = my_malloc(sizeof(double) * 8);
    assert(c != NULL);
    for (int i = 0; i < 8; i++) c[i] = i * 1.5;
    printf("c[7] = %.1f\n", c[7]);
    heap_dump();

    printf("--- calloc zeroes memory ---\n");
    int *z = my_calloc(16, sizeof(int));
    assert(z != NULL);
    for (int i = 0; i < 16; i++) assert(z[i] == 0);
    printf("calloc(16,4) all-zero OK\n");

    printf("--- calloc overflow guard ---\n");
    void *ov = my_calloc(SIZE_MAX, 2);
    assert(ov == NULL);
    printf("calloc(SIZE_MAX,2) rejected OK\n");

    printf("--- realloc grow preserves data ---\n");
    char *s = my_malloc(8);
    strcpy(s, "abcdefg");
    s = my_realloc(s, 64);
    assert(s != NULL);
    assert(strcmp(s, "abcdefg") == 0);
    printf("realloc grow -> \"%s\" preserved OK\n", s);

    printf("--- realloc shrink keeps pointer ---\n");
    char *s2 = my_realloc(s, 4);
    assert(s2 == s);   /* shrink should reuse the same block */
    printf("realloc shrink reused block OK\n");

    printf("--- realloc(NULL,n) == malloc, realloc(p,0) == free ---\n");
    void *rn = my_realloc(NULL, 10);
    assert(rn != NULL);
    void *rz = my_realloc(rn, 0);
    assert(rz == NULL);
    printf("realloc NULL/zero semantics OK\n");

    my_free(c);
    my_free(z);
    my_free(s2);

    printf("--- null / zero edge cases ---\n");
    void *zn = my_malloc(0);
    printf("my_malloc(0) = %p  (expected NULL)\n", zn);
    assert(zn == NULL);
    my_free(NULL);   /* should not crash */
    printf("my_free(NULL) OK\n");

    printf("--- stress: many small allocs then free all ---\n");
    void *ptrs[200];
    int n = 0;
    for (int i = 0; i < 200; i++) {
        ptrs[i] = my_malloc(64);
        if (!ptrs[i]) break;
        memset(ptrs[i], 0xAB, 64);   /* touch every byte */
        n++;
    }
    printf("allocated %d blocks of 64B\n", n);
    for (int i = 0; i < n; i++) my_free(ptrs[i]);

    /* after freeing everything the heap must coalesce back to one block */
    heap_dump();

    printf("All tests passed.\n");
    return 0;
}
