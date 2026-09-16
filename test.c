#include "malloc.h"
#include <stdio.h>
#include <string.h>
#include <assert.h>

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

    printf("--- null / zero edge cases ---\n");
    void *z = my_malloc(0);
    printf("my_malloc(0) = %p  (expected NULL)\n", z);
    my_free(NULL);   /* should not crash */
    printf("my_free(NULL) OK\n");

    my_free(c);
    heap_dump();

    printf("All tests passed.\n");
    return 0;
}
