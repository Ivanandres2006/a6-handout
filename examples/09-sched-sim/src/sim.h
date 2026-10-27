/*
 * COMP 310 - Week 9 - A6: the simulator's interface.
 *
 * NOT YOURS TO EDIT. src/main.c, src/workload.c and the course tests are
 * written against these declarations, and your src/sim.c and src/metrics.c
 * must provide every function declared here exactly as declared.
 *
 * Time is counted in ticks from 0. In each tick the CPU runs one job, or
 * nothing (an idle tick, drawn '.').
 */
#ifndef SIM_H
#define SIM_H

#define MAX_JOBS  26              /* jobs are named A to Z                 */
#define MAX_TICKS 200             /* the longest timeline a run may draw   */

typedef struct {
    char name;                    /* one capital letter                    */
    int  arrival;                 /* the tick it arrives                   */
    int  burst;                   /* ticks of CPU it needs in all (0 or more) */
    int  remaining;               /* ticks it still has to run             */
    int  start;                   /* the tick it first ran; -1 until then  */
    int  finish;                  /* the tick it finished                  */
} Job;

/* ---- src/sim.c (yours) --------------------------------------------------
 *
 * Each function runs the n jobs in jobs[0..n-1] (in input order) under one
 * policy, exactly as the handout's rules state, and
 *   - sets every job's start and finish;
 *   - writes the timeline into chart: one character per tick from tick 0, the
 *     name of the job that ran or '.' for an idle tick, then a '\0'. chart has
 *     room for MAX_TICKS + 1 characters;
 *   - returns the timeline's length: the latest finish of any job.
 * If the timeline would be longer than MAX_TICKS ticks, it returns -1 and
 * writes nothing past chart[MAX_TICKS].
 *
 * On entry every job has remaining == burst, start == -1 and finish == 0
 * (load_workload sets them; main gives each policy a fresh copy).
 */
int sim_fcfs(Job *jobs, int n, char *chart);
int sim_rr(Job *jobs, int n, int slice, char *chart);
int sim_mlfq(Job *jobs, int n, int slice, int allot, int boost, char *chart);

/* ---- src/metrics.c (yours) ----------------------------------------------
 *
 * Called only after a simulation has set every start and finish.
 *   turnaround       finish - arrival     (from arriving to done)
 *   response         first run - arrival  (from arriving to first running)
 *   avg_turnaround   the mean turnaround of the n jobs, n > 0
 *   avg_response     the mean response of the n jobs, n > 0
 *   worst_response   the INDEX of the job with the largest response; when
 *                    two or more share it, the one earliest in input order
 */
int    turnaround(const Job *j);
int    response(const Job *j);
double avg_turnaround(const Job *jobs, int n);
double avg_response(const Job *jobs, int n);
int    worst_response(const Job *jobs, int n);

/* ---- src/workload.c (given) ---------------------------------------------
 *
 * Reads "NAME ARRIVAL BURST" lines ('#' starts a comment line) into jobs and
 * returns how many it read, or -1 after printing what was wrong.
 */
int load_workload(const char *path, Job *jobs);

#endif
