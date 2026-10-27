# COMP 310 — Operating Systems
## Week 9 — Report

<!-- Replace the italic prompt under each heading with your own words. Keep the headings: the specification names them, and `make test` checks that they are here. -->

## Prediction

*Write this after `make workload` and before your first `make compare`, in your own words; `make compare` asks for it first. Looking at your workload and its parameters: which policy will have the lowest average response time, and which the highest, and why? Name one job you expect to finish later under the policy with the lowest average response than under the one with the highest, and say why. At least 40 words; being wrong is fine. Do not edit it afterwards; compare it with the data in `## Results`.*

## Answers

*Fill in the right-hand side of each line, `key: value`, as section 2 of the handout describes: each policy's average response as `bench/compare.txt` prints it, the letter of your reversal job, and the `file:line` in `src/metrics.c` where a job's response time is computed. These are checked automatically, so the form matters.*

fcfs_avg_response:
rr_avg_response:
mlfq_avg_response:
reversal_job:
response_line:

## What

*How your simulator works: how a tick runs its five steps, where each tie rule is decided, and how MLFQ keeps its two queues, its allotment counts and its boost.*

## Results

*From bench/compare.txt: the three policies side by side, your prediction confronted with what happened and every difference explained, and your reversal job followed tick by tick through two of your own timelines.*

## Citations

*Anything you looked up, and where — a `man` page, the POSIX spec, a textbook section. If nothing, write "course material only".*

## AI use

*Write `none`, or name each AI tool you used, what you asked it, and which files or parts its output reached.*
