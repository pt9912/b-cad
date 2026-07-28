---
id: slice-059
titel: Wand auswählen und parametrisch ändern — der Abschluss-Trigger ([LH-FA-WAL-002](../../../../spec/lastenheft.md#lh-fa-wal-002--wandstärke-definieren)/003, [ADR-0021](../../adr/0021-wand-im-2d-canvas.md))
status: open
welle: welle-6-interaktiv-planen
lastenheft_refs: [[LH-FA-WAL-002](../../../../spec/lastenheft.md#lh-fa-wal-002--wandstärke-definieren), [LH-FA-WAL-003](../../../../spec/lastenheft.md#lh-fa-wal-003--wandhöhe-definieren), [LH-FA-WAL-007](../../../../spec/lastenheft.md#lh-fa-wal-007--wandtyp-wählen), [LH-FA-D3-002](../../../../spec/lastenheft.md#lh-fa-d3-002--echtzeitaktualisierung)]
adr_refs: [[ADR-0001](../../adr/0001-hexagonale-architektur.md), [ADR-0008](../../adr/0008-aenderungs-benachrichtigung.md), [ADR-0009](../../adr/0009-gui-framework-qt6.md), [ADR-0010](../../adr/0010-headless-gl-xvfb.md), [ADR-0019](../../adr/0019-drw-2d-canvas.md), [ADR-0021](../../adr/0021-wand-im-2d-canvas.md)]
---

# Slice 059: Wand auswählen und parametrisch ändern

**Status:** open — **Skelett** (Scope-Reservierung + ADR-Bezug, [MR-020](../../../../harness/conventions.md)
§3). Detail-Schnitt und
[MR-006](../../../../harness/conventions.md#mr-006--unabhängiges-plan-review-vor-implementierungs-start)
folgen **beim Start**.

**Welle:** welle-6-interaktiv-planen — **zweite Hälfte des Abschluss-Triggers**. **Mit diesem Slice
ist der Trigger erfüllt: „eine Wand ist im 2D-Canvas zeichenbar UND parametrisch änderbar, ohne
Kommandozeile."** Danach ist die Welle zu **schließen**, nicht weiterzufüllen.

**Setzt [`slice-057`](../in-progress/slice-057-lese-naht-bauteil-identitaet.md) voraus** (ohne Bauteil-Identität
gibt es nichts zu treffen und nichts zu adressieren).

## Auslöser

[ADR-0021](../../adr/0021-wand-im-2d-canvas.md), **Entscheidungen 3, 4, 5, 13, 14, 15, 16, 17**.

## Umfang

| Entscheidung | Was daraus wird |
|---|---|
| **3**/**16** | Treffer-Prüfung im Bildschirmraum, **nur im dargestellten Geschoss**, höchstens **eine** Wand; 2D und 3D bleiben getrennt |
| **17** | Die Auswahl **fällt** bei Modell-Ersetzung, Geschoss-Wechsel und Verschwinden |
| **15** | Die angezeigten Parameter kommen über die schmale Abfrage aus [`slice-057`](../in-progress/slice-057-lese-naht-bauteil-identitaet.md) |
| **4** | Nicht-modaler Eigenschaften-Bereich im Fenster |
| **13** | Änderbar: **Stärke** und **Höhe**; Wandtyp und Material bleiben draußen |
| **5** | Klemmung **mit genanntem übernommenem Wert**, Ablehnung sichtbar |
| **14** | Nachweis auf **zwei** Ebenen (Surrogat **und** Farbmengen-Sonde) |

## Die zwei Stellen, an denen dieser Slice still falsch werden kann

1. **Der Geschoss-Skopus.** Ohne Beschränkung träfe ein Klick bei deckungsgleichen Achsen
   **deterministisch** die nicht dargestellte Wand — im mitgelieferten Demo-Modell sind vier
   Außenwände in zwei Geschossen deckungsgleich. Der Fehler wäre **reproduzierbar** und trotzdem
   unsichtbar: die falsche Wand würde angezeigt **und** geändert.
2. **Die Lebensdauer der Auswahl.** Eine Identität, die eine Modell-Ersetzung überlebt, bezeichnet im
   neuen Stand mit hoher Wahrscheinlichkeit eine **andere, existierende** Wand. Die Fehler-Barriere
   schwiege dabei — eine gültige fremde Identität löst **keine** Ablehnung aus. Die Änderung träfe
   still das Falsche.

**Beide sind Fehler ohne Symptom.** Ihre Orakel müssen den Zustand **vor** und **nach** dem Wechsel
prüfen, nicht nur den Erfolgsfall.

## Der Wellen-Abschluss hängt hier

Die Closure dieses Slice zieht die **Welle-Closure** nach sich: der beobachtbare Trigger ist erfüllt,
und die Lehre der vorigen Welle steht im Wellen-Block — **schließen, sobald der Trigger erfüllt ist,
nicht, wenn die Arbeit ausgeht.** Vor der Welle-Closure steht das
[MR-009](../../../../harness/conventions.md#mr-009--geometrielastiges-code-review-vor-welle-closure)-Code-Review
des ganzen Bauteil-Strangs (Eckenschluss, Nachbar-Rebuild, Raum-Neuerkennung im interaktiven Pfad).

## Sub-Area-Modus-Begründung

### Sub-Area: GUI-Adapter (2D-Canvas)

- **Modus:** GF; **Dichte:** mittel — Treffer-Prüfung, Auswahl-Lebensdauer, Bedienfläche, vier
  Rückmelde-Ausgänge.
- **Risiko:** **hoch für den Strang** — Auswählen ist das Interaktions-Muster, das das Produkt
  **nirgends** hat; und die zwei benannten Fehler sind symptomlos.
