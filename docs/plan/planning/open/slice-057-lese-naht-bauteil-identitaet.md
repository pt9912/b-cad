---
id: slice-057
titel: Die 2D-Lese-Naht bekommt Bauteil-Identität und eine schmale Parameter-Abfrage ([ADR-0021](../../adr/0021-wand-im-2d-canvas.md) E11/E15)
status: open
welle: welle-6-interaktiv-planen
lastenheft_refs: [[LH-FA-WAL-001](../../../../spec/lastenheft.md#lh-fa-wal-001--wand-zeichnen), [LH-FA-WAL-002](../../../../spec/lastenheft.md#lh-fa-wal-002--wandstärke-definieren), [LH-FA-DRW-005](../../../../spec/lastenheft.md#lh-fa-drw-005)]
adr_refs: [[ADR-0001](../../adr/0001-hexagonale-architektur.md), [ADR-0019](../../adr/0019-drw-2d-canvas.md), [ADR-0020](../../adr/0020-driven-adapter-serialisieren-kern-liefert-geometrie.md), [ADR-0021](../../adr/0021-wand-im-2d-canvas.md)]
---

# Slice 057: Bauteil-Identität in der 2D-Lese-Naht

**Status:** open — **Skelett** (Scope-Reservierung + ADR-Bezug, [MR-020](../../../../harness/conventions.md)
§3). Der Detail-Schnitt und das
[MR-006](../../../../harness/conventions.md#mr-006--unabhängiges-plan-review-vor-implementierungs-start)
folgen **beim Start**.

**Welle:** welle-6-interaktiv-planen. **Vorgelagert, ohne UI-Anteil** — Voraussetzung für
[`slice-059`](slice-059-wand-auswaehlen-und-aendern.md);
[`slice-058`](slice-058-wand-zeichnen-im-canvas.md) kommt **ohne** ihn aus.

## Auslöser

[ADR-0021](../../adr/0021-wand-im-2d-canvas.md) **Entscheidung 11 und 15**. Die 2D-Lese-Naht liefert
heute **anonyme** Segmente: vier Koordinaten, keine Herkunft; Wand-Achsen und sichtbare Hilfslinien
liegen in **einer** Liste. Eine Treffer-Prüfung hätte damit nichts zu benennen, und eine
Parameter-Änderung keine Identität zu adressieren.

## Umfang

- **Herkunft je Segment** (Art + Identität), **additiv** — der Werttyp der Projektion wächst, die
  Projektion selbst bleibt **eine** Quelle für Bildschirm und Export.
- **Schmale Detail-Abfrage** zu einer Bauteil-Identität: die **änderbaren** Parameter (Stärke, Höhe)
  als purer Werttyp. **Nicht** ins Segment — es reist im Ableitungs-Bündel zu den Export-Adaptern.
- **Kein** UI-Anteil, **kein** neuer Port-Typ jenseits der bestehenden Lese-Naht.

## Warum vorgelagert

Der Werttyp, den dieser Slice erweitert, wird von **PDF- und PNG-Export** konsumiert und liegt den
**Export-Golden** zugrunde. Das ist ein anderer Risiko-Grad als UI-Interaktion — und es braucht die
Export-Orakel als **eigenes** Netz, das ein Slice, der gleichzeitig ein Werkzeug baut, nicht mehr
sauber zeigen kann. Präzedenz: die Hebung der Projektion in den Kern lief ebenfalls **vor** dem
Canvas.

## Der Orakel-Kern, der diesen Slice ausmacht

**Die sechs Export-Golden bleiben byte-identisch.** Das ist der Additivitäts-Beleg: bleibt kein
einziges Byte anders, sieht **kein** Encoder die neue Herkunft, und die Erweiterung ist erwiesen
folgenlos für den Export-Pfad. Fällt auch nur ein Golden, ist die Erweiterung **nicht** additiv —
dann ist die Entscheidung, nicht der Test, zu korrigieren.

## Benannte Beobachtungspflicht

[ADR-0020](../../adr/0020-driven-adapter-serialisieren-kern-liefert-geometrie.md) führt „das Bündel
wird zu breit" als Re-Eval-Anlass. **Eine** optionale Herkunft je Segment zieht ihn nicht; eine Folge
weiterer Felder täte es. Der Slice hält fest, wo die Grenze verläuft.

## Sub-Area-Modus-Begründung

### Sub-Area: Domänen-Modell + Ports (Hexagon-Kern)

- **Modus:** GF; **Dichte:** klein-mittel — ein Feld, eine Abfrage, ein Netz aus Bestands-Orakeln.
- **Risiko:** mittel — der Werttyp ist geteilt; das Risiko liegt **nicht** im Schreiben, sondern in
  der Frage, ob die Erweiterung wirklich additiv ist. Genau dafür die Golden.
