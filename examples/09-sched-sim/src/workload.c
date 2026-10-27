/*
 * COMP 310 - Week 9 - A6: reading a workload file. NOT YOURS TO EDIT.
 *
 * Week 8's load(), with one change: a burst of 0 is accepted. A job with a
 * zero-length burst needs no CPU; the handout says how a simulation treats it.
 */
#define _POSIX_C_SOURCE 200809L   /* POSIX under -std=c11 (Week 2's rule)  */
#include <stdio.h>
#include <string.h>

#include "sim.h"

int load_workload(const char *path, Job *jobs) {
    FILE *f = fopen(path, "r");
    if (f == NULL) {
        perror(path);
        return -1;
    }
    char line[128];
    int n = 0, lineno = 0;
    while (fgets(line, sizeof line, f) != NULL) {
        lineno++;
        char name;
        int arrival, burst;
        if (line[0] == '#' || strspn(line, " \t\r\n") == strlen(line))
            continue;                             /* comment or blank line  */
        if (sscanf(line, " %c %d %d", &name, &arrival, &burst) != 3
            || name < 'A' || name > 'Z' || arrival < 0 || burst < 0) {
            fprintf(stderr, "%s:%d: expected NAME ARRIVAL BURST, "
                            "such as: A 0 5\n", path, lineno);
            fclose(f);
            return -1;
        }
        if (n == MAX_JOBS) {
            fprintf(stderr, "%s: more than %d jobs\n", path, MAX_JOBS);
            fclose(f);
            return -1;
        }
        jobs[n] = (Job){ name, arrival, burst, burst, -1, 0 };
        n++;
    }
    fclose(f);
    if (n == 0) {
        fprintf(stderr, "%s: no jobs\n", path);
        return -1;
    }
    return n;
}
