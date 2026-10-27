/*
 * COMP 310 — Week 9 — where does each kind of variable live?
 *
 * Prints the address of one thing from each region of the address space,
 * then the regions in address order, lowest first:
 *
 *   ./demo/where
 *
 * The addresses change from run to run: the kernel places each region at a
 * random offset (address space layout randomization). The order does not:
 * code, data, heap, stack, lowest first, on the Linux the course grades on.
 */
#define _POSIX_C_SOURCE 200809L   /* POSIX under -std=c11 (Week 2's rule)    */
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

int counter = 7;                  /* a global with a value: the data region   */

typedef struct {
    const char *region;           /* the name of the region                   */
    const char *what;             /* the thing whose address we took          */
    uintptr_t   addr;             /* its address, as a number we can compare  */
} Place;

int main(void) {
    int local = 0;                            /* a local: the stack          */
    int *block = malloc(sizeof *block);       /* from malloc: the heap       */
    if (block == NULL) {
        perror("malloc");
        return 1;
    }
    Place p[4] = {
        { "code",  "the function main", (uintptr_t)&main },
        { "data",  "the global counter", (uintptr_t)&counter },
        { "heap",  "a block from malloc", (uintptr_t)block },
        { "stack", "the local variable", (uintptr_t)&local },
    };
    for (int i = 0; i < 4; i++)
        printf("%-6s %-20s %#014jx\n", p[i].region, p[i].what,
               (uintmax_t)p[i].addr);

    for (int i = 1; i < 4; i++)               /* sort by address: insertion  */
        for (int j = i; j > 0 && p[j].addr < p[j - 1].addr; j--) {
            Place t = p[j];
            p[j] = p[j - 1];
            p[j - 1] = t;
        }
    printf("lowest address first:");
    for (int i = 0; i < 4; i++)
        printf(" %s", p[i].region);
    printf("\n");
    free(block);
    return 0;
}
