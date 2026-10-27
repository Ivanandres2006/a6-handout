# COMP 310 — Week 9 — the allocators from class.
#
# A demonstration, not an assignment: nothing here is submitted.
#   demo/where   where code, data, heap and stack sit in one process
#   demo/alloc   a bump allocator and a first-fit free list over a 512-byte arena
#   make test    every trace under both allocators, against tests/expected/
#   make bench   both allocators on five seeded random traces
CC     ?= cc
CFLAGS ?= -std=c11 -Wall -Wextra -O2

.PHONY: all test bench clean
all: demo/where demo/alloc

demo/where: demo/where.c
	$(CC) $(CFLAGS) -o demo/where demo/where.c

demo/alloc: demo/alloc.c
	$(CC) $(CFLAGS) -o demo/alloc demo/alloc.c

test: all
	@sh tests/run-tests.sh

bench: demo/alloc
	@./demo/alloc bench

clean:
	@rm -f demo/where demo/alloc
	@rm -rf demo/*.dSYM
