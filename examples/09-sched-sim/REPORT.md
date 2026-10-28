# COMP 310 — Operating Systems
## Week 9 — Report



## Prediction

I think Round Robin will have the lowest average response time because every job gets a turn. FCFS will have the highest because job A takes 14 ticks, making the other jobs wait longer. I also think job A will finish faster with FCFS because it can run without stopping, while Round Robin makes it wait for other jobs.

## Answers

fcfs_avg_response: 17.50
rr_avg_response: 6.38
mlfq_avg_response: 5.50
reversal_job: A
response_line: src/metrics.c:6

## What

My code runs jobs using FCFS, Round Robin, and MLFQ. It checks when each job arrives and when it starts and finishes. FCFS runs one job at a time, Round Robin lets jobs take turns, and MLFQ uses two queues and moves jobs depending on their priority.

## Results

My prediction was partly right. FCFS had the highest average response time with 17.50, but MLFQ had the lowest with 5.50. Round Robin got 6.38.

Job A finished at 14 with FCFS, but at 45 with MLFQ. This happened because FCFS let A run without stopping, while MLFQ made A wait for other jobs. A ran from 0 to 4, 25 to 29, at 30, and 39 to 45, which made it finish much later.

## Citations

course material only


## AI use

I used ChatGPT to help me understand the assignment, explain the code, and fix errors. I also used it to check my work and see what I still needed to finish.
