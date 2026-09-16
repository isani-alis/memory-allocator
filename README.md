# memory-allocator

A custom memory allocator in C implementing `my_malloc` and `my_free` over a fixed static heap buffer.

## Features

- `my_malloc` — allocates from a 64 KB static heap using a free-list; splits blocks on allocation
- `my_free` — marks blocks free and coalesces adjacent free blocks to reduce fragmentation
- `heap_dump()` — debug utility to visualise free-list state (address, size, free/used)

## Build

```bash
make          # build
./allocator   # run tests
make valgrind # run under valgrind (Linux)
make clean    # remove build artifacts
```

## Verified

Tested on Linux (Ubuntu) with **Valgrind 3.26.0** — 0 errors, 0 leaks.

## Stack

C · GCC · Make · Valgrind · Linux
