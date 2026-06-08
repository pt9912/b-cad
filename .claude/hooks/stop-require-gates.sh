#!/usr/bin/env bash
# stop-require-gates — gibt den Stop nur frei, wenn der aktuelle
# Arbeitsbaum durch einen erfolgreichen `make gates`-Lauf abgedeckt ist.
# Nutzt dieselbe Hash-Funktion wie record-gates (keine Logik-Dopplung).
set -euo pipefail
cd "$(git rev-parse --show-toplevel)"

state_file=".harness/state/gates-passed.diffsha"

# Kein Diff und keine untracked files → nichts zu blockieren.
if [ -z "$(git status --porcelain=v1)" ]; then
  cat <<'JSON'
{"decision":"approve"}
JSON
  exit 0
fi

if [ ! -f "$state_file" ]; then
  cat <<'JSON'
{
  "decision": "block",
  "reason": "There are working tree changes, but no recorded successful make gates run. Run `make gates`."
}
JSON
  exit 0
fi

current="$(bash tools/harness/working-tree-hash.sh)"
recorded="$(cat "$state_file")"

if [ "$current" != "$recorded" ]; then
  cat <<'JSON'
{
  "decision": "block",
  "reason": "The working tree changed after the last recorded gates run. Run `make gates` again."
}
JSON
  exit 0
fi

cat <<'JSON'
{"decision":"approve"}
JSON
