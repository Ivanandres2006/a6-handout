# COMP 310 — Operating Systems
## Week 9 — Example: two allocators over one arena

The programs from class. They are not an assignment, and nothing here is
submitted.

```bash
make                                  # builds demo/where and demo/alloc
./demo/where                          # where code, data, heap and stack sit
./demo/alloc bump data/bump.txt       # a bump allocator: never reuses
./demo/alloc list data/reuse.txt      # a free list: first fit, with a split
./demo/alloc list data/frag.txt       # free bytes in pieces too small to use
make test                             # 13 checks against tests/expected/
make bench                            # both allocators on five seeded traces
```

### `demo/where`

Prints the address of the function `main`, a global, a block from `malloc` and
a local variable, then the four regions in address order. The addresses change
on every run (the kernel places each region at a random offset); the order does
not.

### `demo/alloc`

Both allocators manage the same **512-byte arena**, and both round every request
up to a multiple of **16 bytes**, the alignment `malloc` gives on 64-bit Linux
and macOS.

- **`bump`** hands out the next bytes and moves its pointer past them. It keeps
  no record of a block, so it cannot free one: a freed block is never used again.
- **`list`** puts a **16-byte header** in front of every block: the block's size
  and whether it is free. A request takes the **first** free block big enough
  (*first fit*), and **splits** it when the rest could hold another block. If no
  free block fits, a new block is made at the top of what has been handed out.
  It **never merges** two free blocks that sit side by side (no *coalescing*).

A trace is a text file, one request per line. Lines starting with `#` are
comments.

```text
+A 20       ask for 20 bytes, and call the block A
-A          free block A
```

After every request the program prints a map of the arena, **one character for
every 16 bytes**:

| Character | Means |
|---|---|
| `\|` | a header (free list only) |
| `A`–`Z` | a block in use, by its name |
| `-` | a free block (free list only) |
| `a`–`z` | a freed block a bump allocator can never reuse |
| `.` | arena not yet handed out |

The **free list** also walks every block after every request and checks that
each header holds a positive multiple of 16 and that the walk ends exactly where
the handed-out part of the arena ends (`heap consistent after every request`).

The three traces in `data/`:

| File | What it shows |
|---|---|
| `data/bump.txt` | a freed block that a bump allocator never reuses |
| `data/reuse.txt` | a free list reusing a freed block, and splitting it |
| `data/frag.txt` | 192 free bytes, and a request for 100 that still fails |

`make bench` replays five seeded random traces through both allocators and
counts the requests that failed, and of those, how many failed although the free
bytes added up to enough. Every number is exact: the traces come from a fixed
generator, not from the clock or `rand()`, so they are the same on every machine.
