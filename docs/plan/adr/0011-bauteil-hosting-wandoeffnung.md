# ADR-0011: Bauteil-Hosting & Wandöffnungs-Modell

**Status:** Accepted

**Datum:** 2026-06-13

**Autor:** Dietmar Burkard (slice-013a, ausgearbeitet im AI-Harness-Lauf)

**Bezug:** LH-FA-DOR-001..004, LH-FA-WIN-001..005 (Türen/Fenster mit
automatischer Wandöffnung), ADR-0001 (Kern führt, Port-Signatur =
Kern-Hoheit), ADR-0002 (OCC-Boolean-Backend hinter `GeometryKernelPort`,
für Wandöffnungen reserviert), ADR-0006 (`openings → doors/windows`-
Schema), ADR-0007 (Raumerkennung — Innenkante/Wandzug), ADR-0008
(Benachrichtigungs-Vertrag, `WallGeometryChanged`)

---

## Kontext

welle-2-bauteile liefert die parametrischen Bauteile über die Wände
hinaus. Türen (LH-FA-DOR-*) und Fenster (LH-FA-WIN-*) sind der erste
Fall — und der einzige **wand-gehostete**: ein platziertes Tür-/
Fenster-Bauteil erzeugt eine **Öffnung in seiner Wirtswand**
(LH-FA-DOR-004/WIN-005). Vor der Implementierung (slice-013b) sind drei
Lösungsfragen offen, die der reine Spec-Text nicht entscheidet:

1. **Hosting-Repräsentation:** Wie wird ein wand-gehostetes Bauteil im
   framework-freien Domänen-Modell dargestellt (eigenständiges Element
   mit Wand-Referenz vs. Kind der Wand), und wie wird seine Position
   parametrisiert?
2. **Geometrie & Port-Signatur:** Die Wand wird seit slice-012 aus einem
   **Footprint-Polygon** im Kern extrudiert (`extrudeFootprint`/
   `tessellateFootprint`). Wie wird die Öffnung — die laut
   `spezifikation.md` §1 D3-001.a eine **boolesche Subtraktion** ist —
   in dieses Bild eingefügt, ohne Wand-/Tür-Semantik in den
   Geometrie-Adapter zu lecken (ADR-0001, Regel C)?
3. **Folge-Meldung & Transaktion:** Eine Öffnungs-Mutation ändert die
   Geometrie der Wirtswand. Welcher `ModelChangedPort`-`op` wird
   gemeldet, in welcher Reihenfolge, und wie bleibt die
   transaktionale Garantie (Post-Commit total) erhalten?

Zusätzlich braucht die Welle eine **Leitplanke**: ROF/SLB/FND/STR sind
keine wand-gehosteten Öffnungen, folgen aber demselben *Integrations-
Pfad* (Domain → Port → Geometrie → Notification → Persistenz → Viewer).
Diese ADR hält das Muster **einmal** fest, damit die Folge-Slices keine
je eigene Grundsatz-ADR brauchen (nur ihre eigene Geometrie-/Spec-
Entscheidung).

Das Persistenz-Schema ist **nicht** offen: `data-model.yaml` / `schema.sql`
(ADR-0006) tragen `openings → doors/windows` bereits vollständig
(`offset_mm`, `width_mm`, `height_mm`, `sill_height_mm`, `swing_direction`,
`swing_angle_deg`, `is_external`, `frame_material`, `glazing_type`,
`u_value`).

## Entscheidung

1. **Hosting-Repräsentation — eigenständiges Element mit Wand-Referenz.**
   Eine Öffnung ist ein **pures Werttyp-Element** in
   `src/hexagon/model/` (framework-frei) mit einer Referenz auf ihre
   **Wirtswand** (`wall_id`) — kein verschachteltes Kind der Wand,
   spiegelbildlich zur Schema-Abbildung (`openings.wall_id`,
   Class-Table-Inheritance `doors`/`windows`). **Position parametrisch
   entlang der Wandachse** über `offset_mm` (Lage relativ zum
   Wand-Startpunkt) + `width_mm`; vertikale Lage über `sill_height_mm`
   (Brüstung, Default 0 für Türen) + `height_mm`. Tür-spezifisch:
   `swing_direction`/`swing_angle_deg`/`is_external`; Fenster-spezifisch:
   `frame_material`/`glazing_type`/`u_value`. Die exakte
   Offset-Semantik (Bezug auf Nahkante der Öffnung) legt
   `spezifikation.md` §1 fest, nicht diese ADR.

2. **Geometrie — Kern liefert reine Schnitt-Prismen, Port subtrahiert.**
   Die Öffnung wird als **boolesche Subtraktion** aus dem extrudierten
   Wand-Solid entfernt. **Der Kern berechnet die Schnitt-Geometrie als
   reine Werte** (Offset/Breite/Brüstung/Höhe + Wandstärke → ein
   achsen-ausgerichtetes **Schnitt-Prisma** je Öffnung, ausgedrückt als
   Polygon + z-Bereich — dieselbe Werte-Sprache wie das Footprint-
   Polygon aus slice-012); der **`GeometryKernelPort` extrudiert das
   Footprint-Polygon und subtrahiert die Schnitt-Prismen** und liefert
   das Netto-Wand-Solid bzw. dessen Tessellation. **Die Port-Signatur
   ist ADR-0001-Hoheit des Kerns** (slice-012 §6) — ADR-0002 liefert nur
   das OCC-**Boolean-Backend** (`BRepAlgoAPI_Cut`) **hinter** dem Port.
   Der Adapter kennt nur „Polygon extrudieren, Prismen subtrahieren,
   tessellieren" — **keine** Wand-/Tür-/Fenster-Semantik; **kein
   OCC-Typ verlässt den Adapter** (Regel C). Die exakte neue Signatur
   (Erweiterung von `extrudeFootprint` um eine Schnitt-Prismen-Liste vs.
   eigene Operation) legt slice-013b fest.

3. **Geometrie-Erzeugung total + transaktional.** Eine fehlgeschlagene/
   degenerierte Subtraktion meldet `E-GEO-002` (§4); das neue Wand-Solid
   entsteht **vor dem Commit** — schlägt es fehl, bleibt das Modell
   unverändert und es ergeht keine Meldung (Muster slice-012
   Mehr-Element-Transaktion).

4. **Folge-Meldung — `WallGeometryChanged` der Wirtswand, kein neuer op.**
   Die Öffnung ist in welle-2 ein **Hohlraum im Wand-Solid**, kein
   eigenständig gerendertes Solid. Jede Öffnungs-Mutation (anlegen,
   verschieben, Parameter ändern, löschen) meldet daher
   `op = WallGeometryChanged` für die **Wirtswand** (Wiederverwendung
   des slice-012-`op`, §5-Span-Vokabular `bcad.geometry.rebuild`) — kein
   neuer `op`. **Keine Raum-Re-Detektion** (Entscheidung #5).
   Abgelehnte/verworfene Öffnungs-Mutationen melden nicht.

5. **Raumerkennung & Footprint unberührt.** Eine Öffnung ändert **weder
   die Wandachse (Segment) noch die Wandstärke** → die Raumerkennung
   (ADR-0007, rechnet auf Segment-Achsen + Stärken) und der
   Footprint/Eckenschluss (slice-012) sind **unberührt**; die ROM- und
   WAL-006-AK-Tests bleiben textlich unverändert grün. Die Öffnung wirkt
   nur auf das **3D-Wand-Solid** (subtrahierter Hohlraum), nicht auf das
   Grundriss-Footprint-Polygon. Eine 2D-Darstellung der Öffnung
   (Wand-Lücke/Schwenkbogen) ist **nicht** Teil dieser Entscheidung und
   bleibt offen (Re-Evaluierungs-Trigger).

6. **Bauteil-Erweiterungs-Muster (Welle-Leitplanke).** Ein neuer
   parametrischer Bauteil-Typ (Öffnung jetzt; ROF/SLB/FND/STR folgend)
   wird über **denselben Integrations-Pfad** eingeführt; jede Stufe ist
   an einem bestehenden Vertrag verankert:

   1. **Domäne:** pures Werttyp-Element in `src/hexagon/model/`
      (framework-frei, ADR-0001), mit Referenzen statt Verschachtelung.
   2. **Driving-Port:** Operationen (anlegen/verschieben/ändern/löschen)
      am `EditStructurePort`, Parameter gegen §3-Wertebereiche validiert
      (Klemmung → `E-VAL-001`, LH-FA-WAL-002.a).
   3. **Geometrie (Driven):** der Kern liefert **reine
      Geometrie-Werte** (Footprint-Polygone, Schnitt-Prismen, Solid-
      Parameter); der `GeometryKernelPort` extrudiert/subtrahiert/
      tesselliert (OCC-Backend, ADR-0002); keine Domänen-Semantik im
      Adapter, kein OCC-Leck (Regel C). Erzeugung total + transaktional
      vor Commit (#3).
   4. **Notification:** `ModelChangedPort`-`op` je betroffenem Element —
      **bestehenden `op` wiederverwenden**, wenn „die Geometrie eines
      bestehenden Typs änderte sich" (z. B. `WallGeometryChanged`),
      sonst **neuer `op` im §5-Span-Vokabular**; deterministische
      Reihenfolge; total (Post-Commit, ADR-0008 §3).
   5. **Persistenz:** Abbildung auf die vorhandene ADR-0006-Schema-
      Tabelle (für openings/roofs/slabs/stairs bereits modelliert);
      **nur** falls ein Feld fehlt → Schema-Schärfung + `make schema-check`
      (separat, **nicht** in `gates`, MR-007).
   6. **Viewer:** `ViewerScene` behandelt den `op` (Pull + idempotentes
      Ersetzen, ADR-0009), display-frei AK-getestet.
   7. **AK-Tests** mit `LH-`-ID im Namen; arch-check-Regeln A–E halten.

   Die Leitplanke betrifft den **Integrations-Pfad**, nicht die je
   eigene **Geometrie-Entscheidung** (Dachform, Decken-Ausschnitt …) —
   die lebt im jeweiligen Folge-Slice (Lastenheft-AK + Spec), nicht in
   einer neuen Grundsatz-ADR.

## Verglichene Alternativen

### Hosting (Entscheidung #1)

**Option A — eigenständiges Element mit Wand-Referenz (gewählt).** Pro:
deckungsgleich mit dem ADR-0006-Schema (`openings.wall_id` + CTI),
keine Schema-Schärfung; Öffnung über `wall_id` eindeutig zugeordnet,
mehrere Öffnungen je Wand trivial; ADR-0001-Richtung gewahrt. Contra:
referenzielle Integrität (verwaiste Öffnung bei Wand-Löschung) muss im
Service/Schema (`on_delete: cascade`) gehalten werden.

**Option B — Öffnung als verschachteltes Kind der Wand.** Pro: lokale
Kapselung. Contra: bricht die 1:1-Schema-Abbildung (CTI), erschwert die
Persistenz-Abbildung und die getrennte Adressierung beim Verschieben;
Wand-Werttyp würde aufgebläht.

### Geometrie / Port-Signatur (Entscheidung #2)

**Option A — Kern liefert Schnitt-Prismen, Port subtrahiert (gewählt).**
Pro: hält die Wand-/Öffnungs-Semantik im Kern (ADR-0001), der Adapter
bleibt ein reiner Geometrie-Rechner (Polygone/Prismen → Solid); fügt
sich nahtlos in die Footprint-Hoheit (slice-012) ein; OCC gekapselt
(Regel C). Contra: Port-Signatur wächst um die Schnitt-Prismen-Liste.

**Option B — Adapter kennt „Öffnungen" und schneidet selbst.** Pro:
Port-Signatur bleibt schlanker. Contra: Wand-/Öffnungs-Semantik leckt
in den Geometrie-Adapter (ADR-0001-Verletzung); der Adapter müsste
Offset/Brüstung interpretieren — genau die Zuständigkeit, die slice-012
in den Kern verlegt hat.

### Folge-Meldung (Entscheidung #4)

**Option A — `WallGeometryChanged` der Wirtswand wiederverwenden
(gewählt).** Pro: die Öffnung *ist* eine Änderung der Wand-Geometrie;
der Viewer behandelt sie wie jede andere Wand-Geometrie-Änderung
(slice-012); kein neues Vokabular. Contra: bei künftigem eigenen
Tür-/Fenster-Solid (Blatt/Rahmen) braucht es doch einen eigenen `op`
(dann Re-Eval).

**Option B — eigener `op` `OpeningChanged`.** Pro: explizit. Contra: in
welle-2 ist die Öffnung kein eigenes Solid, nur ein Wand-Hohlraum — ein
eigener `op` ohne eigenes gerendertes Element wäre Vokabular ohne
Beobachtungs-Korrelat.

## Konsequenzen

- **Positiv:** Türen/Fenster fügen sich ohne Schema-Änderung und ohne
  neue Notification-Vokabel in die bestehenden Verträge (slice-012-
  Footprint, ADR-0008-Notification, ADR-0006-Schema); die Welle-
  Leitplanke (#6) macht ROF/SLB/FND/STR zu „Geometrie-+-AK"-Slices ohne
  je eigene Grundsatz-ADR; Raumerkennung/Footprint bleiben beweisbar
  unberührt (#5).
- **Folgepflicht (slice-013b):** Domänen-Werttyp(en) für Öffnung/Tür/
  Fenster; `EditStructurePort`-Operationen mit §3-Validierung; Erweiterung
  der `GeometryKernelPort`-Signatur um Schnitt-Prismen + OCC-`BRepAlgoAPI_Cut`
  im Adapter (Regel C); `WallGeometryChanged` der Wirtswand bei
  Öffnungs-Mutation (total/transaktional); `ViewerScene`-Folgen;
  Persistenz-Abbildung auf `openings`/`doors`/`windows`; AK-Tests mit
  `LH-FA-DOR-*`/`LH-FA-WIN-*` im Namen.
- **ADR-0006 bleibt unverändert gültig** — das Öffnungs-Schema trägt
  bereits; keine Schema-Schärfung, kein `make schema-check`-Lauf in
  diesem Strang nötig.
- **ADR-0002 unberührt** — die ADR legt nur das OCC-Backend hinter dem
  Port fest, nicht die Port-Signatur (ADR-0001-Hoheit, slice-012 §6).
- **Offen (bewusst):** eigenes Tür-Blatt/Fenster-Rahmen-Solid und die
  2D-Öffnungs-Darstellung (Wand-Lücke/Schwenkbogen) sind nicht Teil
  dieser Entscheidung (Re-Eval-Trigger).

## Fitness Function

| Tooling | Regel | Make-Target |
|---|---|---|
| AK-Tests (slice-013b) | DOR/WIN Happy/Boundary/Negative gegen analytische Werte (Öffnungs-Volumen vom Wand-Solid subtrahiert; Klemmung; ohne Wirtswand abgelehnt) | `make test` |
| Schichtung | Öffnungs-/Wand-Semantik bleibt pure Domäne; kein OCC-Typ verlässt `src/adapters/geometry/` (Regel C); kein Qt/SQLite-Leck | `make arch-check` (ADR-0001/0002) |
| Raumerkennung unberührt | ROM- + WAL-006-AK-Tests bleiben textlich unverändert grün (Öffnung ändert weder Achse noch Stärke) | `make test` |

## Re-Evaluierungs-Trigger

- **Eigenes Tür-/Fenster-Solid** (Blatt, Rahmen, Verglasung sichtbar) →
  eigenes Domänen-Solid + eigener `ModelChangedPort`-`op` neu bewerten
  (dann Supersedes/Folge-ADR, kein stiller `op`-Zuwachs).
- **2D-Öffnungs-Darstellung** (Grundriss-Lücke, Tür-Schwenkbogen,
  LH-FA-DRW-*) → Footprint-/Viewer-Vertrag gegen die Öffnung neu prüfen.
- **Nicht-rechteckige Öffnungen** (Rundbogen, Schräge) → Schnitt-Prismen-
  Annahme (Polygon + z-Bereich) gegen allgemeine Schnitt-Solids prüfen.
- **Bauteil außerhalb des Integrations-Pfads** (#6 trägt einen künftigen
  Typ nicht) → Leitplanke erweitern oder eigene ADR.

## Geschichte

| Datum | Ereignis | Verweis |
|---|---|---|
| 2026-06-13 | Proposed (slice-013a, ADR + AK-Schärfung DOR/WIN) | slice-013a |
| 2026-06-13 | Accepted — unabhängiger Text-Review vor Accept (keine HIGH/MEDIUM; umgebungsabhängige Behauptungen gegen Schema/ADR-0002/`WallGeometryChanged` belegt); Spec §1 D3-004.a/WIN-005.a + §3 + Lastenheft DOR/WIN umgesetzt | slice-013a |
</content>
