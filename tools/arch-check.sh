#!/usr/bin/env bash
# arch-check — hexagonale Schichtung durchsetzen (ADR-0001).
# Computational feedback (Modul 13). Läuft als Dockerfile-Target-Stage
# über die per COPY eingebackenen Quellen (kein Bind-Mount); rein
# textbasiert, keine Toolchain nötig.
#
# Regel A: Der Kern src/hexagon/ ist framework-frei — kein Qt, kein
#          OpenCascade (*.hxx), kein SQLite, kein Import aus adapters/.
# Regel B: Kein Adapter importiert einen anderen Adapter.
# Regel C: OCC-Header (*.hxx) NUR in src/adapters/geometry/ (ADR-0002 —
#          OCC bleibt im Geometrie-Adapter gekapselt).
# Regel D: SQLite-Header (sqlite3*) NUR in src/adapters/persistence/
#          (ADR-0003 — SQLite bleibt im Persistenz-Adapter gekapselt).
# Regel E: Qt-Header (<Q...>) NUR in src/adapters/ui/ und in der
#          Composition Root src/main.cpp (ADR-0009 (c) — Qt bleibt im
#          UI-Adapter gekapselt; Plugin-Ausnahme erst mit LH-FA-PLG-*).
#
# HINWEIS: Heuristik, kein C++-Parser. Das Qt-Muster `Q[A-Za-z]` würde
# einen künftigen framework-freien Header wie `Queue.h` falsch
# anschlagen (False Positive, Regeln A und E). Heute unkritisch; bei
# Bedarf auf eine Include-Allowlist umstellen.
set -euo pipefail
cd "$(dirname "$0")/.."

status=0

# --- Regel A: Kern-Reinheit ---
a_hits="$(grep -rnE '#include[[:space:]]*[<"](Q[A-Za-z]|sqlite3\.h)|#include[[:space:]]*"adapters/|#include[[:space:]]*[<"][^>"]*\.hxx[>"]' src/hexagon 2>/dev/null || true)"
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

# --- Regel C: OCC-Header (*.hxx) nur in src/adapters/geometry/ (ADR-0002) ---
# Erfasst jede angled/quoted .hxx-Include-Form, auch mit Pfad-Präfix:
#   <Foo.hxx>, "Foo.hxx", <opencascade/Foo.hxx>, "opencascade/Foo.hxx".
c_hits="$(grep -rnE '#include[[:space:]]*[<"][^>"]*\.hxx[>"]' src \
    --include='*.cpp' --include='*.h' 2>/dev/null \
    | grep -vE '^src/adapters/geometry/' || true)"
if [ -n "$c_hits" ]; then
    echo "ARCH-CHECK FAIL (ADR-0002, Regel C): OCC-Header (*.hxx) außerhalb src/adapters/geometry/:"
    echo "$c_hits"
    status=1
fi

# --- Regel D: SQLite-Header (sqlite3*) nur in src/adapters/persistence/ (ADR-0003) ---
# Erfasst <sqlite3.h>/"sqlite3.h" und Varianten wie <sqlite3ext.h>.
d_hits="$(grep -rnE '#include[[:space:]]*[<"]sqlite3[A-Za-z0-9_]*\.h[>"]' src \
    --include='*.cpp' --include='*.h' 2>/dev/null \
    | grep -vE '^src/adapters/persistence/' || true)"
if [ -n "$d_hits" ]; then
    echo "ARCH-CHECK FAIL (ADR-0003, Regel D): SQLite-Header außerhalb src/adapters/persistence/:"
    echo "$d_hits"
    status=1
fi

# --- Regel E: Qt-Header nur in src/adapters/ui/ + src/main.cpp (ADR-0009) ---
# Erfasst angled/quoted Qt-Includes (<QWidget>, <QtGui/...>, "QString").
e_hits="$(grep -rnE '#include[[:space:]]*[<"]Q[A-Za-z]' src \
    --include='*.cpp' --include='*.h' 2>/dev/null \
    | grep -vE '^src/(adapters/ui/|main\.cpp)' || true)"
if [ -n "$e_hits" ]; then
    echo "ARCH-CHECK FAIL (ADR-0009, Regel E): Qt-Header außerhalb src/adapters/ui/ + src/main.cpp:"
    echo "$e_hits"
    status=1
fi

if [ "$status" -eq 0 ]; then
    echo "arch-check ok: hexagonale Schichtung gewahrt (ADR-0001) + OCC im Geometrie-Adapter (ADR-0002) + SQLite im Persistenz-Adapter (ADR-0003) + Qt im UI-Adapter/Composition-Root (ADR-0009) gekapselt"
fi
exit "$status"
