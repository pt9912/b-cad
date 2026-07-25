#!/usr/bin/env bash
# arch-check — Plugin-Import-Allowlist (ADR-0017 Regel P2), der letzte lokale
# Rest der hexagonalen Durchsetzung.
# Computational feedback (Modul 13). Läuft als Dockerfile-Target-Stage über
# die per COPY eingebackenen Quellen (kein Bind-Mount); rein textbasiert,
# keine Toolchain nötig.
#
# SCOPE seit slice-030 / MR-013: Das primäre Architektur-Gate ist auf
# a-check umgestellt (externes, digest-gepinntes Image, `.a-check.yml`,
# `make a-check`). a-check trägt: Kern-Reinheit (vormals Regel A), laterale
# Adapter (B), OCC-/SQLite-/Qt-Tech-Kapselung (C/D/E) — inkl. des plugins/-
# Baums —, den dlfcn.h-INCLUDE, die Schicht-Kanten und die driving/driven-
# Richtung.
#
# SCOPE seit slice-050 / MR-021: Auch **Regel P1** (das dlopen/dlsym/dlclose-
# AUFRUF-Monopol) liegt jetzt bei a-check — als `constructs`-Eintrag in
# `.a-check.yml` (Roh-Text-Monopol, ab a-check v0.16.0; scan-weit inkl.
# plugins/-Baum, main.cpp via `composition_root: forbid`). Der P1-Block ist
# hier deshalb entfallen.
#
# Dieses Skript hält damit NUR NOCH Regel P2:
#
#   P2: Die GESCHLOSSENE Import-Allowlist für plugins/ und src/plugin_api/ —
#       Quote-Include NUR aus plugin_api/ + hexagon/model/ +
#       hexagon/ports/driving/, plus Angle-Include-Verbot für Projekt-Präfixe
#       (src/ liegt für Plugins auf dem Include-Pfad).
#
#       Warum das NICHT nach a-check kann: a-check ist kanten-basiert und
#       beurteilt ein Import-Ziel nur, wenn es auf eine `layers`-Schicht
#       auflöst; ein Ziel ohne Schicht (repo-extern, plugin-lokal/relativ,
#       repo-intern aber schichtlos) bleibt UNBEURTEILT — allow-when-
#       unresolvable. Die VERBOTS-Hälfte von P2 (src/adapters/, Qt, OCC,
#       SQLite, dlfcn.h) deckt a-check vollständig ab, quote- wie angle-Form;
#       die ALLOWLIST-Natur ("nur diese drei Präfixe") ist deny-by-default und
#       bleibt darum hier. `constructs` kann sie nicht ausdrücken: es ist ein
#       Monopol ("nur in Zone X"), P2 bräuchte das Inverse.
#       (slice-050, MR-006-Nachtrag HIGH-2 — fixture-belegt.)
#
# HINWEIS: Heuristik, kein C++-Parser.
set -euo pipefail
cd "$(dirname "$0")/.."

status=0

# --- Regel P2: Import-Grenze für plugins/ und src/plugin_api/ (ADR-0017) ---
# Quote-Includes nur aus plugin_api/, hexagon/model/, hexagon/ports/driving/.
# (Qt/OCC/SQLite im plugins/-Baum fängt a-check über die tech-Regeln — vormals
# P2b, hier entfernt.)
p2_hits="$(grep -rnE '#include[[:space:]]*"' plugins src/plugin_api \
    --include='*.cpp' --include='*.h' 2>/dev/null \
    | grep -vE '#include[[:space:]]*"(plugin_api/|hexagon/model/|hexagon/ports/driving/)' || true)"
if [ -n "$p2_hits" ]; then
    echo "ARCH-CHECK FAIL (ADR-0017, Regel P2): unzulässiger Quote-Include in plugins/ bzw. src/plugin_api/:"
    echo "$p2_hits"
    status=1
fi
# Projekt-Header nur in Quote-Form: Angle-Includes von Projekt-Präfixen
# umgingen sonst die P2-Allowlist (src/ liegt für Plugins auf dem
# Include-Pfad — Code-Review-MED-3 slice-026b).
p2c_hits="$(grep -rnE '#include[[:space:]]*<(adapters/|hexagon/|plugin_api/)' plugins src/plugin_api \
    --include='*.cpp' --include='*.h' 2>/dev/null || true)"
if [ -n "$p2c_hits" ]; then
    echo "ARCH-CHECK FAIL (ADR-0017, Regel P2): Projekt-Header als Angle-Include in plugins/ bzw. src/plugin_api/ (Quote-Form + Allowlist umgangen):"
    echo "$p2c_hits"
    status=1
fi

if [ "$status" -eq 0 ]; then
    echo "arch-check ok: Plugin-P-Rest gewahrt (ADR-0017 Regel P2: geschlossene Import-Allowlist für plugins/ + src/plugin_api/). Kern-Reinheit/laterale Adapter/Tech-Kapselung/Schicht-Kanten/Richtung via a-check (MR-013); das dlopen-Aufruf-Monopol (P1) via a-check constructs (MR-021)."
fi
exit "$status"
