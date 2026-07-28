---
id: slice-057
titel: Die 2D-Lese-Naht bekommt Bauteil-Identität und eine schmale Parameter-Abfrage ([ADR-0021](../../adr/0021-wand-im-2d-canvas.md) E11/E15)
status: open
welle: welle-6-interaktiv-planen
lastenheft_refs: [[LH-FA-WAL-001](../../../../spec/lastenheft.md#lh-fa-wal-001--wand-zeichnen), [LH-FA-WAL-002](../../../../spec/lastenheft.md#lh-fa-wal-002--wandstärke-definieren), [LH-FA-WAL-003](../../../../spec/lastenheft.md#lh-fa-wal-003--wandhöhe-definieren), [LH-FA-DRW-005](../../../../spec/lastenheft.md#lh-fa-drw-005)]
adr_refs: [[ADR-0001](../../adr/0001-hexagonale-architektur.md), [ADR-0019](../../adr/0019-drw-2d-canvas.md), [ADR-0020](../../adr/0020-driven-adapter-serialisieren-kern-liefert-geometrie.md), [ADR-0021](../../adr/0021-wand-im-2d-canvas.md)]
---

# Slice 057: Bauteil-Identität in der 2D-Lese-Naht

**Status:** open — **Detail-Schnitt vollzogen** (2026-07-28; die Skelett-Fassung trug nur
Scope-Reservierung + ADR-Bezug, [MR-020](../../../../harness/conventions.md) §3). **Eigenes
[MR-006](../../../../harness/conventions.md#mr-006--unabhängiges-plan-review-vor-implementierungs-start)
vor dem Start.**

**Welle:** welle-6-interaktiv-planen. **Vorgelagert, ohne UI-Anteil** — Voraussetzung für
[`slice-059`](slice-059-wand-auswaehlen-und-aendern.md);
[`slice-058`](slice-058-wand-zeichnen-im-canvas.md) kommt **ohne** ihn aus. **Nicht
trigger-bindend**, aber trigger-**ermöglichend**.

**Autor:** Dietmar Burkard (AI-Harness-Lauf). **Datum:** 2026-07-28.

## Auslöser

[ADR-0021](../../adr/0021-wand-im-2d-canvas.md) **Entscheidung 11 und 15**, beide vom Text-Review
verschärft. Die 2D-Lese-Naht liefert heute **anonyme** Segmente — vier Koordinaten, keine Herkunft;
`projectPlan` mischt Wand-Achsen und sichtbare Hilfslinien in **eine** Liste, und `paintEvent`
zeichnet beide gleich. Damit hätte eine Treffer-Prüfung nichts zu benennen und eine
Parameter-Änderung keine Identität zu adressieren.

**Der Slice liefert, was die ADR entschieden hat — nicht mehr.** Keine Zeile UI.

## 1. Ziel

1. **Jedes Segment der Projektion trägt seine Herkunft** — Art (Wand-Achse / Hilfslinie) und die
   **echte** Identität des Bauteils bzw. der Zeichen-Entität.
2. **Die Lese-Naht beantwortet eine zweite, schmale Frage:** zu einer Wand-Identität die
   **änderbaren** Parameter (Stärke, Höhe). Nicht mehr — der Umfang ist durch
   [ADR-0021](../../adr/0021-wand-im-2d-canvas.md) Entscheidung 13 begrenzt.
3. **Beides additiv** — kein Encoder, kein Export-Artefakt, kein Schema ändert sich.

## 2. Zwei Entwurfs-Entscheidungen, die nicht selbstverständlich sind

### 2.1 Die Herkunft ist **optional** — damit „vergessen" nicht „falsch" heißt

`model::PlanSegment` ist ein **Aggregat**, das an mehreren Stellen mit
`{x1, y1, x2, y2}`-Klammerform gebaut wird (Projektion **und** Bestands-Tests). Ergänzt man die
Herkunft als **Wert** (Art + Id), initialisiert die Klammerform sie **still** auf den Nullwert der
Aufzählung — ein vergessenes Feld wäre dann nicht leer, sondern **falsch beschriftet**: jedes
Segment gälte als Wand-Achse.

**Deshalb: `std::optional`.** Ein vergessenes Feld ist dann **leer**, und Leere ist prüfbar. Die
Zusage lautet: **die Projektion liefert nie ein Segment ohne Herkunft** — als eigene Orakel-Zeile
(§4-2), nicht als Hoffnung.

**Das ist die `sinks = {}`-Lehre aus slice-053, auf einen Werttyp angewandt:** wo ein Feld
verhaltenstragend ist, ist der **fehlende** stille Default der schärfste Sensor.

### 2.2 Die Parameter-Abfrage sitzt an der bestehenden Lese-Naht — und weitet ihre Bedeutung

[ADR-0021](../../adr/0021-wand-im-2d-canvas.md) E15 verlangt „eine eigene, schmale Abfrage
**derselben** 2D-Lese-Naht". Sie liefert damit erstmals etwas, das **keine 2D-Projektion** ist:
Stärke und Höhe sind Bauteil-Parameter, keine Grundriss-Geometrie.

**Das wird im Port-Vertrag ausgeschrieben, nicht stillschweigend gedehnt:** die Naht liefert, **was
die 2D-Zeichenfläche lesen muss** — die Projektion **und**, zu einer benannten Wand, die
**änderbaren** Parameter. Die Alternative (ein dritter Read-Port neben `ViewModelPort` und
`PlanViewPort`) wäre für **zwei Zahlen** eine eigene Naht mit eigener Implementierung und eigenem
Composition-Root-Zweig — und sie hätte dasselbe Bedeutungs-Problem, nur mit mehr Teilen.

**Vom Plan-Review zu prüfen** (R2): ob die Weitung tragbar ist oder ob der Vertrag anders zu fassen
ist. Der Slice trifft die Entscheidung nicht neu — die ADR hat sie getroffen —, aber er **schreibt
sie aus**, statt sie zu unterstellen.

## 3. Bewusst NICHT Teil

- **Jede Zeile UI.** Kein Werkzeug-Modus, kein Picking, kein Panel — das sind
  [`slice-058`](slice-058-wand-zeichnen-im-canvas.md)/[`059`](slice-059-wand-auswaehlen-und-aendern.md).
- **Herkunft für andere Bauteile** (Türen, Fenster, Treppen, Dächer, Decken) — die Projektion trägt
  heute nur Wand-Achsen und Hilfslinien; mehr zu beschriften wäre Vorrat ohne Konsumenten.
- **Weitere Parameter** (Wandtyp, Material) — [ADR-0021](../../adr/0021-wand-im-2d-canvas.md) E13
  grenzt sie ausdrücklich aus.
- **Jede Änderung an Export-Encodern, Persistenz, Schema.** Die Erweiterung ist **additiv**; genau das
  ist zu beweisen (§4-5).
- **Ein `entity_layers`-Bezug** — [ADR-0021](../../adr/0021-wand-im-2d-canvas.md) §Konsequenzen
  entscheidet ausdrücklich, dass Bauteile keine Ebenen-Zuordnung bekommen.

## 4. Orakel-Schnitt — jede Zeile nennt die Komponente, an der sie diskriminiert

| # | Zusicherung | Wo geprüft | Diskriminierende Gegenprobe |
|---|---|---|---|
| 1 | **Jedes Segment trägt die richtige Art**: Wand-Achsen als Wand-Achse, Hilfslinien als Hilfslinie — im selben Geschoss, in einer Liste | `projectPlan`, Kern | Art vertauscht ⇒ rot |
| 2 | **Die Projektion liefert nie ein Segment ohne Herkunft** — für **jedes** Segment jedes Geschosses ist sie gesetzt | `projectPlan`, Kern | eine der beiden Einfüge-Stellen ohne Herkunft ⇒ rot |
| 3 | **Die Identität ist die echte, nicht der Index**: bei Ids, die **nicht** der Reihenfolge entsprechen (z. B. nach Löschungen), trägt das Segment die Bauteil-Id | `projectPlan`, Kern | Laufindex statt Id ⇒ rot |
| 4 | **Unsichtbare Ebenen bleiben draußen** — die Herkunft ändert am Sichtbarkeits-Filter nichts | Bestands-Orakel `test_plan_projection.cpp` | (Regressions-Netz) |
| 5 | **Die sechs Export-Golden bleiben byte-identisch** — kein Encoder sieht die Herkunft | `GoldenExport.*` in `make test` | s. Absatz unten |
| 6 | **Die Parameter-Abfrage liefert die echten Werte** einer bekannten Wand (Gleichheit, nicht Nähe) | `PlanViewPort`-Implementierung, Kern | Rückgabe auf Default-Werte ⇒ rot |
| 7 | **Unbekannte Identität ⇒ kein Wert** (`nullopt`), **kein Wurf** — die Abfrage ist read-only und **total** | `PlanViewPort`-Implementierung, Kern | Wurf statt `nullopt` ⇒ rot; Default-Wert statt `nullopt` ⇒ rot |
| 8 | **Die Abfrage liefert den geklemmten Ist-Wert**, nicht den zuletzt gewünschten — nach einer geklemmten Setzung stimmt sie mit dem Modell überein | `PlanViewPort` + Bestands-Klemmung | Wert aus einer anderen Quelle ⇒ rot |
| 9 | **Kein Persistenz-/Schema-Diff** — `data-model.yaml`/`schema.sql` byte-unberührt, Round-Trip unverändert | `git diff --stat` + Bestands-Persistenz-Orakel | (Regressions-Netz) |

**Zeile 5 ist die tragende Zusage dieses Slice — und sie braucht eine eigene Gegenprobe, weil sie ein
Netz ist, kein Orakel.** „Nichts hat sich geändert" wird nicht dadurch bewiesen, dass ein Test grün
bleibt. **Die Gegenprobe geht darum andersherum:** ein Export-Encoder wird **absichtlich** um eine
Ausgabe je Segment erweitert — fallen die Golden dann, ist bewiesen, dass sie Encoder-Änderungen
**überhaupt** fangen, und ihr Grün-Bleiben unter der echten Änderung ist eine Aussage. **Ohne diese
Umkehrung wäre Zeile 5 ein Test, der immer grün ist.**

## 5. Definition of Done

- [ ] **`src/hexagon/model/plan_view.h`**: `PlanSegment` trägt eine **optionale** Herkunft (Art +
      Identität); Aggregat-Klammerform bleibt gültig, ein vergessenes Feld ist **leer**, nicht falsch
      beschriftet (§2.1). Neuer Werttyp für die änderbaren Parameter im `model/`.
- [ ] **`src/hexagon/services/geometry/plan_projection.cpp`**: beide Einfüge-Stellen setzen die
      Herkunft mit der **echten** Id; Orakel §4-1..3.
- [ ] **`src/hexagon/ports/driving/plan_view_port.h`**: zweite Abfrage (Identität → änderbare
      Parameter), **total** (`nullopt` statt Wurf). **Der Vertragstext schreibt die Bedeutungs-Weitung
      aus** (§2.2) — die Naht liefert, was die 2D-Fläche lesen muss.
- [ ] **`src/hexagon/services/structure_edit_service.{h,cpp}`**: Implementierung der zweiten Abfrage;
      Orakel §4-6..8.
- [ ] **`make a-check` grün ohne neue Kante** — der Port importiert weiterhin nur `model`.
- [ ] **Die sechs Export-Golden byte-identisch** (§4-5) **plus die Umkehr-Gegenprobe**, die belegt,
      dass sie Encoder-Änderungen fangen.
- [ ] **Kein Persistenz-/Schema-Diff**, am `git diff --stat` belegt; `make schema-check` bleibt der
      Drift-Wächter, **nicht** der Unberührtheits-Sensor.
- [ ] **Orakel §4-1..3 und 6..8 je mit roter Gegenprobe** im Closure-Text, **einzeln** gemessen;
      §4-4/5/9 als Netz benannt.
- [ ] **[ADR-Index](../../adr/README.md)**: die [ADR-0021](../../adr/0021-wand-im-2d-canvas.md)-Folgepflichtzeile
      „Lese-Naht-Slice" auf **erfüllt** nachziehen ([MR-020](../../../../harness/conventions.md)).
- [ ] **Beobachtungspflicht festhalten:** ob die Erweiterung den
      [ADR-0020](../../adr/0020-driven-adapter-serialisieren-kern-liefert-geometrie.md)-Re-Eval
      („Bündel wird zu breit") nähergerückt hat — **eine** optionale Herkunft zieht ihn nicht; die
      Closure sagt, wo die Grenze jetzt steht.
- [ ] **Kein Lastenheft-, kein Spezifikations-Eintrag** — beides hat
      [`slice-056`](../done/slice-056-wand-im-canvas-adr-ak.md) geliefert. (Geprüft und **verneint**.)
- [ ] **CHANGELOG** [Unreleased]-Eintrag.
- [ ] **[MR-006](../../../../harness/conventions.md#mr-006--unabhängiges-plan-review-vor-implementierungs-start)-Report
      unter `docs/reviews/`** je Lauf.
- [ ] **`make gates` grün** (inkl. Ruhe-Marker-Toggle beim `git mv`,
      [MR-017](../../../../harness/conventions.md)); **`make io-smoke` grün**.

## 6. Plan (vor Code)

| Datei / Komponente | Art | Begründung |
|---|---|---|
| `src/hexagon/model/plan_view.h` | ändern | optionale Herkunft am Segment |
| `src/hexagon/model/`-Werttyp für die änderbaren Parameter | neu | Brace-Form `.{h}`, weil noch nicht existent |
| `src/hexagon/services/geometry/plan_projection.cpp` | ändern | Herkunft an beiden Einfüge-Stellen |
| `src/hexagon/ports/driving/plan_view_port.h` | ändern | zweite Abfrage + ausgeschriebener Vertrag |
| `src/hexagon/services/structure_edit_service.{h,cpp}` | ändern | Implementierung |
| `tests/hexagon/test_plan_projection.cpp` | ändern | §4-1..3 |
| `tests/hexagon/`-Test der Parameter-Abfrage | neu/ändern | §4-6..8 |
| `tests/adapters/test_golden_export.cpp` | **unberührt** | genau das ist die Zusage (§4-5) |
| `src/hexagon/CMakeLists.txt`, `tests/CMakeLists.txt` | ggf. ändern | nur bei neuen Dateien |
| `docs/plan/adr/README.md` | ändern | Folgepflichtzeile auf erfüllt |
| `CHANGELOG.md` | ändern | [Unreleased] |
| `docs/reviews/`-Report | neu | das [MR-006](../../../../harness/conventions.md#mr-006--unabhängiges-plan-review-vor-implementierungs-start) vor dem Start |

**Nicht berührt:** `src/adapters/**` (weder UI noch IO — **die Encoder bleiben, wie sie sind**),
`spec/**`, `data-model.yaml`/`schema.sql`, `.a-check.yml`/`.d-check.yml`, `docs/plan/adr/0021-*`.

## 7. Risiken

- **R1 — „additiv" ist eine Behauptung, bis die Golden es sagen.** Sie ist der Kern dieses Slice.
  Fällt auch nur ein Golden, ist **die Entscheidung** zu prüfen, nicht der Test: dann sieht ein
  Encoder die Herkunft, und die Erweiterung ist nicht das, was die ADR beschlossen hat.
- **R2 — die Lese-Naht liefert erstmals etwas, das keine 2D-Projektion ist.** §2.2 schreibt die
  Weitung aus; **ob sie tragbar ist, gehört ins Plan-Review**, nicht in den Vollzug. Die ADR hat den
  Ort entschieden — nicht, wie der Vertragstext ihn benennt.
- **R3 — der stille Aggregat-Default.** §2.1 löst ihn über `optional`; die Gefahr ist real, weil
  `PlanSegment` an mehreren Stellen in Klammerform gebaut wird und ein Nullwert der Aufzählung wie
  eine gültige Art aussähe. **Orakel-Zeile 2 ist deshalb kein Luxus.**
- **R4 — die Parameter-Abfrage darf nicht werfen.** Der Bauteil-**Bearbeitungs**-Weg wirft bei
  unbekannten Bezügen; die **Lese**-Naht ist read-only und total (Muster `planView()`: leeres Modell
  ⇒ leere Sicht, kein Fehler). Ein geerbtes Wurf-Verhalten wäre eine stille Vertrags-Änderung —
  Orakel-Zeile 7 prüft **beide** Fehlformen (Wurf **und** Default-Wert).
- **R5 — die Herkunft könnte Vorrat werden.** Nur Wand-Achsen und Hilfslinien werden beschriftet,
  weil nur sie in der Projektion vorkommen. Jede weitere Art ohne Konsumenten wäre genau das
  Breit-Werden, das [ADR-0020](../../adr/0020-driven-adapter-serialisieren-kern-liefert-geometrie.md)
  als Re-Eval führt.

## 8. Trigger

- [ADR-0021](../../adr/0021-wand-im-2d-canvas.md) Entscheidungen 11 und 15 samt der
  Folgepflicht-Zeile „Lese-Naht-Slice (vorgelagert)" im [ADR-Index](../../adr/README.md).

## 9. Closure-Trigger

- §4-1..3 und 6..8 grün + je diskriminierend belegt; §4-4/5/9 als Netz grün, Zeile 5 zusätzlich mit
  der **Umkehr-Gegenprobe**; `make gates` + `make io-smoke` grün; kein Persistenz-/Schema-/Encoder-Diff
  am `git diff --stat` belegt; ADR-Index nachgezogen; Closure-Notiz.

## 10. Sub-Area-Modus-Begründung

### Sub-Area: Domänen-Modell + Ports + Services (Hexagon-Kern)

- **Modus:** GF; **Dichte:** klein-mittel — ein optionales Feld, ein Werttyp, eine Port-Methode, eine
  Implementierung, neun Orakel-Zeilen.
- **Phase-Reife:** die Projektion liegt seit ihrer Hebung in den Kern; die Export-Golden seit der
  Golden-Infrastruktur; die ADR-Entscheidung seit
  [`slice-056`](../done/slice-056-wand-im-canvas-adr-ak.md).
- **Risiko:** mittel — **nicht im Schreiben**, sondern in der Frage, ob die Erweiterung wirklich
  additiv ist. Der Werttyp ist geteilt: Bildschirm **und** sechs Export-Formate hängen daran.

## 11. [MR-006](../../../../harness/conventions.md#mr-006--unabhängiges-plan-review-vor-implementierungs-start)-Einarbeitung

_(offen — der Lauf steht vor dem Start aus; HIGHs blockieren ihn.)_

## 12. Closure-Notiz

_(bei Ausführung auszufüllen)_
