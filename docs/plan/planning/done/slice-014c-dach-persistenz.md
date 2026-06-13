---
id: slice-014c
titel: Dach-Persistenz — roofs-Round-Trip (footprint_json)
status: done
welle: welle-2-bauteile
lastenheft_refs: [LH-FA-ROF-001, LH-FA-BLD-002, LH-FA-BLD-003]
adr_refs: [ADR-0001, ADR-0003, ADR-0006, ADR-0011]
---

# Slice 014c: Dach-Persistenz — roofs-Round-Trip

**Status:** done (2026-06-13). MR-006-Plan-Review gelaufen (keine HIGH,
MED-1/LOW-1/INFO-2 eingearbeitet); DoD vollständig, `make gates` grün
(95 Tests, Coverage 91,8 %). Closure-Notiz §8.

**Plan-Review (MR-006):**
[Report](../../../reviews/2026-06-13-slice-014c-plan.md) — keine HIGH
(JSON-Array-Ansatz tragfähig, `%.17g` korrekt). MED-1 (Round-Trip-AK mit
nicht-glattem double für reale `%.17g`-Deckung), LOW-1 (ADR-Index-Zeile
ergänzen statt setzen), INFO-2 (Mapper werfen bei Unbekanntem)
eingearbeitet.

**Welle:** welle-2-bauteile (sechster Slice).

**Bezug:** ADR-0011-Folgepflicht (Dach-Persistenz; slice-014b ließ
Dächer nur im Speicher), LH-FA-ROF-001 (das Bauteil), LH-FA-BLD-002/003
(speichern/laden, atomar). ADR-0003 (SQLite-Adapter, atomar), ADR-0006
(`roofs`-Schema, **liegt vor**: `roof_type`, `pitch_deg`, `overhang_mm`,
`height_mm`, `footprint_json`, `material_id`), ADR-0001 (Regel D, SQLite
nur im Adapter).

**Autor:** Dietmar Burkard. **Datum:** 2026-06-13.

**Schnitt-Herkunft:** Persistenz-Hälfte des Dach-Strangs (Muster
013b/013c), in 014b bewusst herausgelöst. Vor dem ersten lebenden
Save-Use-Case (heute kein Save-Pfad in `main` verdrahtet → kein
§2.2-Datenverlust).

---

## 1. Ziel

`Building.roofs` überleben Speichern/Laden: der `SqliteProjectRepository`
schreibt Dächer in die bestehende `roofs`-Tabelle (ADR-0006) in derselben
atomaren Transaktion wie Wände/Öffnungen und liest sie feldgleich zurück.

## 2. Definition of Done

- [x] **`save` schreibt Dächer** in `roofs` (explizite `id`,
      `project_id` Literal `1`, `storey_id`, `roof_type` text
      sattel/walm/pult, `pitch_deg`, `overhang_mm`, `footprint_json`).
      `height_mm`/`material_id` bleiben `NULL` (welle-2-Scope: Firsthöhe
      ist abgeleitet, kein Material). Innerhalb der bestehenden
      `BEGIN`/`COMMIT`-Transaktion, **nach `insertStoreys`** (FK
      `roofs.storey_id → storeys.id`).
- [x] **`footprint_json`-Format entschieden + dokumentiert:** der
      rechteckige Grundriss als JSON-**Array**
      `[origin_x_mm, origin_y_mm, width_mm, depth_mm, base_z_mm]`
      (eigenes, deterministisches Format; `%.17g`-Präzision für
      double-Round-Trip). Kein JSON-Präzedenz im Repo → ein **fokussierter
      Serialisierer + Parser** im Persistenz-Adapter (kapselt; verlässt
      den Adapter nicht). Beim späteren Polygon-Grundriss-Vollumfang wird
      das Format erweitert (dann Ring statt 5-Tupel).
- [x] **`load` rekonstruiert `Building.roofs`**: `SELECT` aus `roofs`
      (geordnet nach `id`); `roof_type` → `RoofType`; `footprint_json`
      → origin/width/depth/base_z. Feldgleich für die welle-2-Domänen-
      felder (`Roof`: id, storey_id, type, origin, width, depth, base_z,
      pitch, overhang).
- [x] **Round-Trip-AK-Test** (`LH-FA-ROF` im Namen): Building mit ≥ 1
      Dach je Typ (oder ein Sattel + ein Walm) → `save` → `load` →
      `roofs` feldgleich (Anzahl, id, storey_id, type, Grundriss-Maße,
      base_z, pitch, overhang). **Mindestens ein nicht-glatter
      `double`** im footprint (z. B. `origin.x = 1234.56789012345`) mit
      `EXPECT_DOUBLE_EQ` — belegt die `%.17g`-Präzision **diskriminierend**
      (glatte `*.0`-Werte kämen auch mit `%.15g` durch; MED-1). Bestehende
      BLD-002/003- und DOR/WIN-Round-Trip-Tests bleiben textlich grün
      (Regression).
- [x] **Kein Schema-Wechsel** (ADR-0006-Tabelle liegt vor) → **kein**
      `make schema-check`-Drift. `make gates` grün; arch-check Regel D;
      Closure-Notiz mit Lerneintrag; CHANGELOG (MR-004); **neue
      ADR-0011-Index-Zeile „Dach-Persistenz → erfüllt durch slice-014c"
      ergänzen** (es existiert keine solche Zeile — LOW-1; analog der
      Öffnungs-Persistenz-Zeile von 013c). roof_type-/footprint-Mapper
      werfen bei Unbekanntem/Format-Fehler neutral (`E-IO`, Muster
      `classToText`/`textToClass`; INFO-2).

## 3. Plan (vor Code)

| Datei / Komponente | Änderungs-Art | Begründung |
|---|---|---|
| `src/adapters/persistence/sqlite_project_repository.cpp` | ändern | `insertRoofs`/`loadRoofs` (+ roof_type-Text-Mapper, footprint-JSON-Ser/De); Aufruf in `save` nach `insertStoreys`, in `load` nach `loadStoreys` |
| `tests/adapters/test_sqlite_project_repository.cpp` | ändern | Round-Trip-AK mit Dächern (Sattel + Walm) |
| `docs/plan/adr/README.md` | ändern | ADR-0011-Folgepflicht „Dach-Persistenz" → erfüllt (slice-014c) |
| `CHANGELOG.md` | ändern | Slice-Eintrag (MR-004) |
| `docs/reviews/{2026-06-13-slice-014c-plan}.md` | neu | MR-006-Report |

## 4. Trigger

- slice-014b done ✓ (`Building.roofs` + Domänen-Typ).
- MR-006-Plan-Review vor Start.

## 5. Closure-Trigger

- DoD vollständig, `make gates` grün, Closure-Notiz → die Dach-Familie
  (014a/b/c) ist abgeschlossen; welle-2 setzt mit SLB+FND, STR fort.

## 6. Risiken und offene Punkte

- **JSON ohne Bibliothek (zentrale Entscheidung):** kein JSON-Präzedenz
  im Repo, keine JSON-Lib (ADR-0004-Toolchain unverändert). Der
  Serialisierer/Parser ist auf das **eigene** deterministische
  5-Zahlen-Array zugeschnitten (liest nur eigene Writes); Robustheit per
  Round-Trip-AK belegt, nicht per allgemeinem JSON-Parser. Ein
  defensiver Parse (Format-Fehler → `E-IO`/neutraler Wurf) hält den
  Adapter total nach außen (kein OCC-/Lib-Typ leckt).
- **Fremdschlüssel-Reihenfolge:** `roofs.storey_id → storeys.id` +
  `ON DELETE CASCADE`; Dächer **nach** `insertStoreys` einfügen.
- **`%.17g`-Präzision:** nötig für exakten double-Round-Trip im
  Text-JSON (sqlite-`bind_double` für die Spalten pitch/overhang ist
  exakt; nur das footprint_json ist Text → Präzision explizit).
- **Welle-2-Feld-Scope:** `height_mm` (abgeleitet) und `material_id`
  bleiben `NULL`; ihr Round-Trip kommt mit Auswertung/Material (welle-3).
- **Explizite `id` in AUTOINCREMENT-PK:** wie `walls`/`openings` (SQLite
  übernimmt gebundene Werte).

## 7. Sub-Area-Modus-Begründung

### Sub-Area: Persistenz-Adapter (`src/adapters/persistence/`)

- **Modus:** GF; Dichte hoch (SQLite nur im Adapter/Regel D, atomar,
  ADR-0006-Schema, E-IO-Codes, JSON-Ser/De gekapselt); Risiko niedrig–
  mittel (erste JSON-Ser/De im Repo — durch Round-Trip-AK gedeckt).

### Sub-Area: Test-Infrastruktur (`tests/`)

- **Modus:** GF; Dichte hoch (Round-Trip-AK mit `LH-`-ID, Feld-Vergleich);
  Risiko niedrig.

## 8. Closure-Notiz

**Closure-Kriterien (beobachtbar):**

- **`save`/`load` round-trippen Dächer** über die `roofs`-Tabelle
  (`sqlite_project_repository.cpp` `insertRoofs`/`loadRoofs` +
  `roofTypeToText`/`textToRoofType` + `footprintToJson`/
  `parseFootprintJson`). Dächer in derselben atomaren Transaktion **nach**
  `insertStoreys` (FK `storey_id`); `roof_type`/`pitch`/`overhang` als
  Spalten, Grundriss als `footprint_json`-Array; `height_mm`/`material_id`
  bleiben `NULL` (welle-2-Scope).
- **JSON-Ser/De (erste im Repo):** `footprint_json` =
  `[origin_x, origin_y, width, depth, base_z]`, `%.17g`; Parser total
  (Format-Fehler → neutraler `E-IO`-Wurf, kein Lib-Typ leckt — Regel D
  gewahrt). **Round-Trip-AK mit nicht-glattem `origin.x`
  (1234.56789012345)** belegt die `%.17g`-Präzision diskriminierend
  (MED-1).
- **Round-Trip-AK** `SqliteProjectRepository_LH_FA_ROF_001.RoundTripErhaeltDaecher`
  (Sattel + Walm feldgleich); bestehende BLD-002/003- + DOR/WIN-
  Round-Trip-Tests textlich unverändert grün.
- **Kein Schema-Wechsel** (ADR-0006 trug `roofs`); `make schema-check`
  nicht ausgelöst. **`make gates` grün:** docs-check 0, arch-check A–E
  (Regel D), lint 0 + suppression-gate, **Tests 95/95**, Coverage 91,8 %.
- **ADR-0011-Index:** neue Folgepflicht-Zeile „Dach-Persistenz → erfüllt
  durch slice-014c" ergänzt (LOW-1).

**Lerneintrag:**

- **Erste JSON-Ser/De im Repo, bewusst minimal:** ein zugeschnittenes
  5-Zahlen-Array statt JSON-Lib (ADR-0004-Toolchain unverändert) — liest
  nur eigene Writes, Round-Trip-getestet, gekapselt. Bei komplexen
  Polygon-Grundrissen (Vollumfang) wird das Format zum Ring erweitert;
  ggf. ist dann eine echte JSON-Lib ein eigener ADR-/Toolchain-Schritt.
- **Schema-Voraussicht (4. Vorkommen):** `roofs` lag vollständig vor —
  rein mechanische Adapter-Erweiterung.

**Restrisiko / Nachfolge:** Dach-Familie (014a/b/c) abgeschlossen.
welle-2-Fortsetzung: SLB+FND (Decken/Fundament), STR (Treppen) entlang
der ADR-0011-Leitplanke (#6). Persistenz weiterhin ohne lebenden
Save-Use-Case (bereit, sobald ein Speichern/Laden-Use-Case verdrahtet
wird).
