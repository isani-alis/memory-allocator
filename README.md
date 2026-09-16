# memory-allocator

A custom memory allocator in C implementing `malloc` / `calloc` / `realloc` / `free`
over a fixed static heap using a free-list with block splitting and coalescing.

## API

| Function | Behaviour |
|----------|-----------|
| `my_malloc(size)`          | First-fit search of the free-list; splits a block when the remainder can hold another header + payload; 8-byte aligned; returns `NULL` on `size==0` or OOM |
| `my_calloc(nmemb, size)`   | Overflow-checked multiply, then `my_malloc` + zero-fill |
| `my_realloc(ptr, size)`    | Shrinks in place; grows by allocate-copy-free; honours `realloc(NULL,n)==malloc` and `realloc(p,0)==free` |
| `my_free(ptr)`             | Marks the block free and coalesces adjacent free blocks in one address-ordered pass |
| `heap_dump()`              | Debug view of the free-list (address, size, free/used) |

## Design notes

- **Header per block**: `{ size, free, next }`. Payload pointer = header + `sizeof(header)`.
- **Splitting**: on allocation, a block is split only if the leftover is large enough
  for a new header plus at least one aligned word, avoiding unusable slivers.
- **Coalescing**: blocks stay in address order (splits insert in place, free never
  reorders), so physical adjacency equals list adjacency and a single forward pass
  fully merges neighbouring free blocks.
- **Alignment**: all payloads are rounded up to 8 bytes.

## Build & test

```bash
make          # build with -Wall -Wextra -std=c11
./allocator   # run the test suite
make asan     # rebuild with AddressSanitizer and run (recommended check)
make valgrind # run under Valgrind (see note below)
make clean
```

## Verification

Verified on **Ubuntu 24.04 / GCC 13.3** with:

- **Test suite** — asserts allocation, 8-byte alignment, `calloc` zero-fill,
  `calloc` overflow rejection, `realloc` grow (data preserved) / shrink (block reused)
  / NULL / zero semantics, and a 200-block stress test that coalesces back to a single
  free block. All pass.
- **AddressSanitizer** (`make asan`) — instruments the test harness' reads and writes
  into the arena and reports **no errors**. This is the meaningful memory check for a
  custom allocator.

> **Note on Valgrind:** `make valgrind` runs clean, but Valgrind only tracks libc's
> heap (`mmap`/`brk`), not a `static` arena, so it validates the test harness rather
> than the allocator itself. AddressSanitizer is used for the real correctness check.

## Stack

C (C11) · GCC · Make · AddressSanitizer · Valgrind · Linux
