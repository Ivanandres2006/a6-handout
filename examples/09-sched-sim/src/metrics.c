/*
 * COMP 310 - Week 9 - A6: each job's turnaround and response, the averages,
 * and the job with the worst response.
 *
 * TODO: write the five functions src/sim.h declares, to the contract there.
 * Until you do, they report nothing useful: every time is 0 and no worst
 * response is reported.
 */

#include "sim.h"

int turnaround(const Job *j) {
    return j->finish - j->arrival;
}

int response(const Job *j) {
    return j->start - j->arrival;
}

double avg_turnaround(const Job *jobs, int n) {
    double total = 0;

    for (int i = 0; i < n; i++) {
        total += turnaround(&jobs[i]);
    }

    return total / n;
}

double avg_response(const Job *jobs, int n) {
    double total = 0;

    for (int i = 0; i < n; i++) {
        total += response(&jobs[i]);
    }

    return total / n;
}

int worst_response(const Job *jobs, int n) {
    int worst = 0;

    for (int i = 1; i < n; i++) {
        if (response(&jobs[i]) > response(&jobs[worst])) {
            worst = i;
        }
    }

    return worst;
}

