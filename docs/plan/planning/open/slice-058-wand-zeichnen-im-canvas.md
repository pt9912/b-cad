---
id: slice-058
titel: Wand zeichnen im 2D-Canvas ([LH-FA-WAL-001](../../../../spec/lastenheft.md#lh-fa-wal-001--wand-zeichnen), [ADR-0021](../../adr/0021-wand-im-2d-canvas.md))
status: open
welle: welle-6-interaktiv-planen
lastenheft_refs: [[LH-FA-WAL-001](../../../../spec/lastenheft.md#lh-fa-wal-001--wand-zeichnen), [LH-FA-WAL-006](../../../../spec/lastenheft.md#lh-fa-wal-006--wand-verbinden), [LH-FA-DRW-001](../../../../spec/lastenheft.md#lh-fa-drw-001), [LH-FA-DRW-005](../../../../spec/lastenheft.md#lh-fa-drw-005)]
adr_refs: [[ADR-0001](../../adr/0001-hexagonale-architektur.md), [ADR-0008](../../adr/0008-aenderungs-benachrichtigung.md), [ADR-0009](../../adr/0009-gui-framework-qt6.md), [ADR-0010](../../adr/0010-headless-gl-xvfb.md), [ADR-0018](../../adr/0018-drw-2d-zeichen-daten.md), [ADR-0019](../../adr/0019-drw-2d-canvas.md), [ADR-0021](../../adr/0021-wand-im-2d-canvas.md)]
---

# Slice 058: Wand zeichnen im 2D-Canvas

**Status:** open — **Skelett** (Scope-Reservierung + ADR-Bezug, [MR-020](../../../../harness/conventions.md)
§3). Detail-Schnitt und
[MR-006](../../../../harness/conventions.md#mr-006--unabhängiges-plan-review-vor-implementierungs-start)
folgen **beim Start**.

**Welle:** welle-6-interaktiv-planen — **erste Hälfte des Abschluss-Triggers** („eine Wand ist im
2D-Canvas **zeichenbar**"). **Unabhängig von [`slice-057`](../in-progress/slice-057-lese-naht-bauteil-identitaet.md)**:
Zeichnen braucht keine Bauteil-Identität.

## Auslöser

[ADR-0021](../../adr/0021-wand-im-2d-canvas.md), **Entscheidungen 1, 2, 5, 6, 7, 8, 9, 10, 12, 14**.

## Umfang

| Entscheidung | Was daraus wird |
|---|---|
| **1** | Werkzeug-Modus im Fenster (Hilfslinie · Wand · Auswahl), Default **Hilfslinie** |
| **2** | Ein Links-Zug im Wand-Modus ⇒ **eine** Wand; Teilumfang-Klausel steht bereits im Lastenheft |
| **9** | Der Endpunkt-Fang gilt auch hier — **Voraussetzung** des Eckenschlusses |
| **7** | Zweite `ui/command/`-Senke am Bauteil-Bearbeitungs-Port, port-frei verdrahtet |
| **6** | Refresh **ausschließlich** über die Änderungs-Meldung; **kein** Selbst-Refresh |
| **5**/**10** | Hinweis-Anzeige für **vier** Ausgänge; die Senke ist die **Fehler-Barriere** (der Port **wirft**) |
| **12** | Gesten-Abbruch ohne Modell-Mutation; **kein** Entfernen (benannte Grenze) |
| **8** | Die Erreichbarkeits-Klausel steht im Lastenheft — hier ist **nichts** zu bauen, und das ist der Punkt |
| **14** | Nachweis auf **zwei** Ebenen: Surrogat-Zustand **und** Farbmengen-Sonde am offscreen gerenderten Widget |

## Was diesen Slice gefährlich macht

**Der Fehler-Ausgang ist neu.** Der erste UI-Mutator lehnte **wertbasiert** ab; dieser Port
**wirft** — bei unbekannten Bezügen und bei Geometrie-Fehlschlag. Ein Wurf aus einem
Ereignis-Handler der Oberfläche ist kein theoretischer Fall: die Zeichen-Ziele sind injizierte
Identitäten, die nach einem Projekt-Wechsel neu aufgelöst werden.

**Und der Refresh ist gegenläufig zum Vorgänger.** Hilfslinien melden nichts, dort **ist** der
Selbst-Refresh die einzige Quelle; hier wäre er einer zu viel. Wer die Bauform des Hilfslinien-Zugs
kopiert, baut den Fehler ein.

## [MR-009](../../../../harness/conventions.md#mr-009--geometrielastiges-code-review-vor-welle-closure) ist EINSCHLÄGIG

`addWall` baut ein Solid, rechnet **Nachbar-Ecken** neu und stößt die **Raum-Neuerkennung** an.
[ADR-0018](../../adr/0018-drw-2d-zeichen-daten.md) hatte das Review für Hilfslinien ausdrücklich
verneint („keine neue Solid-Geometrie") — **diese Verneinung gilt hier nicht** und darf nicht geerbt
werden ([ADR-0021](../../adr/0021-wand-im-2d-canvas.md) §Konsequenzen).

## Sub-Area-Modus-Begründung

### Sub-Area: GUI-Adapter (2D-Canvas)

- **Modus:** GF; **Dichte:** mittel — ein Modus-Zustand, eine zweite Senke, eine Hinweis-Naht, zehn
  Entscheidungen umzusetzen.
- **Risiko:** mittel — nicht im Zeichnen (die Geste existiert), sondern im **Fehler-Ausgang** und im
  **Refresh-Pfad**, die beide gegenläufig zum Vorgänger sind.
