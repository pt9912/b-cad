#!/usr/bin/env bash
# suppression-gate — verbietet Inline-Suppression (AGENTS.md §2.4).
# Ausnahmen gehören zentral in .clang-tidy mit ADR-/Slice-Bezug.
set -euo pipefail
cd "$(dirname "$0")/.."

hits="$(grep -rnE '//[[:space:]]*NOLINT|#[[:space:]]*pragma[[:space:]]+(GCC|clang)[[:space:]]+diagnostic[[:space:]]+ignored' src 2>/dev/null || true)"
if [ -n "$hits" ]; then
    echo "SUPPRESSION-GATE FAIL (AGENTS.md §2.4): Inline-Suppression gefunden — Ausnahmen gehören nach .clang-tidy:"
    echo "$hits"
    exit 1
fi
echo "suppression-gate ok: keine Inline-Suppression"
