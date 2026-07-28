---
id: slice-056
titel: Wand im 2D-Canvas — ADR + AK-Schärfung (Wellen-Kern von welle-6, [LH-FA-WAL-001](../../../../spec/lastenheft.md#lh-fa-wal-001--wand-zeichnen))
status: open
welle: welle-6-interaktiv-planen
lastenheft_refs: [[LH-FA-WAL-001](../../../../spec/lastenheft.md#lh-fa-wal-001--wand-zeichnen), [LH-FA-WAL-002](../../../../spec/lastenheft.md#lh-fa-wal-002--wandstärke-definieren), [LH-FA-WAL-003](../../../../spec/lastenheft.md#lh-fa-wal-003--wandhöhe-definieren), [LH-FA-WAL-006](../../../../spec/lastenheft.md#lh-fa-wal-006--wand-verbinden), [LH-FA-DRW-001](../../../../spec/lastenheft.md#lh-fa-drw-001), [LH-FA-DRW-005](../../../../spec/lastenheft.md#lh-fa-drw-005), [LH-FA-D3-002](../../../../spec/lastenheft.md#lh-fa-d3-002--echtzeitaktualisierung)]
adr_refs: [[ADR-0001](../../adr/0001-hexagonale-architektur.md), [ADR-0008](../../adr/0008-aenderungs-benachrichtigung.md), [ADR-0009](../../adr/0009-gui-framework-qt6.md), [ADR-0010](../../adr/0010-headless-gl-xvfb.md), [ADR-0018](../../adr/0018-drw-2d-zeichen-daten.md), [ADR-0019](../../adr/0019-drw-2d-canvas.md)]
---

# Slice 056: Wand im 2D-Canvas — ADR + AK-Schärfung

**Status:** open — **erster
[MR-006](../../../../harness/conventions.md#mr-006--unabhängiges-plan-review-vor-implementierungs-start)-Lauf
durch** (2 HIGH / 6 MEDIUM / 5 LOW / 3 INFO, Verdikt „nicht startbar"); Einarbeitung in §11. Dieser
Slice schreibt **Doku, keinen Produktions-Code** (Muster
[`slice-041a`](../done/slice-041a-drw-canvas-adr-ak.md): ADR + AK-Schärfung vor dem Impl-Strang).

**Welle:** welle-6-interaktiv-planen — **dieser Strang ist der Wellen-Kern.** Der
[Abschluss-Trigger](../in-progress/roadmap.md) lautet: *eine **Wand** ist im 2D-Canvas **zeichenbar
und parametrisch änderbar**, ohne Kommandozeile.* Bis heute hat er **keinen** Plan; dieser Slice legt
die Entscheidungsgrundlage, die Impl-Slices (057/058, §8) lösen ihn ein.

**Autor:** Dietmar Burkard (AI-Harness-Lauf). **Datum:** 2026-07-28.

## Auslöser

Der Trigger von welle-6 ist seit dem 2026-07-27 formuliert und **unbearbeitet**. Die bisherigen
welle-6-Slices (048b Fangen, 055 Fang-Anzeige) machen das Zeichnen **präziser** und **erklärbarer** —
sie machen es nicht **zum Bauteil-Zeichnen**. Solange das so bleibt, sammelt die Welle Arbeit um ihren
Kern herum an: **genau das Muster, das die welle-5-Closure §5-1 als Sammelbecken benannt hat.**

**Und es fehlt die Entscheidungsgrundlage, nicht nur die Zeit.**
[ADR-0019](../../adr/0019-drw-2d-canvas.md) nimmt „**Bauteile interaktiv zeichnen** (Wände/Räume auf
dem Canvas), **Layer-Bedien-Panel**, **Selektion/Picking**, **Bemaßung**" ausdrücklich aus ihrem
Schnitt heraus („benannte Re-Eval-Trigger, nicht dieser Schnitt"). ADRs sind nach `Accepted`
immutabel ([AGENTS §2.5](../../../../AGENTS.md)) — die Auflösung ist eine **neue ADR**, kein Nachtrag.
*(Das Zitat ist im ersten Review am Artefakt verifiziert worden — Abgrenzungs-Block und Re-Eval-Block
führen es wörtlich.)*

## 1. Ziel

**Zwei Artefakte, beide lösungs- bzw. formfrei an der jeweils richtigen Stelle:**

1. **Eine neue ADR** (nächste freie Nummer: 0021) — die **zehn** Fragen (§2), die der Spec-Text nicht
   entscheidet, beantwortet und begründet; `Proposed` → unabhängiges Text-Review → `Accepted`;
   ADR-Index + Folgepflicht-Block nachgezogen.
2. **AK-Schärfung** — [`LH-FA-WAL-001`](../../../../spec/lastenheft.md#lh-fa-wal-001--wand-zeichnen)
   bekommt einen Block **„Interaktive Erzeugung (2D-Zeichenfläche)"**,
   [`LH-FA-WAL-002`](../../../../spec/lastenheft.md#lh-fa-wal-002--wandstärke-definieren)/003 je
   einen Konjunkt für die **interaktive** Änderung. **Präzedenz:**
   [`slice-041a`](../done/slice-041a-drw-canvas-adr-ak.md) hat für
   [`LH-FA-DRW-005`](../../../../spec/lastenheft.md#lh-fa-drw-005) genau diesen Block ergänzt — dieselbe
   Bauart, dieselbe
   [MR-008](../../../../harness/conventions.md#mr-008--lastenheft-schärfung-bleibt-lösungsfrei)-Grenze.

**Kein Produktions-Code.** Der Slice liefert die Grundlage; 057/058 liefern die Funktion.

## 2. Die zehn Fragen, die der Spec-Text nicht entscheidet

*(Sie sind der Grund, warum hier eine ADR steht und nicht direkt ein Impl-Slice. Jede ist am Artefakt
belegt, nicht vermutet. **F9 und F10 hat der erste Review gefunden** — die Vorfassung führte acht.)*

| # | Frage | Warum offen — am Artefakt |
|---|---|---|
| **F1** | **Werkzeug-Wahl.** Woher weiß der Canvas, ob ein Zug eine **Hilfslinie** oder eine **Wand** erzeugt? | Der Canvas hat heute **eine** Geste: Links-Zug ⇒ das injizierte Schreib-Callable, das über `ui/command/edit_drawing_guide_line_sink.h` auf `addGuideLine` führt. Eine zweite Bauform braucht einen Modus — Werkzeugleiste, Tastatur-Modifikator oder getrennte Flächen. **Das Lastenheft kennt den Begriff „Werkzeug", entscheidet die Frage aber nicht:** die DRW-005-Negative sagt „dann **erzeugt das Werkzeug nichts**" (Prosa in einer fremden AK), und [`LH-FA-UI-005`](../../../../spec/lastenheft.md#modul-benutzeroberfläche-ui) („Anpassbare Werkzeugleisten") ist reine Outline ohne AK. |
| **F2** | **Wie viele Segmente je Zeichen-Geste?** | **Keine Modell-Frage, sondern eine Interaktions-Frage** (Lauf-1-MEDIUM-1). [`LH-FA-WAL-001`](../../../../spec/lastenheft.md#lh-fa-wal-001--wand-zeichnen) fordert „Linienzug mit ≥ 2 Punkten ⇒ **je Segment eine Wand**" — das ist **exakt** der bestehende Einzelsegment-Typ; ein Zug mit *n* Punkten bildet sich auf *n−1* `addWall`-Aufrufe ab, ohne dass das Domänen-Modell wächst, und der Eckenschluss dieser Segmente liegt seit slice-012 vor ([`LH-FA-WAL-006`](../../../../spec/lastenheft.md#lh-fa-wal-006--wand-verbinden)). Die `spezifikation.md`-§2.1-Klausel „Wandzüge/Polylines folgen als Erweiterung" betrifft die **Wand-Entität** mit mehreren Segmenten, **nicht** die Geste. Offen ist damit die Bedien-Frage: mehrpunktiger Zug in einem Rutsch oder ein Segment je Zug — und **wenn Letzteres, unterschreitet v1 den geltenden Happy Path** und braucht eine benannte Teilumfang-Klausel im Lastenheft. |
| **F3** | **Selektion/Picking.** „Parametrisch ändern" setzt voraus, dass der Benutzer **eine bestimmte Wand** benennt. | Es gibt heute **keine** Selektion — weder im Canvas noch im 3D-Viewer ([ADR-0019](../../adr/0019-drw-2d-canvas.md) nimmt sie aus, [ADR-0009](../../adr/0009-gui-framework-qt6.md) verweist die 3D-Selektion auf einen eigenen Re-Eval). Offen: Treffer-Prüfung im Bildschirm- oder Modellraum, Toleranz, Mehrfachtreffer, **und** ob die Auswahl **UI-Zustand** oder **Modell-Zustand** ist (bei Modell-Zustand wären Schema und Persistenz betroffen — dann wäre der Slice ein ganz anderer). |
| **F4** | **Wo ändert man den Parameter?** | `EditStructurePort` bietet `setWallThickness`/`setWallHeight` mit `ParamResult{applied_mm, status}`. Eine **Bedienfläche** dafür existiert nicht (das Fenster trägt seit slice-053 nur ein Datei-Menü). Panel, Dialog oder Inline-Eingabe — und wie die Rückmeldung aussieht, ist Frage F5. |
| **F5** | **Rückmeldung an den Benutzer — alle drei Fälle, nicht nur die Klemmung.** | Drei AK verlangen eine sichtbare Reaktion, und **keine** hat heute eine: (a) [`LH-FA-WAL-002`](../../../../spec/lastenheft.md#lh-fa-wal-002--wandstärke-definieren)/003 „auf Grenzwert **geklemmt + Hinweis**" (`ParamStatus::Clamped`); (b) `ParamStatus::Rejected` (Modell unverändert, [`E-VAL-001`](../../../../spec/spezifikation.md#4-fehler-codes-und-logging-felder)); (c) **die WAL-001-Boundary „Null-Längen-Wand ⇒ verworfen, *Hinweis*"** — `addWall` verwirft heute **still** (`return std::nullopt`), und der Canvas wertet den Rückgabewert seines Schreib-Callables nicht aus (Lauf-1-MEDIUM-6). Ohne Entscheidung bleiben alle drei still, und die AK sind verletzt. |
| **F6** | **Refresh nach der Mutation.** | Anders als Hilfslinien melden Wand-Mutationen einen `op`: `addWall` ruft `notifyListeners({WallAdded, …})` **nach** Nachbar-Rebuild und Raum-Neuerkennung (`structure_edit_service.cpp`). Der Canvas ist bereits `ModelChangedPort`-Beobachter — der Refresh-Pfad **existiert** und ist der 3D-Viewer-Pfad. Zu entscheiden ist, ob der Canvas nach dem **eigenen** Kommando zusätzlich selbst repaintet (wie bei Hilfslinien) oder ausschließlich über die Meldung — **doppeltes Neu-Einrahmen** wäre sonst ein sichtbarer Sprung. **[ADR-0018](../../adr/0018-drw-2d-zeichen-daten.md)s „kein op" ist hier nicht berührt** (das gilt für Zeichen-Daten, nicht für Bauteile). |
| **F7** | **Der zweite UI-Mutator.** | Die slice-043-Option A verdrahtet den Schreibpfad als `std::function` aus `ui/command/`, damit `view/` **keinen** Driving-Port include. Für Wände entsteht die analoge Senke am `EditStructurePort`. **Schwache Frage — ehrlich als solche geführt** (Lauf-1-INFO-2): [ADR-0019](../../adr/0019-drw-2d-canvas.md) Entscheidung 5 und die `.a-check.yml`-Kanten (`ui_command → ports_driving` erlaubt, `ui_view → ports_driving` nicht) entscheiden sie bereits **und setzen sie maschinell durch**. Die ADR **stellt fest**, sie entscheidet hier nichts Neues. |
| **F8** | **[`E-GEO-001`](../../../../spec/spezifikation.md#4-fehler-codes-und-logging-felder) „Eingabe außerhalb des Zeichenbereichs".** | Die Negative-AK von [`LH-FA-WAL-001`](../../../../spec/lastenheft.md#lh-fa-wal-001--wand-zeichnen) nennt diesen Fall. Ein **begrenzter Zeichenbereich in Modell-mm** existiert nicht; der Zoom ist geklemmt (`[1e-4, 100]` px/mm seit dem slice-043-Code-Review) und ein interaktives Pan gibt es nicht — die Begrenzung ist also eine Sicht-, keine Bereichs-Grenze (Lauf-1-LOW-1). Entweder die ADR definiert einen Bereich, oder die AK bekommt die ehrliche Feststellung, dass der Fall in dieser Ausbaustufe **nicht erreichbar** ist. **Was nicht geht: die AK-Zeile stehen lassen und nichts dazu bauen.** |
| **F9** | **Fangen beim Bauteil-Zeichnen** — gilt der 048b-Fang auch für den Wand-Zug? | **Vom ersten Review gefunden (HIGH-2); die Vorfassung hatte die Frage nicht.** [`LH-FA-DRW-001`](../../../../spec/lastenheft.md#lh-fa-drw-001) begrenzt den Fang wörtlich auf „das interaktive Zeichnen von **Hilfslinien**" und führt „**Fangen beim Bauteil-Zeichnen**" im Teilumfang-Block ausdrücklich als **offen**. Die Folge ist nicht kosmetisch: der WAL-001-Happy fordert „verbundene Endpunkte werden **geometrisch verbunden**", [`LH-FA-WAL-006`](../../../../spec/lastenheft.md#lh-fa-wal-006--wand-verbinden) setzt einen **gemeinsamen** Endpunkt voraus, und `spezifikation.md` §3 legt die Toleranz auf **0,1 mm** fest — während ein Bildschirm-Pixel bei Default-Zoom rund **20 mm** entspricht. **Ohne Fang ist der Eckenschluss auf dem interaktiven Weg praktisch unerreichbar.** |
| **F10** | **Fehler-Barriere des zweiten Mutators.** | **Vom ersten Review gefunden (MEDIUM-2).** Der erste UI-Mutator lehnt **wertbasiert** ab (`addGuideLine → std::optional`, Modell unverändert); der zweite **wirft**: `addWall` wirft `std::out_of_range` bei unbekannter Geschoss-Id, `setWallThickness`/`setWallHeight` werfen über `mutableWall` bei unbekannter Wand-Id. Das ist kein Randfall — die Zeichen-Ziele werden als eingefrorene Ids injiziert und nach einem Projekt-Laden nachgezogen (`setTarget`/`setActiveStorey` tragen genau diesen Kommentar). Dieselbe Konstellation führt beim Hilfslinien-Weg zu stiller Ablehnung, beim Wand-Weg zu einem **Wurf aus einem Qt-Event-Handler**. Wo diese Barriere liegt, ist unentschieden. |

## 3. Bewusst NICHT Teil

- **Jeder Produktions-Code.** Der Slice liefert ADR + AK; 057/058 (§8) implementieren.
- **Räume, Türen, Fenster, Treppen, Dächer interaktiv.** Der Wellen-Trigger nennt die **Wand**. Alles
  andere ist eigener Schnitt — sonst entsteht das Sammelbecken erneut.
- **Wand verschieben/teilen** ([`LH-FA-WAL-004`](../../../../spec/lastenheft.md#lh-fa-wal-004)/005) —
  reine Outline-Anforderungen, nicht Trigger-Teil.
- **3D-Selektion** ([ADR-0009](../../adr/0009-gui-framework-qt6.md)-Re-Eval) — unberührt.
- **Layer-Bedien-Panel, Bemaßung** — weitere Re-Eval-Träger aus
  [ADR-0019](../../adr/0019-drw-2d-canvas.md), hier nicht mit-entschieden.
- **Raster/Winkel** ([`LH-FA-DRW-002`](../../../../spec/lastenheft.md#modul-zeichnungsfunktionen-drw)/003) —
  bleiben zurückgestellt (048b-Closure-Deferral). **F9 betrifft nur den Endpunkt-Fang aus 048b**, nicht
  die noch offenen Aid-Arten.
- **Anpassbare Werkzeugleisten** ([`LH-FA-UI-005`](../../../../spec/lastenheft.md#modul-benutzeroberfläche-ui),
  Outline). Eine Werkzeugleisten-Antwort auf F1 **berührt** diese Anforderung; sie **erfüllt** sie
  nicht und schärft sie nicht (Lauf-1-MEDIUM-5).
- **Undo/Redo** ([`LH-QA-003`](../../../../spec/lastenheft.md#lh-qa-003--undoredo)).
  Die Anforderung fordert ≥ 1000 rücknehmbare Schritte, `undo_commands` existiert im Schema, **kein**
  Mutations-Pfad bedient es. Der interaktive Wand-Weg erzeugt Modell-Mutationen ohne Undo-Anbindung —
  eine **Bestandslücke**, die dieser Slice weder einführt noch schließen muss. Sie gehört als eigene
  Entscheidung in Roadmap/Validator-Rolle (Lauf-1-INFO-3). **Hier benannt, damit sie nicht als
  übersehen gilt.**

## 4. Orakel-Schnitt — ein Doku-Slice hat Doku-Sensoren

*(Ehrlich benannt: dieser Slice hat **kein** Verhaltens-Orakel, weil er kein Verhalten liefert. Umso
genauer muss die Spalte „Wo geprüft" stimmen — der erste Lauf hat dort einen HIGH gefunden.)*

| # | Zusicherung | Wo geprüft | Diskriminierende Gegenprobe |
|---|---|---|---|
| 1a | Der Index-Eintrag der ADR ist **auflösbar** und die Kennung überall verlinkt | `make docs-check` (`links`/`anchors`/`ids`) | Link durch nackte Kennung ersetzen ⇒ `id-unlinked`; Ziel verbiegen ⇒ `target-missing` |
| 1b | Die ADR ist **überhaupt im Index geführt** | **kein Sensor** — [AGENTS §4](../../../../AGENTS.md)-Regel ohne Gate | fehlt die Zeile, gibt es **kein Vorkommen**, das ein Modul melden könnte ⇒ die Gegenprobe bliebe grün (Lauf-1-MEDIUM-3) |
| 2 | Die ADR nennt **keine** Slice-Kennung im Körper (no-downward) | `make docs-check` (`matrix`, [MR-014](../../../../harness/conventions.md)) | eine `slice-*`-Kennung in den ADR-Körper schreiben ⇒ rot |
| 3 | Die drei Spec-Straten bleiben **prozess-/zeit-rein** (kein `welle-N`) | `make docs-check` (`matrix`-Klasse `temporal`, [MR-023](../../../../harness/conventions.md#mr-023--spec-straten-sind-prozess-zeit-rein-d-check-matrix-klasse-temporal)) | ein Wellen-Token in die AK schreiben ⇒ rot |
| 4 | **Lastenheft-Header == Version der neu ergänzten Historie-Zeile** | **kein Gate** — der Sensor ist die [MR-006](../../../../harness/conventions.md#mr-006--unabhängiges-plan-review-vor-implementierungs-start)-Linse | — d-check prüft **keine Feld-Gleichheit**; [MR-010](../../../../harness/conventions.md) führt eine computational Prüfung ausdrücklich nur als **Promotion-Ziel** (Lauf-1-HIGH-1) |
| 5 | **Die AK bleiben lösungsfrei** ([MR-008](../../../../harness/conventions.md#mr-008--lastenheft-schärfung-bleibt-lösungsfrei)) — kein Port-, Algorithmus-, Widget- oder Fehler-Code-Vokabular im Lastenheft-Körper | **kein Gate** — [MR-006](../../../../harness/conventions.md#mr-006--unabhängiges-plan-review-vor-implementierungs-start)-Linse | — Urteilsfrage |
| 6 | **Kein Code-Diff** — `src/**`, `tests/**`, `data-model.yaml`, `schema.sql` unberührt | `git diff --stat` in der Closure | eine geänderte Datei erschiene dort. **Nicht** `make schema-check`: das prüft die **Drift zwischen** Modell und DDL, nicht Unberührtheit, und ist kein `gates`-Member (Lauf-1-LOW-4) |

**Drei von sechs Zeilen tragen kein Gate — und das steht jetzt in der Tabelle, nicht darunter.** Die
Vorfassung führte Zeile 4 als `make docs-check`-gedeckt und bündelte in Zeile 1 zwei Zusicherungen,
von denen nur eine einen Sensor hat; beides war falsch. Ein Plan, der einen Sensor behauptet, den es
nicht gibt, ist gefährlicher als einer, der die Lücke benennt: die Lücke sucht jemand, die Behauptung
niemand.

## 5. Definition of Done

- [ ] **`docs/plan/adr/0021-*.{md}`** (neu): Kontext · die **zehn** Fragen aus §2 **je entschieden mit
      verglichenen Alternativen** · Konsequenzen · Folgepflichten · Re-Eval-Trigger. Status
      `Proposed` → **unabhängiges Text-Review** → `Accepted` (Muster
      [ADR-0019](../../adr/0019-drw-2d-canvas.md)).
- [ ] **[`docs/plan/adr/README.md`](../../adr/README.md)**: Index-Zeile + **Folgepflicht-Zeilen** für
      057/058 ([AGENTS §4](../../../../AGENTS.md)). **Ohne Gate** — s. §4-1b.
- [ ] **Lastenheft**: [`LH-FA-WAL-001`](../../../../spec/lastenheft.md#lh-fa-wal-001--wand-zeichnen)-Block
      „Interaktive Erzeugung (2D-Zeichenfläche)" (Happy/Boundary/Negative) +
      [`LH-FA-WAL-002`](../../../../spec/lastenheft.md#lh-fa-wal-002--wandstärke-definieren)/003
      je ein interaktiver Konjunkt — **lösungsfrei**.
- [ ] **Falls F2 zugunsten „ein Segment je Zug" entschieden wird: eine benannte Teilumfang-Klausel an
      [`LH-FA-WAL-001`](../../../../spec/lastenheft.md#lh-fa-wal-001--wand-zeichnen)** (Muster der
      DRW-/ROF-/STR-Teilumfänge). **Ohne sie unterschreitet die Lieferung den geltenden Happy Path**
      („Linienzug mit ≥ 2 Punkten") — die Vorfassung hatte die Klausel nur als Empfehlung im
      Risiko-Abschnitt (Lauf-1-MEDIUM-1).
- [ ] **[`LH-FA-DRW-005`](../../../../spec/lastenheft.md#lh-fa-drw-005)-Teilumfang-Klausel nachziehen
      oder bewusst beibehalten**: sie führt „das **interaktive Zeichnen von Bauteilen** … bleibt
      ausdrücklich offen". Der neue WAL-001-Block macht sie im selben Dokument gegenläufig; **kein Gate
      fängt das** (Lauf-1-MEDIUM-4).
- [ ] **Falls F9 zugunsten „Fangen gilt auch für Wände" entschieden wird: die
      [`LH-FA-DRW-001`](../../../../spec/lastenheft.md#lh-fa-drw-001)-Teilumfang-Zeile nachziehen** —
      sie führt „Fangen beim Bauteil-Zeichnen" heute als **offen**.
- [ ] **Lastenheft-Version + Historie** ([MR-010](../../../../harness/conventions.md)/[MR-012](../../../../harness/conventions.md)):
      Header == Version der neu ergänzten Zeile in
      [`lastenheft-historie.md`](../../../../spec/lastenheft-historie.md); **Platzierung unmittelbar
      nach der `0.1.16`-Zeile** (die Tabelle ist nicht monoton sortiert). Zielnummer beim Vollzug
      feststellen — [`slice-055`](slice-055-fang-anzeige.md) schärft ebenfalls, die Reihenfolge ist
      offen. **Sensor: die [MR-006](../../../../harness/conventions.md#mr-006--unabhängiges-plan-review-vor-implementierungs-start)-Linse, kein Gate** (§4-4).
- [ ] **`spec/spezifikation.md` §1**: Mapping-Block für den interaktiven Wand-Weg (Werkzeug-Modus,
      Selektion als UI-Zustand, Parameter-Rückmeldung, Refresh-Pfad, Fang-Geltung, Fehler-Barriere) —
      die **Mechanik**, die aus dem Lastenheft herausgehalten wird. **§2.1-Klausel „Wandzüge folgen als
      Erweiterung"** je nach F2-Entscheidung **nachziehen oder ausdrücklich stehen lassen**.
- [ ] **`spec/architecture.md` §1.1**: Driving-Ports-Tabelle um die Canvas-Klausel am
      `EditStructurePort` — **meilenstein- und slice-frei** ([AGENTS §2.7](../../../../AGENTS.md)).
- [ ] **F8 ist beantwortet** — entweder Zeichenbereich definiert **oder** die
      [`E-GEO-001`](../../../../spec/spezifikation.md#4-fehler-codes-und-logging-felder)-Negative als
      in dieser Ausbaustufe **nicht erreichbar** benannt. **Kein drittes Ergebnis** („später").
- [ ] **Die Folge-Slices existieren als Plan-Datei** (057/058) in `open/`, `next/` **oder**
      `in-progress/` — **oder** eine **explizite Deferral-Entscheidung** steht in Roadmap/ADR-Index.
      [MR-020](../../../../harness/conventions.md) regelt die Closure einer **Slice** bzw. einer
      **Welle** (eine ADR hat keine Closure) und lässt beide Wege zu (Lauf-1-LOW-2).
- [ ] **CHANGELOG** [Unreleased]-Eintrag.
- [ ] **[MR-006](../../../../harness/conventions.md#mr-006--unabhängiges-plan-review-vor-implementierungs-start)-Report
      unter `docs/reviews/`** je Lauf + **ADR-Text-Review** — als DoD-Zeile, nicht nur in der
      Datei-Tabelle.
- [ ] **`make gates` grün**; **kein** Code-Diff, belegt am `git diff --stat` (§4-6).

## 6. Plan (vor Code)

| Datei / Komponente | Art | Begründung |
|---|---|---|
| `docs/plan/adr/0021-*.{md}` | neu | die zehn Entscheidungen (§2) |
| `docs/plan/adr/README.md` | ändern | Index + Folgepflicht-Block |
| `spec/lastenheft.md`, `spec/lastenheft-historie.md` | ändern | AK-Block + Teilumfang-Klauseln + Version |
| `spec/spezifikation.md` | ändern | §1-Mapping, §2.1-Klausel je nach F2 |
| `spec/architecture.md` | ändern | §1.1 Driving-Ports (slice-/meilensteinfrei) |
| `docs/plan/planning/open/slice-057-*.{md}`, `slice-058-*.{md}` | neu | Folge-Slices als Plan ([MR-020](../../../../harness/conventions.md)) |
| `CHANGELOG.md` | ändern | [Unreleased] |
| `docs/reviews/`-Reports | neu | [MR-006](../../../../harness/conventions.md#mr-006--unabhängiges-plan-review-vor-implementierungs-start) je Lauf + ADR-Text-Review |

**Nicht berührt:** `src/**`, `tests/**`, `data-model.yaml`/`schema.sql`, `.a-check.yml`/`.d-check.yml`,
`Makefile`.

## 7. Risiken

- **R1 — F2 ist eine Umfangs-Wahl, keine Modell-Notwendigkeit** (korrigiert nach Lauf-1-MEDIUM-1).
  Die Vorfassung las die `spezifikation.md`-§2.1-Klausel als Beleg dafür, dass ein mehrpunktiger Zug
  „nicht durch das Modell gedeckt" sei — **das trägt nicht**: die Klausel betrifft die Wand-*Entität*,
  und *n−1* `addWall`-Aufrufe erfüllen den geltenden Happy Path exakt. Die Empfehlung „v1 = ein Segment
  je Zug" bleibt vertretbar (kleinerer Schnitt; der Wellen-Trigger sagt „**eine** Wand"), aber sie
  **unterschreitet eine geltende AK** und braucht deshalb die Teilumfang-Klausel aus der DoD — nicht
  bloß eine Zeile im Risiko-Abschnitt.
- **R2 — F3 (Selektion) ist die eigentliche Neuheit.** Zeichnen ist eine Variante des bekannten Zugs;
  **Auswählen** ist ein Interaktions-Muster, das das Produkt noch **nirgends** hat. Wenn ein Teil
  dieses Strangs unterschätzt wird, ist es dieser. Er trägt auch die Frage, ob die Auswahl im 2D-
  und im 3D-Bild **dieselbe** ist (v1-Antwort vermutlich: getrennt, benannt).
- **R3 — die AK könnte Lösung enthalten.** „Werkzeug", „Panel", „Marker", „Port" haben im Lastenheft
  nichts zu suchen ([MR-008](../../../../harness/conventions.md#mr-008--lastenheft-schärfung-bleibt-lösungsfrei)).
  Die AK sagt, **was der Benutzer beobachtet**; die ADR und `spezifikation.md` §1 sagen, **wie**.
  Kein Gate fängt das — nur das Review (§4-5).
- **R4 — ADR-Immutabilität.** Die neue ADR darf [ADR-0019](../../adr/0019-drw-2d-canvas.md) **nicht**
  ändern, nur **erweitern**; ein echter Widerspruch verlangt `Supersedes`
  ([AGENTS §2.5](../../../../AGENTS.md)). Kandidat für einen unbeabsichtigten Widerspruch: Entscheidung 5
  (Kommando-Naht) — sie soll **fortgeschrieben**, nicht ersetzt werden (s. F7).
- **R5 — der Slice könnte zum Papier werden.** Ein ADR-Slice ohne Impl-Nachfolge ist genau die
  Buchführungs-Fiktion, die welle-5 gekostet hat. **Gegenmittel in der DoD:** 057/058 existieren als
  Plan-Datei, bevor 056 schließt.
- **R6 — die Zählung „zehn Fragen" ist selbst eine Behauptung.** Der erste Review hat **zwei** Lücken
  gefunden (F9, F10); dass jetzt keine elfte fehlt, ist **nicht bewiesen**. Weil die ADR nach
  `Accepted` immutabel ist, trägt der zweite Review-Lauf dieselbe Hauptlast wie der erste: **Suche nach
  der nächsten fehlenden Frage**, nicht Prüfung der vorhandenen.

## 8. Der Strang danach (Sequenz, nicht Umfang dieses Slice)

| Slice | Was | Trigger-Beitrag |
|---|---|---|
| **057** | **Wand zeichnen** im Canvas: Werkzeug-Modus (F1) + Segment-Zahl je Geste (F2) + Zug ⇒ `addWall` über die neue `ui/command/`-Senke (F7) + Refresh (F6) + Fang-Geltung (F9) + Fehler-Barriere (F10) + Headless-AK | „**zeichenbar**" |
| **058** | **Wand auswählen und ändern**: Picking (F3) + Parameter-Bedienung (F4) + Rückmeldung in allen drei Fällen (F5) + Headless-AK | „**parametrisch änderbar**" |

**Mit 058 ist der welle-6-Abschluss-Trigger erfüllt — und die Welle ist dann zu schließen, nicht
weiterzufüllen** (welle-5-Closure §5-1; der Wellen-Block der Roadmap trägt die Regel).

## 9. Closure-Trigger

- Die neue ADR ist `Accepted` (Text-Review durch, 0 HIGH) + Index/Folgepflicht nachgezogen; AK-Block,
  Teilumfang-Klauseln und Spec-§1-Mapping geschrieben; Lastenheft-Header == neue Historie-Zeile;
  057/058 liegen als Plan vor (oder eine Deferral ist notiert); `make gates` grün; **kein** Code-Diff
  am `git diff --stat` belegt; Closure-Notiz.

## 10. Sub-Area-Modus-Begründung

### Sub-Area: Spec-Schreibung + Planning-Lifecycle

- **Modus:** GF; **Dichte:** mittel — eine ADR mit zehn Entscheidungen ist der Aufwand, nicht die
  Zeilenzahl der AK.
- **Phase-Reife:** der Canvas trägt seit 043 einen geprüften Interaktions-Pfad, seit 048b eine
  Eingabe-Quantisierung; der `EditStructurePort` liegt seit slice-003a/013b vollständig vor
  (`addWall`, `setWallThickness`, `setWallHeight` mit `ParamResult`). **Es fehlt keine Kern-Funktion —
  es fehlt der Weg dorthin.**
- **Risiko:** mittel — nicht im Schreiben, sondern in **F2**, **F3** und **F9** (R1/R2 und die
  Eckenschluss-Folge). Alle drei sind Entscheidungen mit Folgekosten, und alle drei gehören genau
  deshalb **vor** den Code.

## 11. [MR-006](../../../../harness/conventions.md#mr-006--unabhängiges-plan-review-vor-implementierungs-start)-Einarbeitung (erster Lauf, 2026-07-28)

Report: [`2026-07-28-slice-056-plan.md`](../../../reviews/2026-07-28-slice-056-plan.md) —
**2 HIGH / 6 MEDIUM / 5 LOW / 3 INFO + 17 Negativbefund-Zeilen, „nicht startbar"**. Unabhängiger
Reviewer ≠ Plan-Autor.

| # | Behandlung |
|---|---|
| **HIGH-1** (§4-Zeile 4 nannte `make docs-check` als Sensor für „Header == oberste Historie-Zeile"; [MR-010](../../../../harness/conventions.md) sagt wörtlich das Gegenteil — „d-check prüft keine Feld-Gleichheit", Sensor ist die [MR-006](../../../../harness/conventions.md#mr-006--unabhängiges-plan-review-vor-implementierungs-start)-Linse, computational nur **Promotion-Ziel**) | **Nachgelesen und bestätigt.** Zeile 4 führt jetzt **kein Gate**, sondern die Review-Linse — wie Zeile 5. Zusätzlich trägt die Tabelle den Satz, dass **drei von sechs** Zeilen kein Gate haben. Ein behaupteter Sensor ist gefährlicher als eine benannte Lücke: die Lücke sucht jemand. |
| **HIGH-2** (**die neunte Frage fehlt:** [`LH-FA-DRW-001`](../../../../spec/lastenheft.md#lh-fa-drw-001) führt „Fangen beim Bauteil-Zeichnen" wörtlich als offen; ohne Entscheidung ist der Eckenschluss unerreichbar — Toleranz 0,1 mm gegen ~20 mm je Pixel bei Default-Zoom) | **Als F9 aufgenommen**, samt der Rechnung, die den Befund trägt. Dazu eine **DoD-Zeile**: fällt F9 zugunsten des Fangens, ist die DRW-001-Teilumfang-Zeile nachzuziehen. **Das ist der Fund, für den §11 der Vorfassung die Linse ausdrücklich beauftragt hatte** — er ist eingetreten. |
| **MEDIUM-1** (F2s Beleg trägt nicht: „je Segment eine Wand" ist der bestehende Typ, *n−1* `addWall`-Aufrufe erfüllen den Happy Path; die §2.1-Klausel betrifft die **Entität**, nicht die Geste — und die empfohlene Teilumfang-Antwort unterschreitet eine geltende AK ohne die dafür nötige Klausel) | **F2 ist neu formuliert: Interaktions-Frage, nicht Modell-Frage.** R1 sagt die Korrektur ausdrücklich („das trägt nicht"), und die Teilumfang-Klausel ist aus dem Risiko-Abschnitt in die **DoD** gewandert. |
| **MEDIUM-2** (die Fehler-Barriere des zweiten Mutators fehlte: `EditStructurePort` **wirft**, `EditDrawingPort` lehnt wertbasiert ab — Wurf aus einem Qt-Event-Handler bei veralteter Id) | **Als F10 aufgenommen**, mit dem realen Auslöser (eingefrorene Ids + Nachziehen nach Projekt-Laden). |
| **MEDIUM-3** (Orakel-Zeile 1 bündelte „im Index geführt" und „auflösbar"; nur die zweite Hälfte hat einen Sensor — fehlt die Zeile, gibt es kein Vorkommen zu melden) | **In 1a (gegatet) und 1b (kein Sensor) getrennt.** |
| **MEDIUM-4** (die DRW-005-Teilumfang-Klausel „interaktives Zeichnen von Bauteilen bleibt offen" wird durch den neuen WAL-001-Block gegenläufig; kein Nachzug vorgesehen) | **Eigene DoD-Zeile** — nachziehen **oder** bewusst beibehalten, aber nicht übersehen. |
| **MEDIUM-5** (F1s „der Spec-Text kennt keinen Werkzeug-Begriff" ist falsch — die DRW-005-Negative und [`LH-FA-UI-005`](../../../../spec/lastenheft.md#modul-benutzeroberfläche-ui) nutzen ihn) | **Korrigiert:** F1 sagt jetzt, das Lastenheft kenne den Begriff, entscheide die Frage aber nicht. **[LH-FA-UI-005](../../../../spec/lastenheft.md#modul-benutzeroberfläche-ui) ist in §3 abgegrenzt.** |
| **MEDIUM-6** (die WAL-001-**Boundary** „Null-Längen-Wand ⇒ verworfen, **Hinweis**" fiel zwischen F5 und F8; `addWall` verwirft still) | **F5 ist auf alle drei Rückmelde-Fälle erweitert** (Klemmung · Ablehnung · stille Verwerfung) — der von F8 formulierte Grundsatz trifft diese Zeile genauso. |
| **LOW-1** (F8: „Pan/Zoom sind frei" trifft den Ist-Stand nicht — kein interaktives Pan, Zoom geklemmt) | Begründungs-Klammer korrigiert; die **tragende** Aussage (kein Zeichenbereich in mm) bleibt. |
| **LOW-2** ([MR-020](../../../../harness/conventions.md) verkürzt: es regelt Slice-/Wellen-Closure, nicht ADR-Closure, und lässt `next/`/`in-progress/` sowie eine Deferral zu) | DoD-Zeile wörtlich nachgezogen. |
| **LOW-3** (F1 schrieb `addGuideLine` dem `mouseReleaseEvent` zu; der Handler ruft das injizierte Callable) | Korrigiert — F1 nennt jetzt dieselbe Naht wie F7. |
| **LOW-4** (`make schema-check` taugt nicht als Sensor für „unberührt" und ist kein `gates`-Member) | Aus Zeile 6 entfernt; `git diff --stat` bleibt der tragende Nachweis. |
| **LOW-5** (`lastenheft_refs` führt WAL-006 nicht, obwohl R1 den Eckenschluss als Folgekosten trägt) | Frontmatter ergänzt — WAL-006, DRW-001 und DRW-005 stehen jetzt drin (F9 und MEDIUM-4 machen auch die zwei DRW-Kennungen tragend). |
| **INFO-1** (das [ADR-0019](../../adr/0019-drw-2d-canvas.md)-Zitat ist wörtlich korrekt) | Im Auslöser-Abschnitt als verifiziert vermerkt — **bestätigt, nicht eingearbeitet**. |
| **INFO-2** (F7 stellt fest, statt zu entscheiden — [ADR-0019](../../adr/0019-drw-2d-canvas.md) E5 + `.a-check.yml` entscheiden sie bereits) | F7 ist jetzt **ausdrücklich als schwache Frage** geführt: die ADR stellt fest, sie entscheidet nichts Neues. Die Zählung ist damit ehrlich. |
| **INFO-3** ([`LH-QA-003`](../../../../spec/lastenheft.md#lh-qa-003--undoredo) Undo/Redo: `undo_commands` existiert im Schema, kein Mutations-Pfad bedient es) | **In §3 als Bestandslücke benannt** — von diesem Slice weder eingeführt noch zu schließen, aber nicht mehr unerwähnt. |

**Positiv bestätigt** (nicht neu prüfen): das [ADR-0019](../../adr/0019-drw-2d-canvas.md)-Zitat ist **wörtlich korrekt** · F3/F4/F5/F6/F7
sind am Artefakt belegt · **keine** der Fragen ist durch eine Accepted-ADR oder die Spec
vorentschieden (außer F7 in der starken Lesart) · die d-check-Module in §4-2/3 (`matrix` no-downward,
`temporal`) existieren genau so, und die neue ADR unterliegt der Regel (exempt nur 0001–0017) ·
`make docs-check` real gelaufen: **266 Dateien, 0 Befunde** — alle Verweise und Kennungen lösen auf.

**Startbar:** **nein** — dieser Plan trägt die Einarbeitung, aber zwei HIGH verlangen einen **zweiten
unabhängigen Lauf** gegen das Eingearbeitete. Sein Auftrag ist unverändert der aus R6: **nicht die
zehn Fragen prüfen, sondern die elfte suchen.** Der erste Lauf hat zwei gefunden — das ist der beste
verfügbare Beleg dafür, dass die Zählung allein nichts zusichert.

## 12. Closure-Notiz

_(bei Ausführung auszufüllen)_
