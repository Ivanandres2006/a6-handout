#!/bin/sh
# process-evidence.sh - a record of how the work was built, packed by `make submit`.
#
# THIS FILE IS NOT YOURS TO EDIT. It came with the starter, and the same file
# ships in every course. It is plain `sh`; nothing in it is hidden from you.
#
# What it records, and why:
#   make test / make bench  add one line to .worklog in this folder: the time
#                           (UTC), what ran, whether it passed, and a hash of
#                           your files at that moment - so the record shows the
#                           work changing over the days you did it.
#   make submit             packs three files into the zip:
#                             worklog.txt   the .worklog lines, plus this submit
#                             test-run.txt  make test, run once more as the zip
#                                           is built, with its time
#                             manifest.txt  when and where the zip was built,
#                                           your git commit (if any), and a
#                                           SHA-256 of every file in the zip
#
# Nothing here reads anything outside this folder, and nothing is sent anywhere:
# it all goes into the zip you upload yourself. .worklog is local to your
# machine - keep it out of git (the starter's .gitignore does).
#
#   sh tests/process-evidence.sh log  <target> <exit-status>
#   sh tests/process-evidence.sh pack <staged submission/ folder>
set -u

WORKLOG=".worklog"

now() { date -u '+%Y-%m-%dT%H:%M:%SZ'; }

# sha256 of stdin or of a file, on macOS, Linux and WSL alike.
sha() {
  if command -v sha256sum >/dev/null 2>&1; then sha256sum "$@" | cut -d' ' -f1
  elif command -v shasum >/dev/null 2>&1; then shasum -a 256 "$@" | cut -d' ' -f1
  else python3 -c 'import hashlib,sys
d=open(sys.argv[1],"rb").read() if len(sys.argv)>1 else sys.stdin.buffer.read()
print(hashlib.sha256(d).hexdigest())' "$@"
  fi
}

# The files that are the work, in a fixed order: everything in this folder
# except hidden folders (version control, local environments, the packing
# stage, tool caches), build output and caches.
work_files() {
  find . -mindepth 1 \( -type d -name '.*' -o -name build -o -name __pycache__ \
            -o -name node_modules \) -prune -o -type f \
         ! -name '*.zip' ! -name "$WORKLOG" ! -name '*.pyc' ! -name '.DS_Store' \
         -print 2>/dev/null | LC_ALL=C sort
}

# One hash for the whole folder: the hash of every file's hash and name.
tree_hash() {
  work_files | while IFS= read -r f; do
    printf '%s  %s\n' "$(sha "$f")" "$f"
  done | sha | cut -c1-16
}

git_line() {
  if command -v git >/dev/null 2>&1 && git rev-parse --is-inside-work-tree >/dev/null 2>&1; then
    head=$(git rev-parse --short HEAD 2>/dev/null || echo none)
    dirty=$(git status --porcelain 2>/dev/null | wc -l | tr -d ' ')
    commits=$(git rev-list --count HEAD 2>/dev/null || echo 0)
    printf 'git_head: %s\ngit_commits: %s\ngit_uncommitted_files: %s\n' "$head" "$commits" "$dirty"
  else
    printf 'git_head: none (not a git repository)\n'
  fi
}

case "${1:-}" in
  log)
    target="${2:-?}"; status="${3:-?}"
    case "$status" in 0) res=pass ;; "?"|-) res="$status" ;; *) res="fail($status)" ;; esac
    printf '%s\t%s\t%s\t%s\n' "$(now)" "$target" "$res" "$(tree_hash)" >> "$WORKLOG" 2>/dev/null
    exit 0 ;;

  pack)
    out="${2:?usage: process-evidence.sh pack <staged submission folder>}"
    [ -d "$out" ] || { echo "  process-evidence: no folder $out"; exit 0; }

    # 1. make test once more, as the zip is built (PE_NO_TESTRUN=1 skips it).
    if [ "${PE_NO_TESTRUN:-0}" != 1 ]; then
      {
        echo "# test-run.txt - make test, run by make submit"
        echo "# started: $(now)"
        make -s test 2>&1
        echo "# exit status: $?"
        echo "# finished: $(now)"
      } > "$out/test-run.txt" 2>&1
    fi

    # 2. this submit, then the whole log.
    printf '%s\t%s\t%s\t%s\n' "$(now)" submit - "$(tree_hash)" >> "$WORKLOG" 2>/dev/null
    {
      echo "# worklog.txt - one line per make test / make bench / make submit on the"
      echo "# machine that built this zip: UTC time, target, result, hash of the files"
      [ -f "$WORKLOG" ] && cat "$WORKLOG"
    } > "$out/worklog.txt"

    # 3. the manifest: when, where, which commit, and a hash of every packed file.
    {
      echo "# manifest.txt - written by make submit; do not edit the zip after it"
      echo "made_by: make submit (process-evidence.sh)"
      echo "utc: $(now)"
      echo "os: $(uname -sr 2>/dev/null || echo unknown)"
      git_line
      echo "files:"
      ( cd "$out" && find . -type f ! -name manifest.txt -print | LC_ALL=C sort | while IFS= read -r f; do
          printf '%s  %s\n' "$(sha "$f")" "${f#./}"
        done )
    } > "$out/manifest.txt"
    echo "  process record: worklog.txt ($(grep -c '	' "$out/worklog.txt" 2>/dev/null || echo 0) line(s)), test-run.txt, manifest.txt"
    exit 0 ;;

  *)
    echo "usage: sh tests/process-evidence.sh log <target> <exit-status> | pack <folder>" >&2
    exit 2 ;;
esac
