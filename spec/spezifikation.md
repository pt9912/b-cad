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
   *Welle-1-Einschränkung:* Schnittpunkte werden erst mit dem
   **WAL-006-Vollumfang** zu Knoten (der slice-012-Teilumfang behandelt
   nur Endpunkt-Ecken) — bis dahin schließen nur endpunkt-verbundene
   Wandzüge Räume.
2. Geschlossene Kreise (**minimale Zyklen**) über planare
   Flächen-Traversierung bestimmen (winkelsortierte Halbkanten je
   Knoten): **geteilte Knoten** (Grad ≥ 3 — Räume mit gemeinsamer
   Wand) sind abgedeckt; Stichkanten (Sackgassen) werden im Umlauf
   ignoriert (die Wand bleibt im Modell, nur das Raumpolygon
   übergeht den Stich).
3. Pro Zyklus das **Innenkanten-Polygon** ableiten: jede Kante um die
   halbe Wandstärke ihres Segments zum Zyklus-Inneren versetzt,
   benachbarte Offset-Geraden geschnitten (ADR-0007).
   *Welle-1-Näherung:* bei **kollinearen Nachbarkanten ungleicher
   Stärke** springt die Ecke auf den Offset-Punkt der Folgekante
   (lineare Überblendung statt exakter Stufenkontur); die exakte
   Stufe kommt mit dem **WAL-006-Vollumfang** (nicht Teil des
   slice-012-Teilumfangs).
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

### LH-FA-WAL-006.a — Eckenschluss (Footprint-Regel, Teilumfang)

Präzisiert den LH-FA-WAL-006-Teilumfang (Lastenheft 0.1.2,
slice-012). Die **Footprint-Hoheit liegt im Kern**: der Grundriss
einer Wand ist ein Polygon, das der Kern aus Segment, Stärke und
Nachbar-Wissen ableitet; der Geometrie-Adapter extrudiert/tesselliert
nur noch das Polygon (Prisma in +Z auf Wandhöhe).

**Eck-Konstruktion:** An einem Endpunkt, den im **selben Geschoss**
genau **eine** weitere Wand teilt (Grad-2-Knoten; Punkt-Gleichheit
über `GEOMETRY_TOLERANCE_MM`), werden die beidseitigen Seitenkanten
der zwei Wände **im Schnittpunkt verbunden** — Prinzip analog der
Innenkanten-Konstruktion der Raumerkennung (ADR-0007), hier auf die
Wand-Außenkontur angewandt. Beide Wände enden an denselben
Eck-Punkten (nahtlos, keine Überlappung, keine Kerbe).

**Begrenzung und Rückfälle (total, wirft nie):** Ragt ein Eck-Punkt
weiter als `WALL_MITER_LIMIT` (§3) über den gemeinsamen Endpunkt
hinaus (sehr spitzer Winkel) oder sind die Richtungen kollinear
(keine Schnittpunkte), enden die Wände **stumpf** (Segment × Stärke,
wie vor slice-012). Grad ≠ 2 (freies Ende, T-Stoß/Stern) → stumpf.
Die Eck-Volumina wandern dabei zwischen den Wänden; Auswertungen
(LH-FA-EVL-*) müssen auf der **Footprint-Fläche** aufsetzen
(Shoelace), nicht auf Länge·Stärke.

### LH-FA-WAL-002.a — Parameter-Validierung und Klemmung

Numerische Bauteil-Parameter werden im Service gegen die Wertebereiche
aus §3 geprüft. Außerhalb des Bereichs: auf den nächsten Grenzwert
klemmen, `E-VAL-001` melden, Modell-Zustand bleibt gültig.

### LH-FA-D3-001.a — Extrusion (2D → 3D)

2D-Bauteile (Wände, Decken, Dach) werden über `GeometryKernelPort` zu
Solids extrudiert; Wandöffnungen für Türen/Fenster (LH-FA-DOR-004,
LH-FA-WIN-005) als boolesche Subtraktion. Parameteränderung triggert
inkrementellen Rebuild des betroffenen Solids (Echtzeit, LH-FA-D3-002).

### LH-FA-DOR-004.a / LH-FA-WIN-005.a — Wandöffnung (Schnitt-Prismen)

Präzisiert die automatische Wandöffnung für Türen (LH-FA-DOR-*) und
Fenster (LH-FA-WIN-*); Modell-Entscheidung in ADR-0011.

**Hosting & Position.** Eine Öffnung ist ein **eigenständiges
Domänen-Element** mit Referenz auf ihre Wirtswand (`wall_id`). Ihre Lage
entlang der Wandachse ist `offset_mm` = Abstand vom **Wand-Startpunkt**
entlang der Wand-Mittellinie bis zur **Nahkante** der Öffnung; die
Öffnung belegt den Achsenbereich `[offset, offset + width]`. Vertikal
belegt sie `[sill, sill + height]` über der Geschoss-Standfläche
(`sill = 0` für Türen, LH-FA-WIN-004 für Fenster). Quer durchsetzt sie
die **volle Wandstärke**.

**Schnitt-Prisma (Kern-Hoheit).** Der Kern berechnet je Öffnung ein
**Schnitt-Prisma** als reine Werte (Polygon + z-Bereich, dieselbe
Werte-Sprache wie das Footprint-Polygon, LH-FA-WAL-006.a): ein Rechteck
quer zur Wand an der Öffnungs-Position, das die Wandstärke voll (mit
geringem Überstand ≥ `GEOMETRY_TOLERANCE_MM` je Seite für einen
sauberen Schnitt) durchsetzt, extrudiert über `[sill, sill + height]`.
Der `GeometryKernelPort` extrudiert das Wand-Footprint-Polygon und
**subtrahiert die Schnitt-Prismen** (OCC-Boolean, ADR-0002) → Netto-
Wand-Solid; der Adapter kennt keine Öffnungs-Semantik (ADR-0011).

**Klemmung (LH-FA-WAL-002.a, `E-VAL-001`).** Breite/Höhe/Brüstung werden
gegen §3 geklemmt; reicht `sill + height` über die Wandhöhe, wird die
Öffnungs-Oberkante auf die Wandhöhe geklemmt. Die Position wird so
begrenzt, dass `[offset, offset + width]` vollständig in der Wandlänge
liegt; passt selbst die Mindestbreite (§3) nicht in die Wirtswand oder
fehlt eine Wirtswand, wird die Platzierung **abgelehnt** (`E-VAL-001`,
kein Durchbruch außerhalb der Wand, keine verwaiste Öffnung).

**Totalität & Transaktion.** Das Netto-Solid entsteht **vor dem
Commit**; eine fehlgeschlagene/degenerierte Subtraktion meldet
`E-GEO-002` (§4), das Modell bleibt unverändert und es ergeht keine
Meldung (Muster slice-012).

**Folge-Meldung.** Eine Öffnungs-Mutation (anlegen/verschieben/Parameter/
löschen) meldet `op = WallGeometryChanged` für die **Wirtswand**
(§5-Vokabular, kein neuer `op`) — die Öffnung ist ein Hohlraum im
Wand-Solid, kein eigenes Solid (welle-2). Es findet **keine
Raum-Re-Detektion** statt: eine Öffnung ändert weder Wandachse noch
Wandstärke, daher bleiben Raumerkennung (ADR-0007) und Footprint/
Eckenschluss (LH-FA-WAL-006.a) unberührt; die ROM- und WAL-006-AK-Tests
bleiben textlich unverändert grün. Abgelehnte Mutationen melden nicht.

### LH-FA-ROF-001.a — Dach-Geometrie (Teilumfang Rechteck-Grundriss)

Sammelblock, deckt **LH-FA-ROF-001..005** (Sattel/Walm/Pult, Neigung,
Überstand); Reifephase-Teilumfang welle-2 (Lastenheft 0.1.4). Modell-
Einordnung über ADR-0011 (#6, Bauteil-Erweiterungs-Muster) — keine
eigene Grundsatz-ADR; die Geometrie wird **hier** normativ festgelegt.

**Grundriss-Herkunft (welle-2):** Das Dach hat einen **expliziten
rechteckigen Grundriss** `b × t` (Parameter; persistiert als
`roofs.footprint_json`, ADR-0006). Die Auto-Ableitung aus dem
Geschoss-Wandumriss bleibt späterer Ausbau (keine Kopplung an
Wand-Mutationen in welle-2).

**Traufrechteck:** der Grundriss `b × t` wird ringsum um den Überstand
`o` (LH-FA-ROF-005) zum **Traufrechteck** `(b+2o) × (t+2o)` vergrößert;
das Dach kragt also um `o` über den Grundriss hinaus.

**Konstruktion je Typ** (Neigung `p`, LH-FA-ROF-004):

1. **Pult (ROF-003):** eine Fläche, von der hohen Traufkante zur
   gegenüberliegenden niedrigen geneigt; Firsthöhe (Hochkante) =
   `(t+2o) · tan(p)`.
2. **Sattel (ROF-001):** First **mittig entlang der längeren
   Traufrechteck-Achse**; zwei symmetrisch geneigte Flächen; Firsthöhe =
   `(kürzere Traufrechteck-Seite / 2) · tan(p)`.
3. **Walm (ROF-002):** wie Sattel, zusätzlich an den beiden Giebelseiten
   geneigte (abgewalmte) Flächen; der First ist beidseitig um den
   **Einrückbetrag** `e = Firsthöhe / tan(p)` (= halbe kürzere Seite,
   gleiche Neigung am Giebel) eingerückt — deterministisch aus Neigung
   und Giebelbreite, keine freie Größe.

**Firsthöhe abgeleitet:** `roofs.height_mm` (nullable, ADR-0006) ist
**nicht** Eingabe, sondern die aus `p`/Überstand berechnete Firsthöhe.

**Klemmung/Totalität:** Neigung `p` auf `[ROOF_PITCH_MIN_DEG,
ROOF_PITCH_MAX_DEG]`, Überstand `o` auf `[ROOF_OVERHANG_MIN_MM,
ROOF_OVERHANG_MAX_MM]` geklemmt (`E-VAL-001`, §3). Ein nicht-rechteckiger
oder degenerierter Grundriss (Seite < `GEOMETRY_TOLERANCE_MM`) erzeugt
**kein** Dach — die Sicht-Query bleibt total (kein Wurf).

**Folge-Meldung:** Eine Dach-Mutation (Anlage/Neigung/Überstand/Form/
Entfernen) meldet `op = RoofChanged` **storey-bezogen** (neuer `op` im
D3-002.a-Vokabular, ADR-0011 #6 neuer Bauteil-Typ); der Beobachter lädt
die Dächer des Geschosses neu (`ViewModelPort.roofMeshes`). **Keine
`RoomsChanged`** (Dächer berühren die Raumerkennung nicht). Das
Dach-Netz entsteht **analytisch im Kern** (`roof_geometry`), nicht über
OCC — der ADR-0009-Vertrag „framework-freie Netze über `ViewModelPort`"
bleibt erfüllt (die OCC-Tessellation gilt dem extrudierten Wand-Solid).

### LH-FA-SLB-001.a — Platten-Geometrie (Decken & Fundament)

Sammelblock, deckt **LH-FA-SLB-001..003** (Decken) **und LH-FA-FND-001..003**
(Fundament/Bodenplatte). Modell-Einordnung über ADR-0011 (#6); keine
eigene Grundsatz-ADR. Beide sind **horizontale Platten** und liegen im
Schema gemeinsam in `slabs` (Diskriminator `slab_type`, ADR-0006).

**Platten-Solid:** Eine Platte ist ein **Grundriss-Polygon, vertikal um
die Dicke extrudiert** — ein flaches Prisma. Geometrisch dasselbe
Vokabular wie die Wand-Footprint-Extrusion (`GeometryKernelPort`,
slice-012), hier auf eine horizontale Platte angewandt; eigenständig
begründet (nicht das Dach-Verfahren — das Dach ist ein analytisches
Polyeder, LH-FA-ROF-001.a).

**Aufstandshöhe `base_z` je `slab_type`** (das `slabs`-Schema trägt
**keine** base_z-Spalte — sie folgt aus Typ/Geschoss): **Decke** =
Oberkante des zugeordneten Geschosses (`storey_id`); **Bodenplatte**
(`FND-003`) = Oberkante auf Höhe 0; **Fundament** (`FND-001/002`) = unter
Gelände, die Fundamenttiefe erstreckt sich von 0 **nach unten**. Die
Dicke/Tiefe ist auf §3 geklemmt (`E-VAL-001`).

**Ausschnitte (`SLB-003`):** als **boolesche Subtraktion** von
Schnitt-Prismen aus dem Platten-Solid — Wiederverwendung von
`model::CutPrism` und des OCC-Boolean-Backends (ADR-0002), wie bei den
Wandöffnungen (LH-FA-DOR-004.a). Ein Ausschnitt wird auf den
Platten-Umriss begrenzt.

**Totalität:** degenerierter/leerer Grundriss (Fläche <
`GEOMETRY_TOLERANCE_MM²`) → keine Platte; die Sicht-Query bleibt total
(kein Wurf); ein fehlgeschlagener mutierender Solid-Bau meldet
`E-GEO-002`.

**Port-Mechanik (slice-015b, entschieden):** der bestehende
`extrudeFootprint`/`tessellateFootprint` (extrudiert ab z=0) bleibt
**unverändert** — das Volumen ist z-invariant, und der Kern **verschiebt
das fertige Platten-Netz um `base_z`** (reine Mesh-Translation, NACH dem
Ausschnitt-Boolean). Damit ist **kein** Port-Signatur-Eingriff nötig
(ADR-0001-Kern-Hoheit gewahrt ohne Migration; die Cutout-`CutPrism`s
liegen relativ zum Solid `[0,Dicke]`, nicht bei `base_z`).

**Folge-Meldung:** Eine Platten-Mutation (Anlage/Dicke/Ausschnitt/
Entfernen) meldet `op = SlabChanged` **storey-bezogen** (neuer `op` im
D3-002.a-Vokabular, ADR-0011 #6); der Beobachter lädt die Platten neu
(`ViewModelPort.slabMeshes`). **Keine `RoomsChanged`** (Platten berühren
die Raumerkennung nicht).

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

**Mehr-Element-Update (Eckenschluss, LH-FA-WAL-006.a/slice-012):**
Ändert eine Mutation die Eck-Geometrie von Nachbar-Wänden
(Wand-Anlage → Grad-Wechsel; Stärke-Änderung → Eck-Schnittpunkte),
wird **jede tatsächlich geänderte Nachbar-Wand einzeln** mit
`op = WallGeometryChanged` gemeldet (der Wert gehört zum
§5-Span-Vokabular). **Reihenfolge deterministisch:** auslösende
Wand-Op → Nachbar-Meldungen einzeln → `RoomsChanged`.
Höhen-Änderungen lassen Footprints unberührt und erzeugen keine
Nachbar-Meldung; unveränderte Footprints werden nicht gemeldet
(keine Über-Meldung). Die transaktionale Garantie umfasst den
gesamten Satz: alle neuen Solids (Wand + Nachbarn) entstehen vor dem
Commit — schlägt eines fehl, bleibt das Modell unverändert und es
ergeht keine Meldung.

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

**Welle-1v-Operationalisierung von „sichtbar" (ADR-0009):**
„Sichtbar" heißt: ein Qt-6-Widgets-3D-Fenster zeigt den
**extrudierten Stand als tesselliertes Netz**, bezogen über den
`ViewModelPort` (framework-freie Dreiecksnetze je `element_id`;
Erzeugung per OCC-Tessellation hinter dem `GeometryKernelPort` —
kein OCC in der GUI). Die Darstellung folgt committeten Mutationen
über den D3-002.a-Vertrag ohne Benutzer-Refresh: der
Viewer-Callback pullt und plant ein Repaint, löst nie Mutationen
aus (Re-Entranz-Verbot strukturell eingehalten). Display-freie
Prüfbarkeit über das **Szenen-Surrogat** (gehaltene Netze +
Zähler wirksamer Szenen-Updates); der benutzer-beobachtbare
ACC-002-Beleg entsteht als manueller Abnahme-Schritt
(`make acc-002-beleg`, kein Gate).

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
| `WALL_MITER_LIMIT` | max(Stärke_A, Stärke_B) | Eckenschluss-Begrenzung: maximales Auskragen der Eck-Geometrie über den gemeinsamen Endpunkt; darüber stumpfes Ende (Formel, keine feste Zahl) | LH-FA-WAL-006 |
| `DOOR_WIDTH_MIN_MM` | 600 | Untergrenze Türbreite | LH-FA-DOR-003 |
| `DOOR_WIDTH_MAX_MM` | 2000 | Obergrenze Türbreite | LH-FA-DOR-003 |
| `DOOR_HEIGHT_MIN_MM` | 1800 | Untergrenze Türhöhe | LH-FA-DOR-003 |
| `DOOR_HEIGHT_MAX_MM` | 2500 | Obergrenze Türhöhe | LH-FA-DOR-003 |
| `WINDOW_WIDTH_MIN_MM` | 300 | Untergrenze Fensterbreite | LH-FA-WIN-003 |
| `WINDOW_WIDTH_MAX_MM` | 3000 | Obergrenze Fensterbreite | LH-FA-WIN-003 |
| `WINDOW_HEIGHT_MIN_MM` | 300 | Untergrenze Fensterhöhe | LH-FA-WIN-003 |
| `WINDOW_HEIGHT_MAX_MM` | 2500 | Obergrenze Fensterhöhe | LH-FA-WIN-003 |
| `WINDOW_SILL_MIN_MM` | 0 | Untergrenze Brüstungshöhe | LH-FA-WIN-004 |
| `WINDOW_SILL_MAX_MM` | 2000 | Obergrenze Brüstungshöhe | LH-FA-WIN-004 |
| `DEFAULT_DOOR_WIDTH_MM` | 900 | Default-Türbreite bei Anlage | LH-FA-DOR-001 |
| `DEFAULT_DOOR_HEIGHT_MM` | 2100 | Default-Türhöhe bei Anlage | LH-FA-DOR-001 |
| `DEFAULT_WINDOW_WIDTH_MM` | 1200 | Default-Fensterbreite bei Anlage | LH-FA-WIN-001 |
| `DEFAULT_WINDOW_HEIGHT_MM` | 1300 | Default-Fensterhöhe bei Anlage | LH-FA-WIN-001 |
| `DEFAULT_WINDOW_SILL_MM` | 900 | Default-Brüstungshöhe bei Anlage | LH-FA-WIN-001/004 |
| `OPENING_CUT_OVERSHOOT_MM` | 1 | Überstand des Öffnungs-Schnittkörpers über die Wandgrenzen (lateral je Seite + Boundary-Höhen) für einen koplanar-freien Boolean; volumen-neutral (≥ `GEOMETRY_TOLERANCE_MM`) | LH-FA-DOR-004/WIN-005 |
| `ROOF_PITCH_MIN_DEG` | 5 | Untergrenze Dachneigung | LH-FA-ROF-004 |
| `ROOF_PITCH_MAX_DEG` | 60 | Obergrenze Dachneigung | LH-FA-ROF-004 |
| `ROOF_OVERHANG_MIN_MM` | 0 | Untergrenze Dachüberstand | LH-FA-ROF-005 |
| `ROOF_OVERHANG_MAX_MM` | 1500 | Obergrenze Dachüberstand | LH-FA-ROF-005 |
| `DEFAULT_ROOF_PITCH_DEG` | 30 | Default-Dachneigung bei Anlage (= `roofs`-Schema-Default) | LH-FA-ROF-001 |
| `DEFAULT_ROOF_OVERHANG_MM` | 500 | Default-Dachüberstand bei Anlage (= `roofs`-Schema-Default) | LH-FA-ROF-005 |
| `SLAB_THICKNESS_MIN_MM` | 100 | Untergrenze Deckendicke | LH-FA-SLB-002 |
| `SLAB_THICKNESS_MAX_MM` | 500 | Obergrenze Deckendicke | LH-FA-SLB-002 |
| `DEFAULT_SLAB_THICKNESS_MM` | 200 | Default-Deckendicke bei Anlage | LH-FA-SLB-001 |
| `FOUNDATION_DEPTH_MIN_MM` | 200 | Untergrenze Fundamenttiefe | LH-FA-FND-002 |
| `FOUNDATION_DEPTH_MAX_MM` | 2000 | Obergrenze Fundamenttiefe | LH-FA-FND-002 |
| `DEFAULT_FOUNDATION_DEPTH_MM` | 500 | Default-Fundamenttiefe bei Anlage | LH-FA-FND-001 |
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
| 2026-06-11 | §1 ROM-001.a präzisiert (Welle-1-Code-Review M1/M2): minimale Zyklen via Flächen-Traversierung — geteilte Knoten (Grad ≥ 3) abgedeckt, Stichkanten ignoriert; Näherung für kollineare Nachbarkanten ungleicher Stärke dokumentiert | ADR-0007 |
| 2026-06-12 | §1 D3-002.a ergänzt: welle-1v-Operationalisierung „sichtbar" (Qt-Widgets-Fenster, Tessellation über `ViewModelPort`, Szenen-Surrogat, ACC-002-Beleg als manueller Abnahme-Schritt) | ADR-0009 |
| 2026-06-12 | §1 LH-FA-WAL-006.a neu (Eckenschluss-Footprint-Regel, Footprint-Hoheit im Kern, Begrenzung/Rückfälle, EVL-Hinweis Shoelace) + D3-002.a-Mehr-Element-Update (`WallGeometryChanged`, Reihenfolge, Transaktions-Satz) + §3 `WALL_MITER_LIMIT`; zwei WAL-006-Verweise auf Vollumfang präzisiert | slice-012 (Lastenheft 0.1.2) |
| 2026-06-13 | §1 LH-FA-DOR-004.a/WIN-005.a neu (Wandöffnung als Schnitt-Prismen im Kern, boolesche Subtraktion über `GeometryKernelPort`, Klemmung/Ablehnung, Totalität/Transaktion, `WallGeometryChanged` der Wirtswand, Raumerkennung/Footprint unberührt) + §3 Tür-/Fenster-/Brüstungs-Wertebereiche | ADR-0011 (slice-013a) |
| 2026-06-13 | §3 Default-Maße bei Tür-/Fenster-Anlage (`DEFAULT_DOOR_*`/`DEFAULT_WINDOW_*`) — Implementierung der Anlage (Muster `DEFAULT_WALL_THICKNESS_MM`) | slice-013b |
| 2026-06-13 | §3 `OPENING_CUT_OVERSHOOT_MM` — Cutter-Überstand für koplanar-freien Boolean (Code-Review-Befund H1: §1-Überstand „je Seite" war nur in Z realisiert, jetzt auch lateral) | slice-013b Code-Review |
| 2026-06-13 | §1 `LH-FA-ROF-001.a` neu (Dach-Geometrie Teilumfang Rechteck-Grundriss: Traufrechteck, Pult/Sattel/Walm-Konstruktion + Höhenformeln, Walm-Einrückbetrag, Firsthöhe abgeleitet, Totalität) + §3 Neigungs-/Überstands-Bereiche + Defaults (= `roofs`-Schema) | slice-014a |
| 2026-06-13 | §1 `LH-FA-SLB-001.a` neu (Platten-Geometrie Decken+Fundament: Polygon × Dicke an `base_z` je `slab_type`, Ausschnitte als Boolean/`CutPrism`, Totalität; Port-base_z-Frage an 015b) + §3 Decken-/Fundament-Dicke-Bereiche + Defaults | slice-015a |
| 2026-06-13 | §1 `LH-FA-SLB-001.a` Port-base_z-Frage geschlossen: kein Port-Wechsel — Mesh-Translation um `base_z` nach dem Boolean, Cutouts relativ `[0,Dicke]`; `SlabChanged`-`op` (storey-bezogen, kein `RoomsChanged`) | slice-015b |

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
