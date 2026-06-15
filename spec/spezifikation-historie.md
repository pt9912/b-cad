# Spezifikation — Historie (Provenance)

Ausgelagerte Änderungs-Historie von [`spezifikation.md`](spezifikation.md)
(slice-018a / `MR-011`). Diese Datei ist **derivative Provenance** und liegt
**außerhalb der `matrix`-Spec-Straten** — so bleiben die ADR-Aufwärts-Verweise
(Regelwerk Regel 5: Provenance am Dokument-Rand) erlaubt **verlinkt**, ohne die
Referenz-Richtungs-Regel der Spec-Straten zu berühren. **Keine eigene
Anforderung**, nicht normativ zitierbar.

| Datum | Änderung | ADR |
|---|---|---|
| 2026-06-08 | Initiale Outline aus Lastenheft-Wertebereichen; Fehler-Codes und OTel-Span-Skelett | Greenfield-Bootstrap |
| 2026-06-11 | §1 `LH-FA-ROM-001`.a präzisiert: Innenkanten-Basis + Ring-Modell, Auslösung bei Modell-Mutation, Endpunkt-Knoten-Einschränkung (welle-1), Erkennung total (kein `E-GEO-002`); §7-Punkt Polygon-Basis geschlossen | [ADR-0007](../docs/plan/adr/0007-raumerkennung-geometrie-basis.md) |
| 2026-06-11 | §1 Kollaps-Kriterium präzisiert: Kantenrichtungs-Erhalt statt reiner Flächen-Prüfung (Doppel-Inversion erzeugt Phantom-Polygon positiver Fläche) | [ADR-0007](../docs/plan/adr/0007-raumerkennung-geometrie-basis.md) |
| 2026-06-11 | §1 `LH-FA-D3-002`.a ergänzt: Benachrichtigungs-Vertrag (Observer-Port, Push-Notify/Pull-State, Reihenfolge nach Re-Detektion, Beobachter-Pflichten) + welle-1-Operationalisierung „sichtbar" | [ADR-0008](../docs/plan/adr/0008-aenderungs-benachrichtigung.md) |
| 2026-06-11 | §1 ROM-001.a präzisiert (Welle-1-Code-Review M1/M2): minimale Zyklen via Flächen-Traversierung — geteilte Knoten (Grad ≥ 3) abgedeckt, Stichkanten ignoriert; Näherung für kollineare Nachbarkanten ungleicher Stärke dokumentiert | [ADR-0007](../docs/plan/adr/0007-raumerkennung-geometrie-basis.md) |
| 2026-06-12 | §1 D3-002.a ergänzt: welle-1v-Operationalisierung „sichtbar" (Qt-Widgets-Fenster, Tessellation über `ViewModelPort`, Szenen-Surrogat, `ACC-002`-Beleg als manueller Abnahme-Schritt) | [ADR-0009](../docs/plan/adr/0009-gui-framework-qt6.md) |
| 2026-06-12 | §1 `LH-FA-WAL-006`.a neu (Eckenschluss-Footprint-Regel, Footprint-Hoheit im Kern, Begrenzung/Rückfälle, EVL-Hinweis Shoelace) + D3-002.a-Mehr-Element-Update (`WallGeometryChanged`, Reihenfolge, Transaktions-Satz) + §3 `WALL_MITER_LIMIT`; zwei WAL-006-Verweise auf Vollumfang präzisiert | slice-012 (Lastenheft 0.1.2) |
| 2026-06-13 | §1 `LH-FA-DOR-004`.a/WIN-005.a neu (Wandöffnung als Schnitt-Prismen im Kern, boolesche Subtraktion über `GeometryKernelPort`, Klemmung/Ablehnung, Totalität/Transaktion, `WallGeometryChanged` der Wirtswand, Raumerkennung/Footprint unberührt) + §3 Tür-/Fenster-/Brüstungs-Wertebereiche | [ADR-0011](../docs/plan/adr/0011-bauteil-hosting-wandoeffnung.md) (slice-013a) |
| 2026-06-13 | §3 Default-Maße bei Tür-/Fenster-Anlage (`DEFAULT_DOOR_*`/`DEFAULT_WINDOW_*`) — Implementierung der Anlage (Muster `DEFAULT_WALL_THICKNESS_MM`) | slice-013b |
| 2026-06-13 | §3 `OPENING_CUT_OVERSHOOT_MM` — Cutter-Überstand für koplanar-freien Boolean (Code-Review-Befund H1: §1-Überstand „je Seite" war nur in Z realisiert, jetzt auch lateral) | slice-013b Code-Review |
| 2026-06-13 | §1 `LH-FA-ROF-001.a` neu (Dach-Geometrie Teilumfang Rechteck-Grundriss: Traufrechteck, Pult/Sattel/Walm-Konstruktion + Höhenformeln, Walm-Einrückbetrag, Firsthöhe abgeleitet, Totalität) + §3 Neigungs-/Überstands-Bereiche + Defaults (= `roofs`-Schema) | slice-014a |
| 2026-06-13 | §1 `LH-FA-SLB-001.a` neu (Platten-Geometrie Decken+Fundament: Polygon × Dicke an `base_z` je `slab_type`, Ausschnitte als Boolean/`CutPrism`, Totalität; Port-base_z-Frage an 015b) + §3 Decken-/Fundament-Dicke-Bereiche + Defaults | slice-015a |
| 2026-06-13 | §1 `LH-FA-SLB-001.a` Port-base_z-Frage geschlossen: kein Port-Wechsel — Mesh-Translation um `base_z` nach dem Boolean, Cutouts relativ `[0,Dicke]`; `SlabChanged`-`op` (storey-bezogen, kein `RoomsChanged`) | slice-015b |
| 2026-06-13 | §1 `LH-FA-SLB-001.a` „auf den Platten-Umriss begrenzt" präzisiert: rand-/außenliegende, degenerierte und nicht-endliche Ausschnitte werden an der API **abgelehnt** (Containment-Vorbedingung) — innenliegende Aussparungen sind damit koplanar-frei, **kein** lateraler Überstand nötig (anders als die Wand durchspannende Öffnung, §1 DOR-004.a) | slice-015b Code-Review (H1) |
| 2026-06-14 | §1 `LH-FA-STR-001.a` neu (Treppen-Geometrie Teilumfang gerade einläufige Treppe: Stufen-Quader-Polyeder im Kern wie `roof_geometry`, `rise = Geschosshöhe/step_count` abgeleitet, feste +x-Aufstiegsrichtung, Geländer als generierte Geometrie ohne Schema-Spalte, `StairChanged`-`op` an `from_storey` gebunden + `stairMeshes` projektweit) + §3 Stair-Wertebereiche (Breite/Stufenanzahl/Auftritt geklemmt, Steigung informativ) + Defaults | slice-016a |
| 2026-06-14 | §1 `LH-FA-EVL-001.a` neu (Auswertungs-Architektur [ADR-0012](../docs/plan/adr/0012-evaluations-architektur.md): `EvaluatePort` read-only/pull; Fläche = Shoelace-Raum-Netto, **Netto-Volumen analytisch im Kern** = Footprint·Höhe − geklemmtes Öffnungsvolumen, **kein** Roh-Prisma/OCC-`GProp`; benannte Miter-Eck-Näherung; Listen-Aggregation über material-tragende `walls`/`roofs`/`slabs`, `windows.frame_material`-Freitext ausgenommen; Material-Auflösungsregel eigenes vs. `wall_type`) + §2.1 `model::Material` + FK-Autorität + §3 `LIVING_AREA_FACTOR` | slice-017a |
