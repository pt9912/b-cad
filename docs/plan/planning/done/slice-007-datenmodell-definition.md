---
id: slice-007
titel: Datenmodell-Definition (Persistenz-Schema + Spezifikation §2)
status: done
welle: welle-1-mvp
lastenheft_refs: [LH-FA-BLD-001, LH-FA-FLR-001, LH-FA-WAL-001, LH-FA-ROM-001, LH-QA-003]
adr_refs: [ADR-0001, ADR-0003, ADR-0006]
---

# Slice 007: Datenmodell-Definition (Persistenz-Schema + Spezifikation §2)

**Status:** done

**Welle:** welle-1-mvp

**Bezug:** OBJ-003 (durchgängiges Datenmodell), ADR-0001 (Kern führt), ADR-0003 (SQLite-Persistenz), ADR-0006 (Schema-Design), LH-FA-BLD/FLR/WAL/ROM/…, LH-QA-003 (Undo).

**Autor:** Dietmar Burkard. **Datum:** 2026-06-09.

---

## 1. Ziel

Das **Datenmodell** an *einem* kanonischen Ort definieren — heute über
`spec/spezifikation.md` §2 (Outline), `architecture.md` §2 und Code
verstreut. Ergebnis: ein maschinenlesbares **Persistenz-Schema**
(`spec/data-model.yaml`, aus dem `.tmp/ddl.yaml`-Vorschlag), normativ
beschrieben in §2, mit der Design-Entscheidung in **ADR-0006**. Reiner
Spec-/Doku-Slice — **kein Code**; speist später den Persistenz-Slice
(BLD-002/003) und die Bauteil-Slices.

## 2. Definition of Done

- [x] `spec/data-model.yaml` committet, **format-konform zum d-migrate
      Neutral-Schema** (`schema_format: "1.0"`; GitHub
      [`pt9912/d-migrate`](https://github.com/pt9912/d-migrate),
      `spec/schema-reference.md`): Spalten-`references`
      statt `foreign_keys`-Block, `constraints`/`indices` statt
      `unique_constraints`/top-level `indexes`, `decimal` mit
      `precision`/`scale`, Klassifikation als `enum`. Namen
      **domänen-angeglichen** (ADR-0001: Kern führt): `storeys`,
      Wand-`thickness_mm`. Header trennt Domänen-Modell (Kern) von
      Persistenz-Abbildung.
- [x] `spec/spezifikation.md` §2 von Outline auf **normative Definition**
      gehoben: Domänen- vs. Persistenz-Sicht getrennt; verweist auf
      `data-model.yaml` als maschinenlesbare Quelle; beschreibt die
      welle-1-Kerntabellen; benennt die **offenen Punkte** (siehe §6).
- [x] **ADR-0006** (relationales Schema-Design) als `Proposed`: per-Typ-
      Tabellen statt polymorpher `element`-Tabelle; `openings` →
      doors/windows als Class-Table-Inheritance; variable Geometrie als
      JSON (`polygon_json`/`footprint_json`/`params_json`); Undo-Stack
      persistiert (`undo_commands`, LH-QA-003). ADR-Index aktualisiert.
- [x] Domänen-Modell-**Drift** in §2 geschlossen: Wand-Auszug an den
      implementierten `Wall`/`Segment` (slice-003a) angeglichen
      (`start`/`end` statt `polyline`; Einzelsegment in welle-1).
- [x] `make gates` grün (docs-check: §2-Link auf `data-model.yaml` löst);
      Closure-Notiz.

## 3. Plan (vor Code)

| Datei / Komponente | Änderungs-Art | Begründung |
|---|---|---|
| `spec/data-model.yaml` | neu | logisches Persistenz-Schema (Single Source, maschinenlesbar) |
| `spec/spezifikation.md` §2 | Änderung | Outline → normative Definition; Domänen-/Persistenz-Split; Verweis |
| `docs/plan/adr/0006-relationales-schema-design.md` | neu | Schema-Design-Entscheidung (per-Typ vs. polymorph; JSON-Geometrie; Undo) |
| `docs/plan/adr/README.md` | Änderung | ADR-0006 in Index |

## 4. Trigger

- slice-003a done (implementiertes Domänen-Modell als Abgleich-Basis) ✓
- ADR-0003 liegt vor (Persistenz-Entscheidung, Proposed) ✓

## 5. Closure-Trigger

- DoD vollständig, `make gates` grün, ADR-0006 reviewed, Closure-Notiz.

## 6. Risiken und offene Punkte (im Slice zu benennen, nicht alle zu lösen)

- **`wall_types`-Tabelle vs. `WallType`-Enum:** Das Schema führt
  `wall_types` als parametrische **Typ-Bibliothek** (Name, Default-Stärke,
  `is_load_bearing`, Material); das Domänen-Modell hat das **Enum**
  `{Innen, Aussen, Trag}` als *Klassifikation* (LH-FA-WAL-007). Beide
  koexistieren (Klassifikation + Bibliotheks-Referenz) — finale Auflösung
  beim WAL-007-Slice.
- **LH-FA-BLD-004 (Projektversionierung) nicht abgedeckt:** Das Schema hat
  `undo_commands` (LH-QA-003), aber **keine** Versions-/History-Tabelle für
  „frühere Stände wiederherstellen". Offener Punkt → eigener Slice oder
  §2-Ergänzung. (Abgrenzung Undo ↔ Versionierung, vgl. Lastenheft.)
- **JSON-Geometrie** (`polygon_json` etc.) ist pragmatisch, aber nicht
  relational abfragbar — bewusst (variable Geometrie); in ADR-0006 begründet.
- **Sizing:** drei dichte DoD-Punkte (Artefakt + §2 + ADR). Falls zu groß:
  ADR-0006 + data-model.yaml zuerst, §2-Vollausbau als 007b.

## 7. Closure-Notiz

**Closure-Kriterien (beobachtbar):**
- `spec/data-model.yaml` ist **tool-validiert**: `d-migrate schema validate`
  (`ghcr.io/pt9912/d-migrate:0.9.7`) → *Validation passed*, 0 Fehler,
  0 Warnungen (17 Tabellen, 140 Spalten, 7 Indizes, 3 Constraints).
- `make gates` grün (docs-check löst §2→`data-model.yaml` + ADR-0006-Links).

**Lerneintrag:**
- **Format führt, nicht der Vorschlag:** Der ursprüngliche `.tmp/ddl.yaml`
  nutzte erfundene Konstrukte (`foreign_keys`/`unique_constraints`/top-level
  `indexes`, `decimal` ohne `precision`/`scale`). Erst der Abgleich gegen
  die d-migrate-Referenz (`schema-reference.md`) + ein realer
  `validate`-Lauf machten die Konformität belastbar.
- **`E017` — FK-Basistyp:** `identifier` ist dem Auto-Increment-PK
  vorbehalten; FK-Spalten tragen `integer`. Vom Tool gefunden, nicht durch
  Lesen — *Beleg schlägt Behauptung* (genau die Grenze, die im Review
  benannt war).
- **Domäne führt die Persistenz (ADR-0001):** Naming an den Kern
  angeglichen (`storeys`, `thickness_mm`); das implementierte
  Pflichtfeld `Wall.type` bekam eine Spalte (`walls.classification`),
  damit der Round-Trip erfüllbar ist.
- **Ehrliche Abgrenzung:** Forward-declared-Tabellen (DRW/Dokumente) und
  die bewusste `entity_layers`-Polymorphie sind als Ausnahmen markiert,
  nicht versteckt; LH-FA-BLD-004 (Versionierung) bleibt offen.

**Restrisiko / Nachfolge:** `wall_types`-Bibliothek ↔ `classification`
(WAL-007), LH-FA-BLD-004 (History), und die SQLite-Generierung
(`d-migrate schema generate`) + Migrationen im Persistenz-Slice
(BLD-002/003).

## 8. Sub-Area-Modus-Begründung

### Sub-Area: Spec-Schreibung

- **Modus:** GF — Doku führt (das Schema ist die Spezifikation, der Code folgt im Persistenz-Slice).
- **Konventionen-Dichte:** hoch — Spec-Stratifizierung (MR-001), §2 existiert.
- **Phase-Reife:** Phase 2→4 (Outline → kohärente Definition).
- **Evidenz-/Diskrepanz-Risiko:** mittel — Abgleich Schema ↔ implementiertes Domänen-Modell (Wall/Storey) muss stimmen.
- **Reconciliation-Aufwand:** dieser Slice schließt die §2-Drift; danach GF-Steady-State.
