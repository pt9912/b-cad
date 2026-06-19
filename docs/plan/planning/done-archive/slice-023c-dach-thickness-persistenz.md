---
id: slice-023c
titel: Dach-Thickness-Persistenz — roofs.thickness_mm Round-Trip (Schema/d-migrate)
status: done
welle: welle-4-austausch
lastenheft_refs: [[LH-FA-ROF-006](../../../../spec/lastenheft.md#lh-fa-rof-006), [LH-FA-BLD-002](../../../../spec/lastenheft.md#lh-fa-bld-002--projekt-speichern), [LH-FA-BLD-003](../../../../spec/lastenheft.md#lh-fa-bld-003--projekt-laden)]
adr_refs: [[ADR-0001](../../adr/0001-hexagonale-architektur.md), [ADR-0003](../../adr/0003-persistenz-sqlite.md), [ADR-0006](../../adr/0006-relationales-schema-design.md)]
---

# Slice 023c: Dach-Thickness-Persistenz — roofs.thickness_mm Round-Trip

**Status:** done (2026-06-19). [MR-006](../../../../harness/conventions.md#mr-006--unabhängiges-plan-review-vor-implementierungs-start)-Plan-Review gelaufen (2 HIGH = falscher Präzedenz/ungegründete Versions-Entscheidung, behoben; 3 MED/2 LOW/3 INFO eingearbeitet). **Nicht geometrieschwer** (reine Persistenz, kein neuer Solid/Mesh) → [MR-009](../../../../harness/conventions.md#mr-009--geometrielastiges-code-review-vor-welle-closure) **n/a**. DoD vollständig, `make schema-check` + `make gates` grün (205/205, Coverage 90,3 %). Closure-Notiz §8.

**Welle:** welle-4-austausch (Dach-Volumen-Initiative, Persistenz-Hälfte; Muster slice-014c
[Dach-Persistenz], 015c/016c). Schließt die in [slice-023b](../done-archive/slice-023b-dach-volumen-impl.md)
**benannte Lücke**: `Roof.thickness_mm` existiert in der Domäne, ist aber bis hier **nicht persistiert**
→ geladene Dächer erhielten die Default-Dicke (keine Round-Trip-Treue der Dicke).

**Bezug:** [LH-FA-ROF-006](../../../../spec/lastenheft.md#lh-fa-rof-006) (Dachdicke / Volumenkörper — die
Dicke muss Speichern/Laden überleben), [LH-FA-BLD-002](../../../../spec/lastenheft.md#lh-fa-bld-002--projekt-speichern)/[003](../../../../spec/lastenheft.md#lh-fa-bld-003--projekt-laden)
(speichern/laden, atomar). [ADR-0006](../../adr/0006-relationales-schema-design.md) (`roofs`-Schema bekommt
eine **neue Spalte** → `schema.sql` ist **d-migrate-generiert** aus `data-model.yaml`, `make schema-check`
prüft die Drift), [ADR-0003](../../adr/0003-persistenz-sqlite.md) (SQLite-Adapter, atomare Transaktion),
[ADR-0001](../../adr/0001-hexagonale-architektur.md) (Regel D: `sqlite3*` nur im Persistenz-Adapter).

**Autor:** Dietmar Burkard (AI-Harness-Lauf). **Datum:** 2026-06-19.

**Schnitt-Herkunft:** Persistenz-Hälfte der Dach-Volumen-Initiative (Muster 014c). Geometrie + Dicke-Feld
+ EVL waren **023b**; STEP-B-Rep der (jetzt soliden) Dächer ist **024**. Diese Sitzung ist rein
mechanische Adapter-/Schema-Erweiterung — **anders als 014c** (dort lag `roofs` vollständig vor) ändert
sich hier das Schema (eine Spalte), also wird `schema.sql` via d-migrate **neu erzeugt** und
`make schema-check` ist das diskriminierende Gate. **Dies ist die erste Schema-Änderung im Repo
überhaupt** (`data-model.yaml` zuletzt slice-007 `849bf66`, `schema.sql` zuletzt slice-008a `11d8de3`,
seither unverändert) — die Persistenz-Slices 013c/014c/015c/016c waren **alle „Kein Schema-Wechsel"**
und sind nur Präzedenz für die **mechanische Adapter-Verdrahtung**, **nicht** für die Versions-/
Migrations-Entscheidung (§6).

**Plan-Review ([MR-006](../../../../harness/conventions.md#mr-006--unabhängiges-plan-review-vor-implementierungs-start), Reviewer ≠ Autor):**
**2 HIGH / 3 MED / 2 LOW / 3 INFO** ([Report](../../../reviews/2026-06-19-slice-023c-plan.md)). Beide
**HIGH** betrafen **dieselbe** Schwäche der Erstfassung: die Versions-/Migrations-Entscheidung stützte sich
auf einen **falschen Präzedenz** (HIGH-1: 013c–016c waren „Kein Schema-Wechsel", nicht additive Spalten)
und war nicht in den Quellen verankert (HIGH-2). **Behoben:** §6 + DoD-1/DoD-2 neu gegründet — *erste*
Schema-Änderung benannt, [`releasing.md` §27](../../../user/releasing.md) Abwärtskompat-Zweig (SQL-`DEFAULT 200`
liest Alt-Zeile als 200) + **konditionale** §6-Lesart (Migration nur bei Versions-**Erhöhung**),
`data-model.yaml version` bleibt `1.0.0` mit expliziter Begründung. **MED-1** (`200` an §3/
`kDefaultRoofThicknessMm` pinnen, Default-Sonde assertiert gegen die Konstante), **MED-2** (Domänen-Default-
Asymmetrie Roof↔Slab als Primärgrund), **MED-3** (explizite Spaltennamen → Index lokal je Statement,
YAML-Position kosmetisch), **LOW-1** (DoD-6 zwei getrennte Gate-Häkchen), **LOW-2** (`bind_double` ist
bit-exakt — nicht-glatter Wert diskriminiert gegen geteilten `0.0`-Default + Spalten-Swap, **nicht** gegen
Präzision), **INFO-3** (Regenerations-Befehl in DoD-2). **HIGH behoben → Start frei.**

---

## 1. Ziel

`Roof.thickness_mm` überlebt Speichern/Laden feldgleich: `data-model.yaml` bekommt die Spalte
`roofs.thickness_mm`; `schema.sql` wird daraus per d-migrate neu erzeugt; der `SqliteProjectRepository`
schreibt die Dicke in `insertRoofs` und liest sie in `loadRoofs` in derselben atomaren Transaktion wie
die übrigen Dach-Parameter zurück. Damit ist die in 023b benannte Persistenz-Lücke geschlossen und das
Dach-Volumen (`bx·ty·d`) ist round-trip-stabil.

## 2. Definition of Done

- [x] **DoD-1 — `data-model.yaml`: Spalte `roofs.thickness_mm`.** Neue Spalte in `roofs.columns`,
      **nach `height_mm`** eingefügt (vor dem JSON/FK-Schwanz `footprint_json`/`material_id` — gruppiert
      die parametrischen `double`s; die YAML-Position ist wegen expliziter Spaltennamen ohnehin kosmetisch,
      MED-3): `thickness_mm: { type: decimal, precision: 12, scale: 3, default: 200 }` — **`default`, nicht
      `required`**: **Primärgrund (MED-2)** — `Roof` trägt einen **Domänen-Default** `kDefaultRoofThicknessMm`,
      `Slab` **nicht** (`slabs.thickness_mm` ist `required` ohne Default); zudem führen die übrigen
      Dach-Parameter (`pitch_deg` default 30, `overhang_mm` default 500) SQL-Defaults. `200` **==
      `DEFAULT_ROOF_THICKNESS_MM` (§3) == `kDefaultRoofThicknessMm`** — der Wert ist an §3 **gepinnt**
      (kein Drift-Gate kreuzprüft YAML↔Konstante, MED-1 → Default-Sonde assertiert gegen die Konstante,
      DoD-5; Closure benennt die Pinnung). Vorteil: eine Dach-Zeile ohne explizite Dicke (Alt-Datei vor
      023c) lädt mit der Default-Dicke — genau das in 023b benannte Degradations-Verhalten
      ([`releasing.md` §27](../../../user/releasing.md) Abwärtskompat-Zweig, §6). Tabellen-`description` um
      „Dicke (`thickness_mm`, Volumenkörper)" ergänzt.
- [x] **DoD-2 — `schema.sql` via d-migrate neu erzeugt (kein Hand-Edit).** `schema.sql` ist ein
      **d-migrate-Artefakt** (Header „Generated by d-migrate"). Nach DoD-1 wird es mit dem **gepinnten**
      d-migrate-Image (`Makefile`-`DMIGRATE`-Digest) neu generiert (Spiegel des `schema-check`-Rezepts,
      stdout→Datei): `docker run --rm -v $(CURDIR)/spec:/work:ro $(DMIGRATE) schema generate
      --source=/work/data-model.yaml --target=sqlite --deterministic --report=/dev/null >
      src/adapters/persistence/schema.sql`. Ergebnis: die `roofs`-CREATE trägt `"thickness_mm" REAL
      DEFAULT 200` an d-migrate-kanonischer Position (+ ein weiterer `W200`-Decimal-Kommentar, auto,
      INFO-2). **`make schema-check` grün** (committetes `schema.sql` == d-migrate(`data-model.yaml`),
      fail-closed Drift). **Kein Hand-Edit** (AGENTS §2.9: das gepinnte Container-Image ist erlaubt, kein
      verbotener Host-`cmake/clang/apt/...`-Call; INFO-3).
- [x] **DoD-3 — `insertRoofs` schreibt `thickness_mm`.** `INSERT INTO roofs (…,thickness_mm,…)` +
      `sqlite3_bind_double(stmt, …, roof.thickness_mm)` (Muster `slabs.thickness_mm` / `pitch_deg`).
      Innerhalb der bestehenden `BEGIN`/`COMMIT`-Transaktion, Spalten-/Parameter-Reihenfolge konsistent.
- [x] **DoD-4 — `loadRoofs` liest `thickness_mm`.** `SELECT …,thickness_mm,… FROM roofs` +
      `roof.thickness_mm = sqlite3_column_double(stmt, idx)`. Feldgleich; Spalten-Index-Verschiebung der
      nachfolgenden Spalten (`footprint_json`, `material_id`) sauber nachgezogen.
- [x] **DoD-5 — Round-Trip-AK-Test (`LH_FA_ROF_006` im Namen).** Der bestehende
      `RoundTripErhaeltDaecher` (Sattel + Walm) wird um **`thickness_mm` mit nicht-glattem, nicht-default
      `double`** erweitert (z. B. Sattel `223.456789012345`, Walm `321.0`) + `EXPECT_DOUBLE_EQ`-Vergleich.
      **Diskriminierend (LOW-2):** `bind_double`/`column_double` sind **bit-exakt** unabhängig von der
      Glattheit (anders als der `%.17g`-Textpfad des `footprint_json`) — der nicht-glatte, **vom Default
      verschiedene** Wert diskriminiert gegen (a) ein Test, der nur auf dem geteilten `0.0`-Default beider
      Seiten „besteht" (heute setzt der Test `thickness_mm` **gar nicht** → `0.0==0.0`), und (b) eine
      Spalten-/Index-Vertauschung in SELECT/INSERT. **Default-Pfad-Sonde (MED-1):** ein Mini-Test belegt
      über den vorhandenen `corruptColumn`-Roh-SQL-Helfer, dass eine **per SQL-`DEFAULT`** entstandene
      `roofs`-Zeile (INSERT **ohne** `thickness_mm`, simuliert eine Alt-Datei vor 023c) beim Laden
      **`kDefaultRoofThicknessMm`** trägt (`EXPECT_DOUBLE_EQ` gegen die **Konstante**, nicht bare `200.0`
      — pinnt YAML-Default ↔ Domänen-Konstante im Test). Bestehende ROF-/BLD-/SLB-Round-Trip-Tests bleiben
      grün (Regression).
- [x] **DoD-6 — Gates grün + Schema-Drift + Closure.** **Zwei getrennte Gate-Belege (LOW-1, je eigener
      Output bei Closure):**
      - [x] `make schema-check` grün — die diskriminierende Prüfung dieses Slice (`schema.sql` ==
            d-migrate(`data-model.yaml`); **nicht** in `make gates`, CI-only).
      - [x] `make gates` grün — docs-check · gate-consistency · arch-check (Regel D unberührt) · lint ·
            test · coverage.

      `CHANGELOG.md` ([MR-004](../../../../harness/conventions.md#mr-004--top-level-changelogmd-keep-a-changelog)) + `roadmap.md`-Fortschritt; Closure-Notiz mit Lerneintrag; Plan nach
      `done-archive/` (reiner `git mv`, [MR-011](../../../../harness/conventions.md#mr-011--referenz-integritäts-gate-matrix-ids-spans-hostpaths)). **Benannte Lücke geschlossen** (023b): Dicke ist ab
      hier round-trip-treu.

## 3. Plan (vor Code)

| Datei / Komponente | Änderungs-Art | Begründung |
|---|---|---|
| `spec/data-model.yaml` | ändern | Spalte `roofs.thickness_mm` (decimal 12,3, default 200) |
| `src/adapters/persistence/schema.sql` | ändern (d-migrate-generiert) | `roofs`-CREATE bekommt `"thickness_mm" REAL DEFAULT 200`; **nicht** hand-ediert, sondern via gepinntem d-migrate neu erzeugt |
| `src/adapters/persistence/sqlite_project_repository.cpp` | ändern | `insertRoofs` (bind) + `loadRoofs` (read) um `thickness_mm`; Spalten-Index-Nachzug |
| `tests/adapters/test_sqlite_project_repository.cpp` | ändern | `RoundTripErhaeltDaecher` um `thickness_mm` (nicht-glatt) erweitern + Default-Sonde |
| `CHANGELOG.md` · `roadmap.md` | ändern (Closure) | 023c done; Dach-Volumen-Initiative-Fortschritt |
| `docs/reviews/2026-06-19-slice-023c-plan.md` | neu | [MR-006](../../../../harness/conventions.md#mr-006--unabhängiges-plan-review-vor-implementierungs-start)-Report |

**Kein** neuer ADR ([ADR-0006](../../adr/0006-relationales-schema-design.md) trägt das `roofs`-Schema, additive Spalte); **kein OCC/Qt** →
`arch-check` Regel C/E unberührt, Regel D (SQLite nur im Adapter) bleibt erfüllt. **Kein** Geometrie-Code
→ [MR-009](../../../../harness/conventions.md#mr-009--geometrielastiges-code-review-vor-welle-closure) n/a.

## 4. Trigger

- slice-023b done ✓ (`Roof.thickness_mm` + `kDefaultRoofThicknessMm` existieren in der Domäne).
- [MR-006](../../../../harness/conventions.md#mr-006--unabhängiges-plan-review-vor-implementierungs-start)-Plan-Review vor Start (HIGHs blockieren).

## 5. Closure-Trigger

- DoD vollständig, `make schema-check` + `make gates` grün, Closure-Notiz → die in 023b benannte
  Persistenz-Lücke ist geschlossen; **024** (Dächer+Treppen STEP-B-Rep) bleibt der nächste Dach-Strang-Slice.

## 6. Risiken und offene Punkte

- **Schema-Drift (PRIMÄR):** `schema.sql` **muss** d-migrate-Ausgabe von `data-model.yaml` sein, sonst
  rot `make schema-check`. Mitigation: **nicht** hand-edieren — mit dem gepinnten `DMIGRATE`-Image neu
  generieren, dann `make schema-check` als fail-closed Wächter. Spalten-**Position** in der CREATE folgt
  der `columns`-Reihenfolge in `data-model.yaml` (d-migrate-kanonisch); der `W200`-DEZIMAL→REAL-Kommentar
  wird je decimal-Spalte erzeugt — die `insertRoofs`/`loadRoofs`-Spaltenliste muss zur generierten
  Reihenfolge passen (explizite Spalten-Namen im SQL, nicht `SELECT *`/positional-blind).
- **`default` statt `required` (MED-2):** bewusst gewählt (DoD-1). **Primärgrund:** `Roof` trägt einen
  **Domänen-Default** (`kDefaultRoofThicknessMm`), `Slab` **nicht** — daher ist `slabs.thickness_mm`
  `required` (kein sinnvoller Default), `roofs.thickness_mm` aber default-tragend (wie `pitch_deg`/
  `overhang_mm`). Caveat: der SQL-`DEFAULT` feuert nur für **nicht-Writer-Zeilen** (`insertRoofs` bindet
  immer explizit) — der Alt-Datei-Degradations-Nutzen ist real, aber nur über den Default-Pfad (DoD-5-Sonde),
  nicht über den normalen Save.
- **Migrations-/Versions-Frage (HIGH-1/HIGH-2 — neu gegründet):** Dies ist die **erste Schema-Änderung im
  Repo** (kein In-Repo-Präzedenz — 013c/014c/015c/016c waren **alle „Kein Schema-Wechsel"**, also kein Beleg
  für „additive Spalte ohne Versions-Bump"). Die Entscheidung „**kein** Versions-Bump, **keine** Migration"
  ist dennoch regelwerk-konform, gegründet auf zwei Quellen:
  - **[`releasing.md` §27](../../../user/releasing.md):** „Projektdatei-Format **abwärtskompatibel oder**
    Migration vorhanden". Eine Spalte mit SQL-`DEFAULT 200`, die eine Alt-Zeile als 200 liest, **ist**
    abwärtskompatibel → der **oder**-Zweig ist erfüllt, eine Migration ist nicht erzwungen.
  - **[`spezifikation.md` §6 „Migrationsregel"](../../../../spec/spezifikation.md):** „Schema-Version steigt
    monoton; **jede Erhöhung** braucht eine getestete Aufwärts-Migration." Die Regel ist **konditional auf
    eine Versions-Erhöhung**; ohne Erhöhung greift die Migrations-Pflicht nicht.

  **`version`-Disposition (explizit):** `data-model.yaml version` bleibt **`1.0.0`** (die d-migrate-Neutral-
  Schema-Version). Begründung: (a) keine Erhöhung → §6-Migrations-Pflicht nicht ausgelöst; (b) additiv +
  abwärtskompatibel → releasing.md §27 erfüllt; (c) kein verdrahteter Save-Use-Case in `main` und kein
  produktiver Bestand zu migrieren; (d) der **reale** Drift-Sensor ist `make schema-check` (schema.sql ==
  d-migrate(data-model.yaml)), **nicht** das `version`-Label — Drift bleibt fail-closed gefangen. Ein
  bewusster Bump (→ 1.1.0) würde §6 die Migrations-Pflicht **auslösen**, die mangels Bestand nur als Carveout
  erfüllbar wäre — die No-Bump-Wahl vermeidet diese künstliche Schuld legitim.
- **Spalten-Index-Verschiebung (MED-3 — präzisiert):** SELECT/INSERT benennen Spalten **explizit** (kein
  `SELECT *`/positional-blind) → die `bind`/`column`-Index-Zuordnung ist **lokal je Statement** und
  **unabhängig** von der CREATE-/YAML-Reihenfolge (die YAML-Position ist kosmetisch). Der „Index-Shift"
  betrifft nur die `material_id`/`footprint_json`-Indizes **innerhalb** der neuen expliziten Liste; der
  Round-Trip-AK (alle Felder feldgleich) fängt eine Vertauschung.
- **`%.17g` vs. `bind_double`:** `thickness_mm` ist eine **Spalte** (`bind_double`, exakt), **nicht**
  Teil von `footprint_json` (Text, `%.17g`) → keine Text-Präzisions-Sorge wie bei origin/width.
- **Kein Datenverlust-Risiko (§2.2):** weiterhin kein verdrahteter Save-Use-Case in `main`; die atomare
  Transaktion (Temp+Rename) ist unberührt.

## 7. Sub-Area-Modus-Begründung

### Sub-Area: Persistenz-Adapter (`src/adapters/persistence/`)

- **Modus:** GF; **Konventionen-Dichte:** hoch (SQLite nur im Adapter/Regel D, atomar, [ADR-0006](../../adr/0006-relationales-schema-design.md)-Schema
  d-migrate-generiert, E-IO-Codes). **Phase-Reife:** Phase 4. **Evidenz-/Diskrepanz-Risiko:** niedrig
  (mechanische additive Spalte; `schema-check` ist der fail-closed Drift-Wächter). **Reconciliation:**
  schließt die 023b-benannte Persistenz-Lücke.

### Sub-Area: Spec-Schreibung (`spec/`)

- **Modus:** GF; **Dichte:** hoch (`data-model.yaml` als Schema-Quelle der Wahrheit, d-migrate-validiert).
  **Phase-Reife:** Phase 4. **Risiko:** niedrig.

### Sub-Area: Test-Infrastruktur (`tests/`)

- **Modus:** GF; **Dichte:** hoch (Round-Trip-AK mit `LH-`-ID, nicht-glatter double, Feld-Vergleich).
  **Phase-Reife:** Phase 4. **Risiko:** niedrig.

## 8. Closure-Notiz

**Closure-Kriterien (beobachtbar, 2026-06-19):**

- **Schema (DoD-1/2):** `spec/data-model.yaml` `roofs.columns` trägt `thickness_mm: { type: decimal,
  precision: 12, scale: 3, default: 200 }` (nach `height_mm`); `src/adapters/persistence/schema.sql` per
  **gepinntem d-migrate** (`0.9.7@sha256:69afc21…`) neu erzeugt → `"thickness_mm" REAL DEFAULT 200` +
  ein auto-`W200`-Kommentar. **`make schema-check` grün** (`schema.sql == d-migrate(data-model.yaml)`).
  **`data-model.yaml version` bleibt `1.0.0`** (keine Erhöhung → §6-Migrations-Pflicht nicht ausgelöst;
  additiv + abwärtskompatibel via SQL-Default → [`releasing.md` §27](../../../user/releasing.md) oder-Zweig).
- **Repository (DoD-3/4):** `insertRoofs` bindet `thickness_mm` (Param 6, nach `overhang_mm`; explizite
  Spaltenliste), `loadRoofs` liest es (Index 5; `footprint_json`/`material_id` auf 6/7 nachgezogen) — in
  derselben atomaren Transaktion. `height_mm` bleibt weiterhin `NULL` (abgeleitet).
- **Tests (DoD-5):** `RoundTripErhaeltDaecher` um `thickness_mm` erweitert (Sattel `223.456789012345`
  nicht-glatt, Walm `321.0` — beide ≠ Default; `EXPECT_DOUBLE_EQ`). Neuer Test
  `SqliteProjectRepository_LH_FA_ROF_006.FehlendeDickeLaedtAlsDefault`: eine ohne `thickness_mm`
  eingefügte Zeile (via `corruptColumn`-Roh-SQL, simuliert eine Alt-Datei) lädt als
  **`kDefaultRoofThicknessMm`** (gegen die **Konstante**, nicht bare `200.0` → pinnt YAML↔Domänen-Default).
- **Gates (DoD-6):** `make schema-check` grün **und** `make gates` grün (**205/205** Tests [+1 = die
  Default-Sonde], Coverage 90,3 %, docs-check 0, arch-check Regel D unberührt, lint 0). Beide separat belegt.
- **Review:** [MR-006](../../../../harness/conventions.md#mr-006--unabhängiges-plan-review-vor-implementierungs-start) **2 HIGH** (beide derselbe Fehler der Erstfassung: falscher
  Präzedenz „additive Spalte ohne Bump" — real ist dies die **erste** Schema-Änderung im Repo; Entscheidung
  war nicht in releasing.md/§6 verankert) → **behoben** vor Start; 3 MED/2 LOW/3 INFO eingearbeitet.
  [MR-009](../../../../harness/conventions.md#mr-009--geometrielastiges-code-review-vor-welle-closure) **n/a** (reine Persistenz).

**Lerneintrag:**

- **Erste Schema-Evolution im Repo (Präzedenz-setzend):** additive, abwärtskompatible Spalte ohne
  Versions-Bump ist regelwerk-konform, **wenn** der Abwärtskompat-Zweig (`releasing.md` §27) per SQL-Default
  erfüllt ist und die Version **nicht** steigt (§6 ist konditional auf eine Erhöhung). `schema.sql` ist ein
  **d-migrate-Artefakt** → via gepinntem Image **regenerieren**, nie hand-edieren; `make schema-check` ist der
  fail-closed Drift-Wächter. Die Plan-Review-Lehre: ein „wir haben das schon 4× gemacht"-Präzedenz **gegen
  die Quellen prüfen** — hier war er falsch (013c–016c waren alle „Kein Schema-Wechsel").
- **`default` statt `required` für `roofs.thickness_mm`:** `Roof` hat einen Domänen-Default
  (`kDefaultRoofThicknessMm`), `Slab` nicht → die Tabellen divergieren bewusst; der SQL-Default feuert nur
  für nicht-Writer-Zeilen (Alt-Datei-Degradation), der normale Save bindet immer explizit.
- **Default-Sonde gegen die Konstante:** der einzige Ort, der YAML-`default: 200` ↔ `kDefaultRoofThicknessMm`
  bindet, ist die Test-Assertion gegen die **Konstante** — kein Gate kreuzprüft YAML↔C++-Konstante.

**Restrisiko / Nachfolge:** Die in 023b benannte Persistenz-Lücke ist geschlossen — Dach-Dicke/-Volumen sind
round-trip-treu. **Nachfolge: 024** (Dächer+Treppen STEP-B-Rep — Dächer sind seit 023b geschlossene Solids,
ihre Dicke seit 023c persistent). Latenter Mini-Cleanup (kein Gate): ein computational Sensor „YAML-Default
== C++-Default-Konstante" bliebe wünschenswert (gleiche Klasse wie der MR-010-Header-Promotion-Kandidat).
