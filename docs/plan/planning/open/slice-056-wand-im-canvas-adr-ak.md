---
id: slice-056
titel: Wand im 2D-Canvas — ADR + AK-Schärfung (Wellen-Kern von welle-6, [LH-FA-WAL-001](../../../../spec/lastenheft.md#lh-fa-wal-001--wand-zeichnen))
status: open
welle: welle-6-interaktiv-planen
lastenheft_refs: [[LH-FA-WAL-001](../../../../spec/lastenheft.md#lh-fa-wal-001--wand-zeichnen), [LH-FA-WAL-002](../../../../spec/lastenheft.md#lh-fa-wal-002--wandstärke-definieren), [LH-FA-WAL-003](../../../../spec/lastenheft.md#lh-fa-wal-003--wandhöhe-definieren), [LH-FA-D3-002](../../../../spec/lastenheft.md#lh-fa-d3-002--echtzeitaktualisierung)]
adr_refs: [[ADR-0001](../../adr/0001-hexagonale-architektur.md), [ADR-0008](../../adr/0008-aenderungs-benachrichtigung.md), [ADR-0009](../../adr/0009-gui-framework-qt6.md), [ADR-0010](../../adr/0010-headless-gl-xvfb.md), [ADR-0018](../../adr/0018-drw-2d-zeichen-daten.md), [ADR-0019](../../adr/0019-drw-2d-canvas.md)]
---

# Slice 056: Wand im 2D-Canvas — ADR + AK-Schärfung

**Status:** open — **eigenes
[MR-006](../../../../harness/conventions.md#mr-006--unabhängiges-plan-review-vor-implementierungs-start)
vor dem Start.** Dieser Slice schreibt **Doku, keinen Produktions-Code** (Muster
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

## 1. Ziel

**Zwei Artefakte, beide lösungs- bzw. formfrei an der jeweils richtigen Stelle:**

1. **Eine neue ADR** (nächste freie Nummer: 0021) — die acht Fragen (§2), die der Spec-Text **nicht** entscheidet, beantwortet und
   begründet; `Proposed` → unabhängiges Text-Review → `Accepted`; ADR-Index + Folgepflicht-Block
   nachgezogen.
2. **AK-Schärfung** — [`LH-FA-WAL-001`](../../../../spec/lastenheft.md#lh-fa-wal-001--wand-zeichnen)
   bekommt einen Block **„Interaktive Erzeugung (2D-Zeichenfläche)"**,
   [`LH-FA-WAL-002`](../../../../spec/lastenheft.md#lh-fa-wal-002--wandstärke-definieren)/003 je
   einen Konjunkt für die **interaktive** Änderung. **Präzedenz:**
   [`slice-041a`](../done/slice-041a-drw-canvas-adr-ak.md) hat für
   [`LH-FA-DRW-005`](../../../../spec/lastenheft.md#lh-fa-drw-005) genau diesen Block ergänzt — dieselbe
   Bauart, dieselbe
   [MR-008](../../../../harness/conventions.md#mr-008--lastenheft-schärfung-bleibt-lösungsfrei)-Grenze.

**Kein Produktions-Code.** Der Slice liefert die Grundlage; 057/058 liefern die Funktion.

## 2. Die acht Fragen, die der Spec-Text nicht entscheidet

*(Sie sind der Grund, warum hier eine ADR steht und nicht direkt ein Impl-Slice. Jede ist am Artefakt
belegt, nicht vermutet.)*

| # | Frage | Warum offen — am Artefakt |
|---|---|---|
| **F1** | **Werkzeug-Wahl.** Woher weiß der Canvas, ob ein Zug eine **Hilfslinie** oder eine **Wand** erzeugt? | Der Canvas hat heute **eine** Geste: Links-Zug ⇒ `addGuideLine` (`canvas_widget.cpp`, `mouseReleaseEvent`). Eine zweite Bauform braucht einen Modus — Werkzeugleiste, Tastatur-Modifikator oder getrennte Flächen. Der Spec-Text kennt keinen Werkzeug-Begriff. |
| **F2** | **Einzelsegment oder Wandzug?** | [`LH-FA-WAL-001`](../../../../spec/lastenheft.md#lh-fa-wal-001--wand-zeichnen) fordert im Happy Path einen **Linienzug mit ≥ 2 Punkten**; das Domänen-Modell trägt aber **Einzelsegment**-Wände, und `spezifikation.md` §2.1 führt *Wandzüge/Polylines … folgen als Erweiterung* ausdrücklich als **offen**. Ein Zug-in-einem-Rutsch ist damit **nicht** durch das Modell gedeckt — entweder v1 zeichnet ein Segment je Zug (Teilumfang, ehrlich benannt), oder das Modell wächst. **Das ist die teuerste der acht Fragen.** |
| **F3** | **Selektion/Picking.** „Parametrisch ändern" setzt voraus, dass der Benutzer **eine bestimmte Wand** benennt. | Es gibt heute **keine** Selektion — weder im Canvas noch im 3D-Viewer ([ADR-0019](../../adr/0019-drw-2d-canvas.md) nimmt sie aus, [ADR-0009](../../adr/0009-gui-framework-qt6.md) verweist die 3D-Selektion auf einen eigenen Re-Eval). Offen: Treffer-Prüfung im Bildschirm- oder Modellraum, Toleranz, Mehrfachtreffer, **und** ob die Auswahl **UI-Zustand** oder **Modell-Zustand** ist (bei Modell-Zustand wären Schema und Persistenz betroffen — dann wäre der Slice ein ganz anderer). |
| **F4** | **Wo ändert man den Parameter?** | `EditStructurePort` bietet `setWallThickness`/`setWallHeight` mit `ParamResult{applied_mm, status}`. Eine **Bedienfläche** dafür existiert nicht (das Fenster trägt seit slice-053 nur ein Datei-Menü). Panel, Dialog oder Inline-Eingabe — und wie die Rückmeldung aussieht, ist Frage F5. |
| **F5** | **`Clamped`/`Rejected` sichtbar machen.** | Der Port meldet drei Ausgänge (`Accepted`/`Clamped`/`Rejected`, [`E-VAL-001`](../../../../spec/spezifikation.md#4-fehler-codes-und-logging-felder)-Familie). [`LH-FA-WAL-002`](../../../../spec/lastenheft.md#lh-fa-wal-002--wandstärke-definieren) fordert bei Überschreitung „auf Grenzwert geklemmt **+ Hinweis**"; **welchen Hinweis die Oberfläche gibt, ist unentschieden** — und ohne Entscheidung wird die Klemmung still, was die AK verletzt. |
| **F6** | **Refresh nach der Mutation.** | Anders als Hilfslinien melden Wand-Mutationen einen `op`: `addWall` ruft `notifyListeners({WallAdded, …})` **nach** Nachbar-Rebuild und Raum-Neuerkennung (`structure_edit_service.cpp`). Der Canvas ist bereits `ModelChangedPort`-Beobachter — der Refresh-Pfad **existiert** und ist der 3D-Viewer-Pfad. Zu entscheiden ist nur, ob der Canvas nach dem **eigenen** Kommando zusätzlich selbst repaintet (wie bei Hilfslinien) oder ausschließlich über die Meldung — **doppeltes Neu-Einrahmen** wäre sonst ein sichtbarer Sprung. **[ADR-0018](../../adr/0018-drw-2d-zeichen-daten.md)s „kein op" ist hier nicht berührt** (das gilt für Zeichen-Daten, nicht für Bauteile). |
| **F7** | **Der zweite UI-Mutator.** | Die slice-043-Option A verdrahtet den Schreibpfad als `std::function` aus `ui/command/` (`EditDrawingGuideLineSink`), damit `view/` **keinen** Driving-Port include. Für Wände entsteht die analoge Senke am `EditStructurePort`. **Kein neuer Gate-Fall** (die `adapter_sink`-Skalarität aus dem [MR-006](../../../../harness/conventions.md#mr-006--unabhängiges-plan-review-vor-implementierungs-start)-HIGH-1 von slice-043 bleibt unberührt, weil `view/` weiterhin **keinen** Port sieht) — die ADR muss das aber **feststellen**, statt es zu unterstellen. |
| **F8** | **[`E-GEO-001`](../../../../spec/spezifikation.md#4-fehler-codes-und-logging-felder) „Eingabe außerhalb des Zeichenbereichs".** | Die Negative-AK von [`LH-FA-WAL-001`](../../../../spec/lastenheft.md#lh-fa-wal-001--wand-zeichnen) nennt diesen Fall. Der Canvas hat heute **keinen** begrenzten Zeichenbereich (Pan/Zoom sind frei, `ViewTransform`). Entweder die ADR definiert einen Bereich, oder die AK bekommt die ehrliche Feststellung, dass der Fall in dieser Ausbaustufe **nicht erreichbar** ist. **Was nicht geht: die AK-Zeile stehen lassen und nichts dazu bauen.** |

## 3. Bewusst NICHT Teil

- **Jeder Produktions-Code.** Der Slice liefert ADR + AK; 057/058 (§8) implementieren.
- **Räume, Türen, Fenster, Treppen, Dächer interaktiv.** Der Wellen-Trigger nennt die **Wand**. Alles
  andere ist eigener Schnitt — sonst entsteht das Sammelbecken erneut.
- **Wand verschieben/teilen** ([`LH-FA-WAL-004`](../../../../spec/lastenheft.md#lh-fa-wal-004)/005) —
  reine Outline-Anforderungen, nicht Trigger-Teil.
- **3D-Selektion** ([ADR-0009](../../adr/0009-gui-framework-qt6.md)-Re-Eval) — unberührt.
- **Layer-Bedien-Panel, Bemaßung** — weitere Re-Eval-Träger aus [ADR-0019](../../adr/0019-drw-2d-canvas.md), hier nicht mit-entschieden.
- **Raster/Winkel** ([`LH-FA-DRW-002`](../../../../spec/lastenheft.md#modul-zeichnungsfunktionen-drw)/003) —
  bleiben zurückgestellt (048b-Closure-Deferral).

## 4. Orakel-Schnitt — ein Doku-Slice hat Doku-Sensoren

*(Ehrlich benannt: dieser Slice hat **kein** Verhaltens-Orakel, weil er kein Verhalten liefert. Die
Sensoren sind computational, nicht inferentiell — das ist der Unterschied zwischen „geprüft" und
„behauptet".)*

| # | Zusicherung | Wo geprüft | Diskriminierende Gegenprobe |
|---|---|---|---|
| 1 | Die ADR ist **im Index geführt** und der Index-Eintrag ist **auflösbar** | `make docs-check` (`links`/`anchors`/`ids`) | Index-Zeile weglassen ⇒ `id-unlinked`/`target-missing` |
| 2 | Die ADR nennt **keine** Slice-Kennung im Körper (no-downward) | `make docs-check` (`matrix`, [MR-014](../../../../harness/conventions.md)) | `slice-057` in den ADR-Körper schreiben ⇒ rot |
| 3 | Die drei Spec-Straten bleiben **prozess-/zeit-rein** (kein `welle-N`) | `make docs-check` (`matrix`-Klasse `temporal`, [MR-023](../../../../harness/conventions.md#mr-023--spec-straten-sind-prozess-zeit-rein-d-check-matrix-klasse-temporal)) | „welle-6" in die AK schreiben ⇒ rot |
| 4 | **Lastenheft-Header == oberste Historie-Zeile** | `make docs-check` + [MR-010](../../../../harness/conventions.md)/[MR-012](../../../../harness/conventions.md) | Version bumpen, Historie vergessen ⇒ rot |
| 5 | **Die AK bleiben lösungsfrei** ([MR-008](../../../../harness/conventions.md#mr-008--lastenheft-schärfung-bleibt-lösungsfrei)) — kein Port-, Algorithmus-, Widget- oder Fehler-Code-Vokabular im Lastenheft-Körper | **[MR-006](../../../../harness/conventions.md#mr-006--unabhängiges-plan-review-vor-implementierungs-start)-Linse, nicht computational** | — **benannte Grenze**, s. u. |
| 6 | **Kein Code-Diff** — `src/**`, `tests/**`, `data-model.yaml`, `schema.sql` unberührt | `git diff --stat` in der Closure + `make schema-check` | (Selbstprüfung; die Behauptung ist am Diff nachweisbar) |

**Zeile 5 ist nicht maschinell prüfbar, und das wird hier gesagt statt umschrieben.** „Lösungsfrei"
ist eine Urteilsfrage; kein d-check-Modul entscheidet sie. Sie hängt am unabhängigen Plan-/Text-Review
— dieselbe ehrliche Lage wie bei [MR-020](../../../../harness/conventions.md).

## 5. Definition of Done

- [ ] **`docs/plan/adr/0021-*.md`** (neu): Kontext · die acht Fragen aus §2 **je entschieden mit
      verglichenen Alternativen** · Konsequenzen · Folgepflichten · Re-Eval-Trigger. Status
      `Proposed` → **unabhängiges Text-Review** → `Accepted` (Muster
      [ADR-0019](../../adr/0019-drw-2d-canvas.md)).
- [ ] **[`docs/plan/adr/README.md`](../../adr/README.md)**: Index-Zeile + **Folgepflicht-Zeilen** für
      057/058 ([AGENTS §4](../../../../AGENTS.md): neue ADR aktualisiert den Index).
- [ ] **Lastenheft**: [`LH-FA-WAL-001`](../../../../spec/lastenheft.md#lh-fa-wal-001--wand-zeichnen)-Block
      „Interaktive Erzeugung (2D-Zeichenfläche)" (Happy/Boundary/Negative) +
      [`LH-FA-WAL-002`](../../../../spec/lastenheft.md#lh-fa-wal-002--wandstärke-definieren)/003
      je ein interaktiver Konjunkt — **lösungsfrei**. Version + oberste Zeile in
      [`lastenheft-historie.md`](../../../../spec/lastenheft-historie.md).
- [ ] **`spec/spezifikation.md` §1**: Mapping-Block für den interaktiven Wand-Weg (Werkzeug-Modus,
      Selektion als UI-Zustand, Parameter-Rückmeldung, Refresh-Pfad) — die **Mechanik**, die aus dem
      Lastenheft herausgehalten wird. **§2.1-Klausel „Wandzüge folgen als Erweiterung"** je nach
      F2-Entscheidung **nachziehen oder ausdrücklich stehen lassen** (nicht stillschweigend übergehen).
- [ ] **`spec/architecture.md` §1.1**: Driving-Ports-Tabelle um die Canvas-Klausel am
      `EditStructurePort` — **meilenstein- und slice-frei** ([AGENTS §2.7](../../../../AGENTS.md)).
- [ ] **F8 ist beantwortet** — entweder Zeichenbereich definiert **oder** die
      [`E-GEO-001`](../../../../spec/spezifikation.md#4-fehler-codes-und-logging-felder)-Negative als
      in dieser Ausbaustufe **nicht erreichbar** benannt. **Kein drittes Ergebnis** („später").
- [ ] **Die Folge-Slices existieren als Plan-Datei** in `open/` (057/058) — sonst blockiert
      [MR-020](../../../../harness/conventions.md) die Closure einer ADR mit offenen Folgepflichten.
- [ ] **CHANGELOG** [Unreleased]-Eintrag.
- [ ] **`make gates` grün**; **kein** Code-Diff (§4-6).

## 6. Plan (vor Code)

| Datei / Komponente | Art | Begründung |
|---|---|---|
| `docs/plan/adr/0021-*.{md}` | neu | die acht Entscheidungen (§2) |
| `docs/plan/adr/README.md` | ändern | Index + Folgepflicht-Block |
| `spec/lastenheft.md`, `spec/lastenheft-historie.md` | ändern | AK-Block + Version |
| `spec/spezifikation.md` | ändern | §1-Mapping, §2.1-Klausel je nach F2 |
| `spec/architecture.md` | ändern | §1.1 Driving-Ports (slice-/meilensteinfrei) |
| `docs/plan/planning/open/slice-057-*.{md}`, `slice-058-*.{md}` | neu | Folge-Slices als Plan ([MR-020](../../../../harness/conventions.md)) |
| `CHANGELOG.md` | ändern | [Unreleased] |
| `docs/reviews/`-Reports | neu | [MR-006](../../../../harness/conventions.md#mr-006--unabhängiges-plan-review-vor-implementierungs-start) (Plan) + ADR-Text-Review |

**Nicht berührt:** `src/**`, `tests/**`, `data-model.yaml`/`schema.sql`, `.a-check.yml`/`.d-check.yml`,
`Makefile`.

## 7. Risiken

- **R1 — F2 (Einzelsegment vs. Wandzug) kann den Schnitt sprengen.** Fällt die Entscheidung für
  echte Wandzüge, wächst das **Domänen-Modell** (Persistenz, Schema, Export, Eckenschluss
  [`LH-FA-WAL-006`](../../../../spec/lastenheft.md#lh-fa-wal-006--wand-verbinden)) — das ist kein
  UI-Slice mehr, sondern ein eigener Fundament-Strang. **Empfehlung dieses Plans: v1 zeichnet ein
  Segment je Zug**, der Linienzug-Konjunkt der AK bleibt eine **benannte Reifephase-Grenze** (Muster
  der DRW-Teilumfang-Klauseln). Der Wellen-Trigger sagt „**eine** Wand" — er ist damit erfüllbar.
  **Die Entscheidung gehört in die ADR, nicht in den Impl-Vollzug.**
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
  ([AGENTS §2.5](../../../../AGENTS.md)). Vor dem Schreiben ist zu prüfen, ob eine der acht Antworten
  eine 0019-Entscheidung **umstößt** (Kandidat: Entscheidung 5, Kommando-Naht — sie sollte
  **fortgeschrieben**, nicht ersetzt werden).
- **R5 — der Slice könnte zum Papier werden.** Ein ADR-Slice ohne Impl-Nachfolge ist genau die
  Buchführungs-Fiktion, die welle-5 gekostet hat. **Gegenmittel in der DoD:** 057/058 existieren als
  Plan-Datei, bevor 056 schließt.

## 8. Der Strang danach (Sequenz, nicht Umfang dieses Slice)

| Slice | Was | Trigger-Beitrag |
|---|---|---|
| **057** | **Wand zeichnen** im Canvas: Werkzeug-Modus (F1) + Zug ⇒ `addWall` über die neue `ui/command/`-Senke (F7) + Refresh (F6) + Headless-AK | „**zeichenbar**" |
| **058** | **Wand auswählen und ändern**: Picking (F3) + Parameter-Bedienung (F4) + `Clamped`/`Rejected`-Rückmeldung (F5) + Headless-AK | „**parametrisch änderbar**" |

**Mit 058 ist der welle-6-Abschluss-Trigger erfüllt — und die Welle ist dann zu schließen, nicht
weiterzufüllen** (welle-5-Closure §5-1; der Wellen-Block der Roadmap trägt die Regel).

## 9. Closure-Trigger

- Die neue ADR ist `Accepted` (Text-Review durch, 0 HIGH) + Index/Folgepflicht nachgezogen; AK-Block und
  Spec-§1-Mapping geschrieben; Lastenheft-Version == oberste Historie-Zeile; 057/058 liegen als Plan
  in `open/`; `make gates` grün; **kein** Code-Diff; Closure-Notiz.

## 10. Sub-Area-Modus-Begründung

### Sub-Area: Spec-Schreibung + Planning-Lifecycle

- **Modus:** GF; **Dichte:** mittel — eine ADR mit acht Entscheidungen ist der Aufwand, nicht die
  Zeilenzahl der AK.
- **Phase-Reife:** der Canvas trägt seit 043 einen geprüften Interaktions-Pfad, seit 048b eine
  Eingabe-Quantisierung; der `EditStructurePort` liegt seit slice-003a/013b vollständig vor
  (`addWall`, `setWallThickness`, `setWallHeight` mit `ParamResult`). **Es fehlt keine Kern-Funktion —
  es fehlt der Weg dorthin.**
- **Risiko:** mittel — nicht im Schreiben, sondern in **F2** und **F3** (R1/R2). Beide sind
  Entscheidungen mit Folgekosten, und beide gehören genau deshalb **vor** den Code.

## 11. [MR-006](../../../../harness/conventions.md#mr-006--unabhängiges-plan-review-vor-implementierungs-start)-Einarbeitung

_(offen — der Lauf steht vor dem Start aus; HIGHs blockieren ihn. Die Linse hat hier eine besondere
Aufgabe: **prüfen, ob die acht Fragen die richtigen acht sind** — eine übersehene neunte würde erst
im Impl auffallen, wenn die ADR schon `Accepted` und damit immutabel ist.)_

## 12. Closure-Notiz

_(bei Ausführung auszufüllen)_
