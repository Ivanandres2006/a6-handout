/*
 * COMP 310 - Week 9 - A6: the simulator's command line and its output.
 *
 * NOT YOURS TO EDIT. Every line this program prints has a fixed form, so a
 * run can be compared with another run, or with a timeline worked by hand:
 *
 *   ./build/sim fcfs    WORKLOAD
 *   ./build/sim rr      WORKLOAD SLICE
 *   ./build/sim mlfq    WORKLOAD SLICE ALLOT BOOST
 *   ./build/sim compare WORKLOAD RR_SLICE MLFQ_SLICE ALLOT BOOST
 *
 * SLICE and ALLOT are at least 1; BOOST is 0 (never) or more. `compare` runs
 * all three policies on the same workload, each on a fresh copy, and ends
 * with one summary line per policy.
 *
 * Exit status: 0 on success; 1 if the workload cannot be read or a
 * simulation returns no timeline; 2 on a usage error.
 */
#define _POSIX_C_SOURCE 200809L   /* POSIX under -std=c11 (Week 2's rule)  */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "sim.h"

enum policy { FCFS, RR, MLFQ };

/* A whole number of at least min, or -1. strtol reports text that is not a
 * number, which atoi (Week 8) does not. */
static int number(const char *s, int min) {
    char *end;
    long v = strtol(s, &end, 10);
    if (end == s || *end != '\0' || v < min || v > 100000)
        return -1;
    return (int)v;
}

static int usage(void) {
    fprintf(stderr,
            "usage: sim fcfs    WORKLOAD\n"
            "       sim rr      WORKLOAD SLICE\n"
            "       sim mlfq    WORKLOAD SLICE ALLOT BOOST\n"
            "       sim compare WORKLOAD RR_SLICE MLFQ_SLICE ALLOT BOOST\n");
    return 2;
}

/* Run one policy on jobs (changed in place). Returns the timeline's length,
 * or -1 after saying why there is none. */
static int run(enum policy p, Job *jobs, int n, int slice, int allot, int boost,
               char *chart, const char *path, const char *title) {
    memset(chart, 0, MAX_TICKS + 1);
    int ticks = -1;
    if (p == FCFS)
        ticks = sim_fcfs(jobs, n, chart);
    else if (p == RR)
        ticks = sim_rr(jobs, n, slice, chart);
    else
        ticks = sim_mlfq(jobs, n, slice, allot, boost, chart);
    if (ticks < 0 || ticks > MAX_TICKS) {
        fprintf(stderr, "%s: %s returned no timeline (%d): longer than %d "
                        "ticks, or not simulated\n", path, title, ticks, MAX_TICKS);
        return -1;
    }
    chart[ticks] = '\0';
    return ticks;
}

/* The title line, the timeline with a ruler above it, every job's numbers,
 * the averages and the worst response: Week 8's report, plus its last line. */
static void report(const char *title, const char *path, const Job *jobs, int n,
                   const char *chart, int ticks) {
    printf("%s on %s\n", title, path);
    printf("time  ");
    for (int t = 0; t < ticks; t += 5)
        printf("%-5d", t);                        /* a mark every 5 ticks   */
    printf("\n      %s\n", chart);
    printf("job  arrival  burst  start  finish  turnaround  response\n");
    for (int i = 0; i < n; i++)
        printf("%c    %7d  %5d  %5d  %6d  %10d  %8d\n", jobs[i].name,
               jobs[i].arrival, jobs[i].burst, jobs[i].start,
               jobs[i].finish, turnaround(&jobs[i]), response(&jobs[i]));
    printf("average turnaround %.2f, average response %.2f\n",
           avg_turnaround(jobs, n), avg_response(jobs, n));
    int w = worst_response(jobs, n);
    if (w >= 0 && w < n)
        printf("worst response %d (job %c)\n", response(&jobs[w]), jobs[w].name);
    else
        printf("worst response: not reported (%d)\n", w);
}

int main(int argc, char *argv[]) {
    if (argc < 3)
        return usage();
    const char *cmd = argv[1], *path = argv[2];
    int slice = 0, allot = 0, boost = 0, rr_slice = 0;
    int compare = strcmp(cmd, "compare") == 0;
    enum policy p = FCFS;
    if (strcmp(cmd, "fcfs") == 0) {
        if (argc != 3)
            return usage();
    } else if (strcmp(cmd, "rr") == 0) {
        p = RR;
        if (argc != 4 || (slice = number(argv[3], 1)) < 0)
            return usage();
    } else if (strcmp(cmd, "mlfq") == 0) {
        p = MLFQ;
        if (argc != 6 || (slice = number(argv[3], 1)) < 0
            || (allot = number(argv[4], 1)) < 0 || (boost = number(argv[5], 0)) < 0)
            return usage();
    } else if (compare) {
        if (argc != 7 || (rr_slice = number(argv[3], 1)) < 0
            || (slice = number(argv[4], 1)) < 0 || (allot = number(argv[5], 1)) < 0
            || (boost = number(argv[6], 0)) < 0)
            return usage();
    } else {
        return usage();
    }

    Job jobs[MAX_JOBS], work[MAX_JOBS];
    char chart[MAX_TICKS + 1];
    int n = load_workload(path, jobs);
    if (n < 0)
        return 1;

    char rr_title[64], mlfq_title[96];
    snprintf(rr_title, sizeof rr_title, "rr (slice %d)", compare ? rr_slice : slice);
    snprintf(mlfq_title, sizeof mlfq_title, "mlfq (slice %d, allotment %d, boost %d)",
             slice, allot, boost);

    if (!compare) {
        const char *title = p == FCFS ? "fcfs" : p == RR ? rr_title : mlfq_title;
        int ticks = run(p, jobs, n, slice, allot, boost, chart, path, title);
        if (ticks < 0)
            return 1;
        report(title, path, jobs, n, chart, ticks);
        return 0;
    }

    static const char *NAMES[] = { "fcfs", "rr", "mlfq" };
    const char *titles[] = { "fcfs", rr_title, mlfq_title };
    double at[3], ar[3];
    int worst[3], wname[3];
    for (int q = FCFS; q <= MLFQ; q++) {
        memcpy(work, jobs, sizeof jobs);          /* a fresh copy each time */
        int ticks = run((enum policy)q, work, n, q == RR ? rr_slice : slice,
                        allot, boost, chart, path, titles[q]);
        if (ticks < 0)
            return 1;
        report(titles[q], path, work, n, chart, ticks);
        printf("\n");
        at[q] = avg_turnaround(work, n);
        ar[q] = avg_response(work, n);
        int w = worst_response(work, n);
        worst[q] = (w >= 0 && w < n) ? response(&work[w]) : -1;
        wname[q] = (w >= 0 && w < n) ? work[w].name : '?';
    }
    printf("policy  avg turnaround  avg response  worst response\n");
    for (int q = FCFS; q <= MLFQ; q++)
        printf("%-6s  %14.2f  %12.2f  %8d (job %c)\n", NAMES[q], at[q], ar[q],
               worst[q], wname[q]);
    return 0;
}
