CC      = gcc
CFLAGS  = -Wall -Wextra -g -std=c11

TARGET  = allocator
SRCS    = malloc.c test.c
OBJS    = $(SRCS:.c=.o)

.PHONY: all clean valgrind asan

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) -o $@ $^

%.o: %.c malloc.h
	$(CC) $(CFLAGS) -c $< -o $@

# AddressSanitizer instruments the test harness' own reads/writes into the
# arena, so it *does* catch out-of-bounds access on our custom heap --
# unlike Valgrind, which cannot see a static-array allocator.
asan: clean
	$(CC) $(CFLAGS) -fsanitize=address -fno-omit-frame-pointer $(SRCS) -o $(TARGET)
	./$(TARGET)

# Valgrind runs the binary cleanly but note: it only tracks libc's heap,
# not our static arena, so it validates the harness, not the allocator.
valgrind: $(TARGET)
	valgrind --leak-check=full --error-exitcode=1 ./$(TARGET)

clean:
	rm -f $(OBJS) $(TARGET)
