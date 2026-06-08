#!/usr/bin/env bash
set -euo pipefail

input="$(cat)"

blocked_patterns='cmake --build|ctest|clang-tidy|apt-get|vcpkg|conan'

if echo "$input" | grep -E "$blocked_patterns" >/dev/null; then
  cat <<'JSON'
{
  "decision": "block",
  "reason": "b-cad is Docker/make-only. Use make targets, not direct host build/test/package commands."
}
JSON
  exit 0
fi

cat <<'JSON'
{"decision": "approve"}
JSON