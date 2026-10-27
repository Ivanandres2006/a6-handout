#!/bin/sh
# Used by the Makefile (`make workload`, `make compare`): writes YOUR workload,
# and the parameters `make compare` runs it with, from the "student" field of
# submission.json. Every netid gives a different workload.
set -u
out=${1:-data/workload.txt}

netid=$(sed -n 's/.*"student"[[:space:]]*:[[:space:]]*"\([^"]*\)".*/\1/p' \
        submission.json 2>/dev/null | head -1 | tr 'A-Z' 'a-z')
case "$netid" in
  ""|yournetid|yourlastname)
    echo "gen-workload: put your netid in the \"student\" field of submission.json first" >&2
    exit 1;;
esac
case "$netid" in
  *[!a-z0-9._-]*)
    echo "gen-workload: \"$netid\" is not a netid (letters, digits, '.', '_' or '-')" >&2
    exit 1;;
esac

if command -v sha256sum >/dev/null 2>&1; then
  h=$(printf '%s:A6' "$netid" | sha256sum | cut -d' ' -f1)
else
  h=$(printf '%s:A6' "$netid" | shasum -a 256 | cut -d' ' -f1)
fi

mkdir -p "$(dirname "$out")"
printf '%s\n' "$h" | awk -v id="$netid" '
  function byte(i) {
    return 16 * (index(hex, substr($0, 2 * i + 1, 1)) - 1) \
              + (index(hex, substr($0, 2 * i + 2, 1)) - 1)
  }
  BEGIN { hex = "0123456789abcdef" }
  {
    n  = 6 + byte(0) % 3
    rr = 1 + byte(1) % 4
    ms = 1 + byte(2) % 4
    if (ms == rr) ms = rr % 4 + 1
    al = ms * (1 + byte(3) % 2) + byte(4) % 2
    bo = 20 + 5 * (byte(5) % 4)
    print "# A6 workload for " id ": NAME ARRIVAL BURST"
    print "# rr_slice " rr
    print "# mlfq_slice " ms " allot " al " boost " bo
    print "A 0 " (10 + byte(6) % 6)
    arr = 0
    for (i = 1; i < n; i++) {
      arr += byte(5 + 2 * i) % 4
      printf "%c %d %d\n", 65 + i, arr, 1 + byte(6 + 2 * i) % 8
    }
  }' > "$out"
