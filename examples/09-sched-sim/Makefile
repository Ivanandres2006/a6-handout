# COMP 310 - Week 9 - A6: a scheduler simulator
#
#   make            build ./build/sim from src/
#   make test       the course tests, your tests (tests/test_mine.sh), then the
#                   shape of the submission
#   make workload   your workload and its parameters, from your netid
#   make compare    asks for your ## Prediction first, then runs all three
#                   policies on your workload -> bench/compare.txt
#   make submit     build the zip you hand in
#
# A recipe piped through tee exits with tee's status, so a crashed run would
# look like a success; pipefail makes the recipe fail with it.
SHELL       := /bin/bash
.SHELLFLAGS := -o pipefail -c

CC      ?= cc
CFLAGS  ?= -std=c11 -Wall -Wextra -O2 -Isrc
SRC      = src/main.c src/workload.c src/sim.c src/metrics.c
WORKLOAD = data/workload.txt
LOCK     = bench/prediction.lock

# The parameters gen-workload.sh writes at the top of your workload.
RR_SLICE = $(shell sed -n 's/^\# rr_slice \([0-9]*\)$$/\1/p' $(WORKLOAD) 2>/dev/null)
MLFQ     = $(shell sed -n 's/^\# mlfq_slice \([0-9]*\) allot \([0-9]*\) boost \([0-9]*\)$$/\1 \2 \3/p' $(WORKLOAD) 2>/dev/null)

.PHONY: all test workload compare clean submit help

all: build/sim

build/sim: $(SRC) src/sim.h
	@mkdir -p build
	$(CC) $(CFLAGS) -o $@ $(SRC)

# Every part runs even when an earlier one fails, so you see everything at once.
test: build/sim
	@st=0; \
	echo "== course tests =="; sh tests/run-tests.sh || st=1; \
	echo; echo "== your tests =="; sh tests/test_mine.sh || st=1; \
	echo; echo "== the submission specification =="; \
	sh tests/check-submission-spec.sh || st=1; \
	if [ -f $(LOCK) ]; then sh tools/lock-prediction.sh verify REPORT.md $(LOCK) || st=1; fi; \
	if command -v python3 >/dev/null 2>&1 && [ -f tests/check_submission_ready.py ]; then \
		echo; python3 tests/check_submission_ready.py || true; \
	fi; \
	sh tests/process-evidence.sh log test $$st; \
	exit $$st

# Rewritten whenever submission.json changes, so it is always your netid's.
$(WORKLOAD): submission.json tools/gen-workload.sh
	@sh tools/gen-workload.sh $(WORKLOAD)

workload: $(WORKLOAD)
	@cat $(WORKLOAD)

# Asks for your ## Prediction in REPORT.md first.
compare: build/sim $(WORKLOAD)
	@mkdir -p bench
	@sh tools/lock-prediction.sh lock REPORT.md $(LOCK)
	@st=0; ./build/sim compare $(WORKLOAD) $(RR_SLICE) $(MLFQ) | tee bench/compare.txt || st=1; \
	sh tests/process-evidence.sh log compare $$st; exit $$st

help:
	@echo "make           - build ./build/sim"
	@echo "make test      - the course tests, your tests, and the submission check"
	@echo "make workload  - your workload and its parameters, from your netid"
	@echo "make compare   - your prediction first, then all three policies -> bench/compare.txt"
	@echo "make submit    - build the zip you hand in"
	@echo "make clean     - remove build output"

clean:
	@rm -rf build

submit:
	@sh tests/make-submission.sh
