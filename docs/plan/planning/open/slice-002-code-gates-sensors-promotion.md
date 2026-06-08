---
id: slice-002
titel: Code-Gates & Sensors-Promotion
status: open
welle: welle-1-mvp
lastenheft_refs: []
req_tec_refs: [REQ-TEC-005]
adr_refs: [ADR-0001]
---

# Slice 002: Code-Gates & Sensors-Promotion

**Status:** open

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

- [ ] `make lint` (clang-tidy + Suppression-Gate), `make arch-check` (Kern importiert kein `adapters/`/Qt/OCC/SQLite), `make test` (GoogleTest) existieren und sind grün — jedes Target mit ID-Kommentar `## <ADR/LH> — …`.
- [ ] „Hello-Hexagon": ein trivialer Driving-Port + Service + Driven-Port-Stub + Test beweist die Layering-Durchsetzung.
- [ ] Sensors in `harness/README.md` und Gate-Tabelle in `AGENTS.md` §3: die neuen Targets aus „Nicht behauptet" in die reale Tabelle promotet; `make gates` aggregiert nur real existierende Targets.

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

<!-- Erst nach Abschluss füllen: Lerneintrag. -->

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
