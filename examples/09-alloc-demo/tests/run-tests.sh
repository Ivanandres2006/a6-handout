#!/bin/sh
# COMP 310 — Week 9 — the allocators' tests.
#
# Each case runs demo/alloc and compares what it prints with a file in
# tests/expected/, or checks the exit status of a bad command line or trace.
# The expected maps were checked by hand: block by block, header by header.
cd "$(dirname "$0")/.." || exit 2
pass=0
fail=0

# same OUTFILE ARGS... : the output must match tests/expected/OUTFILE
same() {
    want="tests/expected/$1"; shift
    if ./demo/alloc "$@" 2>&1 | diff -q - "$want" >/dev/null; then
        pass=$((pass + 1))
    else
        fail=$((fail + 1)); echo "FAIL: ./demo/alloc $* differs from $want"
    fi
}

# status CODE ARGS... : the command must exit with CODE and print to stderr
status() {
    code=$1; shift
    ./demo/alloc "$@" >/dev/null 2>tests/.err
    got=$?
    if [ "$got" -eq "$code" ] && [ -s tests/.err ]; then
        pass=$((pass + 1))
    else
        fail=$((fail + 1)); echo "FAIL: ./demo/alloc $* exited $got (want $code, with a message)"
    fi
    rm -f tests/.err
}

for t in bump reuse frag; do
    same "bump-$t.out" bump "data/$t.txt"
    same "list-$t.out" list "data/$t.txt"
done
same bench.out bench

status 2                                   # no arguments
status 2 best data/reuse.txt               # no such allocator
status 1 list data/missing.txt             # no such file
printf '+A 20\n+B twenty\n' > tests/.bad
status 1 list tests/.bad                   # a line that is not +NAME BYTES
printf '+A 20\n+A 40\n' > tests/.bad
status 1 list tests/.bad                   # A asked for again while in use
rm -f tests/.bad

# where: four regions, each named once, and the order line names all four.
out=$(./demo/where)
n=$(printf '%s\n' "$out" | grep -cE '^(code|data|heap|stack) ')
last=$(printf '%s\n' "$out" | tail -n 1)
case "$last" in
    "lowest address first: "*code*) ok=1 ;;
    *) ok=0 ;;
esac
for r in data heap stack; do
    case "$last" in *"$r"*) ;; *) ok=0 ;; esac
done
if [ "$n" -eq 4 ] && [ "$ok" -eq 1 ]; then
    pass=$((pass + 1))
else
    fail=$((fail + 1)); echo "FAIL: ./demo/where did not print four regions and their order"
fi

echo "$pass passed, $fail failed"
[ "$fail" -eq 0 ]
