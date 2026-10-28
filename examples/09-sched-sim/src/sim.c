
#define _POSIX_C_SOURCE 200809L   /* POSIX under -std=c11 (Week 2's rule)  */
#include <stdio.h>

#include "sim.h"

int sim_fcfs(Job *jobs, int n, char *chart) {
    int time = 0;
    int done = 0;

    while (done < n) {
        int current = -1;
        for (int i = 0; i < n; i++) {
            if (jobs[i].arrival <= time && jobs[i].finish == 0) {
                if (current == -1 || jobs[i].arrival < jobs[current].arrival) {
                    current = i;
                }
            }
        }
        if (current == -1){
            if (time >= MAX_TICKS) return -1;
            chart[time] = '.';
            time++;
            continue;
        }
        jobs[current].start = time;

        while (jobs[current].remaining > 0) {
            if (time >= MAX_TICKS) return -1;

            chart[time] = jobs[current].name;
            jobs[current].remaining--;
            time++;
        }

        jobs[current].finish = time;
        done++;
    }

    chart[time] = '\0';
    return time;
}


int sim_rr(Job *jobs, int n, int slice, char *chart) {
    int time = 0, done = 0;
    int queue[MAX_TICKS + MAX_JOBS];
    int front = 0, back = 0;
    int current = -1, used = 0;

    while (done < n) {
        for (int i = 0; i < n; i++) {
            if (jobs[i].arrival == time) {
                if (jobs[i].burst == 0) {
                    jobs[i].start = time;
                    jobs[i].finish = time;
                    done++;
                } else {
                    queue[back++] = i;
                }
            }
        }

        if (current != -1 && used == slice) {
            queue[back++] = current;
            current = -1;
        }

        if (current == -1 && front < back) {
            current = queue[front++];
            used = 0;

            if (jobs[current].start == -1)
                jobs[current].start = time;
        }

        if (done == n) break;
        if (time >= MAX_TICKS) return -1;

        chart[time] = '.';

        if (current != -1) {
            chart[time] = jobs[current].name;
            jobs[current].remaining--;
            used++;

            if (jobs[current].remaining == 0) {
                jobs[current].finish = time + 1;
                done++;
                current = -1;
            }
        }

        time++;
    }

    chart[time] = '\0';
    return time;
}



int sim_mlfq(Job *jobs, int n, int slice, int allot, int boost, char *chart) {
    int time = 0, done = 0;
    int current = -1, used = 0;
    int high[MAX_JOBS], low[MAX_JOBS];
    int h = 0, l = 0;
    int count[MAX_JOBS] = {0};
    int level[MAX_JOBS] = {0};

    while (done < n) {
        for (int i = 0; i < n; i++) {
            if (jobs[i].arrival == time) {
                if (jobs[i].burst == 0) {
                    jobs[i].start = time;
                    jobs[i].finish = time;
                    done++;
                } else {
                    high[h++] = i;
                }
            }
        }

        if (boost > 0 && time > 0 && time % boost == 0) {
            for (int i = 0; i < l; i++) {
                high[h++] = low[i];
                level[low[i]] = 0;
            }
            l = 0;

            if (current != -1) level[current] = 0;

            for (int i = 0; i < n; i++) count[i] = 0;
        }

        if (current != -1) {
            if (level[current] == 0 && count[current] >= allot) {
                level[current] = 1;
                low[l++] = current;
                current = -1;
            } else if (used >= slice ||
                      (level[current] == 1 && h > 0)) {
                if (level[current] == 0)
                    high[h++] = current;
                else
                    low[l++] = current;

                current = -1;
            }
        }

        if (current == -1) {
            if (h > 0) {
                current = high[0];
                for (int i = 0; i < h - 1; i++)
                    high[i] = high[i + 1];
                h--;
            } else if (l > 0) {
                current = low[0];
                for (int i = 0; i < l - 1; i++)
                    low[i] = low[i + 1];
                l--;
            }

            used = 0;

            if (current != -1 && jobs[current].start == -1)
                jobs[current].start = time;
        }

        if (done == n) break;
        if (time >= MAX_TICKS) return -1;

        chart[time] = '.';

        if (current != -1) {
            chart[time] = jobs[current].name;
            jobs[current].remaining--;
            used++;

            if (level[current] == 0) count[current]++;

            if (jobs[current].remaining == 0) {
                jobs[current].finish = time + 1;
                done++;
                current = -1;
            }
        }

        time++;
    }

    chart[time] = '\0';
    return time;
}

