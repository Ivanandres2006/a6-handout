#!/bin/sh
# COMP 310 - Week 9 - A6: YOUR tests. `make test` runs this file.
#
# Each test prints one line, "[ok]   <name>" or "[FAIL] <name>", and the file
# exits non-zero if any test failed. Keep the three named tests' names exactly
# as they are; add tests of your own below them. A test is worth what it
# catches: break your simulator the way the test is about, watch the test
# fail, then put the code back.
cd "$(dirname "$0")/.." || exit 2
SIM=./build/sim
pass=0
fail=0
T=$(mktemp -d "${TMPDIR:-/tmp}/a6-mine.XXXXXX") || exit 2
trap 'rm -rf "$T"' EXIT

ok()  { pass=$((pass + 1)); printf '[ok]   %s\n' "$1"; }
bad() { fail=$((fail + 1)); printf '[FAIL] %s\n' "$1"; }

# check NAME WANT GOT: passes when the two strings are equal
check() {
    if [ "$2" = "$3" ]; then ok "$1"; else
        bad "$1"; printf '       want: %s\n       got:  %s\n' "$2" "$3"; fi
}

# workload FILE: writes standard input to $T/FILE (use with a here-document)
workload() { cat > "$T/$1"; }

# timeline ARGS...: the timeline ./build/sim ARGS prints, without its indent
timeline() { "$SIM" "$@" 2>/dev/null | sed -n '3s/^ *//p'; }

# row JOB ARGS...: job JOB's line of the table, as "arrival burst start finish turnaround response"
row() {
    j=$1; shift
    "$SIM" "$@" 2>/dev/null | awk -v j="$j" '$1 == j && NF == 7 { print $2, $3, $4, $5, $6, $7 }'
}

# line PATTERN ARGS...: the first line ./build/sim ARGS prints that starts with PATTERN
line() { p=$1; shift; "$SIM" "$@" 2>/dev/null | grep "^$p" | head -1; }

# An example of the form (not one of your tests):
#   workload convoy.txt <<'EOF'
#   A 0 8
#   B 0 2
#   EOF
#   check "a name for what it tests" "AAAAAAAABB" "$(timeline fcfs "$T/convoy.txt")"

# ---- the handout's three named tests: replace each stub ---------------------

bad "the CPU idles until the next job arrives (not written yet)"
bad "a job arriving as a slice ends runs before the job whose slice ended (not written yet)"
bad "a boost lifts a waiting job back to the top queue (not written yet)"

# ---- your own tests ------------------------------------------------------------

echo "$pass passed, $fail failed"
[ "$fail" -eq 0 ]
