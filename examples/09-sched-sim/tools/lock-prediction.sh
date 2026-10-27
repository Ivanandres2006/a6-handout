#!/bin/sh
# Used by the Makefile: asks for your prediction before you measure.
# Write it in the ## Prediction section of REPORT.md first.
set -u
mode=${1:-}; report=${2:-REPORT.md}; lock=${3:-bench/prediction.lock}; min=${4:-40}

section() {
  awk '
    /^## / { if (inp) exit; if ($0 ~ /^## Prediction[ \t]*$/) { inp = 1; next } }
    inp { sub(/[ \t\r]+$/, ""); if ($0 != "") print }
  ' "$report"
}

words() {
  section | awk '
    /<!--/ { c = 1 }
    c { if ($0 ~ /-->/) c = 0; next }
    /^_.*_$/ || /^\*[^*].*\*$/ { next }
    { n += NF } END { print n + 0 }
  '
}

hash() {
  if command -v sha256sum >/dev/null 2>&1; then section | sha256sum | cut -d' ' -f1
  else section | shasum -a 256 | cut -d' ' -f1; fi
}

[ -f "$report" ] || { echo "lock-prediction: no $report" >&2; exit 2; }

case "$mode" in
  lock)
    n=$(words)
    if [ "$n" -lt "$min" ]; then
      echo "Write your prediction first: the ## Prediction section of $report has $n word(s) of your own; it needs $min." >&2
      echo "Say what you expect to see and why, before you measure. Being wrong is fine." >&2
      exit 1
    fi
    mkdir -p "$(dirname "$lock")"
    printf '%s %s\n' "$(date -u +%Y-%m-%dT%H:%M:%SZ)" "$(hash)" >> "$lock"
    ;;
  verify)
    if [ ! -s "$lock" ]; then
      echo "WARNING: no record that your prediction came before your data - run the measurement again." >&2
      exit 1
    fi
    first=$(head -1 "$lock" | cut -d' ' -f2)
    if [ "$first" != "$(hash)" ]; then
      echo "WARNING: your ## Prediction has changed since you first measured." >&2
      echo "Put it back as it was, and write what you learned in your results instead." >&2
      exit 1
    fi
    echo "prediction: unchanged since $(head -1 "$lock" | cut -d' ' -f1)"
    ;;
  *) echo "usage: lock-prediction.sh lock|verify REPORT.md bench/prediction.lock [min_words]" >&2; exit 2;;
esac
