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
Loch-Ringe) mit **Netto-Fläche**.

**Auslösung:** Die Erkennung läuft **automatisch bei Modell-Mutation**
(Wand anlegen/ändern/löschen) im Service — „automatisch … when er
geschlossen wird" ([LH-FA-ROM-001](lastenheft.md#lh-fa-rom-001--raum-automatisch-erkennen) Happy) braucht keinen manuellen
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
   benachbarte Offset-Geraden geschnitten.
   *Welle-1-Näherung:* bei **kollinearen Nachbarkanten ungleicher
   Stärke** springt die Ecke auf den Offset-Punkt der Folgekante
   (lineare Überblendung statt exakter Stufenkontur); die exakte
   Stufe kommt mit dem **WAL-006-Vollumfang** (nicht Teil des
   slice-012-Teilumfangs).
4. Verschachtelte Zyklen: der innere Zyklus erzeugt einen eigenen Raum
   (Innenkante nach innen); im umschließenden Raum wird die
   Außenkontur des inneren Zyklus als **Loch-Ring** geführt —
   Netto-Fläche = äußerer Ring minus Loch-Ringe (keine Doppelzählung
   der Fläche, [LH-FA-ROM-001](lastenheft.md#lh-fa-rom-001--raum-automatisch-erkennen) Boundary).
5. Offene Wandzüge erzeugen keinen Raum (Negative).

**Degenerierte Zyklen — kein Fehlerfall:** Die Raumerkennung ist
**total** und wirft kein `E-GEO-002`. Zyklen, deren Innenkanten-Offset
kollabiert, erzeugen **keinen Raum** — gleiches Verhalten wie offene
Wandzüge, kein Fehler. Kollaps-Kriterium ist der
**Kantenrichtungs-Erhalt**: kehrt sich beim Offset die Richtung einer
Kante um, ist der Ring kein gültiges Raumpolygon (eine reine
Flächen-Prüfung „Netto-Fläche ≤ 0" genügt nicht — Doppel-Inversion in
beiden Achsen erzeugt ein Phantom-Polygon *positiver* Fläche). `E-GEO-002` (§4) bleibt mutierenden
Geometrie-Operationen vorbehalten (z. B. Extrusion, [LH-FA-D3-001](lastenheft.md#modul-3d-modellierung-d3).a).

**Komplexität:** Vollerkennung pro Mutation ist Welle-1-Stand (kleine
Modelle); Zielkomplexität/inkrementelle Erkennung bleibt offener
Punkt (§7, M3).

### LH-FA-WAL-006.a — Eckenschluss (Footprint-Regel, Teilumfang)

Präzisiert den [LH-FA-WAL-006](lastenheft.md#lh-fa-wal-006--wand-verbinden)-Teilumfang (Lastenheft 0.1.2,
slice-012). Die **Footprint-Hoheit liegt im Kern**: der Grundriss
einer Wand ist ein Polygon, das der Kern aus Segment, Stärke und
Nachbar-Wissen ableitet; der Geometrie-Adapter extrudiert/tesselliert
nur noch das Polygon (Prisma in +Z auf Wandhöhe).

**Eck-Konstruktion:** An einem Endpunkt, den im **selben Geschoss**
genau **eine** weitere Wand teilt (Grad-2-Knoten; Punkt-Gleichheit
über `GEOMETRY_TOLERANCE_MM`), werden die beidseitigen Seitenkanten
der zwei Wände **im Schnittpunkt verbunden** — Prinzip analog der
Innenkanten-Konstruktion der Raumerkennung, hier auf die
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
Solids extrudiert; Wandöffnungen für Türen/Fenster ([LH-FA-DOR-004](lastenheft.md#lh-fa-dor-004--wandöffnung-automatisch-erzeugen),
[LH-FA-WIN-005](lastenheft.md#lh-fa-win-005--wandöffnung-automatisch-erzeugen)) als boolesche Subtraktion. Parameteränderung triggert
inkrementellen Rebuild des betroffenen Solids (Echtzeit, [LH-FA-D3-002](lastenheft.md#lh-fa-d3-002--echtzeitaktualisierung)).

### LH-FA-DOR-004.a / LH-FA-WIN-005.a — Wandöffnung (Schnitt-Prismen)

Präzisiert die automatische Wandöffnung für Türen (LH-FA-DOR-*) und
Fenster (LH-FA-WIN-*).

**Hosting & Position.** Eine Öffnung ist ein **eigenständiges
Domänen-Element** mit Referenz auf ihre Wirtswand (`wall_id`). Ihre Lage
entlang der Wandachse ist `offset_mm` = Abstand vom **Wand-Startpunkt**
entlang der Wand-Mittellinie bis zur **Nahkante** der Öffnung; die
Öffnung belegt den Achsenbereich `[offset, offset + width]`. Vertikal
belegt sie `[sill, sill + height]` über der Geschoss-Standfläche
(`sill = 0` für Türen, [LH-FA-WIN-004](lastenheft.md#lh-fa-win-004--brüstungshöhe-definieren) für Fenster). Quer durchsetzt sie
die **volle Wandstärke**.

**Schnitt-Prisma (Kern-Hoheit).** Der Kern berechnet je Öffnung ein
**Schnitt-Prisma** als reine Werte (Polygon + z-Bereich, dieselbe
Werte-Sprache wie das Footprint-Polygon, [LH-FA-WAL-006](lastenheft.md#lh-fa-wal-006--wand-verbinden).a): ein Rechteck
quer zur Wand an der Öffnungs-Position, das die Wandstärke voll (mit
geringem Überstand ≥ `GEOMETRY_TOLERANCE_MM` je Seite für einen
sauberen Schnitt) durchsetzt, extrudiert über `[sill, sill + height]`.
Der `GeometryKernelPort` extrudiert das Wand-Footprint-Polygon und
**subtrahiert die Schnitt-Prismen** (OCC-Boolean) → Netto-
Wand-Solid; der Adapter kennt keine Öffnungs-Semantik.

**Klemmung ([LH-FA-WAL-002](lastenheft.md#lh-fa-wal-002--wandstärke-definieren).a, `E-VAL-001`).** Breite/Höhe/Brüstung werden
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
Wandstärke, daher bleiben Raumerkennung und Footprint/
Eckenschluss ([LH-FA-WAL-006](lastenheft.md#lh-fa-wal-006--wand-verbinden).a) unberührt; die ROM- und WAL-006-AK-Tests
bleiben textlich unverändert grün. Abgelehnte Mutationen melden nicht.

### LH-FA-ROF-001.a — Dach-Geometrie (Teilumfang Rechteck-Grundriss)

Sammelblock, deckt **[LH-FA-ROF-001](lastenheft.md#lh-fa-rof-001--satteldach)..005** (Sattel/Walm/Pult, Neigung,
Überstand); Reifephase-Teilumfang welle-2 (Lastenheft 0.1.4). Modell-
Einordnung über das Bauteil-Erweiterungs-Muster (#6) — keine
eigene Grundsatz-ADR; die Geometrie wird **hier** normativ festgelegt.

**Grundriss-Herkunft (welle-2):** Das Dach hat einen **expliziten
rechteckigen Grundriss** `b × t` (Parameter; persistiert als
`roofs.footprint_json`). Die Auto-Ableitung aus dem
Geschoss-Wandumriss bleibt späterer Ausbau (keine Kopplung an
Wand-Mutationen in welle-2).

**Traufrechteck:** der Grundriss `b × t` wird ringsum um den Überstand
`o` ([LH-FA-ROF-005](lastenheft.md#lh-fa-rof-005--dachüberstand-definieren)) zum **Traufrechteck** `(b+2o) × (t+2o)` vergrößert;
das Dach kragt also um `o` über den Grundriss hinaus.

**Konstruktion je Typ** (Neigung `p`, [LH-FA-ROF-004](lastenheft.md#lh-fa-rof-004--dachneigung-definieren)):

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

**Firsthöhe abgeleitet:** `roofs.height_mm` (nullable) ist
**nicht** Eingabe, sondern die aus `p`/Überstand berechnete Firsthöhe.

**Klemmung/Totalität:** Neigung `p` auf `[ROOF_PITCH_MIN_DEG,
ROOF_PITCH_MAX_DEG]`, Überstand `o` auf `[ROOF_OVERHANG_MIN_MM,
ROOF_OVERHANG_MAX_MM]` geklemmt (`E-VAL-001`, §3). Ein nicht-rechteckiger
oder degenerierter Grundriss (Seite < `GEOMETRY_TOLERANCE_MM`) erzeugt
**kein** Dach — die Sicht-Query bleibt total (kein Wurf).

**Folge-Meldung:** Eine Dach-Mutation (Anlage/Neigung/Überstand/Form/
Entfernen) meldet `op = RoofChanged` **storey-bezogen** (neuer `op` im
D3-002.a-Vokabular, neuer Bauteil-Typ); der Beobachter lädt
die Dächer des Geschosses neu (`ViewModelPort.roofMeshes`). **Keine
`RoomsChanged`** (Dächer berühren die Raumerkennung nicht). Das
Dach-Netz entsteht **analytisch im Kern** (`roof_geometry`), nicht über
OCC — der Vertrag „framework-freie Netze über `ViewModelPort`"
bleibt erfüllt (die OCC-Tessellation gilt dem extrudierten Wand-Solid).

### LH-FA-SLB-001.a — Platten-Geometrie (Decken & Fundament)

Sammelblock, deckt **[LH-FA-SLB-001](lastenheft.md#lh-fa-slb-001--decke-erzeugen)..003** (Decken) **und [LH-FA-FND-001](lastenheft.md#lh-fa-fnd-001--fundament-erzeugen)..003**
(Fundament/Bodenplatte). Modell-Einordnung über das Bauteil-Erweiterungs-Muster; keine
eigene Grundsatz-ADR. Beide sind **horizontale Platten** und liegen im
Schema gemeinsam in `slabs` (Diskriminator `slab_type`).

**Platten-Solid:** Eine Platte ist ein **Grundriss-Polygon, vertikal um
die Dicke extrudiert** — ein flaches Prisma. Geometrisch dasselbe
Vokabular wie die Wand-Footprint-Extrusion (`GeometryKernelPort`,
slice-012), hier auf eine horizontale Platte angewandt; eigenständig
begründet (nicht das Dach-Verfahren — das Dach ist ein analytisches
Polyeder, [LH-FA-ROF-001](lastenheft.md#lh-fa-rof-001--satteldach).a).

**Aufstandshöhe `base_z` je `slab_type`** (das `slabs`-Schema trägt
**keine** base_z-Spalte — sie folgt aus Typ/Geschoss): **Decke** =
Oberkante des zugeordneten Geschosses (`storey_id`); **Bodenplatte**
(`FND-003`) = Oberkante auf Höhe 0; **Fundament** (`FND-001/002`) = unter
Gelände, die Fundamenttiefe erstreckt sich von 0 **nach unten**. Die
Dicke/Tiefe ist auf §3 geklemmt (`E-VAL-001`).

**Ausschnitte (`SLB-003`):** als **boolesche Subtraktion** von
Schnitt-Prismen aus dem Platten-Solid — Wiederverwendung von
`model::CutPrism` und des OCC-Boolean-Backends, wie bei den
Wandöffnungen ([LH-FA-DOR-004](lastenheft.md#lh-fa-dor-004--wandöffnung-automatisch-erzeugen).a). Ein Ausschnitt wird **auf den
Platten-Umriss begrenzt**: rand-/außenliegende, degenerierte (Fläche <
`GEOMETRY_TOLERANCE_MM²`) oder nicht-endliche Ausschnitte werden an der
API abgelehnt (Containment-Vorbedingung). Ein vollständig innenliegender
Ausschnitt ist Boolean-koplanar-frei — **kein** lateraler Überstand nötig
(anders als die Wandöffnung, die die Wand zwangsläufig durchspannt; der
z-Überstand `[−ε,Dicke+ε]` bleibt für Ober-/Unterseite).

**Totalität:** degenerierter/leerer Grundriss (Fläche <
`GEOMETRY_TOLERANCE_MM²`) → keine Platte; die Sicht-Query bleibt total
(kein Wurf); ein fehlgeschlagener mutierender Solid-Bau meldet
`E-GEO-002`.

**Port-Mechanik (slice-015b, entschieden):** der bestehende
`extrudeFootprint`/`tessellateFootprint` (extrudiert ab z=0) bleibt
**unverändert** — das Volumen ist z-invariant, und der Kern **verschiebt
das fertige Platten-Netz um `base_z`** (reine Mesh-Translation, NACH dem
Ausschnitt-Boolean). Damit ist **kein** Port-Signatur-Eingriff nötig
(Kern-Hoheit gewahrt ohne Migration; die Cutout-`CutPrism`s
liegen relativ zum Solid `[0,Dicke]`, nicht bei `base_z`).

**Folge-Meldung:** Eine Platten-Mutation (Anlage/Dicke/Ausschnitt/
Entfernen) meldet `op = SlabChanged` **storey-bezogen** (neuer `op` im
D3-002.a-Vokabular); der Beobachter lädt die Platten neu
(`ViewModelPort.slabMeshes`). **Keine `RoomsChanged`** (Platten berühren
die Raumerkennung nicht).

### LH-FA-STR-001.a — Treppen-Geometrie (Teilumfang gerade einläufige Treppe)

Sammelblock, deckt **[LH-FA-STR-001](lastenheft.md#lh-fa-str-001--treppe-erzeugen)..004** (erzeugen, Stufenanzahl, Laufbreite,
Geländer); Reifephase-Teilumfang welle-2 (Lastenheft 0.1.6). Modell-Einordnung
über das Bauteil-Erweiterungs-Muster (#6) — keine eigene Grundsatz-ADR;
die Geometrie wird **hier** normativ festgelegt.

**Teilumfang (welle-2):** eine **gerade einläufige Treppe**. Das `stairs`-Schema
trägt genau die Parameter dafür (`start_x/y_mm`, `width_mm`,
`step_count`, `rise_mm`, `tread_mm`, `from_storey_id`/`to_storey_id`) und **kein**
Podest-/Wendel-/Richtungs-Feld. Mehrläufige, gewendelte und Podest-Treppen
bleiben offen (späterer Vollumfang).

**Geschoss-Spanne:** Die Treppe verbindet `from_storey` (unten) → `to_storey`
(oben); die **Gesamtsteigung = Geschosshöhe** der unteren Etage. Die
Aufstandshöhe `base_z` ist der Boden der unteren Etage (welle-2-Ein-Geschoss-
Annahme = 0, Muster [`LH-FA-SLB-001.a`](lastenheft.md#lh-fa-slb-001--decke-erzeugen)/`slab_geometry`); Mehr-Geschoss-Stapelung
später.

**Steigung abgeleitet:** `stairs.rise_mm` ist **nicht** freie Eingabe, sondern
die aus Geschosshöhe und Stufenanzahl berechnete Steigung **`rise =
Geschosshöhe / step_count`** (flächenbündiger Anschluss an die obere Ebene; der
gespeicherte `rise_mm` spiegelt diese Ableitung). Eingabe sind `step_count`
(STR-002), `width` (STR-003) und `tread` (Auftritt). Das Muster „abgeleitete
Größe statt Doppel-Eingabe" entspricht der Dach-Firsthöhe ([LH-FA-ROF-001](lastenheft.md#lh-fa-rof-001--satteldach).a).

**Stufen-Konstruktion (analytisches Polyeder im Kern):** die Treppe entsteht
**analytisch im Kern** (Präzedenz `roof_geometry`/`slab_geometry`), **nicht** über
OCC — der Vertrag „framework-freie Netze über `ViewModelPort`" bleibt
erfüllt. `step_count` Quader; Stufe `i` (0-basiert) spannt in Treppen-lokalen
Koordinaten `x ∈ [i·tread, (i+1)·tread]` (Aufstiegsrichtung), `y ∈ [0, width]`
(Laufbreite), `z ∈ [0, (i+1)·rise]` (solides Stufenprofil vom Boden zur
Stufenoberkante). Die Lauflänge ist `step_count · tread`.

**Aufstiegsrichtung (welle-2):** das Schema trägt **keine** Richtungs-Spalte →
die Treppe steigt in einer **festen Konvention `+x` ab dem Startpunkt**
(`start_x/y_mm`) auf. Freie Rotation/Orientierung bräuchte eine Schema-Erweiterung
und bleibt offen (analog ROF „rechteckiger Grundriss").

**Geländer (STR-004):** ein dünnes vertikales Element entlang der Lauf-Seite(n)
auf `STAIR_RAILING_HEIGHT_MM` über den Stufen-Oberkanten, das der Stufenfolge
folgt — **generierte Geometrie aus der Treppe**, kein persistierter Eigenzustand
(das `stairs`-Schema trägt keine Geländer-Spalte). In welle-2 ist das Geländer
**immer Teil der Treppe** (nicht schaltbar); eine persistierte An/Aus- bzw.
Seiten-Option ist späterer Ausbau (Schema-Erweiterung). Da es deterministisch
aus der Stufen-Geometrie folgt, geht beim Speichern nichts verloren (Muster
abgeleitete roofs-`height_mm`, [LH-FA-ROF-001](lastenheft.md#lh-fa-rof-001--satteldach).a).

**Klemmung/Totalität:** `step_count` auf `[STAIR_STEP_COUNT_MIN,
STAIR_STEP_COUNT_MAX]`, `width` auf `[STAIR_WIDTH_MIN_MM, STAIR_WIDTH_MAX_MM]`,
`tread` auf `[STAIR_TREAD_MIN_MM, STAIR_TREAD_MAX_MM]` geklemmt (`E-VAL-001`,
§3). Die abgeleitete `rise` wird **nicht** geklemmt; `STAIR_RISE_MIN/MAX_MM` ist
ein **informativer Komfort-Bereich** (Hinweis, falls Geschosshöhe/`step_count`
ihn verlässt). Eine ungültige Spanne (nur ein Geschoss, `from == to`,
Geschosshöhe ≤ `GEOMETRY_TOLERANCE_MM`) erzeugt **keine** Treppe — die
Sicht-Query bleibt total (kein Wurf); ein fehlgeschlagener mutierender Solid-Bau
meldet `E-GEO-002`.

**Folge-Meldung:** Eine Treppen-Mutation (Anlage/Stufenanzahl/Breite/Entfernen)
meldet `op = StairChanged` (neuer `op` im D3-002.a-§5-Span-Vokabular, neuer Bauteil-Typ). **Geschoss-Bindung (begründet):** anders als Dach/Decke (je
ein Geschoss) spannt die Treppe zwei; die Meldung wird an die **untere Etage
(`from_storey`)** gebunden — dort liegen Start/Anker (`start_x/y_mm`) und die
`base_z` der Treppe (die obere Etage ist nur Ziel der abgeleiteten Steigung). Der
Beobachter lädt die Treppen über `ViewModelPort.stairMeshes` neu — in welle-2
**projektweit** (eine Treppe ist geschossübergreifend; eine geschoss-gefilterte
Sicht wird mit der Mehr-Geschoss-Stapelung später möglich). **Keine
`RoomsChanged`** (Treppen berühren die Raumerkennung nicht).

### LH-FA-D3-002.a — Echtzeitaktualisierung (Benachrichtigungs-Vertrag)

Präzisiert [LH-FA-D3-002](lastenheft.md#lh-fa-d3-002--echtzeitaktualisierung).

**Auslösung und Synchronität:** Jede committete Modell-Mutation
(Geschoss anlegen, Wand anlegen, Wand-Parameter ändern) wird
**synchron im Mutationspfad** gemeldet — nach Abschluss aller
Post-Commit-Schritte (Raum-Re-Detektion, [LH-FA-ROM-001](lastenheft.md#lh-fa-rom-001--raum-automatisch-erkennen).a), sodass ein
Beobachter im Callback einen vollständig konsistenten Stand abfragt.
Abgelehnte/verworfene Mutationen (`E-VAL-001` Rejected,
Null-Längen-Wand) melden nicht.

**Vertrag (Push-Notify, Pull-State):** Gemeldet werden `element_id`
und `op` (Vokabular wie OTel-Span `bcad.geometry.rebuild`, §5); den
aktualisierten Stand holt der Beobachter über die Abfrage-Ports
(Solid, Räume, Modell). Mehrere Meldungen pro Mutation sind zulässig
(z. B. Wand- plus Raum-Änderung; Mehr-Element-Updates nicht verbaut).

**Mehr-Element-Update (Eckenschluss, [LH-FA-WAL-006](lastenheft.md#lh-fa-wal-006--wand-verbinden).a/slice-012):**
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

**Beobachter-Pflichten:** Mehrere Beobachter (2D-/3D-Sicht, [OBJ-003](lastenheft.md#3-projektziele))
über Registrierung (`subscribe`/`unsubscribe`); Callbacks dürfen
abfragen, aber keine Mutationen auslösen (Re-Entranz-Verbot); eine
werfende Beobachter-Implementierung kippt die committete Mutation
nicht und blockiert weitere Beobachter nicht (Kapselung im Service;
Sichtbarkeit der Fehler später über REQ-TEC-006-Telemetrie).

**Welle-1-Operationalisierung von „sichtbar":** Der Kern erfüllt
D3-002 bis zur Benachrichtigungs-/Abfrage-Grenze; die sichtbare
3D-Darstellung liefert der Viewer-Strang (Roadmap, `welle-1v-viewer`)
auf dieser Basis — der Lastenheft-Wortlaut bleibt benutzer-beobachtbar
und wird zusammen mit [ACC-002](lastenheft.md#7-abnahmekriterien) dort erfüllt.

**Welle-1v-Operationalisierung von „sichtbar":**
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
[ACC-002](lastenheft.md#7-abnahmekriterien)-Beleg entsteht als manueller Abnahme-Schritt
(`make acc-002-beleg`, kein Gate).

### LH-FA-EVL-001.a — Auswertungen (Flächen/Volumen/Wohnfläche/Listen)

Sammelblock, deckt **[LH-FA-EVL-001](lastenheft.md#lh-fa-evl-001--flächenberechnung)..006**; Modell-Einordnung über die **Evaluations-Architektur**. Auswertung ist eine **reine read-only-Ableitung** aus
dem committeten Modell über den neuen Driving-Port **`EvaluatePort`** (kein
`DetectRoomsPort`-Überladen; pure Ergebnis-Werttypen; **kein** `…Changed`-`op`,
pull-on-demand). **Analytisch im Kern, kein OCC-`GProp`** (keine
`GeometryKernelPort`-Anfrage für eine Zahl).

**Fläche (EVL-001/003) — Shoelace auf dem Raum-Netto-Polygon.** Die
Netto-Grundfläche je Raum ist die **Shoelace-Fläche des Raumpolygons**
(äußerer Innenkanten-Ring **minus** Loch-Ringe — die Netto-Definition
wird **wiederverwendet**, keine zweite Semantik, keine Doppelzählung). ROM-002
(Raumfläche) / ROM-003 (Raumvolumen) sind die **Per-Raum-Quelle**; EVL
**aggregiert** sie je Geschoss/Gebäude (Bericht) — **eine** Flächen-Semantik.
**Wohnfläche (EVL-003)** = Summe der Raum-Netto-Flächen (welle-3-Teilumfang;
Anrechnungsfaktoren offen, `LIVING_AREA_FACTOR = 1` §3).

**Volumen (EVL-002) — Bauteil-Netto-Volumen analytisch im Kern.** Das
Netto-Material-Volumen deckt in welle-3 **Wand · Decke/Fundament · Treppe** (je
ein `VolumeReport`-Subtotal + Summe `total_m3`; **Dach ausgenommen**, s. u.). Je
Bauteil: **Platte/Treppe** aus ihrem analytischen Solid (Treppe = Σ Stufenkörper
`tread · width · Geschosshöhe · (step_count+1)/2`, **geländer-frei**); **Wand** =
Bauteil-Footprint-Fläche (Shoelace) · Höhe **minus** das **real entfernte,
geklemmte Öffnungsvolumen** je Öffnung
(`width · thickness · clamped_height`,
`clamped_height = min(sill+height, Wandhöhe) − max(0, sill)`) — **NICHT** das
überstands-behaftete Roh-Schnittprisma (`OPENING_CUT_OVERSHOOT_MM`; die
Schnitt-Prismen sind das Boolean-**Werkzeug**, nicht das Volumen-Maß). Die
Auswertung ist **rein analytisch im Kern** und liest **nicht** das adapter-
gemessene `Solid.volume_mm3` (das wäre eine driven-Volumenmessung — keine reine
Kern-Query).
**Eck-Näherung (welle-3, benannt):** die Summe der Wand-Volumina **doppelzählt**
den Miter-Sporn endpunkt-verbundener Wände (slice-012-Footprint) — kleine
Über-Zählung, bewusst in Kauf genommen; das exakte vereinigte Volumen
(Footprint-Union je Geschoss) ist Re-Eval (parallel
WAL-006-Vollumfang).
**Aussparungs-Näherung (welle-3, benannt):** überlappende Platten-Aussparungen
werden analytisch über die **Summe** der Einzelflächen abgezogen (nicht ihre
Vereinigung) — eine kleine **Unter-Zählung** im seltenen Überlappungsfall (der
reale Boolean entfernt die Vereinigung); das exakte Vereinigungs-Volumen ist
Re-Eval (parallel zur Wand-Eck-Näherung).

**Dach-Volumen — welle-3 zurückgestellt (benannte Lücke).** Das Dachmodell ist
**dicke-los** (Shell aus geneigten Flächen, unten offen) und trägt damit **kein
wohldefiniertes Bauteil-/Material-Solid**; der einzige berechenbare Dachkörper
wäre **umbauter Raum**, kein Material-Volumen, und gehört nicht in dieselbe
Netto-Material-Summe wie Wand/Decke/Treppe (Massenermittlung). Das Geländer der
Treppe ist generierte Render-Geometrie (kein Material) und bleibt ebenso
**ausgenommen**. **Re-Eval:** sobald das Dach eine Dicke-/Material-Semantik
erhält (nicht-prismatisches/komplexes Volumen), wird die analytische
Volumen-Schicht neu bewertet.

**Listen (EVL-004/005/006) — Aggregation über das Modell.** Die Materialliste
(EVL-004) gruppiert die **material-tragenden Bauteile mit Netto-Volumen**
(`walls`/`slabs` mit `material_id`) je Material und summiert die Menge =
**Netto-Volumen (m³)** über Wand + Decke/Fundament (EVL-002/`volume_geometry`,
welle-3-Einheit). Bauteile **ohne** Material werden nicht gruppiert (Boundary).
**Kosten (MAT-006):** je Material `Menge × cost_per_m3` (Projekt-Summe Σ); ein
Material **ohne** `cost_per_m3` trägt **keine** Kosten (NULL, nicht „kostenlos").
**`cost_per_m2`** (Flächen-Kosten) ist welle-3 **nicht** genutzt — EVL-004 führt
Volumen (benannte Lücke; Re-Eval mit flächen-basierter Aggregation).
**Dach welle-3 ausgenommen:** ein `roof` ist zwar material-tragend, sein
**Volumen ist zurückgestellt** (dicke-loses Modell, s. o.) → **nicht** in die
EVL-004-Aggregation (benannte Lücke; Re-Eval mit Dach-Volumen-Semantik). Ebenso
tragen `stairs`/`openings`/`doors`/`windows` kein `material_id` (**benannte
Lücke**), und **`windows.frame_material` ist Freitext (kein `materials`-FK) und
fließt NICHT in die EVL-004-Aggregation** (sonst zwei inkonsistente Quellen).
Tür-/Fensterlisten (EVL-005/006) zählen die `openings`/`doors`/`windows` mit
ihren Maßen (Anzahl = Listengröße).

**Material-Auflösungsregel (Datenfluss):** das **effektive** Material eines
Bauteils ist sein eigenes `material_id`; **fehlt es, gilt das `material_id`
seines `wall_type`** (Default über den Typ — **welle-3 zurückgestellt: geliefert
ist Override-only**, s. „Auflösungs-Teilumfang" unten) — `wall_types.material_id`
als Vorlage, das Bauteil-`material_id` als Override (Werttyp/FK-Autorität siehe
§2.1).

**Material-Verwaltung/-Zuweisung (welle-3-Teilumfang, MAT-001/002/003).**
Materialien sind **projekt-eigen** (`materials.project_id`) und werden über den
Bauteil-Edit-Port **angelegt/geändert/entfernt** (MAT-001) und **gelistet**
(MAT-002); ein Material **ohne Name** (leer oder nur Whitespace) wird
**abgelehnt** (MAT-001-Negative). Zuweisung (MAT-003) setzt das **eigene
`material_id`** an Wand/Dach/Decke (Override) bzw. **wählt ab** („kein Material",
kein Fehler). Ein **noch zugewiesenes** Material ist **nicht löschbar**
(`on_delete: restrict` — kein stiller Verlust der Zuweisung; erst löschbar, wenn
unreferenziert). Material-Mutationen sind **op-frei** (per Pull
von der Auswertung konsumiert, **kein** gerendertes Szenen-Korrelat in welle-3 —
Farbe/Textur = MAT-004, Sicht). **Auflösungs-Teilumfang:** geliefert ist der
**Override** (eigenes `material_id` → effektives Material); der
**`wall_type`-Template-Fallback** ist **zurückgestellt** — das Domänenmodell
trägt den Wandtyp als Enum, **keine** material-tragende Wall-Type-Entität;
**Re-Eval** mit Einführung einer solchen Entität.
**Persistenz:** die `materials`-Bibliothek und die `material_id`-Zuweisung
(Wand/Dach/Decke) round-trippen über die Projekt-Persistenz (SQLite, in der
atomaren Speicher-Transaktion); optionale Kennwerte werden **NULL-sicher**
geladen (kein Wert ⇒ `null`, **nicht** `0`). Die Schema-Spalte `materials.density`
hat **kein** Domänenfeld (spez. §2.1) und wird daher **nicht** round-getrippt
(`NULL` via Spalten-Auslassung) — sie round-trippt, sobald die Domäne `density`
trägt.

**Totalität:** ein leeres/lückenhaftes Modell liefert eine Null-/leere
Auswertung (kein Wurf); die Auswertung **mutiert nie** (read-only).

## 2. Datenstrukturen und Schemas

Das Datenmodell hat **zwei Sichten**, die getrennt zu halten sind
(die Abhängigkeit zeigt nach innen):

1. **Domänen-Modell (Kern, Wahrheit)** — pure Werttypen in
   `src/hexagon/model/`, framework-frei. Quelle der Wahrheit für *was* ein
   Bauteil ist.
2. **Persistenz-Schema (Adapter-Abbildung)** — wie das Domänen-Modell in
   SQLite gespeichert wird. **Es bildet das Domänen-Modell ab,
   treibt es nicht.**

### 2.1 Domänen-Modell (Kern)

Pure Werttypen in `src/hexagon/model/`, framework-frei. Implementiert
(slice-003a): `Building`, `Storey`, `Wall`, `Point2D`, `Segment`, `Solid`,
`WallType`. Wand-Auszug (Stand: Einzelsegment, welle-1):
`{ id, storey_id, start: Point2D, end: Point2D, thickness_mm, height_mm, type ∈ {Innen, Aussen, Trag} }`.

*Wandzüge/Polylines* (mehrere verbundene Segmente, [LH-FA-WAL-001](lastenheft.md#lh-fa-wal-001--wand-zeichnen)/006)
folgen als Erweiterung; `Storey` gewinnt später `level_index`/`elevation`
aus dem Persistenz-Schema (§2.2).

**Material (welle-3, LH-FA-MAT-*, von EVL konsumiert):** `model::Material`
als pure Werte aus dem `materials`-Schema:
`{ id, name, category, u_value?, cost_per_m2?, cost_per_m3?, color_hex?, texture_path? }`.
**FK-Zuweisungs-Autorität:** ein Bauteil (`walls`/`roofs`/`slabs`) trägt ein
**eigenes** `material_id` (Override); `wall_types.material_id` ist die
**Typ-Vorlage**. Die **effektive** Auflösung (eigenes `material_id`, sonst über
den `wall_type`) ist Datenfluss → §1 [`LH-FA-EVL-001.a`](lastenheft.md#lh-fa-evl-001--flächenberechnung) ([LH-FA-MAT-003](lastenheft.md#lh-fa-mat-003--materialzuweisung).a).
`stairs`/`openings`/`doors` tragen in welle-3 **kein** Material (benannte Lücke);
`windows.frame_material` ist Freitext, **kein** `materials`-FK.

### 2.2 Persistenz-Schema (SQLite)

Maschinenlesbare **Quelle der Wahrheit**: [`data-model.yaml`](data-model.yaml)
im **d-migrate Neutral-Format** (`schema_format: "1.0"`) — d-migrate
generiert daraus die dialekt-spezifische DDL (Ziel: SQLite, kein
hand-geschriebenes SQL). Design: per-Typ-Tabellen,
`openings`-Spezialisierung, JSON-Geometrie, persistierter Undo-Stack.

Kerntabellen (welle-1) — vollständig in `data-model.yaml`:

| Tabelle | Inhalt |
|---|---|
| `projects` | Projekt-Metadaten, `file_version`, Einheit |
| `storeys` | Geschosse (`level_index`, `elevation_mm`, `height_mm`) |
| `walls` | Wände (Segment `start/end`, `thickness_mm`, `height_mm`, Typ/Material) |
| `rooms` | Räume (Polygon als `polygon_json`, Fläche/Volumen) |
| `openings` → `doors`/`windows` | Wandöffnungen mit 1:1-Spezialisierung |
| `materials`, `wall_types` | Material- und Wandtyp-Bibliothek |
| `undo_commands` | persistierter Undo-Stack ([LH-QA-003](lastenheft.md#lh-qa-003--undoredo)) |

**Migrationsregel:** Schema-Version steigt monoton; jede Erhöhung braucht
eine getestete Aufwärts-Migration (vgl. `releasing.md`).

**Offene Punkte:** (a) `wall_types`-Bibliothek vs. `WallType`-Enum
(Klassifikation, [LH-FA-WAL-007](lastenheft.md#lh-fa-wal-007--wandtyp-wählen)) — Koexistenz, Auflösung im WAL-007-Slice;
(b) **[LH-FA-BLD-004](lastenheft.md#lh-fa-bld-004--projektversionierung) Projektversionierung** (frühere Stände) ist **nicht**
im Schema (nur Undo) — eigener Slice.

## 3. Defaults und Konstanten

| Name | Wert | Begründung | ADR / Bezug |
|---|---|---|---|
| `WALL_THICKNESS_MIN_MM` | 50 | Untergrenze Wandstärke | [LH-FA-WAL-002](lastenheft.md#lh-fa-wal-002--wandstärke-definieren) |
| `WALL_THICKNESS_MAX_MM` | 1000 | Obergrenze Wandstärke | [LH-FA-WAL-002](lastenheft.md#lh-fa-wal-002--wandstärke-definieren) |
| `WALL_HEIGHT_MIN_MM` | 500 | Untergrenze Wandhöhe | [LH-FA-WAL-003](lastenheft.md#lh-fa-wal-003--wandhöhe-definieren) |
| `WALL_HEIGHT_MAX_MM` | 10000 | Obergrenze Wandhöhe | [LH-FA-WAL-003](lastenheft.md#lh-fa-wal-003--wandhöhe-definieren) |
| `DEFAULT_WALL_THICKNESS_MM` | 240 | Default-Wandstärke bei Wand-Anlage (typ. Außenwand 24 cm) | [LH-FA-WAL-001](lastenheft.md#lh-fa-wal-001--wand-zeichnen) |
| `DEFAULT_STOREY_HEIGHT_MM` | 2500 | Default-Geschosshöhe bei Projekt/Geschoss-Anlage | [LH-FA-BLD-001](lastenheft.md#lh-fa-bld-001--projekt-anlegen), [LH-FA-FLR-004](lastenheft.md#modul-geschosse-flr) |
| `GEOMETRY_TOLERANCE_MM` | 0.1 | Toleranz für Punkt-Gleichheit / Wandverbindung | [LH-FA-WAL-006](lastenheft.md#lh-fa-wal-006--wand-verbinden), [LH-FA-ROM-001](lastenheft.md#lh-fa-rom-001--raum-automatisch-erkennen) |
| `WALL_MITER_LIMIT` | max(Stärke_A, Stärke_B) | Eckenschluss-Begrenzung: maximales Auskragen der Eck-Geometrie über den gemeinsamen Endpunkt; darüber stumpfes Ende (Formel, keine feste Zahl) | [LH-FA-WAL-006](lastenheft.md#lh-fa-wal-006--wand-verbinden) |
| `DOOR_WIDTH_MIN_MM` | 600 | Untergrenze Türbreite | [LH-FA-DOR-003](lastenheft.md#lh-fa-dor-003--türparameter-bearbeiten) |
| `DOOR_WIDTH_MAX_MM` | 2000 | Obergrenze Türbreite | [LH-FA-DOR-003](lastenheft.md#lh-fa-dor-003--türparameter-bearbeiten) |
| `DOOR_HEIGHT_MIN_MM` | 1800 | Untergrenze Türhöhe | [LH-FA-DOR-003](lastenheft.md#lh-fa-dor-003--türparameter-bearbeiten) |
| `DOOR_HEIGHT_MAX_MM` | 2500 | Obergrenze Türhöhe | [LH-FA-DOR-003](lastenheft.md#lh-fa-dor-003--türparameter-bearbeiten) |
| `WINDOW_WIDTH_MIN_MM` | 300 | Untergrenze Fensterbreite | [LH-FA-WIN-003](lastenheft.md#lh-fa-win-003--fensterparameter-bearbeiten) |
| `WINDOW_WIDTH_MAX_MM` | 3000 | Obergrenze Fensterbreite | [LH-FA-WIN-003](lastenheft.md#lh-fa-win-003--fensterparameter-bearbeiten) |
| `WINDOW_HEIGHT_MIN_MM` | 300 | Untergrenze Fensterhöhe | [LH-FA-WIN-003](lastenheft.md#lh-fa-win-003--fensterparameter-bearbeiten) |
| `WINDOW_HEIGHT_MAX_MM` | 2500 | Obergrenze Fensterhöhe | [LH-FA-WIN-003](lastenheft.md#lh-fa-win-003--fensterparameter-bearbeiten) |
| `WINDOW_SILL_MIN_MM` | 0 | Untergrenze Brüstungshöhe | [LH-FA-WIN-004](lastenheft.md#lh-fa-win-004--brüstungshöhe-definieren) |
| `WINDOW_SILL_MAX_MM` | 2000 | Obergrenze Brüstungshöhe | [LH-FA-WIN-004](lastenheft.md#lh-fa-win-004--brüstungshöhe-definieren) |
| `DEFAULT_DOOR_WIDTH_MM` | 900 | Default-Türbreite bei Anlage | [LH-FA-DOR-001](lastenheft.md#lh-fa-dor-001--tür-platzieren) |
| `DEFAULT_DOOR_HEIGHT_MM` | 2100 | Default-Türhöhe bei Anlage | [LH-FA-DOR-001](lastenheft.md#lh-fa-dor-001--tür-platzieren) |
| `DEFAULT_WINDOW_WIDTH_MM` | 1200 | Default-Fensterbreite bei Anlage | [LH-FA-WIN-001](lastenheft.md#lh-fa-win-001--fenster-platzieren) |
| `DEFAULT_WINDOW_HEIGHT_MM` | 1300 | Default-Fensterhöhe bei Anlage | [LH-FA-WIN-001](lastenheft.md#lh-fa-win-001--fenster-platzieren) |
| `DEFAULT_WINDOW_SILL_MM` | 900 | Default-Brüstungshöhe bei Anlage | [LH-FA-WIN-001](lastenheft.md#lh-fa-win-001--fenster-platzieren)/004 |
| `OPENING_CUT_OVERSHOOT_MM` | 1 | Überstand des Öffnungs-Schnittkörpers über die Wandgrenzen (lateral je Seite + Boundary-Höhen) für einen koplanar-freien Boolean; volumen-neutral (≥ `GEOMETRY_TOLERANCE_MM`) | [LH-FA-DOR-004](lastenheft.md#lh-fa-dor-004--wandöffnung-automatisch-erzeugen)/WIN-005 |
| `ROOF_PITCH_MIN_DEG` | 5 | Untergrenze Dachneigung | [LH-FA-ROF-004](lastenheft.md#lh-fa-rof-004--dachneigung-definieren) |
| `ROOF_PITCH_MAX_DEG` | 60 | Obergrenze Dachneigung | [LH-FA-ROF-004](lastenheft.md#lh-fa-rof-004--dachneigung-definieren) |
| `ROOF_OVERHANG_MIN_MM` | 0 | Untergrenze Dachüberstand | [LH-FA-ROF-005](lastenheft.md#lh-fa-rof-005--dachüberstand-definieren) |
| `ROOF_OVERHANG_MAX_MM` | 1500 | Obergrenze Dachüberstand | [LH-FA-ROF-005](lastenheft.md#lh-fa-rof-005--dachüberstand-definieren) |
| `DEFAULT_ROOF_PITCH_DEG` | 30 | Default-Dachneigung bei Anlage (= `roofs`-Schema-Default) | [LH-FA-ROF-001](lastenheft.md#lh-fa-rof-001--satteldach) |
| `DEFAULT_ROOF_OVERHANG_MM` | 500 | Default-Dachüberstand bei Anlage (= `roofs`-Schema-Default) | [LH-FA-ROF-005](lastenheft.md#lh-fa-rof-005--dachüberstand-definieren) |
| `SLAB_THICKNESS_MIN_MM` | 100 | Untergrenze Deckendicke | [LH-FA-SLB-002](lastenheft.md#lh-fa-slb-002--deckendicke-definieren) |
| `SLAB_THICKNESS_MAX_MM` | 500 | Obergrenze Deckendicke | [LH-FA-SLB-002](lastenheft.md#lh-fa-slb-002--deckendicke-definieren) |
| `DEFAULT_SLAB_THICKNESS_MM` | 200 | Default-Deckendicke bei Anlage | [LH-FA-SLB-001](lastenheft.md#lh-fa-slb-001--decke-erzeugen) |
| `FOUNDATION_DEPTH_MIN_MM` | 200 | Untergrenze Fundamenttiefe | [LH-FA-FND-002](lastenheft.md#lh-fa-fnd-002--fundamenttiefe-definieren) |
| `FOUNDATION_DEPTH_MAX_MM` | 2000 | Obergrenze Fundamenttiefe | [LH-FA-FND-002](lastenheft.md#lh-fa-fnd-002--fundamenttiefe-definieren) |
| `DEFAULT_FOUNDATION_DEPTH_MM` | 500 | Default-Fundamenttiefe bei Anlage | [LH-FA-FND-001](lastenheft.md#lh-fa-fnd-001--fundament-erzeugen) |
| `STAIR_WIDTH_MIN_MM` | 800 | Untergrenze Laufbreite (Wohnbau-Komfort, keine Statik) | [LH-FA-STR-003](lastenheft.md#lh-fa-str-003--laufbreite-definieren) |
| `STAIR_WIDTH_MAX_MM` | 2000 | Obergrenze Laufbreite | [LH-FA-STR-003](lastenheft.md#lh-fa-str-003--laufbreite-definieren) |
| `STAIR_STEP_COUNT_MIN` | 2 | Untergrenze Stufenanzahl je gerader Lauf | [LH-FA-STR-002](lastenheft.md#lh-fa-str-002--stufenanzahl-definieren) |
| `STAIR_STEP_COUNT_MAX` | 30 | Obergrenze Stufenanzahl je gerader Lauf | [LH-FA-STR-002](lastenheft.md#lh-fa-str-002--stufenanzahl-definieren) |
| `STAIR_TREAD_MIN_MM` | 210 | Untergrenze Auftritt (Stufentiefe) | [LH-FA-STR-001](lastenheft.md#lh-fa-str-001--treppe-erzeugen) |
| `STAIR_TREAD_MAX_MM` | 350 | Obergrenze Auftritt | [LH-FA-STR-001](lastenheft.md#lh-fa-str-001--treppe-erzeugen) |
| `STAIR_RISE_MIN_MM` | 140 | **Informativer** Komfort-Bereich der abgeleiteten Steigung — **kein** `E-VAL-001`-Klemmpunkt (`rise = Geschosshöhe / step_count`) | [LH-FA-STR-001](lastenheft.md#lh-fa-str-001--treppe-erzeugen) |
| `STAIR_RISE_MAX_MM` | 200 | **Informative** Komfort-Obergrenze der abgeleiteten Steigung (Hinweis, keine Klemmung) | [LH-FA-STR-001](lastenheft.md#lh-fa-str-001--treppe-erzeugen) |
| `STAIR_RAILING_HEIGHT_MM` | 900 | Handlaufhöhe des Treppengeländers über Stufenoberkante | [LH-FA-STR-004](lastenheft.md#lh-fa-str-004--treppengeländer) |
| `DEFAULT_STAIR_WIDTH_MM` | 1000 | Default-Laufbreite bei Anlage | [LH-FA-STR-001](lastenheft.md#lh-fa-str-001--treppe-erzeugen) |
| `DEFAULT_STAIR_STEP_COUNT` | 15 | Default-Stufenanzahl bei Anlage | [LH-FA-STR-001](lastenheft.md#lh-fa-str-001--treppe-erzeugen) |
| `DEFAULT_STAIR_TREAD_MM` | 280 | Default-Auftritt bei Anlage | [LH-FA-STR-001](lastenheft.md#lh-fa-str-001--treppe-erzeugen) |
| `AUTOSAVE_INTERVAL_S` | 300 | Autosave-Intervall | [LH-QA-004](lastenheft.md#lh-qa-004--autosave) |
| `UNDO_DEPTH_MIN` | 1000 | Mindesttiefe Undo/Redo | [LH-QA-003](lastenheft.md#lh-qa-003--undoredo) |
| `PROJECT_OPEN_BUDGET_S` | 3 | Performance-Budget Projektöffnung (Standardprojekt) | [LH-QA-001](lastenheft.md#lh-qa-001--performance-projektöffnung) |
| `MEMORY_BUDGET_GB` | 2 | RAM-Budget Standardprojekt | [LH-QA-002](lastenheft.md#lh-qa-002--speicherverbrauch) |
| `SUPPORTED_LOCALES` | `de`, `en` | Mehrsprachigkeit | [LH-QA-006](lastenheft.md#lh-qa-006--mehrsprachigkeit) |
| `LIVING_AREA_FACTOR` | 1 | Wohnflächen-Anrechnungsfaktor (welle-3-Teilumfang: Wohnfläche = Netto-Grundfläche; Schrägen-/Balkon-Faktoren offen) | [LH-FA-EVL-003](lastenheft.md#lh-fa-evl-003--wohnflächenberechnung) |

Die Default-**Wandhöhe** bei Anlage ist die **Höhe des Geschosses**
(parametrisch, kein eigener Konstant; [LH-FA-WAL-001](lastenheft.md#lh-fa-wal-001--wand-zeichnen)). `slice-003a` hat
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
| `bcad.project.save` | `element_count`, `duration_ms`, `atomic_replace` | [LH-FA-BLD-002](lastenheft.md#lh-fa-bld-002--projekt-speichern), [LH-QA-005](lastenheft.md#lh-qa-005--crash-recovery) |
| `bcad.project.open` | `element_count`, `duration_ms` | [LH-QA-001](lastenheft.md#lh-qa-001--performance-projektöffnung) |
| `bcad.geometry.rebuild` | `element_id`, `op`, `duration_ms` | [LH-FA-D3-002](lastenheft.md#lh-fa-d3-002--echtzeitaktualisierung) |
| `bcad.room.detect` | `wall_count`, `room_count`, `duration_ms` | [LH-FA-ROM-001](lastenheft.md#lh-fa-rom-001--raum-automatisch-erkennen) |
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
| OpenCascade | im Adapter gepinnt | `GeometryKernelPort` — Solids, boolesche Operationen, Extrusion |
| Qt | 6.x (REQ-TEC-002) | GUI-Adapter, keine Kern-Bindung |
| SQLite | im Adapter gepinnt | `ProjectRepositoryPort`, atomar |
| IFC | Schema-Version offen (ADR-Folge) | `ModelImporterPort`/`ModelExporterPort` |

## 7. Offene Punkte

- ~~Raumerkennung: Mittellinie vs. Innenkante als Polygon-Basis
  (beeinflusst Wohnflächenberechnung [LH-FA-EVL-003](lastenheft.md#lh-fa-evl-003--wohnflächenberechnung))~~ → entschieden
  (Innenkante, Ring-Modell; §1).
- Performance-Zielkomplexität der Raumerkennung (M3).
- IFC-Schema-Version und -Bibliothek (ADR in welle-4-austausch).
- Zielplattformen (siehe `releasing.md`).

## 8. Historie

Ausgelagert nach [`spezifikation-historie.md`](spezifikation-historie.md)
(slice-018a / [MR-011](../harness/conventions.md#mr-011--referenz-integritäts-gate-matrix-ids-spans-hostpaths) — Provenance-Datei außerhalb der `matrix`-Spec-Straten).

## 9. Technische Rahmenbedingungen (REQ-TEC)

Die technischen Rahmenbedingungen aus dem Domänen-Ursprung. Sie sind
**fortschreibbar** (technische Schicht) — ADRs dürfen sie schärfen, das
Lastenheft bleibt unberührt. ID-Klasse `REQ-TEC-<NNN>` deklariert in
[`../harness/conventions.md` MR-002](../harness/conventions.md#mr-002--id-schema-für-b-cad).

| ID | Rahmenbedingung | Wahl |
|---|---|---|
| REQ-TEC-001 | Sprache | C++20 |
| REQ-TEC-002 | GUI | Qt 6 |
| REQ-TEC-003 | Geometrie-Kern | OpenCascade |
| REQ-TEC-004 | Build | CMake |
| REQ-TEC-005 | Tests | GoogleTest |
| REQ-TEC-006 | Logging/Observability | OpenTelemetry |
| REQ-TEC-007 | Persistenz | SQLite |
| REQ-TEC-008 | Plugin-Architektur | Shared Libraries |
| REQ-TEC-009 | Containerisierung | Docker DevContainer |
