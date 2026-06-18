---
id: slice-022
titel: io-smoke — headless Binary-Roundtrip-Smoke aller IO-Formate (Behaviour-Sensor, KEIN gates-Member)
status: done
welle: welle-4-austausch
lastenheft_refs: [[LH-FA-IO-001](../../../../spec/lastenheft.md#lh-fa-io-001--ifc-import), [LH-FA-IO-002](../../../../spec/lastenheft.md#lh-fa-io-002), [LH-FA-IO-003](../../../../spec/lastenheft.md#lh-fa-io-003), [LH-FA-IO-004](../../../../spec/lastenheft.md#lh-fa-io-004), [LH-FA-IO-005](../../../../spec/lastenheft.md#lh-fa-io-005), [LH-FA-IO-006](../../../../spec/lastenheft.md#lh-fa-io-006)]
adr_refs: [[ADR-0001](../../adr/0001-hexagonale-architektur.md), [ADR-0010](../../adr/0010-headless-gl-xvfb.md), [ADR-0013](../../adr/0013-ifc-bibliothek.md), [ADR-0014](../../adr/0014-step-stl-export-backend.md), [ADR-0015](../../adr/0015-dxf-backend.md)]
---

# Slice 022: io-smoke — headless Binary-Roundtrip-Smoke aller IO-Formate

**Status:** in-progress. **Vor Start:** [MR-006](../../../../harness/conventions.md#mr-006--unabhängiges-plan-review-vor-implementierungs-start)-Plan-Review (Reviewer ≠ Autor) — **HIGHs blockieren den Start**.

**Welle:** welle-4-austausch (**Quergewerk** — Behaviour-Sensor über die IO-Deliverables;
Sub-Areas *Build & Toolchain* + *Test-Infrastruktur*. Kein welle-4-Feature, Sequenz unberührt).

**Bezug:** Die IO-Adapter sind port-tief integrationsgetestet (slice-019b/c, 020b, 021b über
`ExchangeService` → Port → Adapter → Datei), aber die **`main.cpp`-Glue** (CLI-Parsing +
Composition-Root-Verdrahtung der `--import-*`/`--export-*`-Pfade) ist **bewusst
coverage-ausgenommen** (Composition Root) und nur manuell geübt. Dieser Slice schließt **genau
diese Schicht** mit einem headless Binary-Smoke — über das **bestehende
[`make acc-002-beleg`](../../../../harness/README.md#sensors-feedback-gates)-Muster** (Binary in
der `build`-Stage unter `xvfb-run`, [ADR-0010](../../adr/0010-headless-gl-xvfb.md)), **ohne neues
Test-Framework**.

**Autor:** Dietmar Burkard (AI-Harness-Lauf). **Datum:** 2026-06-18.

**Schnitt-Herkunft:** Aufkommende Frage „gibt es Binary-/CLI-E2E?" → Nein (nur port-tiefe
Integration). Kleinste sinnvolle Schließung: **ein** `make`-Target, das das verdrahtete Binary
headless je Format aufruft. **Reine Build-/Doku-Änderung, kein Produktions-Code** (die
`--import-*`/`--export-*`-Flags existieren seit slice-019c/020b/021b).

**Plan-Review ([MR-006](../../../../harness/conventions.md#mr-006--unabhängiges-plan-review-vor-implementierungs-start), Reviewer ≠ Autor):**
**0 HIGH / 2 MED / 3 LOW / 1 INFO — Start nicht blockiert**
([Report](../../../reviews/2026-06-18-slice-022-plan.md)). Eingearbeitet:
**MED-1** (DoD-2: die [`conventions.md`](../../../../harness/conventions.md)-LH-Bindung-Klasse bindet
heute an „reale **Gates**" + nennt `coverage-gate-critical` als ersten Kandidaten → der Nachzug
formuliert **ohne Selbstwiderspruch**: Klasse gilt für **Sensor = Gate *oder* CI-only**, io-smoke =
erste reale Bindung eines **CI-only**-Sensors, `coverage-gate-critical` bleibt erster *Gate*-Kandidat);
**MED-2** (DoD-1: **fail-closed nicht allein über `set -e`** — `errexit` greift in `&&`-Ketten/Funktionen/
Subshells unter `xvfb-run`/`timeout` nicht zuverlässig → **expliziter `|| { echo FAIL; exit 1; }`-Guard je
Aufruf** + **Negativ-Selbsttest** im Closure, der `make io-smoke` künstlich **rot** zeigt);
**LOW-1** (roadmap `## Historische Trigger-Verschiebungen`-Vermerk **nicht optional**, Präzedenz
slice-018); **LOW-2** (Smoke-Dateien in container-internem `/tmp`, hermetisch, kein Host-Mount);
**LOW-3** (Brace-Pfade docs-check-bestätigt). INFO-1 = Proportionalität bestätigt (reale Glue-Lücke).

---

## 1. Ziel

Ein **`make io-smoke`**-Target, das das gebaute `b-cad`-Binary **headless** (xvfb) je
Austauschformat über die echte CLI-/Composition-Root-Schicht aufruft und **fail-closed**
prüft, dass jeder Lauf **exit 0** liefert und eine **nicht-leere** Datei entsteht — für **IFC**
und **DXF** zusätzlich ein **Export → Re-Import**-Durchlauf (bidirektional), für **STEP/STL**
nur Export (export-only). Damit ist die heute ungetestete `main.cpp`-Glue der IO-Pfade
**beobachtbar** belegt.

**Bewusst KEIN `make gates`-Member.** Wie [`acc-002-beleg`](../../../../harness/README.md#sensors-feedback-gates)
startet io-smoke das **GUI-gelinkte Binary unter xvfb** (`QApplication` wird in `main.cpp`
unbedingt gebaut) — schwerer + mit dem bekannten xvfb-Cleanup-Race (Timeout-Wrapper). Es ist
ein **realer Sensor in der CI-Befehlsliste** (Muster `schema-check`), nicht im hermetischen
`gates`-Aggregat.

## 2. Definition of Done

- [x] **DoD-1 — `make io-smoke` real + fail-closed.** Neues Target (+ `.PHONY`) baut die
      `build`-Stage und führt `tools/io-smoke.sh` im Container aus. Das Skript läuft unter
      **einem** `timeout … xvfb-run -a`-Wrapper (ein X-Server für alle Aufrufe; Race-Schutz wie
      [`acc-002-beleg`](../../../../harness/README.md#sensors-feedback-gates)) mit `set -euo pipefail`
      und ruft `b-cad`:
      **IFC** `--export-ifc` (→ `test -s`) + `--import-ifc`; **DXF** `--export-dxf` (→ `test -s`)
      + `--import-dxf`; **STEP** `--export-step` (→ `test -s`); **STL** `--export-stl` (→ `test -s`).
      Jeder Nicht-Null-Exit / leere Datei lässt das Target **scheitern**. **Kein Produktions-Code**
      (Flags existieren).
- [x] **DoD-2 — als realer Sensor dokumentiert, ehrlich abgegrenzt.**
      [`harness/README.md` §Sensors](../../../../harness/README.md#sensors-feedback-gates) +
      [`AGENTS.md` §3](../../../../AGENTS.md): io-smoke als **real** geführt, **explizit „kein
      `gates`-Member → CI-Befehlsliste"** (Muster `schema-check`/`acc-002-beleg`), **Bindung**
      = **LH-Bindung** auf [LH-FA-IO-001](../../../../spec/lastenheft.md#lh-fa-io-001--ifc-import)…
      [006](../../../../spec/lastenheft.md#lh-fa-io-006) (die in
      [`conventions.md`](../../../../harness/conventions.md) deklarierte, bisher **ungebundene**
      Zusatzklasse — **erste reale Bindung**; conventions-Hinweis nachgezogen).
      [`make gate-consistency`](../../../../harness/README.md#sensors-feedback-gates) bleibt grün
      (jeder dokumentierte `make`-Befehl existiert).
- [x] **DoD-3 — grün belegt, Gates unberührt, Closure.** `make io-smoke` läuft lokal **grün**
      (echte Ausgabe im Closure). `make gates` **unberührt + grün** (io-smoke ist nicht darin).
      `make docs-check` grün (neue Pfade/IDs sauber). `CHANGELOG.md`-Eintrag; Closure-Notiz §8.
      **Kein** Spec-/ADR-/Lastenheft-Inhalt geändert.

## 3. Plan (vor Code)

| Datei / Komponente | Änderungs-Art | Begründung |
|---|---|---|
| `tools/io-smoke.{sh}` | neu | Smoke-Skript: xvfb + `set -euo pipefail` + je Format Export/Import + `test -s` |
| `Makefile` | ändern | `io-smoke`-Target (`build`-Stage + `docker run … bash tools/io-smoke.sh`) + `.PHONY` |
| [`harness/README.md`](../../../../harness/README.md) | ändern | §Sensors-Zeile io-smoke (real, **kein gates-Member → CI**, **LH-Bindung** LH-FA-IO-*) |
| [`AGENTS.md`](../../../../AGENTS.md) | ändern | §3 real-Tabelle: io-smoke (nicht in gates → CI-Liste, Muster `schema-check`) |
| [`harness/conventions.md`](../../../../harness/conventions.md) | ändern | LH-Bindung-Zusatzklasse: io-smoke als **erste reale Bindung** notieren (Tippfehler-Schutz) |
| `CHANGELOG.md` | ändern (Closure) | ein Eintrag io-smoke |
| `docs/reviews/{2026-06-18-slice-022-plan}.md` | neu | [MR-006](../../../../harness/conventions.md#mr-006--unabhängiges-plan-review-vor-implementierungs-start)-Report |

**Kein** roadmap-Eintrag nötig (Sensor-Quergewerk, kein welle-Feature — Muster `schema-check`;
optional ein `## Historische Trigger-Verschiebungen`-Vermerk wie slice-018).

## 4. Trigger

- Keiner — **startbar** nach [MR-006](../../../../harness/conventions.md#mr-006--unabhängiges-plan-review-vor-implementierungs-start)-Review.
  Die IO-Flags + das xvfb-Binary-Muster ([`acc-002-beleg`](../../../../harness/README.md#sensors-feedback-gates),
  [ADR-0010](../../adr/0010-headless-gl-xvfb.md)) existieren.

## 5. Closure-Trigger

- DoD vollständig, `make io-smoke` + `make gates` grün, Closure-Notiz mit echter Ausgabe.

## 6. Risiken und offene Punkte

- **xvfb-Cleanup-Race ([ADR-0010](../../adr/0010-headless-gl-xvfb.md), [`acc-002-beleg`](../../../../harness/README.md#sensors-feedback-gates)-Lehre):**
  `xvfb-run` hängt als Container-PID-1 nach App-Ende → **`timeout`-Wrapper** als
  Prozessgruppen-Leader; **ein** xvfb-Lauf für alle Aufrufe (nicht je Format ein eigener).
- **Headless-Exit je Flag:** `main.cpp` muss bei `--export-*`/`--import-*` **vor** der GUI
  zurückkehren (exit 0/1) — verifiziert (`runExportIfRequested` / `--import-*`-Block returnen
  vor `QMainWindow`). `QApplication` wird trotzdem gebaut → xvfb nötig (daher kein In-Process-
  gtest, sondern Binary-Smoke).
- **STEP/STL export-only:** **kein** Re-Import im Smoke (Import würde per Vertrag mit [`E-IO-003`](../../../../spec/spezifikation.md#4-fehler-codes-und-logging-felder)
  scheitern). Nur Export + `test -s`. IFC/DXF bidirektional.
- **Ehrlichkeit (Harness-Lüge vermeiden):** io-smoke wird **nicht** als `gates`-Member behauptet;
  die §Sensors-Bindung-Spalte trägt **LH-Bindung** + die Klarstellung „CI-Befehlsliste, nicht in
  `gates`". `gate-consistency` deckt nur **Existenz**, nicht Gehalt — der Smoke selbst ist der Gehalt.
- **Smoke ≠ Korrektheit:** prüft **Lauf + nicht-leere Datei + Re-Import-exit**, nicht die
  Roundtrip-Treue (die liegt in den Unit-/Integrationstests 019–021). Bewusst flach (Behaviour-
  Smoke der Glue), **kein** Doppel der port-tiefen Orakel.
- **Determinismus:** Lauf in der gepinnten `build`-Stage (Modul 14), wie `acc-002-beleg`.

## 7. Sub-Area-Modus-Begründung

### Sub-Area: Build & Toolchain (`Makefile`, `tools/`)

- **Modus:** GF; **Konventionen-Dichte:** hoch (Dockerfile-Stage-/Make-Target-Konvention,
  Sensor-Honesty AGENTS §3, [MR-003](../../../../harness/conventions.md#mr-003--docs-check-als-vendored-doku-sensor)-/Reproduzierbarkeits-Disziplin).
  **Phase-Reife:** Phase 4. **Risiko:** niedrig (additives Target, `acc-002-beleg`-Muster).

### Sub-Area: Konventionen & Harness-Doku (`harness/`)

- **Modus:** GF; **Dichte:** hoch (Sensors-Tabelle real-only, LH-Bindung-Zusatzklasse).
  **Phase-Reife:** Phase 4. **Risiko:** niedrig.

## 8. Closure-Notiz

**Closure-Kriterien (beobachtbar, 2026-06-18):**

- **`make io-smoke` real + grün (EXIT=0):** baut die `build`-Stage und fährt `tools/io-smoke.sh`
  unter **einem** `timeout … xvfb-run -a`-Lauf; IFC (Export 6569 B + Re-Import 2 Geschosse/
  9 Wände), DXF (Export 818 B + Re-Import 2/9), STEP (Export 152367 B), STL (Export 5484 B) —
  alle exit 0, Dateien nicht leer.
- **Fail-closed belegt (MED-2):** expliziter `|| fail`-Guard je Aufruf (nicht allein `set -e`);
  **Negativ-Selbsttest** `BCAD_SMOKE_SELFTEST=1 make io-smoke` → **EXIT≠0**
  („SELFTEST belegt fail-closed: leere Datei → rot").
- **Ehrlich abgegrenzt:** als realer Sensor in [`harness/README.md` §Sensors](../../../../harness/README.md#sensors-feedback-gates)
  + [`AGENTS.md` §3](../../../../AGENTS.md) geführt, **explizit kein `gates`-Member**
  (CI-Befehlsliste, Muster `schema-check`/`acc-002-beleg`); `make gates` **unberührt + grün**;
  `gate-consistency` grün (Target existiert). **LH-Bindung** [LH-FA-IO-001](../../../../spec/lastenheft.md#lh-fa-io-001--ifc-import)…
  [006](../../../../spec/lastenheft.md#lh-fa-io-006) — **erste reale Bindung** der Zusatzklasse
  (MED-1: Klasse auf „Sensor = Gate *oder* CI-only" geweitet, `coverage-gate-critical` bleibt
  erster *Gate*-Kandidat).
- **Kein Produktions-Code** (Flags existieren); Smoke-Dateien in container-internem `/tmp`
  (hermetisch, LOW-2). roadmap `## Historische Trigger-Verschiebungen`-Vermerk (LOW-1); CHANGELOG.

**Lerneintrag:**

- **Smoke ≠ Korrektheit:** prüft Lauf + nicht-leere Datei + Re-Import-exit der `main.cpp`-Glue —
  **nicht** Roundtrip-Treue (die liegt port-tief in slice-019–021). Bewusst flacher
  Behaviour-Sensor, **kein** Orakel-Doppel.
- **Kein neues Test-Harness nötig:** das `acc-002-beleg`-Muster (Binary in der `build`-Stage
  unter xvfb, [ADR-0010](../../adr/0010-headless-gl-xvfb.md)) trägt den Binary-Smoke ohne
  Framework; `QApplication` wird in `main.cpp` unbedingt gebaut → xvfb nötig (daher
  Binary-Smoke statt In-Process-gtest).
- **Normativität (Projektinhaber-Steuerung):** Makefile/`tools/`-Skript tragen **nur** die
  normative `LH-FA-IO-*`-Bindung, **keine** volatile Slice-ID-Provenance — Slice-Refs leben in
  Planung/CHANGELOG/Commit-Historie. Bestands-Drift (`Makefile`-Kommentare mit Slice-Refs) als
  Nachzug benannt.

**Restrisiko / Nachfolge:** Bestands-Slice-Refs in `Makefile`-Kommentaren bei Gelegenheit
nachziehen (Entropy Management); CI muss `make io-smoke` als Schritt führen. welle-4-Sequenz
unberührt (Dächer/Treppen-STEP-B-Rep · PDF/PNG · Welle-4-Verifikation → M4).
