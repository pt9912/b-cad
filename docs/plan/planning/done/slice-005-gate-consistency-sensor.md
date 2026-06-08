---
id: slice-005
titel: Gate-Consistency-Sensor (Doku ↔ Makefile)
status: in-progress
welle: welle-1-mvp
lastenheft_refs: []
req_tec_refs: []
adr_refs: [ADR-0001]
---

# Slice 005: Gate-Consistency-Sensor (Doku ↔ Makefile)

**Status:** in-progress

**Welle:** welle-1-mvp

**Bezug:** Modul 13 (keine halluzinierten Gates), Modul 15 (Doku-Konsistenz-Agent). Review-getrieben (direkt in `in-progress`, kein `open/`-Vorlauf).

**Autor:** Dietmar Burkard. **Datum:** 2026-06-08.

---

## 1. Ziel

Die wiederkehrende Drift-Klasse „Doku behauptet einen `make`-Befehl,
der nicht existiert (oder umgekehrt)" **maschinell** schließen — die
Klasse, die `docs-check` (nur Markdown-Links) nicht fängt und die in
slice-001/002 zweimal per Review auffiel.

## 2. Definition of Done

- [ ] `tools/gate-consistency.sh`: jeder als **real** dargestellte `make <target>` in `AGENTS.md` und `harness/README.md` (außerhalb von „Geplant/Nicht behauptet"-Abschnitten) existiert als Makefile-Target.
- [ ] **Negativtest** belegt: ein behaupteter, nicht existenter Gate → rot.
- [ ] `make gate-consistency` als Dockerfile-Target-Stage (kein Mount); in `make gates` aggregiert.
- [ ] Als realer Sensor promotet (`harness/README.md` §Sensors, `AGENTS.md` §3).
- [ ] `make gates` grün; Closure-Notiz.

## 3. Plan (vor Code)

| Datei / Komponente | Änderungs-Art | Begründung |
|---|---|---|
| `tools/gate-consistency.sh` | neu | Doku↔Makefile-Abgleich (bash/grep/awk) |
| `.devcontainer/Dockerfile` | update | Stage `gate-consistency` (FROM ubuntu, COPY, RUN) |
| `Makefile` | update | `gate-consistency`-Target + in `gates` |
| `harness/README.md`, `AGENTS.md` §3 | update | Sensor-Promotion |

## 4. Trigger

- Review-Befund (slice-002/spike-001): Prosa-/Befehls-Drift, die docs-check nicht fängt.

## 5. Closure-Trigger

- DoD vollständig, `make gates` grün (inkl. neuem Gate), Closure-Notiz.

## 6. Risiken und offene Punkte

- Abschnitts-Erkennung („real" vs „geplant") ist heuristisch (awk) — Negativtest + „Heuristik, kein Parser"-Hinweis im Skript.
- Reverse-Richtung (jedes Makefile-Gate ist dokumentiert) zunächst out-of-scope (rauschanfälliger); Fokus auf die Harness-Lüge-Richtung.

## 7. Closure-Notiz

<!-- Erst nach Abschluss füllen. -->

## 8. Sub-Area-Modus-Begründung

### Sub-Area: Build & Toolchain

- **Modus:** GF
- **Konventionen-Dichte:** hoch — Gate-/Honesty-Konvention in AGENTS §3, Modul 13.
- **Phase-Reife:** Phase 4.
- **Evidenz-/Diskrepanz-Risiko:** niedrig — reiner Doku/Makefile-Textabgleich.
- **Reconciliation-Aufwand:** keiner (GF).
