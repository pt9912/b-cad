---
id: slice-005
titel: Gate-Consistency-Sensor (Doku ↔ Makefile)
status: done
welle: welle-1-mvp
lastenheft_refs: []
req_tec_refs: []
adr_refs: [ADR-0001]
---

# Slice 005: Gate-Consistency-Sensor (Doku ↔ Makefile)

**Status:** done

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

- [x] `tools/gate-consistency.sh`: jeder als **real** dargestellte `make <target>` in `AGENTS.md` und `harness/README.md` (außerhalb von „Geplant/Nicht behauptet"-Abschnitten) existiert als Makefile-Target.
- [x] **Negativtest** belegt: injizierter `make frobnicate` in realer Tabelle → rot (exit 1).
- [x] `make gate-consistency` als Dockerfile-Target-Stage (kein Mount); in `make gates` aggregiert.
- [x] Als realer Sensor promotet (`harness/README.md` §Sensors + Stand-Absatz, `AGENTS.md` §3).
- [x] `make gates` grün (6 Gates); Closure-Notiz (§7).

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

**Abgeschlossen am:** 2026-06-08.

**Was funktioniert hat:** `make gate-consistency` ist grün und Teil von
`make gates` (jetzt 6 Gates). Positiv-Lauf bestätigt alle real
dokumentierten `make`-Befehle; Negativtest (injizierter `make frobnicate`
in der realen Sensors-Tabelle) wird korrekt rot (exit 1).

**Steering-Loop:** Dieser Sensor schließt die Drift-Klasse, die in
slice-001 (Review: kaputter `tools/README.md`-Befehl), slice-002 und
zuletzt am `harness/README.md`-Stand-Absatz + `planning/README.md`-
Lauf-Status je per **Review** auffiel — `docs-check` (nur Links) fängt sie
nicht. Aus „gehört in die Review-Checkliste" wird damit ein
**maschineller Gate** (Modul 13: was prüfbar wird, muss nicht im Review
landen).

**Bewusste Grenze (Heuristik, kein Parser):** Die „real vs geplant"-
Erkennung kippt an Text-Markern (`**Real…`, `Geplant`, `Nicht behauptet`,
`##`-Überschrift). Bei Umbau der Doku-Struktur müssen die Marker
mitgeführt werden; der Negativtest sichert die Wirksamkeit ab. Die
Reverse-Richtung (jedes Makefile-Gate ist dokumentiert) ist bewusst
out-of-scope (rauschanfälliger) — Kandidat für einen Folge-Slice, falls
undokumentierte Gates auftreten.

**Folge-Slice:** keiner nötig.

## 8. Sub-Area-Modus-Begründung

### Sub-Area: Build & Toolchain

- **Modus:** GF
- **Konventionen-Dichte:** hoch — Gate-/Honesty-Konvention in AGENTS §3, Modul 13.
- **Phase-Reife:** Phase 4.
- **Evidenz-/Diskrepanz-Risiko:** niedrig — reiner Doku/Makefile-Textabgleich.
- **Reconciliation-Aufwand:** keiner (GF).
