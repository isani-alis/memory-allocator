CC      = gcc
CFLAGS  = -Wall -Wextra -g -std=c11

TARGET  = allocator
SRCS    = malloc.c test.c
OBJS    = $(SRCS:.c=.o)

.PHONY: all clean valgrind

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) -o $@ $^

%.o: %.c malloc.h
	$(CC) $(CFLAGS) -c $< -o $@

valgrind: $(TARGET)
	valgrind --leak-check=full --error-exitcode=1 ./$(TARGET)

clean:
	rm -f $(OBJS) $(TARGET)
