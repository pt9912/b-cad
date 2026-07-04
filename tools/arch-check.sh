#!/usr/bin/env bash
# arch-check — Plugin-System-P-Rest der hexagonalen Durchsetzung (ADR-0017).
# Computational feedback (Modul 13). Läuft als Dockerfile-Target-Stage über
# die per COPY eingebackenen Quellen (kein Bind-Mount); rein textbasiert,
# keine Toolchain nötig.
#
# SCOPE seit slice-030 / MR-013: Das primäre Architektur-Gate ist auf
# a-check umgestellt (externes, digest-gepinntes Image, `.a-check.yml`,
# `make a-check`). a-check trägt jetzt: Kern-Reinheit (vormals Regel A),
# laterale Adapter (B), OCC-/SQLite-/Qt-Tech-Kapselung (C/D/E) — inkl. des
# plugins/-Baums —, den dlfcn.h-INCLUDE, die Schicht-Kanten und die
# driving/driven-Richtung. Dieses Skript hält nur noch den P-Rest, den
# a-check strukturell NICHT sieht (a-check prüft ausschließlich Include-/
# Import-KANTEN):
#
#   P1 (Aufruf): dlopen/dlsym/dlclose als FUNKTIONSAUFRUF (keine Include-Kante)
#               NUR in src/adapters/plugin/ — kein anderer Adapter, nicht der
#               Kern, nicht main.cpp, und KEIN Plugin im plugins/-Baum lädt
#               selbst dynamisch. Den dlfcn.h-Include deckt a-check
#               (tech-Regel `dlfcn.h`, composition_root: forbid).
#   P2:         Feine Import-Allowlist für plugins/ und src/plugin_api/ —
#               Quote-Include NUR aus plugin_api/ + hexagon/model/ +
#               hexagon/ports/driving/, plus Angle-Include-Verbot für
#               Projekt-Präfixe (src/ liegt für Plugins auf dem Include-Pfad).
#               Die grobe Import-KANTE (plugins → nur diese Ebenen) deckt
#               a-check zusätzlich über `edges`; die Quote-vs-Angle-Granularität
#               (slice-026b, Code-Review-MED-3) liegt unter der Kanten-Ebene
#               und bleibt darum hier.
#
# HINWEIS: Heuristik, kein C++-Parser.
set -euo pipefail
cd "$(dirname "$0")/.."

status=0

# --- Regel P1 (Aufruf): dynamisches Laden nur im Plugin-Host (ADR-0017) ---
# NUR das dlopen/dlsym/dlclose-AUFRUFmuster in src/ und plugins/ (das Monopol
# schließt den plugins/-Baum ein: ein Plugin, das selbst dynamisch lädt,
# unterliefe Lifecycle und Fehler-Barriere). Den dlfcn.h-Include deckt a-check.
p1_hits="$(grep -rnE '\bdl(m?open|sym|close)[[:space:]]*\(' src plugins \
    --include='*.cpp' --include='*.h' 2>/dev/null \
    | grep -vE '^src/adapters/plugin/' || true)"
if [ -n "$p1_hits" ]; then
    echo "ARCH-CHECK FAIL (ADR-0017, Regel P1): dlopen/dlsym/dlclose-Aufruf außerhalb src/adapters/plugin/:"
    echo "$p1_hits"
    status=1
fi

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
    echo "arch-check ok: Plugin-P-Rest gewahrt (ADR-0017: dlopen/dlsym/dlclose-Aufruf-Monopol im Plugin-Host + P2-Import-Allowlist plugins//plugin_api). Kern-Reinheit/laterale Adapter/Tech-Kapselung/Schicht-Kanten/Richtung via a-check (MR-013)."
fi
exit "$status"
