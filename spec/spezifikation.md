# Spezifikation — b-cad

**Status:** Outline (Phase 2). **Letzte Änderung:** 2026-06-08.

**Bezug zum Lastenheft:** Diese Spezifikation präzisiert die in
[`lastenheft.md`](lastenheft.md) formulierten Anforderungen (`LH-*`-IDs).
Bei Konflikt gewinnt das Lastenheft. ADRs dürfen diese Spezifikation
schärfen, **nicht** das Lastenheft.

---

## 1. Algorithmen und Datenflüsse

### LH-FA-ROM-001.a — Raum-Autoerkennung

**Eingabe:** Menge der Wände eines Geschosses.
**Ausgabe:** Menge erkannter Räume; je Raum ein Polygon auf
**Innenkanten-Basis** im **Ring-Modell** (äußerer Ring + 0..n
Loch-Ringe) mit **Netto-Fläche** (ADR-0007).

**Auslösung:** Die Erkennung läuft **automatisch bei Modell-Mutation**
(Wand anlegen/ändern/löschen) im Service — „automatisch … when er
geschlossen wird" (LH-FA-ROM-001 Happy) braucht keinen manuellen
Abruf-Schritt; UI-Ereignisse sind Adapter-Belang. Eine
Abfrage-Schnittstelle (Driving-Port) liefert den zuletzt erkannten
Stand und löst selbst keine Erkennung aus.

**Schritte:**

1. Graph über Wand-Segmente: Knoten = Segment-Endpunkte
   (Punkt-Gleichheit über `GEOMETRY_TOLERANCE_MM`), Kanten = Segmente.
   *Welle-1-Einschränkung:* Schnittpunkte werden erst mit der
   Wandverschneidung (LH-FA-WAL-006) zu Knoten — bis dahin schließen
   nur endpunkt-verbundene Wandzüge Räume.
2. Geschlossene Kreise (minimale Zyklen) bestimmen.
3. Pro Zyklus das **Innenkanten-Polygon** ableiten: jede Kante um die
   halbe Wandstärke ihres Segments zum Zyklus-Inneren versetzt,
   benachbarte Offset-Geraden geschnitten (ADR-0007).
4. Verschachtelte Zyklen: der innere Zyklus erzeugt einen eigenen Raum
   (Innenkante nach innen); im umschließenden Raum wird die
   Außenkontur des inneren Zyklus als **Loch-Ring** geführt —
   Netto-Fläche = äußerer Ring minus Loch-Ringe (keine Doppelzählung
   der Fläche, LH-FA-ROM-001 Boundary).
5. Offene Wandzüge erzeugen keinen Raum (Negative).

**Degenerierte Zyklen — kein Fehlerfall:** Die Raumerkennung ist
**total** und wirft kein `E-GEO-002`. Zyklen, deren Innenkanten-Offset
kollabiert, erzeugen **keinen Raum** — gleiches Verhalten wie offene
Wandzüge, kein Fehler. Kollaps-Kriterium ist der
**Kantenrichtungs-Erhalt**: kehrt sich beim Offset die Richtung einer
Kante um, ist der Ring kein gültiges Raumpolygon (eine reine
Flächen-Prüfung „Netto-Fläche ≤ 0" genügt nicht — Doppel-Inversion in
beiden Achsen erzeugt ein Phantom-Polygon *positiver* Fläche). `E-GEO-002` (§4) bleibt mutierenden
Geometrie-Operationen vorbehalten (z. B. Extrusion, LH-FA-D3-001.a).

**Komplexität:** Vollerkennung pro Mutation ist Welle-1-Stand (kleine
Modelle); Zielkomplexität/inkrementelle Erkennung bleibt offener
Punkt (§7, M3).

### LH-FA-WAL-002.a — Parameter-Validierung und Klemmung

Numerische Bauteil-Parameter werden im Service gegen die Wertebereiche
aus §3 geprüft. Außerhalb des Bereichs: auf den nächsten Grenzwert
klemmen, `E-VAL-001` melden, Modell-Zustand bleibt gültig.

### LH-FA-D3-001.a — Extrusion (2D → 3D)

2D-Bauteile (Wände, Decken, Dach) werden über `GeometryKernelPort` zu
Solids extrudiert; Wandöffnungen für Türen/Fenster (LH-FA-DOR-004,
LH-FA-WIN-005) als boolesche Subtraktion. Parameteränderung triggert
inkrementellen Rebuild des betroffenen Solids (Echtzeit, LH-FA-D3-002).

### LH-FA-D3-002.a — Echtzeitaktualisierung (Benachrichtigungs-Vertrag)

Präzisiert LH-FA-D3-002; Mechanik-Entscheidung in ADR-0008.

**Auslösung und Synchronität:** Jede committete Modell-Mutation
(Geschoss anlegen, Wand anlegen, Wand-Parameter ändern) wird
**synchron im Mutationspfad** gemeldet — nach Abschluss aller
Post-Commit-Schritte (Raum-Re-Detektion, LH-FA-ROM-001.a), sodass ein
Beobachter im Callback einen vollständig konsistenten Stand abfragt.
Abgelehnte/verworfene Mutationen (`E-VAL-001` Rejected,
Null-Längen-Wand) melden nicht.

**Vertrag (Push-Notify, Pull-State):** Gemeldet werden `element_id`
und `op` (Vokabular wie OTel-Span `bcad.geometry.rebuild`, §5); den
aktualisierten Stand holt der Beobachter über die Abfrage-Ports
(Solid, Räume, Modell). Mehrere Meldungen pro Mutation sind zulässig
(z. B. Wand- plus Raum-Änderung; Mehr-Element-Updates nicht verbaut).

**Beobachter-Pflichten:** Mehrere Beobachter (2D-/3D-Sicht, OBJ-003)
über Registrierung (`subscribe`/`unsubscribe`); Callbacks dürfen
abfragen, aber keine Mutationen auslösen (Re-Entranz-Verbot); eine
werfende Beobachter-Implementierung kippt die committete Mutation
nicht und blockiert weitere Beobachter nicht (Kapselung im Service;
Sichtbarkeit der Fehler später über REQ-TEC-006-Telemetrie).

**Welle-1-Operationalisierung von „sichtbar":** Der Kern erfüllt
D3-002 bis zur Benachrichtigungs-/Abfrage-Grenze; die sichtbare
3D-Darstellung liefert der Viewer-Strang (Roadmap, `welle-1v-viewer`)
auf dieser Basis — der Lastenheft-Wortlaut bleibt benutzer-beobachtbar
und wird zusammen mit ACC-002 dort erfüllt.

## 2. Datenstrukturen und Schemas

Das Datenmodell hat **zwei Sichten**, die getrennt zu halten sind
(ADR-0001 — die Abhängigkeit zeigt nach innen):

1. **Domänen-Modell (Kern, Wahrheit)** — pure Werttypen in
   `src/hexagon/model/`, framework-frei. Quelle der Wahrheit für *was* ein
   Bauteil ist.
2. **Persistenz-Schema (Adapter-Abbildung)** — wie das Domänen-Modell in
   SQLite gespeichert wird (ADR-0003). **Es bildet das Domänen-Modell ab,
   treibt es nicht.**

### 2.1 Domänen-Modell (Kern)

Pure Werttypen in `src/hexagon/model/`, framework-frei. Implementiert
(slice-003a): `Building`, `Storey`, `Wall`, `Point2D`, `Segment`, `Solid`,
`WallType`. Wand-Auszug (Stand: Einzelsegment, welle-1):
`{ id, storey_id, start: Point2D, end: Point2D, thickness_mm, height_mm, type ∈ {Innen, Aussen, Trag} }`.

*Wandzüge/Polylines* (mehrere verbundene Segmente, LH-FA-WAL-001/006)
folgen als Erweiterung; `Storey` gewinnt später `level_index`/`elevation`
aus dem Persistenz-Schema (§2.2).

### 2.2 Persistenz-Schema (SQLite, ADR-0003 / ADR-0006)

Maschinenlesbare **Quelle der Wahrheit**: [`data-model.yaml`](data-model.yaml)
im **d-migrate Neutral-Format** (`schema_format: "1.0"`) — d-migrate
generiert daraus die dialekt-spezifische DDL (Ziel: SQLite, kein
hand-geschriebenes SQL). Design (per-Typ-Tabellen,
`openings`-Spezialisierung, JSON-Geometrie, persistierter Undo-Stack):
[ADR-0006](../docs/plan/adr/0006-relationales-schema-design.md).

Kerntabellen (welle-1) — vollständig in `data-model.yaml`:

| Tabelle | Inhalt |
|---|---|
| `projects` | Projekt-Metadaten, `file_version`, Einheit |
| `storeys` | Geschosse (`level_index`, `elevation_mm`, `height_mm`) |
| `walls` | Wände (Segment `start/end`, `thickness_mm`, `height_mm`, Typ/Material) |
| `rooms` | Räume (Polygon als `polygon_json`, Fläche/Volumen) |
| `openings` → `doors`/`windows` | Wandöffnungen mit 1:1-Spezialisierung |
| `materials`, `wall_types` | Material- und Wandtyp-Bibliothek |
| `undo_commands` | persistierter Undo-Stack (LH-QA-003) |

**Migrationsregel:** Schema-Version steigt monoton; jede Erhöhung braucht
eine getestete Aufwärts-Migration (vgl. `releasing.md`, ADR-0003).

**Offene Punkte:** (a) `wall_types`-Bibliothek vs. `WallType`-Enum
(Klassifikation, LH-FA-WAL-007) — Koexistenz, Auflösung im WAL-007-Slice;
(b) **LH-FA-BLD-004 Projektversionierung** (frühere Stände) ist **nicht**
im Schema (nur Undo) — eigener Slice.

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

- ~~Raumerkennung: Mittellinie vs. Innenkante als Polygon-Basis
  (beeinflusst Wohnflächenberechnung LH-FA-EVL-003)~~ → entschieden in
  ADR-0007 (Innenkante, Ring-Modell; §1).
- Performance-Zielkomplexität der Raumerkennung (M3).
- IFC-Schema-Version und -Bibliothek (ADR in welle-4-austausch).
- Zielplattformen (siehe `releasing.md`).

## 8. Historie

| Datum | Änderung | ADR |
|---|---|---|
| 2026-06-08 | Initiale Outline aus Lastenheft-Wertebereichen; Fehler-Codes und OTel-Span-Skelett | Greenfield-Bootstrap |
| 2026-06-11 | §1 LH-FA-ROM-001.a präzisiert: Innenkanten-Basis + Ring-Modell, Auslösung bei Modell-Mutation, Endpunkt-Knoten-Einschränkung (welle-1), Erkennung total (kein `E-GEO-002`); §7-Punkt Polygon-Basis geschlossen | ADR-0007 |
| 2026-06-11 | §1 Kollaps-Kriterium präzisiert: Kantenrichtungs-Erhalt statt reiner Flächen-Prüfung (Doppel-Inversion erzeugt Phantom-Polygon positiver Fläche) | ADR-0007 |
| 2026-06-11 | §1 LH-FA-D3-002.a ergänzt: Benachrichtigungs-Vertrag (Observer-Port, Push-Notify/Pull-State, Reihenfolge nach Re-Detektion, Beobachter-Pflichten) + welle-1-Operationalisierung „sichtbar" | ADR-0008 |

## 9. Technische Rahmenbedingungen (REQ-TEC)

Die technischen Rahmenbedingungen aus dem Domänen-Ursprung. Sie sind
**fortschreibbar** (technische Schicht) — ADRs dürfen sie schärfen, das
Lastenheft bleibt unberührt. ID-Klasse `REQ-TEC-<NNN>` deklariert in
[`../harness/conventions.md` MR-002](../harness/conventions.md#mr-002--id-schema-für-b-cad).

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
