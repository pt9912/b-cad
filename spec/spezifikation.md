# Spezifikation — b-cad

**Status:** Outline (Phase 2). **Letzte Änderung:** 2026-06-08.

**Bezug zum Lastenheft:** Diese Spezifikation präzisiert die in
[`lastenheft.md`](lastenheft.md) formulierten Anforderungen (`LH-*`-IDs).
Bei Konflikt gewinnt das Lastenheft. ADRs dürfen diese Spezifikation
schärfen, **nicht** das Lastenheft.

---

## 1. Algorithmen und Datenflüsse

### LH-FA-ROM-001.a — Raum-Autoerkennung

**Eingabe:** Menge der Wandzüge eines Geschosses.
**Ausgabe:** Menge erkannter Räume (geschlossene Polygone) mit Fläche.

**Schritte (Outline):**

1. Wand-Mittellinien (oder Innenkanten, siehe offener Punkt) als
   Graph aufbauen: Knoten = Endpunkte/Schnittpunkte, Kanten = Segmente.
2. Geschlossene Kreise (minimale Zyklen) bestimmen.
3. Pro Zyklus ein Raumpolygon; verschachtelte Zyklen als innen/außen
   trennen (keine Doppelzählung der Fläche, LH-FA-ROM-001 Boundary).
4. Offene Wandzüge erzeugen keinen Raum (Negative).

**Komplexität:** Zielwert für M3 zu spezifizieren; offener Punkt §7.
**Fehlermodi:** degenerierte Geometrie → `E-GEO-002`.

### LH-FA-WAL-002.a — Parameter-Validierung und Klemmung

Numerische Bauteil-Parameter werden im Service gegen die Wertebereiche
aus §3 geprüft. Außerhalb des Bereichs: auf den nächsten Grenzwert
klemmen, `E-VAL-001` melden, Modell-Zustand bleibt gültig.

### LH-FA-D3-001.a — Extrusion (2D → 3D)

2D-Bauteile (Wände, Decken, Dach) werden über `GeometryKernelPort` zu
Solids extrudiert; Wandöffnungen für Türen/Fenster (LH-FA-DOR-004,
LH-FA-WIN-005) als boolesche Subtraktion. Parameteränderung triggert
inkrementellen Rebuild des betroffenen Solids (Echtzeit, LH-FA-D3-002).

## 2. Datenstrukturen und Schemas

### Projektdatei

Persistiert über `ProjectRepositoryPort` (SQLite, ADR-0003). Schema
(Outline, wird im Persistenz-Slice geschärft):

| Tabelle | Inhalt |
|---|---|
| `project` | Projekt-Metadaten, Schema-Version |
| `storey` | Geschosse (Höhe, Reihenfolge) |
| `element` | Bauteile (Typ, Geschoss, Parameter, Geometrie-Referenz) |
| `material` | Materialbibliothek-Einträge (U-Wert, Kostenkennwert) |
| `history` | Änderungshistorie (LH-FA-BLD-004) |

**Migrationsregel:** Schema-Version steigt monoton; jede Erhöhung
braucht eine getestete Aufwärts-Migration (vgl. `releasing.md`).

### Bauteil (Domain-Modell, Auszug)

Pure Werttypen in `src/hexagon/model/`, framework-frei. Konkrete
Felder werden pro Bauteil-Slice ergänzt; Beispiel Wand:
`{ id, storeyId, polyline, thickness_mm, height_mm, type∈{innen,außen,trag} }`.

## 3. Defaults und Konstanten

| Name | Wert | Begründung | ADR / Bezug |
|---|---|---|---|
| `WALL_THICKNESS_MIN_MM` | 50 | Untergrenze Wandstärke | LH-FA-WAL-002 |
| `WALL_THICKNESS_MAX_MM` | 1000 | Obergrenze Wandstärke | LH-FA-WAL-002 |
| `WALL_HEIGHT_MIN_MM` | 500 | Untergrenze Wandhöhe | LH-FA-WAL-003 |
| `WALL_HEIGHT_MAX_MM` | 10000 | Obergrenze Wandhöhe | LH-FA-WAL-003 |
| `DEFAULT_WALL_THICKNESS_MM` | 240 | Default-Wandstärke bei Wand-Anlage (typ. Außenwand 24 cm) | LH-FA-WAL-001 |
| `DEFAULT_STOREY_HEIGHT_MM` | 2500 | Default-Geschosshöhe bei Projekt/Geschoss-Anlage | LH-FA-BLD-001, LH-FA-FLR-004 |
| `GEOMETRY_TOLERANCE_MM` | 0.1 | Toleranz für Punkt-Gleichheit / Wandverbindung | LH-FA-WAL-006, LH-FA-ROM-001 |
| `AUTOSAVE_INTERVAL_S` | 300 | Autosave-Intervall | LH-QA-004 |
| `UNDO_DEPTH_MIN` | 1000 | Mindesttiefe Undo/Redo | LH-QA-003 |
| `PROJECT_OPEN_BUDGET_S` | 3 | Performance-Budget Projektöffnung (Standardprojekt) | LH-QA-001 |
| `MEMORY_BUDGET_GB` | 2 | RAM-Budget Standardprojekt | LH-QA-002 |
| `SUPPORTED_LOCALES` | `de`, `en` | Mehrsprachigkeit | LH-QA-006 |

Die Default-**Wandhöhe** bei Anlage ist die **Höhe des Geschosses**
(parametrisch, kein eigener Konstant; LH-FA-WAL-001). `slice-003a` hat
`DEFAULT_WALL_THICKNESS_MM` ergänzt (zuvor Spec-Lücke: das Lastenheft
fordert eine Default-Stärke, §3 nannte keinen Wert).

## 4. Fehler-Codes und Logging-Felder

| Code | Bedingung | Aktion |
|---|---|---|
| `E-IO-001` | Kein Schreibrecht im Zielpfad (Projekt anlegen/speichern) | Fehlerdialog, kein Zustandsverlust, Log `event=io_no_permission` |
| `E-IO-002` | Zielmedium voll / Schreibfehler | vorheriger Stand intakt (atomar), Log `event=persist_error` |
| `E-IO-003` | Import-Format nicht erkannt / invalide | kein Teil-Import, Log `event=import_rejected` |
| `E-VAL-001` | Parameter außerhalb des Wertebereichs | auf Grenzwert geklemmt, Hinweis, Log `event=validation_rejected` |
| `E-GEO-001` | Eingabe außerhalb des Zeichenbereichs | abgelehnt, Log `event=geometry_out_of_range` |
| `E-GEO-002` | Geometrie-Operation fehlgeschlagen / degeneriert | Operation rückgängig, Modell unverändert, Log `event=geometry_error` |
| `E-PLG-001` | Plugin-Fehlverhalten | Plugin isoliert/entladen, Modell unverändert, Log `event=plugin_error` |

## 5. Metriken und Tracing-Felder

OTel-Spans (Pflicht-Attribute werden pro Slice geschärft, ADR-Folge):

| Span | Pflicht-Attribute (Outline) | Quelle |
|---|---|---|
| `bcad.project.save` | `element_count`, `duration_ms`, `atomic_replace` | LH-FA-BLD-002, LH-QA-005 |
| `bcad.project.open` | `element_count`, `duration_ms` | LH-QA-001 |
| `bcad.geometry.rebuild` | `element_id`, `op`, `duration_ms` | LH-FA-D3-002 |
| `bcad.room.detect` | `wall_count`, `room_count`, `duration_ms` | LH-FA-ROM-001 |
| `bcad.io.exchange` | `format`, `direction`, `element_count`, `result` | LH-FA-IO-* |

Jeder App-Span trägt zusätzlich `requirement.id` (die bediente `LH-*`-ID)
für die Traceability-Kette Span → Slice → ADR → Anforderung.

**Abgrenzung.** Diese Tabelle beschreibt die **Laufzeit-Telemetrie der
Anwendung** (REQ-TEC-006). Die **Agenten-Lauf-Telemetrie** (Trace eines
Plan→Implement→Review→Verify-Durchlaufs mit `slice.id`, `agent.role`,
`tokens.*`, `cache.*` — Kurs-Modul 15) ist ein **separates, späteres**
Artefakt unter `otel/` und entsteht mit dem ersten realen Agentenlauf,
nicht im Bootstrap.

## 6. Externe Verträge

| System | Version | Vertrag (Outline) |
|---|---|---|
| OpenCascade | im Adapter gepinnt (ADR-0002) | `GeometryKernelPort` — Solids, boolesche Operationen, Extrusion |
| Qt | 6.x (REQ-TEC-002) | GUI-Adapter, keine Kern-Bindung |
| SQLite | im Adapter gepinnt (ADR-0003) | `ProjectRepositoryPort`, atomar |
| IFC | Schema-Version offen (ADR-Folge) | `ModelImporterPort`/`ModelExporterPort` |

## 7. Offene Punkte

- Raumerkennung: Mittellinie vs. Innenkante als Polygon-Basis
  (beeinflusst Wohnflächenberechnung LH-FA-EVL-003).
- Performance-Zielkomplexität der Raumerkennung (M3).
- IFC-Schema-Version und -Bibliothek (ADR in welle-4-austausch).
- Zielplattformen (siehe `releasing.md`).

## 8. Historie

| Datum | Änderung | ADR |
|---|---|---|
| 2026-06-08 | Initiale Outline aus Lastenheft-Wertebereichen; Fehler-Codes und OTel-Span-Skelett | Greenfield-Bootstrap |

## 9. Technische Rahmenbedingungen (REQ-TEC)

Die technischen Rahmenbedingungen aus dem Domänen-Ursprung. Sie sind
**fortschreibbar** (technische Schicht) — ADRs dürfen sie schärfen, das
Lastenheft bleibt unberührt. ID-Klasse `REQ-TEC-<NNN>` deklariert in
[`../harness/conventions.md` MR-002](../harness/conventions.md#mr-002-id-schema-für-b-cad).

| ID | Rahmenbedingung | Wahl | ADR / Bezug |
|---|---|---|---|
| REQ-TEC-001 | Sprache | C++20 | ADR-0001 |
| REQ-TEC-002 | GUI | Qt 6 | ADR-Folge (GUI-Adapter) |
| REQ-TEC-003 | Geometrie-Kern | OpenCascade | ADR-0002 |
| REQ-TEC-004 | Build | CMake | ADR-0001 §CMake-Targets |
| REQ-TEC-005 | Tests | GoogleTest | — |
| REQ-TEC-006 | Logging/Observability | OpenTelemetry | §5, ADR-Folge |
| REQ-TEC-007 | Persistenz | SQLite | ADR-0003 |
| REQ-TEC-008 | Plugin-Architektur | Shared Libraries | ADR-Folge (Plugin-API) |
| REQ-TEC-009 | Containerisierung | Docker DevContainer | ADR-Folge (Modul 14) |
