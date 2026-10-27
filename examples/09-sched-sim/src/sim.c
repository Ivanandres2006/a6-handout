/*
 * COMP 310 - Week 9 - A6: FCFS, round robin and a two-queue MLFQ.
 *
 * TODO: write the three functions src/sim.h declares, to the contract there
 * and to the rules in the handout's section 1: the five steps of every tick,
 * the two tie rules, a zero-length burst, and MLFQ's Rules 1 to 5. Until you
 * do, each returns -1 (no timeline), and `make test` shows 0 course tests
 * passing.
 *
 * How you organize it is your decision: one tick loop the three policies
 * share, or one loop each. Week 8's demo/sched.c shows a tick loop for FCFS
 * and round robin; MLFQ's second queue and its rules are new.
 */
#define _POSIX_C_SOURCE 200809L   /* POSIX under -std=c11 (Week 2's rule)  */
#include <stdio.h>

#include "sim.h"

int sim_fcfs(Job *jobs, int n, char *chart) {
    (void)jobs;
    (void)n;
    (void)chart;
    fprintf(stderr, "sim_fcfs: not written yet\n");
    return -1;
}

int sim_rr(Job *jobs, int n, int slice, char *chart) {
    (void)jobs;
    (void)n;
    (void)slice;
    (void)chart;
    fprintf(stderr, "sim_rr: not written yet\n");
    return -1;
}

int sim_mlfq(Job *jobs, int n, int slice, int allot, int boost, char *chart) {
    (void)jobs;
    (void)n;
    (void)slice;
    (void)allot;
    (void)boost;
    (void)chart;
    fprintf(stderr, "sim_mlfq: not written yet\n");
    return -1;
}
