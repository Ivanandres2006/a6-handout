# COMP 310 — Operating Systems
## Week 9 — Readme


## Build

The command is: `make`

this creates the build/sim file to run the program

## Run

I used these commands to run my program:

./build/sim fcfs data/workload.txt
./build/sim rr data/workload.txt 3
./build/sim mlfq data/workload.txt 4 4 30

I used `make compare` to see the results of all three

## File map

- `src/sim.c` — Runs FCFS, RR, and MLFQ
- `src/metrics.c` — Calculates the times for each job
- `tests/test_mine.sh` — Checks if my code works
- `REPORT.md` — Has my prediction and results
- `README.md` — Shows how to run my code
- `submission.json` — Has my information

## Notes

All 8 course tests and my 5 tests passed. I compared the three scheduling methods and saw that MLFQ had the lowest average response time for my workload
