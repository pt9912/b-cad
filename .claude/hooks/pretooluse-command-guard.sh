#!/usr/bin/env bash
# pretooluse-command-guard — verbietet direkte Host-Build/Test/Package-
# Tools (b-cad ist Docker/make-only, AGENTS.md §2.9). Nur das
# tool_input.command-Feld wird geprüft (nicht description/tool_name),
# damit ein legitimes `make`-Command nicht blockiert wird, bloß weil
# seine Beschreibung z. B. "cmake" enthält.
set -euo pipefail

input="$(cat)"

# Command robust aus dem JSON ziehen (Wortgrenzen sollen am Command-
# Anfang greifen, nicht am führenden JSON-Quote).
cmd="$(printf '%s' "$input" | node -e '
  let s = "";
  process.stdin.on("data", d => s += d);
  process.stdin.on("end", () => {
    try { const j = JSON.parse(s); process.stdout.write(String((j.tool_input && j.tool_input.command) || "")); }
    catch { process.stdout.write(""); }
  });
')"

# Verbotene Befehlsnamen an Wortgrenzen (Start | Separator | Klammer |
# Whitespace) … (Whitespace | Separator | Ende). Längere Varianten
# zuerst. `make`/`docker`/`git` sind erlaubt und stehen NICHT in der Liste.
blocked='(^|[;&|]|[[:space:]]|\()(cmake|ctest|clang-tidy|clang\+\+|clang|apt-get|apt|vcpkg|conan)([[:space:]]|[;&|]|$)'

if printf '%s' "$cmd" | grep -Eq "$blocked"; then
  cat <<'JSON'
{
  "decision": "block",
  "reason": "b-cad is Docker/make-only (AGENTS.md §2.9). Use make targets, not direct host build/test/package commands (cmake/clang/ctest/apt/vcpkg/conan)."
}
JSON
  exit 0
fi

cat <<'JSON'
{"decision": "approve"}
JSON
