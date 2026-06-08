#!/usr/bin/env bash
# gate-consistency — Doku ↔ Makefile-Konsistenz (Honesty, Modul 13/15).
# Läuft als Dockerfile-Target-Stage über COPY-eingebackene Quellen (kein
# Bind-Mount); rein textbasiert.
#
# Regel: Jeder `make <target>`, der in AGENTS.md oder harness/README.md
#        als REAL dargestellt ist (d. h. NICHT unter einem
#        'Geplant'/'Nicht behauptet'-Abschnitt), muss als Makefile-Target
#        existieren. Fängt halluzinierte Gates — die Drift-Klasse, die
#        docs-check (nur Markdown-Links) nicht sieht.
#
# HINWEIS: Heuristik, kein Parser. Die 'real vs geplant'-Erkennung kippt
# an Markern (**Real… / Geplant / Nicht behauptet / ##-Überschrift). Bei
# Umbau der Doku-Struktur Marker mitführen. Negativtest sichert die
# Wirksamkeit ab.
set -euo pipefail
cd "$(dirname "$0")/.."

# 1) reale Makefile-Targets als Menge
targets="$(grep -oE '^[a-zA-Z][a-zA-Z0-9_-]*:' Makefile | sed 's/:$//' | sort -u)"
is_target() { printf '%s\n' "$targets" | grep -qx "$1"; }

status=0
for doc in AGENTS.md harness/README.md; do
    [ -f "$doc" ] || continue
    # `make <target>`-Tokens außerhalb 'Geplant/Nicht behauptet' sammeln
    claimed="$(awk '
        /^\*\*Real|^##[^#]/                 { planned=0 }
        /[Gg]eplant|[Nn]icht behauptet/     { planned=1 }
        {
            if (!planned) {
                s=$0
                while (match(s, /make [a-zA-Z][a-zA-Z0-9_-]*/)) {
                    print substr(s, RSTART+5, RLENGTH-5)
                    s=substr(s, RSTART+RLENGTH)
                }
            }
        }' "$doc" | sort -u)"
    while IFS= read -r t; do
        [ -n "$t" ] || continue
        if ! is_target "$t"; then
            echo "GATE-CONSISTENCY FAIL: $doc stellt 'make $t' als real dar, aber Makefile hat kein Target '$t:'"
            status=1
        fi
    done <<< "$claimed"
done

if [ "$status" -eq 0 ]; then
    echo "gate-consistency ok: alle als real dargestellten make-Targets existieren im Makefile"
fi
exit "$status"
