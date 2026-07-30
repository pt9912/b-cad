---
id: slice-056
titel: Wand im 2D-Canvas — ADR + AK-Schärfung (Wellen-Kern von welle-6, [LH-FA-WAL-001](../../../../spec/lastenheft.md#lh-fa-wal-001--wand-zeichnen))
status: done
welle: welle-6-interaktiv-planen
lastenheft_refs: [[LH-FA-WAL-001](../../../../spec/lastenheft.md#lh-fa-wal-001--wand-zeichnen), [LH-FA-WAL-002](../../../../spec/lastenheft.md#lh-fa-wal-002--wandstärke-definieren), [LH-FA-WAL-003](../../../../spec/lastenheft.md#lh-fa-wal-003--wandhöhe-definieren), [LH-FA-WAL-006](../../../../spec/lastenheft.md#lh-fa-wal-006--wand-verbinden), [LH-FA-DRW-001](../../../../spec/lastenheft.md#lh-fa-drw-001), [LH-FA-DRW-005](../../../../spec/lastenheft.md#lh-fa-drw-005), [LH-FA-D3-002](../../../../spec/lastenheft.md#lh-fa-d3-002--echtzeitaktualisierung)]
adr_refs: [[ADR-0001](../../adr/0001-hexagonale-architektur.md), [ADR-0008](../../adr/0008-aenderungs-benachrichtigung.md), [ADR-0009](../../adr/0009-gui-framework-qt6.md), [ADR-0010](../../adr/0010-headless-gl-xvfb.md), [ADR-0018](../../adr/0018-drw-2d-zeichen-daten.md), [ADR-0019](../../adr/0019-drw-2d-canvas.md)]
---

# Slice 056: Wand im 2D-Canvas — ADR + AK-Schärfung

**Status:** done (2026-07-28) — **vier
[MR-006](../../../../harness/conventions.md#mr-006--unabhängiges-plan-review-vor-implementierungs-start)-Läufe
durch** (Lauf 1: 2 HIGH · Lauf 2: 1 HIGH · Lauf 3: 1 HIGH / 6 MED / 4 LOW / 4 INFO), alle drei
„nicht startbar"; Einarbeitung in §11/§11a/§11b. **Die Fragen-Zahl ist von acht über zehn und
dreizehn auf vierzehn gewachsen — jede Erweiterung kam aus einem Review, keine vom Autor.** Dieser
Slice schreibt **Doku, keinen Produktions-Code** (Muster
[`slice-041a`](../done/slice-041a-drw-canvas-adr-ak.md): ADR + AK-Schärfung vor dem Impl-Strang).

**Welle:** welle-6-interaktiv-planen — **dieser Strang ist der Wellen-Kern.** Der
[Abschluss-Trigger](../in-progress/roadmap.md) lautet: *eine **Wand** ist im 2D-Canvas **zeichenbar
und parametrisch änderbar**, ohne Kommandozeile.* Bis heute hat er **keinen** Plan; dieser Slice legt
die Entscheidungsgrundlage, die Folge-Slices (057 Lese-Naht · 058 zeichnen · 059 ändern, §8) lösen ihn ein.

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

1. **Eine neue ADR** (nächste freie Nummer: 0021) — die **vierzehn** Fragen (§2), die der Spec-Text nicht
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

**Kein Produktions-Code.** Der Slice liefert die Grundlage; 057–059 liefern die Funktion.

## 2. Die vierzehn Fragen, die der Spec-Text nicht entscheidet

*(Sie sind der Grund, warum hier eine ADR steht und nicht direkt ein Impl-Slice. Jede ist am Artefakt
belegt, nicht vermutet. **F9/F10 fand der erste Review, F11/F12/F13 der zweite, F14 der dritte** —
die Vorfassung führte acht. Die Zahl ist kein Gütesiegel und **auch keine Vollständigkeits-Zusage**:
sie ist der Stand nach drei Suchläufen. Wo die Vollständigkeit wirklich zu belegen ist, sagt R6.)*

| # | Frage | Warum offen — am Artefakt |
|---|---|---|
| **F1** | **Werkzeug-Wahl.** Woher weiß der Canvas, ob ein Zug eine **Hilfslinie** oder eine **Wand** erzeugt? | Der Canvas hat heute **eine** Geste: Links-Zug ⇒ das injizierte Schreib-Callable, das über `ui/command/edit_drawing_guide_line_sink.h` auf `addGuideLine` führt. Eine zweite Bauform braucht einen Modus — Werkzeugleiste, Tastatur-Modifikator oder getrennte Flächen. **Das Lastenheft kennt den Begriff „Werkzeug", entscheidet die Frage aber nicht:** die DRW-005-Negative sagt „dann **erzeugt das Werkzeug nichts**" (Prosa in einer fremden AK), und [`LH-FA-UI-005`](../../../../spec/lastenheft.md#modul-benutzeroberfläche-ui) („Anpassbare Werkzeugleisten") ist reine Outline ohne AK. |
| **F2** | **Wie viele Segmente je Zeichen-Geste?** | **Keine Modell-Frage, sondern eine Interaktions-Frage** (Lauf-1-MEDIUM-1). [`LH-FA-WAL-001`](../../../../spec/lastenheft.md#lh-fa-wal-001--wand-zeichnen) fordert „Linienzug mit ≥ 2 Punkten ⇒ **je Segment eine Wand**" — das ist **exakt** der bestehende Einzelsegment-Typ; ein Zug mit *n* Punkten bildet sich auf *n−1* `addWall`-Aufrufe ab, ohne dass das Domänen-Modell wächst, und der Eckenschluss dieser Segmente liegt seit slice-012 vor ([`LH-FA-WAL-006`](../../../../spec/lastenheft.md#lh-fa-wal-006--wand-verbinden)). Die `spezifikation.md`-§2.1-Klausel „Wandzüge/Polylines folgen als Erweiterung" betrifft die **Wand-Entität** mit mehreren Segmenten, **nicht** die Geste. Offen ist damit die Bedien-Frage: mehrpunktiger Zug in einem Rutsch oder ein Segment je Zug — und **wenn Letzteres, unterschreitet v1 den geltenden Happy Path** und braucht eine benannte Teilumfang-Klausel im Lastenheft. **Konjunkt (nur bei mehrpunktigem Zug):** der **Teilerfolg** — Transaktion ist der **Port-Aufruf**, Benutzer-Einheit die **Geste**; wird ein Zwischen-Segment abgelehnt, bleibt ein halber Wandzug stehen (ohne Rückweg, s. F12), und das Neu-Einrahmen läuft mitten in der Geste. |
| **F3** | **Selektion/Picking.** „Parametrisch ändern" setzt voraus, dass der Benutzer **eine bestimmte Wand** benennt. | Es gibt heute **keine** Selektion — weder im Canvas noch im 3D-Viewer ([ADR-0019](../../adr/0019-drw-2d-canvas.md) nimmt sie aus, [ADR-0009](../../adr/0009-gui-framework-qt6.md) verweist die 3D-Selektion auf einen eigenen Re-Eval). Offen: Treffer-Prüfung im Bildschirm- oder Modellraum, Toleranz, Mehrfachtreffer, **und** ob die Auswahl **UI-Zustand** oder **Modell-Zustand** ist (bei Modell-Zustand wären Schema und Persistenz betroffen — dann wäre der Slice ein ganz anderer). |
| **F4** | **Wo ändert man den Parameter?** | `EditStructurePort` bietet `setWallThickness`/`setWallHeight` mit `ParamResult{applied_mm, status}`. Eine **Bedienfläche** dafür existiert nicht (das Fenster trägt seit slice-053 nur ein Datei-Menü). Panel, Dialog oder Inline-Eingabe — und wie die Rückmeldung aussieht, ist Frage F5. |
| **F5** | **Rückmeldung an den Benutzer — alle drei Fälle, nicht nur die Klemmung.** | Drei AK verlangen eine sichtbare Reaktion, und **keine** hat heute eine: (a) [`LH-FA-WAL-002`](../../../../spec/lastenheft.md#lh-fa-wal-002--wandstärke-definieren)/003 „auf Grenzwert **geklemmt + Hinweis**" (`ParamStatus::Clamped`); (b) `ParamStatus::Rejected` (Modell unverändert, [`E-VAL-001`](../../../../spec/spezifikation.md#4-fehler-codes-und-logging-felder)); (c) **die WAL-001-Boundary „Null-Längen-Wand ⇒ verworfen, *Hinweis*"** — `addWall` verwirft heute **still** (`return std::nullopt`), und der Canvas wertet den Rückgabewert seines Schreib-Callables nicht aus (Lauf-1-MEDIUM-6). Ohne Entscheidung bleiben alle drei still, und die AK sind verletzt. |
| **F6** | **Refresh nach der Mutation.** | Anders als Hilfslinien melden Wand-Mutationen einen `op`: `addWall` ruft `notifyListeners({WallAdded, …})` **nach** Nachbar-Rebuild und Raum-Neuerkennung (`structure_edit_service.cpp`). Der Canvas ist bereits `ModelChangedPort`-Beobachter — der Refresh-Pfad **existiert** und ist der 3D-Viewer-Pfad. Zu entscheiden ist, ob der Canvas nach dem **eigenen** Kommando zusätzlich selbst repaintet (wie bei Hilfslinien) oder ausschließlich über die Meldung — **doppeltes Neu-Einrahmen** wäre sonst ein sichtbarer Sprung. **[ADR-0018](../../adr/0018-drw-2d-zeichen-daten.md)s „kein op" ist hier nicht berührt** (das gilt für Zeichen-Daten, nicht für Bauteile). |
| **F7** | **Der zweite UI-Mutator.** | Die slice-043-Option A verdrahtet den Schreibpfad als `std::function` aus `ui/command/`, damit `view/` **keinen** Driving-Port include. Für Wände entsteht die analoge Senke am `EditStructurePort`. **Schwache Frage — ehrlich als solche geführt** (Lauf-1-INFO-2): [ADR-0019](../../adr/0019-drw-2d-canvas.md) Entscheidung 5 und die `.a-check.yml`-Kanten (`ui_command → ports_driving` erlaubt, `ui_view → ports_driving` nicht) entscheiden sie bereits **und setzen sie maschinell durch**. Die ADR **stellt fest**, sie entscheidet hier nichts Neues. |
| **F8** | **[`E-GEO-001`](../../../../spec/spezifikation.md#4-fehler-codes-und-logging-felder) „Eingabe außerhalb des Zeichenbereichs".** | Die Negative-AK von [`LH-FA-WAL-001`](../../../../spec/lastenheft.md#lh-fa-wal-001--wand-zeichnen) nennt diesen Fall. Ein **begrenzter Zeichenbereich in Modell-mm** existiert nicht; der Zoom ist geklemmt (`[1e-4, 100]` px/mm seit dem slice-043-Code-Review) und ein interaktives Pan gibt es nicht — die Begrenzung ist also eine Sicht-, keine Bereichs-Grenze (Lauf-1-LOW-1). Entweder die ADR definiert einen Bereich, oder die AK bekommt die ehrliche Feststellung, dass der Fall in dieser Ausbaustufe **nicht erreichbar** ist. **Was nicht geht: die AK-Zeile stehen lassen und nichts dazu bauen.** |
| **F9** | **Fangen beim Bauteil-Zeichnen** — gilt der 048b-Fang auch für den Wand-Zug? | **Vom ersten Review gefunden (HIGH-2); die Vorfassung hatte die Frage nicht.** [`LH-FA-DRW-001`](../../../../spec/lastenheft.md#lh-fa-drw-001) begrenzt den Fang wörtlich auf „das interaktive Zeichnen von **Hilfslinien**" und führt „**Fangen beim Bauteil-Zeichnen**" im Teilumfang-Block ausdrücklich als **offen**. Die Folge ist nicht kosmetisch: der WAL-001-Happy fordert „verbundene Endpunkte werden **geometrisch verbunden**", [`LH-FA-WAL-006`](../../../../spec/lastenheft.md#lh-fa-wal-006--wand-verbinden) setzt einen **gemeinsamen** Endpunkt voraus, und `spezifikation.md` §3 legt die Toleranz auf **0,1 mm** fest — während ein Bildschirm-Pixel bei Default-Zoom rund **20 mm** entspricht. **Ohne Fang ist der Eckenschluss auf dem interaktiven Weg praktisch unerreichbar.** |
| **F10** | **Fehler-Barriere des zweiten Mutators.** | **Vom ersten Review gefunden (MEDIUM-2).** Der erste UI-Mutator lehnt **wertbasiert** ab (`addGuideLine → std::optional`, Modell unverändert); der zweite **wirft**: `addWall` wirft `std::out_of_range` bei unbekannter Geschoss-Id, `setWallThickness`/`setWallHeight` werfen über `mutableWall` bei unbekannter Wand-Id. Das ist kein Randfall — die Zeichen-Ziele werden als eingefrorene Ids injiziert und nach einem Projekt-Laden nachgezogen (`setTarget`/`setActiveStorey` tragen genau diesen Kommentar). Dieselbe Konstellation führt beim Hilfslinien-Weg zu stiller Ablehnung, beim Wand-Weg zu einem **Wurf aus einem Qt-Event-Handler**. Wo diese Barriere liegt, ist unentschieden. |
| **F11** | **Die 2D-Lese-Naht selbst** — womit soll der Canvas eine **Wand** treffen und benennen? | **Vom zweiten Review gefunden (HIGH-1) — und [ADR-0019](../../adr/0019-drw-2d-canvas.md) benennt die Folge in ihrem Re-Eval-Block wörtlich:** »2D-Lese-Naht (`PlanViewPort`) um **Bauteil-/Treffer-Queries erweitern**«. Der Plan zitierte dieselbe ADR zweimal, diesen Satz nicht. `model::PlanSegment` trägt `{x1,y1,x2,y2}` — **keine `WallId`, keine Art**; `projectPlan` mischt Wand-Achsen und sichtbare Hilfslinien in **eine** Liste, `paintEvent` zeichnet beide gleich. **F3** (Picking) hätte damit nur **anonyme** Segmente zu treffen, **F4** (Parameter ändern) braucht eine `WallId`, die die Naht nicht liefert, und die geplante WAL-001-Happy-AK wäre nicht als **Wand** beobachtbar. Das ist die teuerste offene Entscheidung des Strangs, weil sie einen **Kern-Werttyp** und einen **Read-Port** berührt — nicht nur das Widget. **Und sie zieht einen fremden Re-Eval-Trigger** (Lese-Runde **L3**): `PlanView` reist im `DerivedGeometry`-Bündel, und [ADR-0020](../../adr/0020-driven-adapter-serialisieren-kern-liefert-geometrie.md) führt „das Bündel wird zu breit" als Anlass, das Varianten-/Bedarfs-Modell neu zu bewerten. |
| **F12** | **Der Rückweg aus einer Wand-Geste.** | **Vom zweiten Review gefunden (MEDIUM-2).** `EditStructurePort` hat **kein `removeWall`** — der Vorgänger-Mutator hat `removeGuideLine`. Der Canvas kennt keinen **Gesten-Abbruch** (Escape, Fokusverlust), und Undo ist unbedient. Eine falsch gezogene Wand ist damit in der Oberfläche **nicht rücknehmbar**; §3 grenzte bisher nur die Undo-Hälfte ab. |
| **F13** | **Welche Parameter gehören zu „parametrisch änderbar"?** | **Vom zweiten Review gefunden (MEDIUM-3).** F4 führt Stärke und Höhe. Das Lastenheft führt daneben [`LH-FA-WAL-007`](../../../../spec/lastenheft.md#lh-fa-wal-007--wandtyp-wählen) (**Wandtyp**, mit Wertemenge und Wirkung), und der Port bietet `setWallMaterial`. Beide sind weder in F4 geführt noch in §3 abgegrenzt — obwohl §3 die Nachbarn WAL-004/005 ausdrücklich abgrenzt und der Wellen-Trigger genau „**parametrisch** änderbar" lautet. |
| **F14** | **Woran wird reiner UI-Zustand nachgewiesen?** Auswahl (F3), Werkzeug-Modus (F1) und Hinweis (F5) haben **kein Korrelat außerhalb des Widgets**. | **Vom dritten Review gefunden (HIGH-1) — und ZWEI Accepted-ADRs benennen genau das wörtlich als offen.** Jede bisherige Interaktions-AK dieses Repos hängt an einem **Modell**-Surrogat (`building().guide_lines`, über Persistenz und Export weiter beobachtbar). Auswahl, Werkzeug-Modus und Hinweis sind **nicht persistiert, nicht exportiert, nicht im `Building`** — es gibt nichts, woran eine AK sie festmachen könnte außer dem Widget selbst. [ADR-0009](../../adr/0009-gui-framework-qt6.md) (f) hält `tests/e2e/` leer, bis „ein Treiber mit **Interaktion/Selektion** relevant wird"; [ADR-0019](../../adr/0019-drw-2d-canvas.md) E7 schreibt: „`tests/e2e/` bleibt daher leer, **bis eine AK entsteht, die nur echt end-to-end prüfbar ist (z. B. Selektion / mehrschrittige Interaktion)**". **Dieser Strang liefert beide dort genannten Beispiele** — und `tests/e2e/` enthält bis heute nur `.gitkeep`. §8 buchte für 058/059 „Headless-AK" — **die Antwort, bevor die Frage gestellt war**; das ist nach Lauf 4 entfernt, die §8-Zeilen tragen jetzt „die **in F14 entschiedene** Nachweis-Naht". |

## 3. Bewusst NICHT Teil

- **Jeder Produktions-Code.** Der Slice liefert ADR + AK; 057–059 (§8) implementieren.
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
- **Anpassbare Werkzeugleisten** ([`LH-FA-UI-005`](../../../../spec/lastenheft.md#modul-benutzeroberfläche-ui))
  **und Docking-Fenster** ([`LH-FA-UI-001`](../../../../spec/lastenheft.md#modul-benutzeroberfläche-ui)),
  beide Outline. Eine Werkzeugleisten-Antwort auf F1 und eine Panel-Antwort auf F4 **berühren** diese
  Anforderungen; sie **erfüllen** sie nicht und schärfen sie nicht (Lauf-1-MEDIUM-5, Lauf-3-MEDIUM-5).
*(Der **Teilerfolg einer mehrpunktigen Geste** stand hier in der Vorfassung — er ist **kein**
Nicht-Teil. Er ist jetzt als **Konjunkt von F2** geführt und in der DoD verankert; Lauf-4-MEDIUM-6:
etwas gleichzeitig „bewusst nicht Teil" zu nennen **und** einem ADR-Abschnitt zuzuweisen, ist keine
Verortung.)*
- **Bauteil-Ebenen** — eine interaktiv gezeichnete Wand erbt **nicht** die aktive Zeichen-Ebene des
  Canvas; Wände tragen weiterhin **keine** `layer_id` (Lese-Runde **L2**, §13). Das ist eine
  **Entscheidung, keine Auslassung**: [ADR-0018](../../adr/0018-drw-2d-zeichen-daten.md) führt
  „Layer-Zuordnung für Bauteile" als Re-Eval-Trigger, der die polymorphe
  `entity_layers`-Zuordnung aktivierte und **Layer cross-cutting** machte — eine Schema-Erweiterung,
  die dieser Strang nicht beschließt und nicht stillschweigend auslösen darf.
- **Undo/Redo** ([`LH-QA-003`](../../../../spec/lastenheft.md#lh-qa-003--undoredo)).
  Die Anforderung fordert ≥ 1000 rücknehmbare Schritte, `undo_commands` existiert im Schema, **kein**
  Mutations-Pfad bedient es. Der interaktive Wand-Weg erzeugt Modell-Mutationen ohne Undo-Anbindung —
  eine **Bestandslücke**, die dieser Slice weder einführt noch schließen muss. Sie gehört als eigene
  Entscheidung in Roadmap/Validator-Rolle (Lauf-1-INFO-3). **Hier benannt, damit sie nicht als
  übersehen gilt.** *(Der **Rückweg** aus einer Geste ist damit **nicht** mit abgegrenzt — er ist als
  **F13**-Nachbar in **F12** eine eigene Frage: Undo ist nur eine von drei möglichen Antworten.)*

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

- [x] **`docs/plan/adr/0021-*.{md}`** (neu): Kontext · die **vierzehn** Fragen aus §2 **je entschieden mit
      verglichenen Alternativen** · Konsequenzen · Folgepflichten · Re-Eval-Trigger. Status
      `Proposed` → **unabhängiges Text-Review** → `Accepted` (Muster
      [ADR-0019](../../adr/0019-drw-2d-canvas.md)).
- [x] **Die ADR trägt eine Fitness Function** — jede ADR seit 0011 hat eine, und
      [ADR-0019](../../adr/0019-drw-2d-canvas.md) fordert sie für den Canvas ausdrücklich
      („der Canvas muss `screenToModel` + den beobachtbaren Zustand exponieren"). Die Vorfassung
      verlangte sie nicht (Lauf-3-MEDIUM-1); nach **F14** ist sie der Kern, nicht die Zugabe.
- [x] **[`docs/plan/adr/README.md`](../../adr/README.md)**: Index-Zeile + **Folgepflicht-Zeilen** für
      057–059 ([AGENTS §4](../../../../AGENTS.md)). **Ohne Gate** — s. §4-1b.
- [x] **Lastenheft**: [`LH-FA-WAL-001`](../../../../spec/lastenheft.md#lh-fa-wal-001--wand-zeichnen)-Block
      „Interaktive Erzeugung (2D-Zeichenfläche)" (Happy/Boundary/Negative) +
      [`LH-FA-WAL-002`](../../../../spec/lastenheft.md#lh-fa-wal-002--wandstärke-definieren)/003
      je ein interaktiver Konjunkt — **lösungsfrei**.
- [x] **Falls F2 zugunsten des mehrpunktigen Zugs entschieden wird: der Teilerfolg ist mitzuentscheiden**
      (F2-Konjunkt) — ein abgelehntes Zwischen-Segment hinterlässt sonst einen halben Wandzug ohne
      Rückweg (Lauf-4-MEDIUM-6).
- [x] **Falls F2 zugunsten „ein Segment je Zug" entschieden wird: eine benannte Teilumfang-Klausel an
      [`LH-FA-WAL-001`](../../../../spec/lastenheft.md#lh-fa-wal-001--wand-zeichnen)** (Muster der
      DRW-/ROF-/STR-Teilumfänge). **Ohne sie unterschreitet die Lieferung den geltenden Happy Path**
      („Linienzug mit ≥ 2 Punkten") — die Vorfassung hatte die Klausel nur als Empfehlung im
      Risiko-Abschnitt (Lauf-1-MEDIUM-1).
- [x] **[`LH-FA-DRW-005`](../../../../spec/lastenheft.md#lh-fa-drw-005)-Teilumfang-Klausel nachziehen
      oder bewusst beibehalten**: sie führt „das **interaktive Zeichnen von Bauteilen** … bleibt
      ausdrücklich offen". Der neue WAL-001-Block macht sie im selben Dokument gegenläufig; **kein Gate
      fängt das** (Lauf-1-MEDIUM-4).
- [x] **F9 hat ZWEI Entscheidungsrichtungen, und beide kosten Arbeit** (Lauf-2-MEDIUM-1):
      `snappedModelPos` sitzt **unbedingt** in Press/Release, also in der **Geste** — nicht im
      Hilfslinien-Sink. Ein Wand-Werkzeug **fängt damit per Default**. Fällt F9 **dafür**, ist die
      [`LH-FA-DRW-001`](../../../../spec/lastenheft.md#lh-fa-drw-001)-Teilumfang-Zeile nachzuziehen
      („Fangen beim Bauteil-Zeichnen" steht dort als **offen**); fällt F9 **dagegen**, ist
      **Unterdrückungs-Arbeit** im Canvas nötig — sonst steht die Lastenheft-Zeile **still falsch**.
      **Kein drittes Ergebnis.**
- [x] **Lastenheft-Version + Historie** ([MR-010](../../../../harness/conventions.md)/[MR-012](../../../../harness/conventions.md)):
      Header == Version der neu ergänzten Zeile in
      [`lastenheft-historie.md`](../../../../spec/lastenheft-historie.md); **Platzierung unmittelbar
      nach der `0.1.16`-Zeile** (die Tabelle ist nicht monoton sortiert). Zielnummer beim Vollzug
      feststellen — [`slice-055`](../done/slice-055-fang-anzeige.md) schärft ebenfalls, die Reihenfolge ist
      offen. **Sensor: die [MR-006](../../../../harness/conventions.md#mr-006--unabhängiges-plan-review-vor-implementierungs-start)-Linse, kein Gate** (§4-4).
- [x] **`spec/spezifikation.md` §1**: Mapping-Block für den interaktiven Wand-Weg (Werkzeug-Modus,
      Selektion, Parameter-Rückmeldung, Refresh-Pfad, Fang-Geltung, Fehler-Barriere) —
      die **Mechanik**, die aus dem Lastenheft herausgehalten wird — **einschließlich der 2D-Lese-Naht
      aus F11 und der Nachweis-Naht aus F14**. **Die Aufzählung nennt die Themen, nicht die Antworten**
      (Lauf-4-MEDIUM-5): ob die Auswahl UI- oder Modell-Zustand ist, entscheidet **F3**, nicht diese
      DoD-Zeile — die Vorfassung schrieb „Selektion als UI-Zustand" fest und nahm damit zugleich die
      Prämisse von F14 vorweg (Lauf-3-MEDIUM-2: die Zeile zählte unverändert dieselben
      sechs Punkte, obwohl der Lauf-2-HIGH sie ausdrücklich benannt hatte). **§2.1-Klausel „Wandzüge
      folgen als Erweiterung"** je nach F2-Entscheidung **nachziehen oder ausdrücklich stehen lassen**.
- [x] **`spec/spezifikation.md` §6**: die Vertragszeile „**2D-Zeichenfläche (DRW-Canvas)**" (seit
      slice-041a) trägt den Konjunkt „**Selbst-Refresh ohne `op`**". Der Wand-Weg macht ihn
      unvollständig (Wand-Mutationen **melden** einen `op`) — nachziehen (Lauf-2-MEDIUM-4).
- [x] **`spec/architecture.md` §1.1**: Driving-Ports-Tabelle um die Canvas-Klausel am
      `EditStructurePort` **und** — je nach **F11**-Entscheidung — um die erweiterte 2D-Lese-Naht
      (`PlanViewPort`); **meilenstein- und slice-frei** ([AGENTS §2.7](../../../../AGENTS.md)).
- [x] **Falls F11 die Lese-Naht erweitert: die Folge für `model::PlanView`/`PlanSegment` ist im
      ADR-Text zu tragen** — es wäre die erste Änderung an einem **Kern-Werttyp** in diesem Strang und
      berührt PDF/PNG-Export und die Golden-Files, die dieselbe Projektion konsumieren.
- [x] **F8 ist beantwortet** — entweder Zeichenbereich definiert **oder** die
      [`E-GEO-001`](../../../../spec/spezifikation.md#4-fehler-codes-und-logging-felder)-Negative als
      in dieser Ausbaustufe **nicht erreichbar** benannt. **Kein drittes Ergebnis** („später").
- [x] **Die Folge-Slices existieren als Plan-Datei** (057 Lese-Naht · 058 zeichnen · 059 auswählen
      und ändern — die Sequenz-Entscheidung steht in §8) in `open/`, `next/` **oder**
      `in-progress/` — **oder** eine **explizite Deferral-Entscheidung** steht in Roadmap/ADR-Index.
      [MR-020](../../../../harness/conventions.md) regelt die Closure einer **Slice** bzw. einer
      **Welle** (eine ADR hat keine Closure) und lässt beide Wege zu (Lauf-1-LOW-2).
- [x] **Die ADR stellt die [MR-009](../../../../harness/conventions.md#mr-009--geometrielastiges-code-review-vor-welle-closure)-Einschlägigkeit fest** (Lese-Runde **L1**, §13):
      [ADR-0018](../../adr/0018-drw-2d-zeichen-daten.md) hat [MR-009](../../../../harness/conventions.md#mr-009--geometrielastiges-code-review-vor-welle-closure) für Hilfslinien ausdrücklich
      **verneint** („keine neue Solid-Geometrie") — für **Wände** gilt das Gegenteil (Solid,
      Nachbar-Eckenschluss, Raum-Neuerkennung). Ohne die Feststellung erben 058/059 stillschweigend
      eine Verneinung, die für sie falsch ist. Präzedenz: 0018 **und** 0019 treffen die Aussage je
      für ihren Umfang.
- [x] **CHANGELOG** [Unreleased]-Eintrag.
- [x] **[MR-006](../../../../harness/conventions.md#mr-006--unabhängiges-plan-review-vor-implementierungs-start)-Report
      unter `docs/reviews/`** je Lauf + **ADR-Text-Review** — als DoD-Zeile, nicht nur in der
      Datei-Tabelle.
- [x] **`make gates` grün**; **kein** Code-Diff, belegt am `git diff --stat` (§4-6).

## 6. Plan (vor Code)

| Datei / Komponente | Art | Begründung |
|---|---|---|
| `docs/plan/adr/0021-*.{md}` | neu | die vierzehn Entscheidungen (§2) |
| `docs/plan/adr/README.md` | ändern | Index + Folgepflicht-Block |
| `spec/lastenheft.md`, `spec/lastenheft-historie.md` | ändern | AK-Block + Teilumfang-Klauseln + Version |
| `spec/spezifikation.md` | ändern | §1-Mapping, §2.1-Klausel je nach F2 |
| `spec/architecture.md` | ändern | §1.1 Driving-Ports (slice-/meilensteinfrei) |
| `docs/plan/planning/open/slice-057-*.{md}`, `slice-058-*.{md}`, `slice-059-*.{md}` | neu | Folge-Slices als Plan ([MR-020](../../../../harness/conventions.md)) |
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
  Buchführungs-Fiktion, die welle-5 gekostet hat. **Gegenmittel in der DoD:** 057–059 existieren als
  Plan-Datei, bevor 056 schließt.
- **R6 — die Vollständigkeit der Fragenliste ist am PLAN nicht belegbar, und dieser Plan behauptet es
  auch nicht mehr.** Drei Läufe fanden sechs Lücken (Lauf 1: F9/F10 · Lauf 2: F11/F12/F13 · Lauf 3:
  F14) — acht → zehn → dreizehn → vierzehn.
  **Das frühere Kriterium „weitersuchen, bis ein Lauf leer ausgeht", ist zurückgenommen** (2026-07-28):
  es terminiert nicht. Ein gründlicher Reviewer findet an einem Plan dieser Länge immer noch etwas;
  die Regel liefe ins Unendliche und sähe dabei wie Sorgfalt aus. **Und die Zahlenreihe trägt sie
  ohnehin nicht:** F14 war der schwerste Fund aller drei Läufe und kam im Lauf mit der **geringsten**
  Ausbeute — sinkende Menge ist hier kein Argument für Vollständigkeit.
  **Die Vollständigkeitslast liegt beim ADR-Text-Review, nicht beim Plan-Review.** Der Plan listet
  Überschriften; die ADR trägt ausformulierte Entscheidungen mit Alternativen und Konsequenzen — daran
  ist eine **fehlende** Entscheidung besser zu sehen als an einer Tabellenzeile. Das Text-Review ist in
  der DoD verlangt, und der `Proposed`-Status ist das Netz: **immutabel wird nichts vor `Accepted`.**
  **Abbruchregel für die Plan-Läufe (vorab festgelegt, damit sie nicht im Nachhinein passend gewählt
  wird): 0 HIGH ⇒ startbar** — MEDIUM werden eingearbeitet, nicht mit einem weiteren Lauf beantwortet.
- **R7 — F11 (die Lese-Naht) ist nicht mehr nur eine UI-Frage.** Erweitert die ADR `PlanViewPort` um
  Bauteil-/Treffer-Queries, ändert sich ein **Kern-Werttyp** (`model::PlanSegment`), den auch PDF-,
  PNG-Export und die Golden-Files konsumieren. Das ist ein **anderer Risiko-Grad** als der Rest des
  Strangs — und der Grund, warum diese Frage vor 057/058 gehört und nicht in sie hinein.
  **Zusätzlich zieht sie einen fremden Trigger** (L3): wächst der Werttyp, der im
  `DerivedGeometry`-Bündel reist, ist der
  [ADR-0020](../../adr/0020-driven-adapter-serialisieren-kern-liefert-geometrie.md)-Re-Eval
  „Bündel zu breit" gezogen — die ADR muss das **benennen**, nicht später entdecken.

## 8. Der Strang danach (Sequenz, nicht Umfang dieses Slice)

| Slice | Was | Trigger-Beitrag |
|---|---|---|
| **057** | **Die 2D-Lese-Naht** (F11) — was der Canvas lesen muss, um eine **Wand** zu treffen und zu benennen. **Vorgelagert, ohne UI-Anteil**; die 2D-Export-Orakel und die Golden-Files sind das Netz | Voraussetzung von **059**; **058 kommt ohne sie aus** (`addWall(StoreyId, Segment)` braucht keine Bauteil-Identität) — die Vorlagerung ist eine Wahl, s. u. |
| **058** | **Wand zeichnen** im Canvas: Werkzeug-Modus (F1) + Segment-Zahl je Geste (F2) + Zug ⇒ `addWall` über die neue `ui/command/`-Senke (F7) + Refresh (F6) + Fang-Geltung (F9) + Fehler-Barriere (F10) + Gesten-Abbruch/Rückweg (F12) + Zeichenbereich/[`E-GEO-001`](../../../../spec/spezifikation.md#4-fehler-codes-und-logging-felder) (F8) + der **in F14 entschiedenen** Nachweis-Naht | „**zeichenbar**" |
| **059** | **Wand auswählen und ändern**: Picking (F3) + Parameter-Bedienung (F4/F13) + Rückmeldung in allen drei Fällen (F5) + der **in F14 entschiedenen** Nachweis-Naht | „**parametrisch änderbar**" |

### Die Sequenz-Entscheidung ist gefallen, die ADR-Entscheidung nicht (2026-07-28)

Der zweite Review stellte den Schnitt 057/058 in Frage, weil F11 ihn verändern kann. **Der
Projektinhaber hat die Sequenz vorab entschieden: es gibt einen vorgelagerten Lese-Naht-Slice.**

**Was am Artefakt belegt ist und was nicht** (präzisiert nach Lauf-3-MEDIUM-3 — die Vorfassung
überdehnte hier): belegt ist, dass die Lese-Seite **Arbeit braucht** — `PlanSegment` trägt keine
Identität, F3 hätte anonyme Segmente zu treffen und F4 keine `WallId` zu setzen. **Nicht** belegt ist,
dass diese Arbeit einen **eigenen vorgelagerten** Slice braucht: **058 kommt ohne sie aus**
(`addWall(StoreyId, Segment)` verlangt keine Bauteil-Identität), und ob die Naht einen Kern-Werttyp
berührt, entscheidet erst F11.

**Die Vorlagerung ist deshalb eine Projektinhaber-WAHL, keine Ableitung** — getroffen aus
Risiko-Gründen (s. u.), nicht aus Zwang. Sie greift der ADR nicht vor: das *Wie* von F11 bleibt offen,
und fällt F11 auf eine Antwort **ohne** Kern-Werttyp-Berührung, ist der Zuschnitt von 057 neu zu
bewerten — **das ist dann ein Befund des ADR-Text-Reviews, kein Fehler dieses Plans**.

**Warum vorgelagert und nicht in 058/059 gefaltet** (Präzedenz [`slice-042b`](../done/slice-042b-export-refactor-2d-projektion.md),
die 2D-Projektions-Hebung lief **vor** dem Canvas): berührt die Antwort einen **Kern-Werttyp**, hängen
PDF-/PNG-Export und die **Golden-Files** mit dran (R7). Das ist ein anderer Risiko-Grad als
UI-Interaktion und braucht die bestehenden Export-Orakel als eigenes Netz — genau das kann ein Slice,
der gleichzeitig ein Werkzeug baut, nicht mehr sauber zeigen.

**Mit 059 ist der welle-6-Abschluss-Trigger erfüllt — und die Welle ist dann zu schließen, nicht
weiterzufüllen** (welle-5-Closure §5-1; der Wellen-Block der Roadmap trägt die Regel).

## 9. Closure-Trigger

- Die neue ADR ist `Accepted` (Text-Review durch, 0 HIGH) + Index/Folgepflicht nachgezogen; AK-Block,
  Teilumfang-Klauseln und Spec-§1-Mapping geschrieben; Lastenheft-Header == neue Historie-Zeile;
  057–059 liegen als Plan vor (oder eine Deferral ist notiert); `make gates` grün; **kein** Code-Diff
  am `git diff --stat` belegt; Closure-Notiz.

## 10. Sub-Area-Modus-Begründung

### Sub-Area: Spec-Schreibung + Planning-Lifecycle

- **Modus:** GF; **Dichte:** mittel-groß — eine ADR mit vierzehn Entscheidungen ist der Aufwand, nicht die
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

**Startbar nach Lauf 1:** nein. Der Auftrag an den zweiten Lauf lautete: **nicht die zehn Fragen
prüfen, sondern die elfte suchen.** Er hat drei gefunden.

## 11a. [MR-006](../../../../harness/conventions.md#mr-006--unabhängiges-plan-review-vor-implementierungs-start)-Einarbeitung (zweiter Lauf, 2026-07-28)

Report: [`2026-07-28-slice-056-plan-2.md`](../../../reviews/2026-07-28-slice-056-plan-2.md) —
**1 HIGH / 4 MEDIUM / 4 LOW / 4 INFO + 18 Negativbefund-Zeilen, „nicht startbar"**. Unabhängiger
Reviewer ≠ Plan-Autor ≠ Reviewer des ersten Laufs.

**Sein Urteil über die erste Einarbeitung: echt.** Alle sechzehn Zeilen am Artefakt aufgelöst, keine
zeigt auf später; die §4-Orakel-Tabelle nennt zeilenweise den **realen** Sensor (gegen `.d-check.yml`
und [AGENTS §3](../../../../AGENTS.md) einzeln geprüft, **kein Phantom-Gate mehr**); F2 trägt sachlich
als Interaktions-Frage samt DoD-Klausel.

| # | Behandlung |
|---|---|
| **HIGH-1** (**die elfte Frage: die 2D-Lese-Naht.** `model::PlanSegment` trägt `{x1,y1,x2,y2}` — **keine `WallId`, keine Art**; `projectPlan` mischt Wand-Achsen und Hilfslinien, `paintEvent` zeichnet beide gleich. F3 hätte nur **anonyme** Segmente zu treffen, F4 braucht eine `WallId`, und die WAL-001-Happy-AK wäre nicht als **Wand** beobachtbar. **[ADR-0019](../../adr/0019-drw-2d-canvas.md) benennt die Folge in ihrem Re-Eval-Block wörtlich** — »2D-Lese-Naht (`PlanViewPort`) um **Bauteil-/Treffer-Queries erweitern**« —, und der Plan zitierte dieselbe ADR zweimal, diesen Satz nicht) | **Nachgelesen und bestätigt** ([ADR-0019](../../adr/0019-drw-2d-canvas.md), Re-Eval-Block). **Als F11 aufgenommen** + zwei DoD-Zeilen (`architecture.md` §1.1; die Kern-Werttyp-Folge im ADR-Text) + **R7**: F11 ist die einzige Frage des Strangs, die einen **Kern-Werttyp** und damit PDF-/PNG-Export und die Golden-Files berührt. §8 sagt jetzt, dass der Schnitt 057/058 **nach** F11 zu prüfen ist — plausibel ein dritter, vorgelagerter Slice (Präzedenz slice-042b). |
| **MEDIUM-1** (F9 hat **zwei** Entscheidungsrichtungen, die DoD nur eine: `snappedModelPos` sitzt **unbedingt** in Press/Release, also in der **Geste** — ein Wand-Werkzeug fängt per Default, und eine Entscheidung **gegen** den Wand-Fang verlangt Unterdrückungs-Arbeit, die niemand nennt) | DoD-Zeile auf **beide** Richtungen erweitert, samt der Folge, dass die Lastenheft-Zeile sonst **still falsch** stünde. **Kein drittes Ergebnis.** |
| **MEDIUM-2** (kein Rückweg aus einer gezeichneten Wand: `EditStructurePort` hat **kein `removeWall`**, der Canvas keinen Gesten-Abbruch, Undo ist unbedient — §3 benannte nur die Undo-Hälfte) | **Als F12 aufgenommen.** §3 sagt jetzt ausdrücklich, dass Undo nur **eine von drei** möglichen Antworten ist und der Rückweg deshalb **nicht** mit-abgegrenzt war. |
| **MEDIUM-3** ([`LH-FA-WAL-007`](../../../../spec/lastenheft.md#lh-fa-wal-007--wandtyp-wählen) (Wandtyp) und `setWallMaterial` sind weder in F4 geführt noch in §3 abgegrenzt — während §3 die Nachbarn WAL-004/005 abgrenzt und der Trigger „**parametrisch** änderbar" lautet) | **Als F13 aufgenommen** — welche Parameter „parametrisch änderbar" überhaupt meint, ist eine Entscheidung, keine Selbstverständlichkeit. |
| **MEDIUM-4** (`spezifikation.md` **§6** fehlte im Nachzug: die Vertragszeile „2D-Zeichenfläche (DRW-Canvas)" trägt den Konjunkt „Selbst-Refresh **ohne `op`**", den der Wand-Weg unvollständig macht) | **Eigene DoD-Zeile.** Am Artefakt bestätigt (§6-Tabelle). |

**Ein Lauf-1-Befund ist vom zweiten Lauf widerlegt worden:** die dortige Quellenzuordnung
„[MR-023](../../../../harness/conventions.md#mr-023--spec-straten-sind-prozess-zeit-rein-d-check-matrix-klasse-temporal)
verlangt eine Teilumfang-Klausel" trifft nicht zu — [MR-023](../../../../harness/conventions.md#mr-023--spec-straten-sind-prozess-zeit-rein-d-check-matrix-klasse-temporal) regelt die **Formulierung** der Straten,
nicht die Pflicht zur Klausel. **Der Plan hatte den Fehler nicht geerbt** (die DoD-Zeile steht ohne
diese Begründung da).

**Ausdrücklich geprüft und NICHT als Befund geführt** (die Negativbefunde des Laufs — sie sind der
Teil, der die Suchtiefe belegt): Geschoss-Bezug · Ebenen-Bezug · Sichtbarkeit/Filterung im Canvas ·
gleichzeitige 3D-Sicht · Raum-Neuerkennung · Persistenz · Mehrfach-Auswahl · Projekt-Wechsel.
**Insbesondere der `ProjectSessionPort` ist kein Befund:** `isDirty` ist ein **Wertvergleich** über
`model::Building`, jede Wand-Mutation erfasst die 052a-Rückfrage damit automatisch.

**Startbar nach Lauf 2:** nein. Auftrag an den dritten Lauf (R6): die **vierzehnte** Frage suchen.

## 11b. [MR-006](../../../../harness/conventions.md#mr-006--unabhängiges-plan-review-vor-implementierungs-start)-Einarbeitung (dritter Lauf, 2026-07-28)

Report: [`2026-07-28-slice-056-plan-3.md`](../../../reviews/2026-07-28-slice-056-plan-3.md) —
**1 HIGH / 6 MEDIUM / 4 LOW / 4 INFO, „nicht startbar"**. Unabhängiger Reviewer ≠ Plan-Autor ≠ beide
Vor-Reviewer.

**Er hat sie gefunden.**

| # | Behandlung |
|---|---|
| **HIGH-1** (**die vierzehnte Frage: die Nachweis-Naht für reinen UI-Zustand.** Auswahl, Werkzeug-Modus und Hinweis haben **kein Korrelat außerhalb des Widgets** — anders als jede bisherige Interaktions-AK dieses Repos, die an einem **Modell**-Surrogat hängt. **Zwei Accepted-ADRs benennen genau das wörtlich als offen:** [ADR-0009](../../adr/0009-gui-framework-qt6.md) (f) und [ADR-0019](../../adr/0019-drw-2d-canvas.md) E7 halten `tests/e2e/` leer, „bis eine AK entsteht, die nur echt end-to-end prüfbar ist (**z. B. Selektion / mehrschrittige Interaktion**)" — und dieser Strang liefert **beide** dort genannten Beispiele; `tests/e2e/` enthält bis heute nur `.gitkeep`. §8 buchte für 058/059 bereits „Headless-AK", also die **Antwort vor der Frage**) | **Beide Zitate selbst nachgelesen und bestätigt. Als F14 aufgenommen** + neue DoD-Zeile: die ADR **trägt eine Fitness Function** (Lauf-3-MEDIUM-1). **Das ist der Befund, für den die drei Läufe gelaufen sind:** eine ADR, die diese Frage nicht stellt, hätte den Strang auf ein Prüf-Muster festgelegt, das für seine eigenen Zusagen nicht reicht — und wäre danach immutabel gewesen. |
| **MEDIUM-2** (der Lauf-2-HIGH war nur zu zwei Dritteln eingearbeitet: die DoD-Zeile für `spezifikation.md` §1 zählte unverändert dieselben sechs Mapping-Punkte **ohne** die Lese-Naht) | Nachgezogen — §1-Mapping führt jetzt **F11 und F14**. **Ein sauber gefundener Nachlässigkeits-Rest: der Befund war benannt, die eine von drei Stellen blieb stehen.** |
| **MEDIUM-3** (die Sequenz-Entscheidung überdehnt: widerlegt ist nur „die Naht braucht keine Arbeit", **nicht** „die Arbeit braucht einen eigenen vorgelagerten Slice" — 058 kommt mit `addWall(StoreyId, Segment)` ohne Bauteil-Identität aus) | **Zurückgenommen und ehrlich neu geschrieben** (§8): die Vorlagerung ist eine **Projektinhaber-Wahl aus Risiko-Gründen**, keine Ableitung. Fällt F11 auf eine Antwort ohne Kern-Werttyp-Berührung, ist 057 neu zu bewerten — **als Befund des ADR-Text-Reviews, nicht als Fehler dieses Plans**. |
| **MEDIUM-4** (§11a behandelte nur 5 der 13 Lauf-2-Befunde; die vier LOW standen unverändert im Text und waren nirgends als verworfen vermerkt) | **Erste Behandlung war eine Behauptung ohne Vollzug** („die textwirksamen sind nachgezogen" — es war keiner; Lauf-4-MEDIUM-3 hat es gemessen). **Jetzt wirklich vollzogen:** die vier Lauf-2-LOW sind unten einzeln geführt. **Der Befund trifft eine Lücke der Einarbeitungs-Disziplin: eine Behandlungstabelle, die nur die schweren Befunde führt, sieht aus wie Vollständigkeit — und ein Satz, der die Nachbesserung behauptet, macht sie unsichtbar.** |

**Die vier LOW aus Lauf 2 — nachgeholt statt behauptet** (Lauf-4-MEDIUM-3): (a) die §2-Überschrift
zählte F7 als „Frage, die der Spec-Text nicht entscheidet", obwohl F7 nur **feststellt** — steht seit
Lauf 1 so in F7 selbst, die Überschrift bleibt bewusst, weil sie den Abschnitt benennt, nicht jede
Zeile qualifiziert. (b) „Selektion als UI-Zustand" in der DoD nahm F3 vorweg — **korrigiert**
(Lauf-4-MEDIUM-5). (c) „unterschreitet den Happy Path" stand dreifach — die Formulierung bleibt, sie
ist an der AK belegt. (d) F8 hatte in §8 keinen Träger — **korrigiert**, F8 steht jetzt in der
057-Zeile.
| **MEDIUM-5** ([`LH-FA-UI-001`](../../../../spec/lastenheft.md#modul-benutzeroberfläche-ui) Docking-Fenster ist nicht abgegrenzt, obwohl F4 „Panel" führt — identisch zur bereits akzeptierten UI-005-Konstellation) | In §3 abgegrenzt, zusammen mit UI-005. |
| **MEDIUM-6** (Teilerfolg einer mehrpunktigen Geste: Transaktion = Port-Aufruf, Benutzer-Einheit = Geste ⇒ halber Wandzug ohne Rückweg; ausdrücklich **nicht** als 14. Frage gezählt) | In §3 aufgenommen — **bedingt auf die F2-Entscheidung** und dort demselben ADR-Abschnitt zugeordnet wie F2/F12, statt als eigene Frage zu zählen. Die Zurückhaltung des Reviewers ist übernommen, nicht überschrieben. |

**Suchbreite des Laufs** (der Teil, der ein „keine weitere Frage" später belastbar machen wird): alle
zehn Driving-Ports · alle sieben Entscheidungen, vier Re-Eval-Trigger und die Fitness Function von
[ADR-0019](../../adr/0019-drw-2d-canvas.md) · [ADR-0009](../../adr/0009-gui-framework-qt6.md) (a)–(f) ·
beide DRW-§1-Blöcke absatzweise · das ganze WAL-Modul samt ROM/D3/DRW/UI/QA · die §2.2-/§7-Offene-
Punkte-Listen · der komplette Interaktions-Code-Pfad. **Sieben weitere Kandidaten geprüft und mit
Grund verworfen** (Default-Parameter, Raum-Sichtbarkeit, Ebenen-Sperre, Re-Entranz, `ModelReplaced`,
ACC-Berührung, [`E-GEO-002`](../../../../spec/spezifikation.md#4-fehler-codes-und-logging-felder)).

**Startbar nach Lauf 3:** nein — **aus EINEM Grund: Lauf 3 hat einen HIGH gefunden.** [MR-006](../../../../harness/conventions.md#mr-006--unabhängiges-plan-review-vor-implementierungs-start)
blockiert den Start, und die Auflösung eines HIGH wird in diesem Repo von einem **unabhängigen** Lauf
geprüft — dieses Muster hat bei 053, 052a und 052b jedes Mal etwas gefangen, und bei
[`slice-055`](../done/slice-055-fang-anzeige.md) fand Lauf 2, dass der in Lauf 1 verlangte Sensor **nicht
diskriminierte**.

**Sein Auftrag ist eng und terminierend:** ist F14 samt der sechs MEDIUM **echt** eingearbeitet, und
erzeugt die Einarbeitung neue Widersprüche? Dazu ein letzter Sweep. **Abbruchregel steht vorab
(R6): 0 HIGH ⇒ startbar**, unabhängig von der Zahl der MEDIUM.

**Nicht mehr Auftrag war „die fünfzehnte Frage suchen, bis keine mehr kommt".** Diese Last trägt das
**ADR-Text-Review** (R6) — dort ist sie schärfer und billiger, und der `Proposed`-Status hält die
Entscheidungen bis dahin änderbar. **Kein Prozess-Selbstzweck:** F14 zeigt, was eine übersehene Frage
kostet — sie hätte den ganzen Strang auf ein Prüf-Muster festgelegt, das seine eigenen Zusagen nicht
trägt. Genau deshalb gehört die Suche dorthin, wo sie am meisten sieht.

## 11c. [MR-006](../../../../harness/conventions.md#mr-006--unabhängiges-plan-review-vor-implementierungs-start)-Einarbeitung (vierter Lauf, 2026-07-28) — **0 HIGH**

Report: [`2026-07-28-slice-056-plan-4.md`](../../../reviews/2026-07-28-slice-056-plan-4.md) —
**0 HIGH / 6 MEDIUM / 5 LOW / 4 INFO + 13 Negativbefund-Zeilen, Verdikt „startbar"**. Unabhängiger
Reviewer ≠ Plan-Autor ≠ die drei Vor-Reviewer.

**Der Lauf sagt sein Nichts ausdrücklich:** keine fehlende Frage geführt · keine Quellen-Behauptung,
die ihrer Quelle widerspricht (F14s **beide** ADR-Zitate und das Fitness-Function-Zitat sind wörtlich
korrekt) · kein behaupteter Sensor ohne Entsprechung · keine Hard Rule berührt.

**Sein Urteil über die Lauf-3-Einarbeitung: im Kern echt, am Rand nicht.** F14 ist substanziell
aufgenommen (nicht umformuliert, nicht delegiert), MEDIUM-1/2/5 sauber aufgelöst — **aber das von
Lauf 3 diagnostizierte Muster hat sich zweimal wiederholt.** Das ist der eigentliche Ertrag dieses
Laufs.

| # | Behandlung |
|---|---|
| **MEDIUM-1** (die §8-Tabellenzelle „057 = Voraussetzung **beider** Trigger-Hälften" widerspricht der drei Zeilen darunter neu geschriebenen Prosa „**058 kommt ohne sie aus**" — Lauf-3-MEDIUM-3 zu zwei Dritteln eingearbeitet) | Tabellenzelle nachgezogen. **Zweite Wiederholung derselben Bauart: Prosa neu, Tabelle alt.** |
| **MEDIUM-2** (**die neue R6 widerruft sich selbst**: drei Zeilen nach der Rücknahme und der Abbruchregel stand der unveränderte Rest der Vorfassung — „jeder weitere Lauf … Suche nach der nächsten fehlenden Frage … erst ein leerer Lauf ist ein Argument für Vollständigkeit"; per `git diff` als Rest belegt) | **Entfernt.** Der Reviewer hat die Rücknahme nicht am Wortlaut geglaubt, sondern am Diff gemessen — und gefunden, dass ich den Satz ersetzt und den Nachsatz stehen gelassen hatte. **Eine Regel, die zwei Absätze später ihr Gegenteil sagt, ist keine.** |
| **MEDIUM-3** (§11b behauptete für Lauf-3-MEDIUM-4 „die textwirksamen sind nachgezogen" — am Artefakt war **keiner** der vier Lauf-2-LOW verändert; §11b führte selbst wieder nur die schweren Befunde) | **Behauptung zurückgenommen und vollzogen:** die vier Lauf-2-LOW stehen jetzt einzeln in §11b, zwei davon führten zu echten Korrekturen. **Ein Satz, der eine Nachbesserung behauptet, macht sie unsichtbar** — genau deshalb misst dieser Prozess Einarbeitungen, statt sie zu lesen. |
| **MEDIUM-4** (§8 buchte weiterhin „+ Headless-AK" für 058/059 — **genau der Vorgriff, den F14 als Beleg zitiert**; F14 und F8 hatten in keiner §8-Zeile einen Träger) | „Headless-AK" ist aus beiden §8-Zeilen entfernt; sie tragen jetzt die **in F14 entschiedene** Nachweis-Naht, und F8 steht in der 058-Zeile. **Ich hatte die Frage aufgenommen und die vorweggenommene Antwort daneben stehen lassen.** |
| **MEDIUM-5** (die DoD-§1-Zeile schrieb „**Selektion als UI-Zustand**" fest — nimmt F3 vorweg und zugleich die Prämisse von F14; drittes Vorkommen dieser Klasse) | Formulierung entfernt; die DoD nennt jetzt die **Themen**, nicht die Antworten, und sagt das ausdrücklich. |
| **MEDIUM-6** (der „Teilerfolg einer mehrpunktigen Geste" stand unter „Bewusst NICHT Teil" **und** wurde im selben Satz dem ADR-Abschnitt F2/F12 zugewiesen — ohne DoD-Anker, während die DoD den ADR-Inhalt auf „die vierzehn Fragen aus §2" abschließt) | **Verortet statt geparkt:** jetzt **Konjunkt von F2** (Teil der Frage, nicht daneben) + eigene DoD-Zeile. §3 vermerkt die Verschiebung. |

**Was der Lauf ausdrücklich bestätigt:** die **R6-Rücknahme trägt in der Sache** — sie definiert
keinen Prüf-Bedarf weg, sondern verschiebt ihn dorthin, wo ausformulierte Entscheidungen stehen;
gebunden in DoD und §9, präzedenziert durch [ADR-0019](../../adr/0019-drw-2d-canvas.md) (nach
0-HIGH-Text-Review `Accepted`), und `Proposed` ist ein echtes Netz. Ebenso trägt die **neue
Sequenz-Begründung**: Beleg („die Lese-Seite braucht Arbeit") und Wahl („eigener vorgelagerter
Slice") sind sauber getrennt und am Code gegengeprüft.

**Startbar: JA.** 0 HIGH — die in R6 **vorab** festgelegte Abbruchregel greift, und sie ist nicht im
Nachhinein passend gewählt worden. **Vier Läufe, vier Verdikte, jedes Mal ein echter Fund.** Die sechs
MEDIUM sind hier eingearbeitet; **sie gehören vor das Schreiben des ADR-Textes**, weil vier von ihnen
ADR-Antworten vorwegnahmen oder unverortet ließen — und mit dem `Accepted` immutabel geworden wären.


## 13. Lese-Runde vor dem Schreiben (2026-07-28) — Deckungs-Nachweis

*(Die Disziplin, deren Fehlen in dieser Sitzung **F11** und **F14** gekostet hat: beide standen
wörtlich in Konsequenzen-/Re-Eval-Blöcken von ADRs, die der Plan zitierte. Regel-Kandidat in
`status.md`, Zähler 2. Hier angewandt **bevor** eine Zeile ADR-Text steht.)*

**Gelesen (vollständig, nicht überflogen):** die Blöcke *Konsequenzen*, *Fitness Function*,
*Re-Evaluierungs-Trigger* und die zugehörigen Folgepflicht-Zeilen im
[ADR-Index](../../adr/README.md) von [ADR-0009](../../adr/0009-gui-framework-qt6.md),
[ADR-0018](../../adr/0018-drw-2d-zeichen-daten.md), [ADR-0019](../../adr/0019-drw-2d-canvas.md) und
[ADR-0020](../../adr/0020-driven-adapter-serialisieren-kern-liefert-geometrie.md).

**Ergebnis: keine fünfzehnte Frage** — aber **drei Verpflichtungen**, die der Plan nicht trug. Alle
drei stammen aus Konsequenzen-Blöcken, keine aus einem Entscheidungs-Block.

| # | Fundstelle | Was daraus folgt |
|---|---|---|
| **L1** | [ADR-0018](../../adr/0018-drw-2d-zeichen-daten.md) §Konsequenzen erklärt [MR-009](../../../../harness/conventions.md#mr-009--geometrielastiges-code-review-vor-welle-closure) ausdrücklich für **n/a** — mit Begründung: „Hilfslinie = 2 Punkte, **keine neue Solid-Geometrie**" | **Für Wände gilt exakt das Gegenteil.** `addWall` baut ein Solid, rechnet **Nachbar-Ecken** neu ([`LH-FA-WAL-006`](../../../../spec/lastenheft.md#lh-fa-wal-006--wand-verbinden)) und stößt die **Raum-Neuerkennung** an. [MR-009](../../../../harness/conventions.md#mr-009--geometrielastiges-code-review-vor-welle-closure) ist für den Strang **einschlägig** — und die neue ADR muss das **feststellen**, wie 0018 und 0019 es für ihren Umfang tun. Fehlt die Aussage, erbt 058/059 stillschweigend die 0018-Verneinung, die für sie falsch ist. → **neue DoD-Zeile** |
| **L2** | [ADR-0018](../../adr/0018-drw-2d-zeichen-daten.md) §Re-Eval: „**Layer-Zuordnung für Bauteile** (Wände/Räume auf benutzer-Layern) → Aktivierung der polymorphen `entity_layers`-Zuordnung ([ADR-0006](../../adr/0006-relationales-schema-design.md)-#6), **Layer wird cross-cutting**" | Der Canvas hält beim Zeichnen eine **aktive Zeichen-Ebene** (der Hilfslinien-Sink trägt eine `LayerId`). Sobald dort eine **Wand** entsteht, ist die Frage gestellt, ob sie diese Ebene erbt — Wände tragen heute **keine** `layer_id`. Die Antwort „nein, Wände bleiben ebenenlos" ist vertretbar, **muss aber dastehen**: sie triggert sonst unbemerkt eine Schema-Erweiterung, die diese ADR nicht beschlossen hat. → **§3-Abgrenzung** |
| **L3** | [ADR-0020](../../adr/0020-driven-adapter-serialisieren-kern-liefert-geometrie.md) §Re-Eval: „**Das `DerivedGeometry`-Bündel wird zu breit** (viele optionale Felder je neuem Format) → Varianten-/Bedarfs-Modell neu bewerten" | **F11** kann genau das auslösen: erweitert die Antwort `PlanView`/`PlanSegment` um Bauteil-Identität, wächst der Werttyp, der **im Bündel reist**. Der [ADR-0020](../../adr/0020-driven-adapter-serialisieren-kern-liefert-geometrie.md)-Trigger ist dann **gezogen** — das gehört als benannte Folge in F11/R7, nicht als Überraschung in 057. → **F11 und R7 ergänzt** |

**Ausdrücklich geprüft und NICHT als Verpflichtung geführt:**
[ADR-0009](../../adr/0009-gui-framework-qt6.md) §Re-Eval „Selektion/Picking **im Viewport** → AIS/V3d
neu bewerten, **als Supersedes-ADR**" — das betrifft die **3D**-Selektion und ist in §3 bereits
abgegrenzt; die 2D-Selektion (F3) läuft im eigenen `view/`-Widget und berührt weder AIS noch V3d ·
„Mehr-Fenster/Nebenläufigkeit" ([`LH-FA-UI-004`](../../../../spec/lastenheft.md#modul-benutzeroberfläche-ui))
— nicht berührt · „Render-/Latenz-Budget wird Anforderung" — die „**sofort**"-Zusage aus
[`LH-FA-WAL-002`](../../../../spec/lastenheft.md#lh-fa-wal-002--wandstärke-definieren)/
[`LH-FA-D3-002`](../../../../spec/lastenheft.md#lh-fa-d3-002--echtzeitaktualisierung) ist **kein**
Budget: D3-002 führt „Latenz-/Performance-Budget" ausdrücklich als **Out-of-Scope** ·
[ADR-0018](../../adr/0018-drw-2d-zeichen-daten.md) §Re-Eval „Benutzer-Layer → eigener DXF-Layer-Name"
(setzt L2 voraus, das verneint wird) und „Bemaßung/Schraffur/Gruppen" (§3 abgegrenzt) ·
[ADR-0020](../../adr/0020-driven-adapter-serialisieren-kern-liefert-geometrie.md) §Re-Eval
„Import-Adapter braucht Kern-Geometrie" — der Strang berührt keinen Import-Pfad.

**Was die Runde über sich selbst sagt:** sie hat **keine** Architektur-Entscheidung gefunden, die
gefehlt hätte — die vier Review-Läufe haben die Fragenliste offenbar erschöpft. Gefunden hat sie
**Verpflichtungen**: eine Prozess-Pflicht (L1), eine Abgrenzung (L2) und eine Folge-Kette (L3). Das
ist genau das Material, das in Konsequenzen-Blöcken steht und in Entscheidungs-Blöcken nicht.

## 12. Closure-Notiz

**Vollzogen 2026-07-28.** `make gates` **EXIT=0** (docs-check **0 Befunde / 277 Dateien** · a-check 0 ·
arch-check ok · **366/366** Tests · Coverage 92,1 %), `make schema-check` ok. **Kein Code-Diff** —
`git diff --stat` über `src/`, `tests/`, `data-model.yaml` und `schema.sql` ist **leer** (§4-6; nicht
über `make schema-check` behauptet, das ist ein Drift-Wächter).

**[ADR-0021](../../adr/0021-wand-im-2d-canvas.md) ist `Accepted`** — mit **siebzehn** Entscheidungen,
nicht vierzehn.

### Die Bilanz der Prüfungen

| Lauf | Ergebnis | Neue Fragen |
|---|---|---|
| Plan-Review 1 | 2 HIGH / 6 MED / 5 LOW / 3 INFO | F9 · F10 |
| Plan-Review 2 | 1 HIGH / 4 MED / 4 LOW / 4 INFO | F11 · F12 · F13 |
| Plan-Review 3 | 1 HIGH / 6 MED / 4 LOW / 4 INFO | F14 |
| Plan-Review 4 | **0 HIGH** / 6 MED / 5 LOW / 4 INFO | — (Abbruchregel greift) |
| **Lese-Runde** (kein Review) | 3 Verpflichtungen | — |
| **ADR-Text-Review** | 3 HIGH / 7 MED / 5 LOW / 6 INFO | **E15 · E16 · E17** |

**Acht → siebzehn.** Neun Entscheidungen kamen aus Prüfungen, keine vom Autor allein.

### Die drei Funde des Text-Reviews — und warum sie dort auffielen

1. **Kein Port lieferte die Wand-Parameter**, die der Eigenschaften-Bereich anzeigen sollte.
2. **Die Treffer-Prüfung hätte deterministisch die unsichtbare Wand getroffen** — im mitgelieferten
   Demo-Modell liegen vier Außenwände in zwei Geschossen deckungsgleich.
3. **Eine Auswahl hätte eine Modell-Ersetzung überlebt** und danach still eine **andere existierende**
   Wand mutiert; die Fehler-Barriere schweigt bei einer gültigen fremden Identität.

**Alle drei fehlten, weil eine *andere* Entscheidung die Frage scheinbar schon beantwortet hatte** —
„die Id genügt", „wie der Fang", „die Senke fängt", je nur für den halben Fall. **Das ist der Beleg,
dass die Verlagerung der Vollständigkeitslast vom Plan auf den ADR-Text richtig war:** eine Liste von
Frage-Überschriften kann diese Bauart nicht zeigen, ein ausformulierter Entscheidungs-Text schon.

### Zwei Befunde außerhalb der Prüf-Läufe

- **Die Lese-Runde fand keine fehlende Entscheidung, aber drei Verpflichtungen** — die
  [MR-009](../../../../harness/conventions.md#mr-009--geometrielastiges-code-review-vor-welle-closure)-Einschlägigkeit
  (die Vorgänger-ADR hatte sie für Hilfslinien **verneint**), die Bauteil-Ebenen-Abgrenzung und eine
  Beobachtungspflicht. **Alle drei standen in Konsequenzen-Blöcken, keine in einem
  Entscheidungs-Block.**
- **Eine Lastenheft-Aussage war seit der Fang-Lieferung unwahr:** die Hilfslinien-Teilumfang-Klausel
  behauptete „in dieser Ausbaustufe wird **frei** gezeichnet — Fangen … bleibt offen". **Kein Review
  hat das gefunden**; es fiel beim Nachziehen auf, weil dieselbe Klausel für die Selektion angefasst
  werden musste. **Ein Beleg dafür, dass Nachzieh-Arbeit selbst ein Sensor ist.**

### Was der Slice bewusst nicht liefert

**Keine Zeile Produktions-Code.** Der Trigger der Welle ist **nicht** erfüllt — er wird es mit
[`slice-059a`](../done/slice-059a-wand-auswaehlen.md). Die drei Folge-Pläne liegen als
Skelett in `open/` ([MR-020](../../../../harness/conventions.md) §3); jeder trägt die Stellen, an
denen er still falsch werden kann.

### Reihenfolge des Strangs

**057** (Lese-Naht, vorgelagert, ohne UI-Anteil) · **058** (zeichnen, **unabhängig** von 057) ·
**059** (auswählen und ändern, **setzt 057 voraus**). Vor der Welle-Closure steht das
[MR-009](../../../../harness/conventions.md#mr-009--geometrielastiges-code-review-vor-welle-closure)-Code-Review
des ganzen Bauteil-Strangs.
