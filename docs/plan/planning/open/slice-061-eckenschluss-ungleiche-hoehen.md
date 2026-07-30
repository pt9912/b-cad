---
id: slice-061
titel: Eckenschluss bei ungleichen Wandhöhen ([LH-FA-WAL-006](../../../../spec/lastenheft.md#lh-fa-wal-006--wand-verbinden) Boundary) — vertagt aus [MR-009](../../../../harness/conventions.md#mr-009--geometrielastiges-code-review-vor-welle-closure)-HIGH-2
status: open
welle: (keine — nach welle-6 zu terminieren)
lastenheft_refs: [[LH-FA-WAL-006](../../../../spec/lastenheft.md#lh-fa-wal-006--wand-verbinden), [LH-FA-WAL-003](../../../../spec/lastenheft.md#lh-fa-wal-003--wandhöhe-definieren)]
adr_refs: [[ADR-0001](../../adr/0001-hexagonale-architektur.md), [ADR-0002](../../adr/0002-geometrie-kern-opencascade.md)]
---

# Slice 061: Eckenschluss bei ungleichen Wandhöhen

**Status:** open — **Skelett** (Scope-Reservierung + Deferral-Beleg). Detail-Schnitt,
**ADR-Entscheidung** und
[MR-006](../../../../harness/conventions.md#mr-006--unabhängiges-plan-review-vor-implementierungs-start)
folgen **beim Start**.

## Herkunft: ein vertagter HIGH-Befund

[MR-009](../../../../harness/conventions.md#mr-009--geometrielastiges-code-review-vor-welle-closure)-Code-Review
vom 2026-07-29 ([`Report`](../../../reviews/2026-07-29-mr-009-bauteil-strang.md)), **HIGH-2**.

**Der Befund, gemessen:** die Boundary-Zeile von
[LH-FA-WAL-006](../../../../spec/lastenheft.md#lh-fa-wal-006--wand-verbinden) — *„Given ungleiche
Wandhöhen, then ist die Ecke bis zur niedrigeren Wandhöhe geschlossen, **darüber endet die höhere Wand
stumpf**"* — ist **nicht umgesetzt**. Der Footprint ist höhen-blind; der Adapter extrudiert das
gemiterte Polygon über die **volle** Höhe. Oberhalb der niedrigeren Wand steht damit ein
**freitragender Eck-Sporn** und daneben eine gleich große **Kerbe**: 465 von 3721 Abtastpunkten je,
über die volle Höhendifferenz.

**Warum kein Bestands-Orakel das sieht:** der Eckschnitt ist **flächen-erhaltend**. Das Solid-Volumen
stimmt auf den Kubikmillimeter mit dem stumpfen Ende überein (`3 840 000 000 mm³`); Volumen-,
Bounding-Box- und Dreiecks-Orakel sind blind. Der Fehler ist ausschließlich an der **Form** sichtbar —
die Begründung von
[MR-009](../../../../harness/conventions.md#mr-009--geometrielastiges-code-review-vor-welle-closure)
im Wortlaut.

**Warum er erst jetzt zählt:** innerhalb eines Geschosses waren bisher **alle** Wandhöhen gleich
(`addWall` erbt die Geschosshöhe, `setWallHeight` hatte keine Oberflächen-Anbindung).
[`slice-059b`](../done/slice-059b-wand-parameter-aendern.md) hat den Fall erstmals im **Bedienfluss**
erreichbar gemacht.

## Die Vertagung ist eine Entscheidung, keine Erledigung

**Entschieden vom Projektinhaber am 2026-07-29:** HIGH-2 wird von
[`slice-060`](slice-060-eckenschluss-kollaps-kriterium.md) **abgetrennt**; die welle-6-Closure erfolgt
mit diesem **offenen HIGH**.

**Das ist eine benannte Abweichung von
[MR-009](../../../../harness/conventions.md#mr-009--geometrielastiges-code-review-vor-welle-closure)**,
die schreibt: „**HIGH-Findings blockieren die Closure** und werden vorher behoben." Die Regel kennt
**keine** Vertagungs-Klausel — anders als
[MR-020](../../../../harness/conventions.md#mr-020--adr-folgepflicht-sichtbarkeit-closure-disziplin),
die für ADR-Folgepflichten ausdrücklich eine Deferral-Entscheidung vorsieht. **Die Abweichung steht
deshalb im Wellen-Ergebnis**, nicht nur hier, und sie ist an einen benannten Nachfolger gebunden —
dieses Skelett ist der Beleg dafür, dass sie terminiert.

**Warum die Vertagung vertretbar ist** (die Abwägung, nicht ihre Rechtfertigung):

- Der Fehler ist **nicht erreichbar ohne bewusste Höhen-Änderung** einer einzelnen Wand — er entsteht
  nicht beim Zeichnen, sondern erst, wenn ein Benutzer zwei sich berührende Wände auf verschiedene
  Höhen setzt.
- Er **korrumpiert nichts**: der Körper ist geschlossen, konsistent orientiert und volumen-treu; er
  hat die falsche **Form**. Das unterscheidet ihn scharf von HIGH-1, wo ein Körper offen war.
- Der Fix ist **kein Fix-Aufwand, sondern ein Entwurf** (s. u.) — ihn in einen Fix-Slice zu drängen
  hieße, eine Architektur-Entscheidung unter Zeitdruck zu treffen.

## Die Entscheidung, die dieser Slice treffen muss

Der Eckenschluss ist heute eine **reine 2D-Footprint-Regel**, und die Spezifikation schreibt fest:
*„der Geometrie-Adapter extrudiert/tesselliert **nur noch das Polygon** (Prisma in +Z auf
Wandhöhe)"*. Die Boundary-Zeile verlangt aber ein Verhalten, das **von z abhängt**.

| Weg | Konsequenz |
|---|---|
| **Höhen-geschichteter Körper** — gemiterter Footprint bis `min(hA, hB)`, stumpfer darüber | bricht die zitierte Spec-Zusage: ein Wandkörper ist dann **zwei** gestapelte Extrusionen. Berührt Tessellation, STEP/STL-Export, Volumen-Auswertung und die Golden |
| **Boolean-Schnitt** — volles Prisma, Sporn oberhalb `min(hA, hB)` abschneiden | hält die Footprint-Hoheit formal, führt aber eine Boolean-Operation in den Wand-Pfad ein (bisher nur bei Öffnungen) |
| **Teilumfang-Rücknahme** — die AK-Zeile wird als Reifephase-Grenze zurückgenommen | ehrlich und billig, **nimmt aber eine zugesagte Fähigkeit zurück** — eine Produkt-Entscheidung, keine technische |

**Alle drei sind vertretbar; keine ist offensichtlich.** Die ersten zwei brauchen mindestens einen
**Spec-Nachzug**, vermutlich eine **ADR** (die Footprint-Hoheit ist ein tragendes Prinzip); der dritte
braucht eine Lastenheft-Schärfung mit Historien-Zeile und die Zustimmung des Projektinhabers.

## Was beim Start zu tun ist

1. **Entscheidung treffen** (die drei Wege oben), mit Messung statt Vermutung: was kostet die
   Schichtung im Export/in der Auswertung, und was kostet der Boolean an Laufzeit und Robustheit?
2. Detail-Schnitt + eigenes
   [MR-006](../../../../harness/conventions.md#mr-006--unabhängiges-plan-review-vor-implementierungs-start).
3. **Ein Orakel für die Form**, nicht für das Volumen — die Abtast-Sonde des Review-Belegs (Raster
   über dem Eck-Kasten, je Ebene z) ist die Bauform. **Volumen-, Bounding-Box- und Dreiecks-Orakel
   sind gegen diesen Fehler strukturell blind**; das ist der Kern der Lehre.
4. Den [MR-009](../../../../harness/conventions.md#mr-009--geometrielastiges-code-review-vor-welle-closure)-Report
   auf „behoben" bzw. „durch Teilumfang zurückgenommen" nachziehen — **die Vertagung endet erst
   dort**.
