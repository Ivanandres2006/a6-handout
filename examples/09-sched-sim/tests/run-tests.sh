#!/bin/sh
# COMP 310 - Week 9 - A6: the course tests. NOT YOURS TO EDIT.
#
# Each test runs ./build/sim on a workload in data/ and compares what it
# prints with a file in tests/expected/. Every expected timeline was checked by
# hand against the handout's rules.
#   schedule  the timeline, and each job's start and finish: what src/sim.c decides
#   metrics   everything printed: also what src/metrics.c computes
# A failing test shows the first lines that differ: `<` is what your
# simulator printed, `>` what was expected.
cd "$(dirname "$0")/.." || exit 2
pass=0
fail=0
T=$(mktemp -d "${TMPDIR:-/tmp}/a6-test.XXXXXX") || exit 2
trap 'rm -rf "$T"' EXIT

if grep -q '"student"[[:space:]]*:[[:space:]]*"yournetid"' submission.json 2>/dev/null; then
    echo "  note: submission.json still says \"yournetid\": set your netid before make workload"
fi

# The title, ruler and timeline lines, then each job's name, arrival, burst,
# start and finish.
schedule_of() {
    awk 'NR <= 3 { print; next } NF == 7 && $1 ~ /^[A-Z]$/ { print $1, $2, $3, $4, $5 }'
}

# run KIND EXPECTED ARGS... : compare what ./build/sim ARGS prints with tests/expected/EXPECTED
run() {
    kind=$1; want="tests/expected/$2"; shift 2
    ./build/sim "$@" > "$T/got" 2>/dev/null
    if [ "$kind" = schedule ]; then
        schedule_of < "$T/got" > "$T/a"; schedule_of < "$want" > "$T/b"
    else
        cp "$T/got" "$T/a"; cp "$want" "$T/b"
    fi
    if diff "$T/a" "$T/b" > "$T/diff" 2>&1; then
        pass=$((pass + 1)); printf '  [ok]   %-8s ./build/sim %s\n' "$kind" "$*"
    else
        fail=$((fail + 1)); printf '  [FAIL] %-8s ./build/sim %s differs from %s\n' "$kind" "$*" "$want"
        sed -n '1,8s/^/           /p' "$T/diff"
    fi
}

run schedule fcfs-convoy.out   fcfs    data/convoy.txt
run schedule fcfs-gap.out      fcfs    data/gap.txt
run schedule rr2-class.out     rr      data/class.txt 2
run schedule rr1-convoy.out    rr      data/convoy.txt 1
run schedule mlfq-mlfq.out     mlfq    data/mlfq.txt 2 4 0
run schedule mlfq-boost.out    mlfq    data/boost.txt 1 2 5
run metrics  mlfq-class.out    mlfq    data/class.txt 2 3 0
run metrics  compare-class.out compare data/class.txt 2 3 6 12

echo "  $pass passed, $fail failed"
[ "$fail" -eq 0 ]
