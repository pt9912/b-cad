#!/usr/bin/env bash
# arch-check — hexagonale Schichtung durchsetzen (ADR-0001).
# Computational feedback (Modul 13). Läuft als Dockerfile-Target-Stage
# über die per COPY eingebackenen Quellen (kein Bind-Mount); rein
# textbasiert, keine Toolchain nötig.
#
# Regel A: Der Kern src/hexagon/ ist framework-frei — kein Qt, kein
#          OpenCascade (*.hxx), kein SQLite, kein Import aus adapters/.
# Regel B: Kein Adapter importiert einen anderen Adapter.
#
# HINWEIS: Heuristik, kein C++-Parser. Das Qt-Muster `Q[A-Za-z]` würde
# einen künftigen framework-freien Kern-Header wie `Queue.h` falsch
# anschlagen (False Positive). Heute unkritisch (Kern ist Qt-frei); bei
# Bedarf auf eine Include-Allowlist umstellen.
set -euo pipefail
cd "$(dirname "$0")/.."

status=0

# --- Regel A: Kern-Reinheit ---
a_hits="$(grep -rnE '#include[[:space:]]*[<"](Q[A-Za-z]|sqlite3\.h)|#include[[:space:]]*"adapters/|#include[[:space:]]*<[A-Za-z0-9_]+\.hxx>' src/hexagon 2>/dev/null || true)"
if [ -n "$a_hits" ]; then
    echo "ARCH-CHECK FAIL (ADR-0001, Regel A): Kern src/hexagon/ importiert Qt/OpenCascade/SQLite/adapters:"
    echo "$a_hits"
    status=1
fi

# --- Regel B: keine Adapter-zu-Adapter-Importe ---
while IFS= read -r f; do
    own="$(printf '%s' "$f" | sed -E 's#^src/adapters/([^/]+)/.*#\1#')"
    includes="$(grep -nE '#include[[:space:]]*"adapters/' "$f" || true)"
    [ -n "$includes" ] || continue
    bad="$(printf '%s\n' "$includes" | grep -vE "\"adapters/${own}/" || true)"
    if [ -n "$bad" ]; then
        echo "ARCH-CHECK FAIL (ADR-0001, Regel B): Adapter $f importiert einen anderen Adapter:"
        echo "$bad"
        status=1
    fi
done < <(find src/adapters -type f \( -name '*.cpp' -o -name '*.h' \) 2>/dev/null)

if [ "$status" -eq 0 ]; then
    echo "arch-check ok: hexagonale Schichtung gewahrt (ADR-0001)"
fi
exit "$status"
