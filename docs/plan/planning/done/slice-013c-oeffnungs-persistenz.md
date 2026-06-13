---
id: slice-013c
titel: Öffnungs-Persistenz — openings/doors/windows-Round-Trip
status: done
welle: welle-2-bauteile
lastenheft_refs: [LH-FA-DOR-001, LH-FA-WIN-001, LH-FA-BLD-002, LH-FA-BLD-003]
adr_refs: [ADR-0001, ADR-0003, ADR-0006, ADR-0011]
---

# Slice 013c: Öffnungs-Persistenz — openings/doors/windows-Round-Trip

**Status:** done (2026-06-13). MR-006-Plan-Review gelaufen (1 HIGH
redaktionell + 1 MED + 1 LOW eingearbeitet); DoD vollständig,
`make gates` grün (83 Tests, Coverage 93,4 %). Closure-Notiz §8.

**Plan-Review (MR-006):**
[Report](../../../reviews/2026-06-13-slice-013c-plan.md) (H/M/L-IDs) —
H1 (`openings.project_id`/Zeitstempel-Spalten in der DoD-Spaltenliste
ergänzt), M1 (`openings.name`/Zeitstempel-Scope klargestellt), L1
(AUTOINCREMENT-Beleg) eingearbeitet.

**Welle:** welle-2-bauteile (dritter Slice).

**Bezug:** ADR-0011-Folgepflicht „Persistenz-Abbildung" (slice-013b
ließ Öffnungen nur im Speicher); LH-FA-DOR-001/WIN-001 (die Bauteile),
LH-FA-BLD-002/003 (speichern/laden, atomar — der bestehende Vertrag).
ADR-0003 (SQLite-Adapter, atomar), ADR-0006 (`openings → doors/windows`-
Schema, **liegt bereits vollständig vor**), ADR-0001 (Schichtung,
SQLite nur im Adapter, Regel D).

**Autor:** Dietmar Burkard. **Datum:** 2026-06-13.

**Schnitt-Herkunft:** Persistenz-Hälfte des Tür/Fenster-Strangs, in
slice-013b bewusst herausgelöst (Welle-Sizing). **Auflage aus dem
slice-013b-Review (M1): vor dem ersten lebenden Save-Use-Case** — der
existiert noch nicht (`SqliteProjectRepository` ist nicht in `main`
verdrahtet), daher schließt dieser Slice die Lücke rechtzeitig und ohne
realen Datenverlust-Zeitraum.

---

## 1. Ziel

`Building.openings` (Türen/Fenster aus slice-013b) überleben
Speichern/Laden: der `SqliteProjectRepository` schreibt sie in die
bestehenden Tabellen `openings`/`doors`/`windows` (ADR-0006) innerhalb
derselben atomaren Transaktion wie Geschosse/Wände und liest sie
feldgleich zurück. Damit ist die ADR-0011-Folgepflicht „Persistenz"
erfüllt und Öffnungen sind nicht mehr nur im Speicher.

## 2. Definition of Done

- [x] **`save` schreibt Öffnungen** in `openings` (explizite `id`,
      `project_id` Literal `1` (NOT NULL, Muster `storeys`/`walls`),
      `wall_id`, `opening_type` door/window, `offset_mm`/`width_mm`/
      `height_mm`/`sill_height_mm`, `created_at`/`updated_at` =
      `kSentinelTs`; `name` bleibt `NULL`) + die 1:1-Spezialisierung:
      Türen nach `doors` (`swing_direction` left/right; `doors`/`windows`
      tragen **keine** Zeitstempel-Spalten), Fenster nach `windows`
      (nur `opening_id`) — Class-Table-Inheritance gemäß ADR-0006.
      **Innerhalb der bestehenden `BEGIN`/`COMMIT`-Transaktion und nach
      `insertWalls`** (Fremdschlüssel `openings.wall_id → walls.id`).
      `PRAGMA foreign_keys=ON` ist bereits gesetzt.
- [x] **`load` rekonstruiert `Building.openings`**: `SELECT` aus
      `openings` (geordnet nach `id`) mit `LEFT JOIN doors` für die
      `swing_direction`; `kind` aus `opening_type`. Feldgleich für die
      **welle-2-Domänenfelder** (`Opening`: id, wall_id, kind, offset,
      width, height, sill, swing-für-Türen).
- [x] **Welle-2-Scope der Felder (M4, slice-013b):** das Domänenmodell
      trägt `swing_angle_deg`/`is_external`/`frame_material`/
      `glazing_type`/`u_value` sowie `openings.name` **nicht** — beim
      Speichern erhalten sie die Schema-Defaults/`NULL`, beim Laden
      werden sie ignoriert (kein Round-Trip dieser Felder, bis das
      Domänenmodell sie trägt). In der Closure benannt.
- [x] **Round-Trip-AK-Test** (`LH-FA-DOR`/`WIN` im Namen): Building mit
      ≥ 1 Tür **und** ≥ 1 Fenster an einer Wand → `save` → `load` →
      `openings` feldgleich (Anzahl, id, wall_id, kind, Maße, Brüstung,
      Tür-Anschlag); Reihenfolge stabil. Plus: leeres Projekt
      (keine Öffnungen) bleibt leer; Bestehende BLD-002/003-Round-Trip-
      Tests bleiben textlich grün (Regression).
- [x] **Kein Schema-Wechsel** (ADR-0006-Tabellen liegen vor) → **kein**
      `make schema-check`-Drift; falls doch eine Spalte fehlte, wäre es
      `data-model.yaml` + `schema.sql` + `make schema-check` (nicht in
      `gates`). `make gates` grün; arch-check Regel D (kein `sqlite3*`
      außerhalb `adapters/persistence/`); Closure-Notiz mit Lerneintrag;
      CHANGELOG-Eintrag (MR-004); ADR-0011-Folgepflicht „Persistenz" im
      Index auf **erfüllt** setzen.

## 3. Plan (vor Code)

| Datei / Komponente | Änderungs-Art | Begründung |
|---|---|---|
| `src/adapters/persistence/sqlite_project_repository.cpp` | ändern | `insertOpenings`/`loadOpenings` (+ kind/swing-Text-Mapper); Aufruf in `save` nach `insertWalls`, in `load` nach `loadWalls` |
| `tests/adapters/test_sqlite_project_repository.cpp` | ändern | Round-Trip-AK mit Tür+Fenster; `sampleBuilding` um Öffnungen erweitern oder eigener Test |
| `docs/plan/adr/README.md` | ändern | ADR-0011-Folgepflicht „Persistenz" → erfüllt (slice-013c) |
| `CHANGELOG.md` | ändern | Slice-Eintrag (MR-004) |
| `docs/reviews/{2026-06-13-slice-013c-plan}.md` | neu | MR-006-Report |

## 4. Trigger

- slice-013b done ✓ (`Building.openings` + Domänen-Typen existieren).
- MR-006-Plan-Review vor Implementierungs-Start.

## 5. Closure-Trigger

- DoD vollständig, `make gates` grün, Closure-Notiz → ADR-0011 vollständig
  erfüllt; welle-2 setzt mit **ROF (Dach)** / SLB+FND / STR entlang der
  ADR-0011-Leitplanke fort.

## 6. Risiken und offene Punkte

- **Fremdschlüssel-Reihenfolge:** `openings.wall_id → walls.id` +
  `PRAGMA foreign_keys=ON` ⇒ Öffnungen **nach** Wänden einfügen, und beim
  Löschen/Ersetzen greift `ON DELETE CASCADE`. Reihenfolge im `save`
  beachten (Risiko: FK-Constraint-Fehler → E-IO-002).
- **CTI-Konsistenz:** jede `openings`-Zeile braucht **genau eine**
  `doors`- **oder** `windows`-Zeile (nach `opening_type`). Ein Test mit
  beiden Typen deckt beide Pfade.
- **Explizite `id` vs. AUTOINCREMENT:** `openings.id` ist
  `AUTOINCREMENT`-PK; die Domänen-`OpeningId` wird **explizit** gesetzt
  (wie `walls.id`), damit `wall_id`-Bezüge und ids stabil round-trippen.
  SQLite übernimmt einen explizit gebundenen Wert in eine
  AUTOINCREMENT-PK; nur leere Inserts vergeben automatisch (L1).
- **Welle-2-Feld-Scope (M4):** Nicht-Domänen-Spalten (swing_angle/
  is_external/frame/glazing/u_value) bekommen Defaults/`NULL` — beim
  späteren Domänen-Ausbau wird ihr Round-Trip nachgezogen (kein stiller
  Verlust, da die Domäne sie heute nicht kennt).
- **Sentinel-Zeitstempel:** `created_at`/`updated_at` wie bei
  storeys/walls über `kSentinelTs` (welle-1 kennt keine Zeitstempel-
  Semantik; hält den Round-Trip byte-stabil).
- **Keine `Building`-Identität über `==`:** der AK vergleicht Felder
  einzeln (Muster bestehender Round-Trip-Test), kein Struktur-`operator==`.

## 7. Sub-Area-Modus-Begründung

### Sub-Area: Persistenz-Adapter (`src/adapters/persistence/`)

- **Modus:** GF; Dichte hoch (SQLite nur im Adapter/Regel D, atomar,
  ADR-0006-Schema-Abbildung, E-IO-Fehlercodes); Risiko niedrig —
  mechanische Erweiterung parallel zu `insertWalls`/`loadWalls`, Schema
  liegt vor.

### Sub-Area: Test-Infrastruktur (`tests/`)

- **Modus:** GF; Dichte hoch (Round-Trip-AK mit `LH-`-ID, Feld-Vergleich,
  Registrierung); Risiko niedrig.

## 8. Closure-Notiz

**Closure-Kriterien (beobachtbar):**

- **`save`/`load` round-trippen Öffnungen** über `openings` +
  `doors`/`windows`-CTI (`sqlite_project_repository.cpp`
  `insertOpenings`/`loadOpenings` + kind/swing-Text-Mapper). Öffnungen
  in derselben atomaren `BEGIN`/`COMMIT`-Transaktion **nach**
  `insertWalls` (FK `wall_id`); `load` via `LEFT JOIN doors`.
- **Round-Trip-AK** `SqliteProjectRepository_LH_FA_DOR_WIN.RoundTripErhaeltOeffnungen`:
  Tür (Anschlag rechts) + Fenster (Brüstung 900) an einer Wand →
  feldgleich zurück (id, wall_id, kind, offset/width/height/sill,
  Tür-Anschlag). Bestehende BLD-002/003-Round-Trip-Tests **textlich
  unverändert grün** (Regression).
- **Kein Schema-Wechsel** — `data-model.yaml`/`schema.sql` (ADR-0006)
  trugen die Tabellen bereits; `make schema-check` nicht ausgelöst.
- **`make gates` grün** (2026-06-13): docs-check 0, gate-consistency,
  arch-check **A–E** (Regel D: `sqlite3*` nur im Persistenz-Adapter),
  lint 0 + suppression-gate, **Tests 83/83** (zuvor 82, +Round-Trip-
  Öffnungen), Coverage **93,4 %**.
- **ADR-0011-Folgepflicht „Persistenz" erfüllt** (Index nachgezogen) —
  ADR-0011 ist damit **vollständig** umgesetzt (Geometrie/Verhalten
  013b, Persistenz 013c).

**Lerneintrag:**

- **Schema-Voraussicht zahlt zum zweiten Mal:** wie bei 013a/013b trug
  das ADR-0006-Schema die Öffnungs-Tabellen bereits vollständig — die
  Persistenz war eine rein mechanische Adapter-Erweiterung parallel zu
  `insertWalls`/`loadWalls`, ohne Schema-Schärfung. Bestätigt den
  013b-Closure-Kandidaten *Datenmodell vorausschauend für die
  Roadmap-Bauteile anlegen* (2. Vorkommen).

**Restrisiko / Nachfolge:**

- **M4-Feld-Scope (welle-2):** `swing_angle_deg`/`is_external`/
  `frame_material`/`glazing_type`/`u_value`/`openings.name` sind nicht
  im Domänenmodell und persistieren als Default/`NULL` — ihr Round-Trip
  wird mit dem Domänen-Ausbau (eigenes Tür-/Fenster-Solid, Material,
  welle-3/Re-Eval) nachgezogen.
- **Save-Use-Case:** weiterhin kein lebender Speicher-Pfad in `main`
  verdrahtet — aber die Persistenz ist jetzt **bereit**, sobald ein
  Speichern/Laden-Use-Case kommt (kein §2.2-Datenverlust-Fenster mehr,
  M1-Auflage erfüllt).
- **welle-2 Fortsetzung:** ROF (Dach), SLB+FND, STR entlang der
  ADR-0011-Bauteil-Erweiterungs-Leitplanke (#6).
