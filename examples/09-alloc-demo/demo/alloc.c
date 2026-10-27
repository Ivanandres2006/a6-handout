/*
 * COMP 310 — Week 9 — two allocators over one fixed arena.
 *
 * Reads a trace, one request per line, runs it through one allocator, and
 * prints a map of the arena after every request:
 *
 *   ./demo/alloc bump data/bump.txt     a bump allocator: never reuses
 *   ./demo/alloc list data/reuse.txt    a free list: first fit, with a split
 *   ./demo/alloc bench                  both, on five seeded random traces:
 *                                       how many requests fail, and how many
 *                                       of those had enough free bytes
 *
 * A trace line is "+A 20" (ask for 20 bytes and call the block A) or "-A"
 * (free block A). Lines starting with '#' are comments.
 *
 * The arena is 512 bytes. Every size is rounded up to a multiple of 16, the
 * alignment malloc gives on 64-bit Linux and macOS. The free list keeps a
 * 16-byte header in front of every block. It reuses a freed block, and splits
 * a big one, but it never merges two free neighbors: coalescing is left out.
 *
 * In the map one character is 16 bytes:
 *   |   a header            A-Z  a block in use, by its name
 *   -   a free block        a-z  (bump only) freed, but never reused
 *   .   arena not yet handed out
 *
 * Exit status: 0 on success, 1 if the trace cannot be read, 2 on a usage error.
 */
#define _POSIX_C_SOURCE 200809L   /* POSIX under -std=c11 (Week 2's rule)    */
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define ARENA_SIZE 512            /* bytes in the arena                       */
#define ALIGN      16             /* every block starts on a multiple of 16   */
#define UNITS      (ARENA_SIZE / ALIGN)   /* map characters: one per 16 bytes */

/* The header in front of every free-list block. Two size_t fields make it 16
 * bytes on a 64-bit machine, so the payload after it stays 16-aligned. */
typedef struct {
    size_t size;                  /* payload bytes after this header          */
    size_t free;                  /* 1 if the block is free, 0 if in use      */
} Block;

_Static_assert(sizeof(Block) == ALIGN, "a header must be one alignment unit");

static _Alignas(ALIGN) unsigned char arena[ARENA_SIZE];
static size_t top;                /* bytes handed out so far: the bump point  */

/* Round n up to the next multiple of ALIGN (a power of two). */
static size_t align_up(size_t n) {
    return (n + (ALIGN - 1)) & ~(size_t)(ALIGN - 1);
}

/* ---- The bump allocator ------------------------------------------------- */

/* Hand out the next n bytes, rounded up. It never reuses: free is a no-op. */
static void *bump_alloc(size_t n) {
    if (n == 0)
        return NULL;
    size_t need = align_up(n);
    if (need > ARENA_SIZE - top)  /* unsigned-safe: top never passes the end */
        return NULL;
    void *p = arena + top;        /* the first byte not yet handed out        */
    top += need;                  /* bump: move the point past the block      */
    return p;
}

/* ---- The free-list allocator -------------------------------------------- */

/* The header just past b's payload: the next block in address order. */
static Block *next_block(Block *b) {
    return (Block *)((unsigned char *)(b + 1) + b->size);
}

/* First fit: walk the blocks from the start of the arena and take the first
 * free one that is big enough. Split it when the rest could hold another
 * block. If none fits, carve a new block off the top. */
static void *list_alloc(size_t n) {
    if (n == 0)
        return NULL;
    size_t need = align_up(n);
    for (Block *b = (Block *)arena; (unsigned char *)b < arena + top;
         b = next_block(b)) {
        if (!b->free || b->size < need)
            continue;                         /* in use, or too small      */
        if (b->size >= need + sizeof(Block) + ALIGN) {   /* room to split  */
            Block *rest = (Block *)((unsigned char *)(b + 1) + need);
            rest->size = b->size - need - sizeof(Block);  /* minus a header */
            rest->free = 1;
            b->size = need;
        }
        b->free = 0;
        return b + 1;                         /* the payload, after the header */
    }
    if (sizeof(Block) + need > ARENA_SIZE - top)
        return NULL;                          /* nothing fits, and no room  */
    Block *b = (Block *)(arena + top);        /* a new block at the top     */
    b->size = need;
    b->free = 0;
    top += sizeof(Block) + need;
    return b + 1;
}

/* Mark p's block free. The header is just before the payload. */
static void list_free(void *p) {
    if (p == NULL)
        return;
    Block *b = (Block *)p - 1;
    b->free = 1;
}

/* Walk every block from the start of the arena to top and check what must
 * always hold: a size that is a positive multiple of ALIGN, a free flag of 0
 * or 1, and a walk that lands exactly on top. Returns -1 if the heap is
 * consistent, or the offset of the first header that is not. */
static long heap_check(void) {
    size_t off = 0;
    while (off < top) {
        Block *b = (Block *)(arena + off);
        if (b->size == 0 || b->size % ALIGN != 0 || b->free > 1
            || b->size > top - off - sizeof(Block))
            return (long)off;
        off += sizeof(Block) + b->size;
    }
    return off == top ? -1 : (long)off;
}

/* ---- The driver: traces, the map, the totals ---------------------------- */

enum mode { BUMP, LIST };

static void *ptr[26];             /* each named block's payload, or NULL      */
static size_t asked[26];          /* the bytes each live block asked for      */
static char owner[UNITS];         /* bump only: who holds each 16-byte unit   */

/* One character per 16 bytes of the arena (see the comment at the top). */
static void draw(enum mode m, char *map) {
    memset(map, '.', UNITS);
    map[UNITS] = '\0';
    if (m == BUMP) {
        for (size_t u = 0; u < top / ALIGN; u++)
            map[u] = owner[u];
        return;
    }
    for (Block *b = (Block *)arena; (unsigned char *)b < arena + top;
         b = next_block(b)) {
        size_t u = (size_t)((unsigned char *)b - arena) / ALIGN;
        char c = '-';
        for (int i = 0; i < 26; i++)
            if (ptr[i] == (void *)(b + 1) && !b->free)
                c = (char)('A' + i);
        map[u] = '|';
        for (size_t k = 1; k <= b->size / ALIGN; k++)
            map[u + k] = c;
    }
}

/* Totals after the trace: what was asked for, what it cost, what is left. */
static void totals(enum mode m, int requests, int failed) {
    size_t want = 0, given = 0, free_bytes = 0, largest = 0;
    int live = 0, holes = 0, headers = 0;
    for (int i = 0; i < 26; i++)
        if (ptr[i] != NULL) {
            live++;
            want += asked[i];
            given += align_up(asked[i]);
        }
    printf("requests %d, failed %d\n", requests, failed);
    printf("in use   %d blocks: %zu bytes asked, %zu given, %zu lost to rounding\n",
           live, want, given, given - want);
    if (m == LIST) {
        for (Block *b = (Block *)arena; (unsigned char *)b < arena + top;
             b = next_block(b)) {
            headers++;
            if (b->free) {
                holes++;
                free_bytes += b->size;
                if (b->size > largest)
                    largest = b->size;
            }
        }
        printf("headers  %d, %zu bytes\n", headers, headers * sizeof(Block));
        printf("free     %d blocks, %zu bytes, largest %zu; %zu never handed out\n",
               holes, free_bytes, largest, ARENA_SIZE - top);
    } else {
        printf("free     %zu bytes never handed out\n", ARENA_SIZE - top);
    }
}

/* Run one trace file; print a line and a map per request. */
static int run(enum mode m, const char *path) {
    FILE *f = fopen(path, "r");
    if (f == NULL) {
        perror(path);
        return 1;
    }
    printf("%s on %s, arena %d bytes, one character = %d bytes\n",
           m == BUMP ? "bump" : "list (first fit)", path, ARENA_SIZE, ALIGN);
    char line[128], result[32], map[UNITS + 1];
    int lineno = 0, requests = 0, failed = 0, broken = 0;
    while (fgets(line, sizeof line, f) != NULL) {
        lineno++;
        char op, name;
        unsigned long n = 0;
        if (line[0] == '#' || strspn(line, " \t\r\n") == strlen(line))
            continue;                             /* comment or blank line   */
        int got = sscanf(line, " %c%c %lu", &op, &name, &n);
        if (name < 'A' || name > 'Z'          /* BYTES from 1 to the arena  */
            || !((op == '+' && got == 3 && n > 0 && n <= ARENA_SIZE)
                 || (op == '-' && got == 2))) {
            fprintf(stderr, "%s:%d: expected +NAME BYTES (1 to %d) or -NAME, "
                            "such as: +A 20\n", path, lineno, ARENA_SIZE);
            fclose(f);
            return 1;
        }
        int i = name - 'A';
        if (op == '+' && ptr[i] != NULL) {
            fprintf(stderr, "%s:%d: %c is already in use; free it first\n",
                    path, lineno, name);
            fclose(f);
            return 1;
        }
        if (op == '+') {
            requests++;
            size_t before = top;
            void *p = (m == BUMP) ? bump_alloc(n) : list_alloc(n);
            if (p == NULL) {
                failed++;
                snprintf(result, sizeof result, "%c failed", name);
            } else {
                ptr[i] = p;
                asked[i] = n;
                if (m == BUMP)
                    memset(owner + before / ALIGN, name, (top - before) / ALIGN);
                snprintf(result, sizeof result, "%c at %ld", name,
                         (long)((unsigned char *)p - arena));
            }
        } else if (ptr[i] == NULL) {
            snprintf(result, sizeof result, "%c not in use", name);
        } else {
            if (m == LIST) {
                list_free(ptr[i]);
                snprintf(result, sizeof result, "%c freed", name);
            } else {                              /* bump: mark it, keep it  */
                size_t u = (size_t)((unsigned char *)ptr[i] - arena) / ALIGN;
                for (size_t k = 0; k < align_up(asked[i]) / ALIGN; k++)
                    owner[u + k] = (char)(name - 'A' + 'a');
                snprintf(result, sizeof result, "%c ignored", name);
            }
            ptr[i] = NULL;
        }
        if (m == LIST && heap_check() >= 0)
            broken++;
        line[strcspn(line, "\r\n")] = '\0';
        draw(m, map);
        printf("%-8s %-13s %s\n", line, result, map);
    }
    fclose(f);
    totals(m, requests, failed);
    if (m == LIST)
        printf("heap     %s after every request\n",
               broken ? "BROKEN" : "consistent");
    return 0;
}

/* ---- bench: both allocators on seeded random traces --------------------- */

static unsigned long seed;

/* A fixed linear congruential generator, so every machine gets the same
 * trace for the same seed (rand() differs between C libraries). */
static unsigned long next_random(void) {
    seed = (seed * 1103515245UL + 12345UL) % 2147483648UL;
    return seed / 65536;
}

/* One random trace: ops steps, each a request (1 to 64 bytes) or, two times
 * in four, the free of an earlier request still held. The trace is made from
 * the seed alone, never from what an allocator did, so both allocators are
 * given exactly the same requests. */
#define OPS 100
static int    kind[OPS];          /* 1 = a request, 0 = a free               */
static size_t bytes[OPS];         /* a request: how many bytes               */
static int    which[OPS];         /* a free: the step of the request it frees */

static int make_trace(unsigned long s) {
    int held[OPS], nheld = 0, requests = 0;
    seed = s;
    for (int k = 0; k < OPS; k++) {
        if (nheld > 0 && next_random() % 4 < 2) {
            int j = (int)(next_random() % (unsigned long)nheld);
            kind[k] = 0;
            which[k] = held[j];
            held[j] = held[--nheld];      /* forget it: freed once only     */
        } else {
            kind[k] = 1;
            bytes[k] = 1 + next_random() % 64;
            held[nheld++] = k;
            requests++;
        }
    }
    return requests;
}

/* The free bytes in the arena: every free block's payload, plus what was
 * never handed out. */
static size_t free_bytes(void) {
    size_t sum = ARENA_SIZE - top;
    for (Block *b = (Block *)arena; (unsigned char *)b < arena + top;
         b = next_block(b))
        if (b->free)
            sum += b->size;
    return sum;
}

/* Replay the trace through one allocator, from an empty arena. Returns the
 * failed requests. For the free list, *roomy counts the requests that failed
 * although the free bytes added up to enough, and *holes the free blocks left
 * at the end. */
static int replay(enum mode m, int *roomy, int *holes) {
    void *got[OPS];
    int failed = 0;
    top = 0;
    memset(arena, 0, sizeof arena);
    *roomy = 0;
    *holes = 0;
    for (int k = 0; k < OPS; k++) {
        if (kind[k]) {
            got[k] = (m == BUMP) ? bump_alloc(bytes[k]) : list_alloc(bytes[k]);
            if (got[k] == NULL) {
                failed++;
                if (m == LIST && free_bytes() >= align_up(bytes[k]))
                    (*roomy)++;           /* enough in total, not in one place */
            }
        } else if (m == LIST) {
            list_free(got[which[k]]);     /* list_free(NULL) does nothing   */
        }
    }
    if (m == LIST)
        for (Block *b = (Block *)arena; (unsigned char *)b < arena + top;
             b = next_block(b))
            if (b->free)
                (*holes)++;
    return failed;
}

static int bench(void) {
    printf("%d random steps (requests of 1-64 bytes, and frees), arena %d bytes\n",
           OPS, ARENA_SIZE);
    printf("seed  requests  bump failed  list failed  "
           "of those, with enough free bytes  free blocks at end\n");
    for (unsigned long s = 1; s <= 5; s++) {
        int roomy, holes, requests = make_trace(s);
        int fb = replay(BUMP, &roomy, &holes);
        int fl = replay(LIST, &roomy, &holes);
        printf("%4lu  %8d  %11d  %11d  %32d  %18d\n", s, requests, fb, fl,
               roomy, holes);
    }
    return 0;
}

static int usage(void) {
    fprintf(stderr, "usage: alloc bump|list TRACE\n"
                    "       alloc bench\n");
    return 2;
}

int main(int argc, char *argv[]) {
    if (argc == 2 && strcmp(argv[1], "bench") == 0)
        return bench();
    if (argc != 3)
        return usage();
    if (strcmp(argv[1], "bump") == 0)
        return run(BUMP, argv[2]);
    if (strcmp(argv[1], "list") == 0)
        return run(LIST, argv[2]);
    return usage();
}
