#!/usr/bin/env bash
set -euo pipefail

mkdir -p .harness/state

if ! git diff --quiet; then
  if [ ! -f .harness/state/gates-passed.diffsha ]; then
    cat <<'JSON'
{
  "decision": "block",
  "reason": "There are uncommitted changes, but no recorded successful make gates run."
}
JSON
    exit 0
  fi

  current="$(git diff | sha256sum | awk '{print $1}')"
  recorded="$(cat .harness/state/gates-passed.diffsha)"

  if [ "$current" != "$recorded" ]; then
    cat <<'JSON'
{
  "decision": "block",
  "reason": "The diff changed after the last recorded gates run. Run make gates again."
}
JSON
    exit 0
  fi
fi

cat <<'JSON'
{"decision": "approve"}
JSON