---
id: slice-016c
titel: Treppen-Persistenz — stairs-Round-Trip (rise abgeleitet/write-derived)
status: in-progress
welle: welle-2-bauteile
lastenheft_refs: [LH-FA-STR-001, LH-FA-STR-002, LH-FA-STR-003, LH-FA-BLD-002, LH-FA-BLD-003]
adr_refs: [ADR-0001, ADR-0003, ADR-0006, ADR-0011]
---

# Slice 016c: Treppen-Persistenz — stairs-Round-Trip

**Status:** done (2026-06-14). MR-006-Plan-Review gelaufen
([Report](../../../reviews/2026-06-14-slice-016c-plan.md) — **keine HIGH**;
Designkern `rise`-write-derived + Adapter→Kern-Include verifiziert sauber; MED-1
`from_storey`-Lookup total/`E-IO`, MED-2 Negativ-Parse-Test, LOW/INFO-Schärfungen
eingearbeitet). DoD vollständig, `make gates` grün (114 Tests, Coverage 92,3 %).
Closure-Notiz §8.

**Welle:** welle-2-bauteile (zwölfter Slice; **letzter Bauteil-Slice der
Welle** — Treppen-Familie 016a/b/c abgeschlossen danach).

**Bezug:** ADR-0011-Folgepflicht (#6) „Persistenz" für das Treppen-Modul
(slice-016b ließ Treppen nur im Speicher), LH-FA-STR-001..003,
LH-FA-BLD-002/003 (speichern/laden, atomar). ADR-0003 (SQLite-Adapter,
atomar), ADR-0006 (`stairs`-Schema, **liegt vor**: `from/to_storey_id`
RESTRICT, `stair_type`, `start_x/y_mm`, `width_mm`, `step_count`, `rise_mm`,
`tread_mm`, `name`), ADR-0001 (Regel D, SQLite nur im Adapter).

**Autor:** Dietmar Burkard. **Datum:** 2026-06-14.

**Schnitt-Herkunft:** Persistenz-Hälfte des Treppen-Strangs (Muster
013c/014c/015c), in 016b bewusst herausgelöst. Vor dem ersten lebenden
Save-Use-Case (kein Save-Pfad in `main` → kein §2.2-Datenverlust-Fenster).

---

## 1. Ziel

`Building.stairs` (gerade einläufige Treppen aus slice-016b) überleben
Speichern/Laden: der `SqliteProjectRepository` schreibt Treppen in die
bestehende `stairs`-Tabelle (ADR-0006) in derselben atomaren Transaktion wie
Geschosse/Wände/Dächer/Platten und liest die **Domänenfelder** feldgleich
zurück. **`rise_mm` ist abgeleitet** (Geschosshöhe/step_count) → write-derived,
**nicht** round-getrippt; **Geländer render-only** → keine Spalte, nichts zu
persistieren.

## 2. Definition of Done

- [x] **`save` schreibt Treppen** in `stairs` (explizite `id`, `project_id`
      Literal `1`, `from_storey_id`, `to_storey_id`, `stair_type` text gerade,
      `start_x_mm`/`start_y_mm`, `width_mm`, `step_count`, `tread_mm`).
      Innerhalb der bestehenden `BEGIN`/`COMMIT`-Transaktion, **nach
      `insertStoreys`** (FK `from/to_storey_id → storeys.id`, `RESTRICT`).
      `name` bleibt `NULL` (welle-2-Scope, nicht im Domänenmodell).
- [x] **`rise_mm` write-derived (Abgrenzung zu 015c-Cutouts):** das Domänen-
      modell trägt **kein** `rise`-Feld (spez. §1 `LH-FA-STR-001.a`: rise =
      Geschosshöhe/step_count abgeleitet). Die Schema-Spalte `rise_mm` ist
      `NOT NULL` → `save` schreibt den **abgeleiteten** Wert über den
      kanonischen Kern-Helfer **`stairRiseMm(stair, from_storey_height)`**
      (kein Formel-Duplikat; from_storey-Höhe aus `building.storeys`). **Der
      Höhen-Lookup ist total (MED-1):** fehlt die `from_storey` in
      `building.storeys`, wirft `insertStairs` neutral `E-IO` (Roll-back in der
      Transaktion) — **kein stilles `rise=0`** (b-cad „kein stiller Verlust").
      `load` liest `rise_mm` **nicht** zurück (der Kern leitet es bei Bedarf neu ab).
      **Gegenstück zu 015c:** dort round-trippten `cutouts`, weil domänen-
      getragen; hier ist `rise` *abgeleitet* (wie roofs-`height_mm`) → kein
      Round-Trip, kein stiller Verlust. **Geländer** hat keinen persistierbaren
      Eigenzustand (render-only, keine Spalte) — nichts zu speichern.
- [x] **`stair_type`-Mapper total:** `stairTypeToText`/`textToStairType`
      (Muster `slabTypeToText`) — `stairs.stair_type` trägt **keine**
      CHECK-Constraint → der Mapper erzwingt die gültige Menge {gerade} und
      wirft bei Unbekanntem neutral (`E-IO`, kein Lib-Typ leckt).
- [x] **`load` rekonstruiert `Building.stairs`**: `SELECT` aus `stairs`
      (geordnet nach `id`); `stair_type` → `StairType`; Feldgleich für die
      welle-2-Domänenfelder (`Stair`: id, from_storey_id, to_storey_id, type,
      start, width, step_count, tread). **Nicht** geladen: `rise_mm`
      (abgeleitet), `name` (nicht im Modell).
- [x] **Round-Trip-AK-Test** (`LH-FA-STR` im Namen): Building mit ≥ 1 Treppe
      (zwei Geschosse + Treppe; **mindestens ein nicht-glatter `double`**, z. B.
      `start.x = 1234.56789012345`, mit `EXPECT_DOUBLE_EQ` → belegt die
      **`sqlite3_bind_double`-Exaktheit** diskriminierend; **kein `%.17g`-Text-
      Pfad** für stairs, anders als roofs/slabs — INFO-3) → `save` → `load` →
      `stairs` feldgleich (id, from/to_storey, type, start-Punkt, width,
      step_count, tread). `LeeresProjektRoundTrip` um `stairs.empty()` erweitert.
      **Negativ-Parse-Test (MED-2):** white-box-korrumpiertes `stair_type`
      (z. B. `'wendel'`) → `load` wirft neutral `E-IO` (`textToStairType` total;
      Muster 015c `MalformedSpaltenWerfenNeutral`). Bestehende BLD-002/003-,
      DOR/WIN-, ROF- und **SLB/FND-Round-Trip-Tests** bleiben textlich grün
      (Regression).
- [x] **Kein Schema-Wechsel** (ADR-0006-Tabelle liegt vor) → **kein**
      `make schema-check`-Drift. `make gates` grün; arch-check Regel D (kein
      `sqlite3*` außerhalb `adapters/persistence/`); Closure-Notiz mit
      Lerneintrag; CHANGELOG (MR-004); **neue ADR-0011-(#6)-Index-Zeile
      „Treppen (LH-FA-STR-*) → erfüllt durch slice-016a/b/c"** ergänzen (analog
      Dach-/Decken-Zeile). **Treppen-Familie 016a/b/c komplett → Welle-2-Closure
      wird startbar.**

## 3. Plan (vor Code)

| Datei / Komponente | Änderungs-Art | Begründung |
|---|---|---|
| `src/adapters/persistence/sqlite_project_repository.cpp` | ändern | `stairTypeToText`/`textToStairType`, `insertStairs` (rise via `stairRiseMm`, from_storey-Höhe aus `building.storeys`), `loadStairs`; Aufruf in `save` nach `insertSlabs`, in `load` nach `loadSlabs`; `#include "hexagon/services/stair_geometry.h"` (Adapter→Kern, erlaubte Richtung) |
| `tests/adapters/test_sqlite_project_repository.cpp` | ändern | Round-Trip-AK mit Treppe (nicht-glatter double) + Leer-Pfad-`stairs.empty()`; using-Decls (Stair/StairId/StairType) |
| `docs/plan/adr/README.md` | ändern | ADR-0011-(#6)-Folgepflicht „Treppen" → erfüllt (slice-016a/b/c) |
| `CHANGELOG.md` | ändern | Slice-Eintrag (MR-004) |
| `docs/reviews/2026-06-14-slice-016c-plan.md` | neu | MR-006-Report |

## 4. Trigger

- slice-016b done ✓ (`Building.stairs` + Domänen-Typ `Stair`).
- MR-006-Plan-Review vor Start.

## 5. Closure-Trigger

- DoD vollständig, `make gates` grün, Closure-Notiz → **Treppen-Familie
  (016a/b/c) abgeschlossen; STR war das letzte welle-2-Bauteil** → Welle-2-Closure
  (unabhängige Verifikation + `done/welle-2-results.md` inkl. Carveout-Audit) wird
  startbar. Empfohlen davor: geometrielastiges Code-Review von 016b.

## 6. Risiken und offene Punkte

- **`rise_mm` write-derived (zentrale Entscheidung):** das Schema verlangt
  `rise_mm NOT NULL`, das Domänenmodell trägt es nicht. `save` schreibt den
  abgeleiteten Wert via `stairRiseMm` (Reuse des Kern-Helfers statt
  Formel-Duplikat im Adapter); `load` ignoriert die Spalte. Risiko: ein extern
  geändertes `rise_mm` ginge beim nächsten `save` verloren — bewusst, da `rise`
  in welle-2 **nie** Eingabe ist (abgeleitet). In der Closure benannt.
- **Adapter→Kern-Include (`stair_geometry.h`):** erlaubte Abhängigkeitsrichtung
  (Adapter nutzt Kern; arch-check Regel A betrifft nur den umgekehrten Weg).
  Begründung: Single-Source der rise-Formel statt Duplikat.
- **Fremdschlüssel-Reihenfolge (RESTRICT):** `stairs.from/to_storey_id →
  storeys.id`, `ON DELETE RESTRICT` (kein Cascade — eine Treppe hält ihre
  Geschosse). Treppen **nach** `insertStoreys` einfügen; beide Geschosse müssen
  existieren (im AK durch zwei Geschosse gedeckt).
- **`%.17g`-Präzision:** für exakten double-Round-Trip von `start_x/y`/`width`/
  `tread` im Text bzw. `bind_double` (sqlite-`bind_double` ist exakt; ein
  nicht-glatter double im AK belegt es diskriminierend, MED-1-Präzedenz 014c).
- **`step_count` ist `int`** → `sqlite3_bind_int`/`sqlite3_column_int`
  (exakt, kein double-Pfad).
- **`stair_type` ohne CHECK:** Mapper erzwingt {gerade}, wirft bei Unbekanntem
  (sonst schlüge erst `textToStairType` beim `load` fehl).
- **Welle-2-Feld-Scope:** `name` bleibt `NULL` (nicht im Domänenmodell);
  `rise_mm` write-derived; Geländer render-only. Ihr (ggf.) Round-Trip kommt mit
  dem Domänen-Ausbau (Material/Bewehrung/Podest, welle-3+).
- **Explizite `id` in AUTOINCREMENT-PK:** wie `walls`/`roofs`/`slabs`.
- **Keine `Building`-Identität über `==`:** der AK vergleicht Felder einzeln
  (Muster bestehender Round-Trip-Tests).

## 7. Sub-Area-Modus-Begründung

### Sub-Area: Persistenz-Adapter (`src/adapters/persistence/`)

- **Modus:** GF; Dichte hoch (SQLite nur im Adapter/Regel D, atomar,
  ADR-0006-Schema, E-IO-Codes); Risiko niedrig — mechanische Erweiterung
  parallel zu `insertSlabs`/`loadSlabs`, Schema liegt vor. Einzige Neuheit: der
  write-derived `rise_mm` (durch Reuse von `stairRiseMm` minimal gehalten).

### Sub-Area: Test-Infrastruktur (`tests/`)

- **Modus:** GF; Dichte hoch (Round-Trip-AK mit `LH-`-ID, Feld-Vergleich);
  Risiko niedrig.

## 8. Closure-Notiz

**Closure-Kriterien (beobachtbar):**

- **`save`/`load` round-trippen Treppen** über die `stairs`-Tabelle
  (`sqlite_project_repository.cpp` `insertStairs`/`loadStairs` +
  `stairTypeToText`/`textToStairType`). Treppen in derselben atomaren
  `BEGIN`/`COMMIT`-Transaktion **nach** `insertStoreys` (FK from/to_storey
  `RESTRICT`); from/to_storey/stair_type/start_x/y/width/step_count/tread als
  Spalten (`bind_double`/`bind_int`, kein Text/JSON-Pfad → kein `%.17g`).
- **`rise_mm` write-derived (Abgrenzung zu 015c):** das Domänenmodell trägt
  **kein** `rise`-Feld; `save` schreibt den abgeleiteten Wert über den
  kanonischen Kern-Helfer **`stairRiseMm`** (from_storey-Höhe aus
  `building.storeys`; **Lookup total** → `E-IO`-Wurf + Roll-back bei fehlendem
  Geschoss, MED-1, **kein stilles `rise=0`**); `load` liest `rise_mm` **nicht**
  zurück (der Kern leitet es neu ab — Muster roofs-`height_mm`). **Geländer**
  render-only (keine Spalte), `name` `NULL` — nichts zu round-trippen. Der
  „Verlust" eines extern geänderten `rise_mm` ist **kein Modell-Datenverlust**
  (das Modell trägt `rise` nicht; LOW-1).
- **`stair_type`-Mapper total:** Schema **ohne** CHECK → der Mapper erzwingt
  {gerade} und wirft bei Unbekanntem (`E-IO`).
- **Round-Trip-AK** `SqliteProjectRepository_LH_FA_STR_001.RoundTripErhaeltTreppen`:
  Treppe (zwei Geschosse, nicht-glatter `start.x` 1234.56789012345 + `tread`
  287.3125) → feldgleich (id, from/to_storey, type, start, width, step_count,
  tread). `LeeresProjektRoundTrip` um `stairs.empty()` erweitert.
  **Negativ-Parse-AK** `MalformedStairTypeWirftNeutral` (CR-MED-2): korrumpiertes
  `stair_type='wendel'` → `load` wirft `E-IO`. Bestehende BLD-002/003-, DOR/WIN-,
  ROF- und SLB/FND-Round-Trip-Tests **textlich unverändert grün** (Regression).
- **Kein Schema-Wechsel** (ADR-0006 trug `stairs`); `schema.sql`/`data-model.yaml`
  unangetastet → `make schema-check` (CI-only, **nicht** in `make gates`) nicht
  ausgelöst. **`make gates` grün** (2026-06-14): docs-check 0, gate-consistency,
  arch-check **A–E** (Regel D: `sqlite3*` nur im Persistenz-Adapter), lint 0 +
  suppression-gate, **Tests 114/114** (zuvor 112, +Round-Trip +Negativ-Parse),
  Coverage **92,3 %**.
- **ADR-0011 (#6)-Index:** neue Folgepflicht-Zeile „Treppen (LH-FA-STR-*) →
  erfüllt durch slice-016a/b/c" ergänzt. **Treppen-Familie 016a/b/c komplett.**

**Lerneintrag:**

- **Write-derived-Feld via Kern-Reuse (neuer Präzedenzfall, INFO-2):** `rise_mm`
  ist die erste Schema-Spalte, die der Persistenz-Adapter **abgeleitet
  berechnet** (statt nur zu mappen). Statt die Formel zu duplizieren, ruft der
  Adapter den kanonischen Kern-Helfer `stairRiseMm` (erster `services`-Include
  im Persistenz-Adapter — erlaubte Adapter→Kern-Richtung). Muster für künftige
  write-derived-Felder: **Single-Source der Ableitung im Kern, der Adapter
  konsumiert sie.**
- **„Domänen-getragen ⇒ round-trippt" — die dritte Anwendung der Trennlinie:**
  013c (Öffnungen) / 015c (cutouts round-trippen) zeigten die *positive* Seite;
  hier greift die *negative* sauber: `rise` (abgeleitet) und Geländer
  (render-only) sind **nicht** domänen-getragen → kein Round-Trip, write-derived
  bzw. gar nicht persistiert, ohne stillen Verlust. Der Grundsatz trägt jetzt
  beide Richtungen über vier Bauteil-Familien hinweg konsistent.
- **Schema-Voraussicht (6. Vorkommen):** `stairs` lag vollständig vor — rein
  mechanische Adapter-Erweiterung, kein DDL-Touch.

**Restrisiko / Nachfolge:** **Treppen-Familie (016a/b/c) abgeschlossen → STR war
das letzte welle-2-Bauteil → Welle-2-Closure ist fällig** (unabhängige
Verifikation analog welle-1/-1v + `done/welle-2-results.md` inkl. **zwingendem
Carveout-Audit** — aktuell keine aktiven Carveouts). **Empfohlen davor:**
geometrielastiges Code-Review von slice-016b (Praxis 3×+ nach welle-1, fand bei
013b/014b/015b je 1 HIGH trotz grüner Gates). Persistenz weiterhin ohne lebenden
Save-Use-Case (bereit, kein §2.2-Fenster). Offen (welle-3+): `name`/Material,
Podest-/Wendeltreppe, freie Rotation, echte Mehr-Geschoss-Elevation.
