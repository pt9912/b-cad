---
id: slice-002
titel: Code-Gates & Sensors-Promotion
status: done
welle: welle-1-mvp
lastenheft_refs: []
req_tec_refs: [REQ-TEC-005]
adr_refs: [ADR-0001]
---

# Slice 002: Code-Gates & Sensors-Promotion

**Status:** done

**Welle:** welle-1-mvp

**Bezug:** ADR-0001 (arch-check setzt Layering durch), REQ-TEC-005 (GoogleTest)

**Autor:** Dietmar Burkard. **Datum:** 2026-06-08.

---

## 1. Ziel

Die Code-Gates real machen und aus dem „Nicht behauptet"-Block in die
echte Sensors-Tabelle promoten — `lint`, `arch-check`, `test`,
`coverage-gate` existieren im Makefile und sind grün, belegt durch einen
minimalen „Hello-Hexagon"-Pfad (ein Driving-Port ruft über einen
Driven-Port-Stub).

## 2. Definition of Done

- [x] `make arch-check`, `make lint` (clang-tidy + Suppression-Gate), `make test` existieren und sind grün — als **Dockerfile-Target-Stages ohne Bind-Mounts**, mit ID-Kommentar.
- [x] `make coverage-gate` (bootstrap-aware, gcov, Schwelle 70 %, Composition Root ausgenommen) grün — über die ursprüngliche DoD hinaus ergänzt (cmake-xray-Alignment, siehe §7).
- [x] „Hello-Hexagon": `GreetPort` (driving) → `GreetingService` → `GreetingSourcePort` (driven) + Test mit Double beweist die Layering-Durchsetzung.
- [x] Sensors in `harness/README.md` und Gate-Tabelle in `AGENTS.md` §3 promotet; `make gates` aggregiert nur reale Targets.

## 3. Plan (vor Code)

| Datei / Komponente | Änderungs-Art | Begründung |
|---|---|---|
| `Makefile` | update | `lint`/`arch-check`/`test`/`coverage-gate` mit ID-Kommentar |
| `.clang-tidy` | neu | Suppression-Gate-Basis (AGENTS.md §2.4) |
| `tests/` + minimaler Hexagon-Pfad | neu | Layering-Beweis + arch-check-Substrat |
| `harness/README.md`, `AGENTS.md` §3 | update | Sensors-Promotion (Promotion-Trigger) |

## 4. Trigger

- slice-001 done (Build-Skelett + DevContainer stehen).

## 5. Closure-Trigger

- DoD vollständig, `make gates` grün im Container, Closure-Notiz geschrieben.

## 6. Risiken und offene Punkte

- arch-check-Werkzeug für C++ (Include-Graph-Prüfung) noch zu wählen; bis dahin trägt die CMake-Target-Trennung (ADR-0001) die Durchsetzung.
- coverage-gate startet bootstrap-aware mit niedriger, dokumentierter Schwelle und Hochschalt-Trigger (Modul 13).

## 7. Closure-Notiz

**Abgeschlossen am:** 2026-06-08.

**Was funktioniert hat:** Fünf reale Gates grün — `docs-check`,
`arch-check` (inkl. Negativtest: Kern-Datei mit `<QtGlobal>` → rot),
`lint` (clang-tidy 0 Befunde in `src/` + Suppression-Gate), `test`
(5/5), `coverage-gate` (100 % Lines, Schwelle 70 %). Hello-Hexagon-Port-
Roundtrip ist mit Double getestet (kein Adapter).

**Steering-Loop-Eintrag 1 — Gate-Selbsttest:** Der erste `arch-check`
gab still Exit 1 *ohne* Befund — ein `set -euo pipefail`-Bug (eine
grep-Pipeline lieferte 1, wenn eine Datei keinen `adapters/`-Include
hat). Gefangen durch einen **Negativtest** (bewusst eine Verletzung
eingebaut → Gate muss rot werden). Lehre (Fortsetzung des slice-001-
Reviews): ein Gate muss *bewiesen* fangen, nicht nur passieren — jeder
neue Gate bekommt einen Negativtest.

**Steering-Loop-Eintrag 2 — Reproduzierbarkeit der Gates:** Gates auf
**Dockerfile-Target-Stages** umgestellt (Quelle per `COPY`, Gate als
`RUN`) statt `docker run`-Bind-Mounts (auf Nutzer-Wunsch „Mounts
minimieren", Vorbild cmake-xray, Modul 14). Das deckte sofort einen
Folgefehler auf: ein stale `build-cov/` wurde per `COPY .` eingebacken
→ `.dockerignore` auf `build*/` gehärtet.

**Scope-Erweiterung (dokumentiert):** `coverage-gate` war nicht in der
ursprünglichen DoD; ergänzt im Zuge des cmake-xray-Gate-Vergleichs
(„das hat auch schon gates"), passt zum Slice-Thema „Code-Gates".

**Offene Stelle → Folge-Slice (keine stille Lücke):** Die Lib-/
Toolchain-**Versionen** sind über `apt-get install <name>` nicht
gepinnt (driften mit dem Ubuntu-Archiv), und die **Base-Version**
(24.04 vs. 26.04) ist offen. Bewusst **nicht** in slice-002 gelöst, um
die grünen Gates nicht zu destabilisieren. Übergeben an
[`spike-001`](../open/spike-001-toolchain-reproduzierbarkeit.md) →
mündet in ADR-0004 (Container-/Dependency-Pinning).

**§8-Prüfpunkt Test-Infrastruktur:** Die Test-Konvention (GoogleTest,
`tests/hexagon` + `tests/adapters`, ID im Testnamen, Port-Doubles für
den Kern) ist mit diesem Slice **verkörpert**; zusammen mit der
Modus-Tabelle in `harness/conventions.md` ist die Sub-Area damit GF.

## 8. Sub-Area-Modus-Begründung

### Sub-Area: Build & Toolchain

- **Modus:** GF
- **Konventionen-Dichte:** hoch — Gate-Konventionen in AGENTS.md §3, MR-003, ADR-0001.
- **Phase-Reife:** Phase 4 (Vertrag steht; Targets werden real).
- **Evidenz-/Diskrepanz-Risiko:** niedrig.
- **Reconciliation-Aufwand:** keiner (GF).

### Sub-Area: Test-Infrastruktur

- **Modus:** GF
- **Konventionen-Dichte:** mittel — GoogleTest-/Determinismus-Konvention wird **mit diesem Slice** erstmals in `harness/conventions.md`/`AGENTS.md` verankert. *Prüfpunkt:* solange das Test-Layout dort nicht steht, ist diese Sub-Area faktisch noch nicht GF — daher ist das Verankern Teil der DoD.
- **Phase-Reife:** Phase 2→4 (Outline → kohärent mit diesem Slice).
- **Evidenz-/Diskrepanz-Risiko:** mittel — Tests ohne `LH-`/`ADR`-Bezug wären eine Diskrepanz; Gegenmittel: ID im Testnamen.
- **Reconciliation-Aufwand:** gering; mit dem Verankern der Konvention erreicht die Sub-Area stabil GF.
