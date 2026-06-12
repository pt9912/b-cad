# ADR-0006: Relationales Schema-Design des Gebäudemodells

**Status:** Accepted

**Datum:** 2026-06-09

**Autor:** Dietmar Burkard

**Bezug:** OBJ-003, ADR-0001 (Kern führt), ADR-0003 (SQLite-Persistenz), LH-FA-WAL/DOR/WIN/ROF/SLB/STR/ROM/MAT-*, LH-QA-003 (Undo)

---

## Kontext

Das durchgängige Gebäudemodell (OBJ-003) wird über `ProjectRepositoryPort`
in SQLite persistiert (ADR-0003). Offen war die **Form** des Schemas:
Wie werden die heterogenen Bauteile (Wände, Türen, Fenster, Dächer,
Decken, Treppen, generische Objekte) relational abgebildet? Der
Spezifikations-Outline (`spezifikation.md` §2, früher) skizzierte eine
*einzelne polymorphe* `element`-Tabelle. Der ausgearbeitete
Schema-Vorschlag (`spec/data-model.yaml`) wählt einen anderen Weg.

**Abgrenzung (ADR-0001):** Diese ADR entscheidet das **Persistenz-Schema**
(Adapter-Abbildung), nicht das Domänen-Modell. Der Kern
(`src/hexagon/model/`) bleibt die Wahrheit; das Schema bildet ihn ab.

## Entscheidung

1. **Per-Typ-Tabellen** statt einer polymorphen `element`-Tabelle: jede
   Bauteil-Art hat eine eigene Tabelle mit **expliziten, typisierten
   Spalten** (`walls`, `rooms`, `roofs`, `slabs`, `stairs`, `objects`).
2. **Class-Table-Inheritance für Öffnungen:** eine Basis-Tabelle
   `openings` (Position/Maße in der Wand) mit 1:1-Spezialisierungen
   `doors`/`windows` (`opening_id` unique). Bildet LH-FA-DOR-004/WIN-005
   (Wandöffnung) strukturell ab.
3. **Variable Geometrie als JSON-Spalten** (`polygon_json`,
   `footprint_json`, `params_json`): Raumpolygone, Dach-Footprints und
   generische Objekt-Parameter sind formvariabel und werden **nicht**
   relational normalisiert.
4. **Undo-Stack persistiert** (`undo_commands`, `command_index` +
   `payload_json`): erfüllt LH-QA-003 (≥1000 Schritte) auch über
   Sitzungs-/Projektgrenzen.
5. **Referentielle Integrität & Lösch-Politik:** *besitzende* Beziehungen
   (Projekt → Bauteil, Geschoss → Bauteil) `on_delete: cascade`; *geteilte*
   Referenzen (`material_id`, `wall_type_id`) und die Zwei-Geschoss-Spanne
   von `stairs` `on_delete: restrict` (kein stiller Datenverlust — Material/
   Geschoss erst löschbar, wenn nicht mehr referenziert). **Jede** FK trägt
   ein explizites `on_delete`; Indizes auf den Fremdschlüssel-Pfaden.
6. **Eine bewusste Ausnahme — Layer-Zuordnung polymorph:** `entity_layers`
   ordnet Layer cross-cutting über *N* Bauteiltypen zu; je-Typ-Junction-
   Tabellen würden explodieren. Daher eine **polymorphe Assoziation**
   (`entity_type` + `entity_id`, ohne FK auf `entity_id`) — die *einzige*
   Abweichung von #1/#5. Integrität app-seitig (der Persistenz-Adapter
   validiert `entity_type`/`entity_id` beim Schreiben). Bewusst, weil das
   Tagging-Muster sonst quadratisch in der Bauteil-Typen-Zahl wächst.

**Schema-Autoring & Generierung.** Das Schema wird im **d-migrate
Neutral-Format** (`schema_format: "1.0"`; GitHub
[`pt9912/d-migrate`](https://github.com/pt9912/d-migrate),
`spec/schema-reference.md`) in <!-- d-check:ignore (Datei im d-migrate-Repo) -->
[`spec/data-model.yaml`](../../../spec/data-model.yaml) gepflegt;
**d-migrate generiert** daraus die dialekt-spezifische DDL (Ziel: SQLite,
ADR-0003) — kein hand-geschriebenes SQL. Damit ist das Schema
dialekt-portabel (Postgres/MySQL als spätere Optionen), und die
Migrationsdisziplin (ADR-0003) hängt an *einem* maschinenlesbaren Modell.
Format-Pflichten u. a.: `decimal` mit `precision`/`scale`; FKs als
Spalten-`references`; Uniques/Indizes als `constraints`/`indices`.
FK-Spalten tragen den **`integer`-Basistyp** (nicht `identifier`, der dem
Auto-Increment-PK vorbehalten ist — sonst `E017`). Konformität ist mit
`d-migrate schema validate` (`ghcr.io/pt9912/d-migrate:0.9.7`) belegt:
0 Fehler, 0 Warnungen.

## Verglichene Alternativen

### Option A — Per-Typ-Tabellen (gewählt)

- Pro: typsichere Spalten (kein „alles ist `params_json`"); klare
  Constraints/Indizes; Auswertungen (Flächen/Mengen, EVL) ohne JSON-
  Parsing; nah am Domänen-Modell (ein Typ ↔ eine Tabelle).
- Contra: mehr Tabellen; ein neuer Bauteil-Typ braucht Schema-Migration.

### Option B — Polymorphe `element`-Tabelle (Single-Table)

- Pro: ein Bauteil-Typ mehr = nur ein neuer `type`-Wert, keine Migration.
- Contra: Spalten entweder breit-nullable oder alles in JSON → keine
  Typsicherheit, schwache Constraints, EVL-Auswertung nur über JSON.

### Option C — EAV (Entity-Attribute-Value)

- Pro: maximal flexibel.
- Contra: für ein CAD-Modell mit festen Bauteil-Schemata Over-Engineering;
  unprüfbar, langsam, gegen die Auditierbarkeit.

## Konsequenzen

- Positiv: Auswertungen (LH-FA-EVL-*) und Constraints ruhen auf typisierten
  Spalten; das Schema spiegelt die Domäne 1:1.
- Negativ / Folgepflicht: neuer Bauteil-Typ = Schema-Migration (ADR-0003
  Migrationsdisziplin); JSON-Geometrie ist nicht relational abfragbar
  (bewusst, da formvariabel).
- **Round-Trip `Wall.type`:** das implementierte Domänen-Pflichtfeld
  (WallType-Enum) wird als Spalte `walls.classification` persistiert — der
  Round-Trip Domain ↔ SQLite ist damit erfüllt (Review-Finding 1).
- **Offen (nicht hier entschieden):** (a) die *Verzahnung* der
  `walls.classification` mit der parametrischen `wall_types`-Bibliothek
  (LH-FA-WAL-007) — Auflösung im WAL-007-Slice; (b) LH-FA-BLD-004
  Projektversionierung (History) ist **nicht** Teil dieses Schemas (nur
  Undo, LH-QA-003).

## Fitness Function

| Tooling | Regel | Make-Target |
|---|---|---|
| Schema↔Modell-Drift | Persistenz-Adapter-Tests prüfen Round-Trip Domain ↔ SQLite gegen `spec/data-model.yaml` | `make test` (im Persistenz-Slice BLD-002/003) |
| Schema-Versionierung | monoton steigende Version + getestete Aufwärts-Migration | `make test` (ADR-0003) |

## Re-Evaluierungs-Trigger

- Wenn formvariable JSON-Geometrie doch relational abgefragt werden muss
  (z. B. räumliche Indizes) → Teil-Normalisierung neu bewerten.
- Wenn die Bauteil-Typen-Zahl so wächst, dass Per-Typ-Migrationen zur Last
  werden → Option B für Rand-Typen neu bewerten.

## Geschichte

| Datum | Ereignis | Verweis |
|---|---|---|
| 2026-06-09 | Proposed (aus `spec/data-model.yaml`, slice-007) | slice-007 |
| 2026-06-09 | Accepted — `d-migrate schema validate` (0.9.7) grün (0 Fehler); 6 Review-Findings + `E017` (FK-Basistyp) behoben | slice-007 |
