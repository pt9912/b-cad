---
id: slice-048a
titel: DRW-Zeichen-Aid Fangpunkte — AK-Schärfung [LH-FA-DRW-001](../../../../spec/lastenheft.md#modul-zeichnungsfunktionen-drw) & Spec-Mapping (parametrisiert auf [ADR-0019](../../adr/0019-drw-2d-canvas.md) Entscheidung 6)
status: done
welle: welle-5-erweiterung
lastenheft_refs: [[LH-FA-DRW-001](../../../../spec/lastenheft.md#modul-zeichnungsfunktionen-drw), [LH-FA-DRW-005](../../../../spec/lastenheft.md#lh-fa-drw-005), [LH-FA-DRW-006](../../../../spec/lastenheft.md#lh-fa-drw-006)]
adr_refs: [[ADR-0001](../../adr/0001-hexagonale-architektur.md), [ADR-0009](../../adr/0009-gui-framework-qt6.md), [ADR-0018](../../adr/0018-drw-2d-zeichen-daten.md), [ADR-0019](../../adr/0019-drw-2d-canvas.md), [ADR-0020](../../adr/0020-driven-adapter-serialisieren-kern-liefert-geometrie.md)]
---

# Slice 048a: DRW-Zeichen-Aid Fangpunkte — AK-Schärfung & Spec-Mapping

**Status:** done (2026-07-24 — Lastenheft **0.1.16**, spez. §1 [`LH-FA-DRW-001.a`](../../../../spec/lastenheft.md#lh-fa-drw-001), `make gates` grün; s. §8).
**[MR-006](../../../../harness/conventions.md#mr-006--unabhängiges-plan-review-vor-implementierungs-start)-Review**
2026-07-24 (Reviewer ≠ Autor): **0 HIGH / 2 MED / 3 LOW / 2 INFO → startbar**
([Report](../../../reviews/2026-07-24-slice-048a-plan.md); MED-1 [„Kandidat" raus aus dem Lastenheft-AK-Körper →
§1] + MED-2 [§1-`.a`-Bezug auf den frisch gehobenen Anker `#lh-fa-drw-001`, nicht Modul-Anker] eingearbeitet;
LOW-1 [Tie-Break in §1 **ausschreiben**] + LOW-3 [Review-Report als DoD-Zeile] übernommen; LOW-2/INFO
bestätigt). **Reine Doku/Entscheidung — kein Code, kein Schema.**

**Welle:** welle-5-erweiterung (DRW-Strang, **Zeichen-Aids-Sub-Linie** — die dritte DRW-Sub-Linie nach dem
Fundament [slice-032a](../done/slice-032a-drw-fundament-ak-spec.md)/[032b](../done/slice-032b-drw-impl.md)/[032c](../done/slice-032c-drw-export.md)
und dem Interaktiv-Canvas [slice-041a](../done/slice-041a-drw-canvas-adr-ak.md)/[slice-043](../done/slice-043-drw-canvas-impl.md);
Muster **AK-Schärfung vor Impl** wie [slice-032a](../done/slice-032a-drw-fundament-ak-spec.md)/[slice-025a](../done/slice-025a-pdf-png-ak-spec.md)/[slice-026a](../done/slice-026a-plg-ak-spec.md)).

**Bezug:** [LH-FA-DRW-001](../../../../spec/lastenheft.md#modul-zeichnungsfunktionen-drw) (Fangpunkte), bisher
**Outline**-Einzeiler (Bullet ohne AK, ohne per-ID-Anker; Ursprung 0.1.0-Outline). Der Fang wirkt auf das
**interaktive Zeichnen von Hilfslinien** ([LH-FA-DRW-005](../../../../spec/lastenheft.md#lh-fa-drw-005), interaktive
Erzeugung seit [slice-043](../done/slice-043-drw-canvas-impl.md)) und respektiert die
Ebenen-Sichtbarkeit ([LH-FA-DRW-006](../../../../spec/lastenheft.md#lh-fa-drw-006)).
[OBJ-003](../../../../spec/lastenheft.md#3-projektziele) (»2D- und 3D-Darstellung aus demselben Datenmodell« —
kein DRW-`ACC` existiert oder entsteht). **Parametrisiert auf
[ADR-0019](../../adr/0019-drw-2d-canvas.md) Entscheidung 6** — die **Heimat** der Zeichen-Aids ist dort
**bereits entschieden** (Fang/Raster/Winkel = **UI-Interaktions-Zustand im Canvas**, sie quantisieren die
Eingabe-Bildschirmposition zu Modell-mm; der **gefangene mm-Wert** wird an `addGuideLine` übergeben; **keine**
persistierten Modell-Daten, **keine** `§3`-Bauteil-Konstante). Dieser Slice **löst die von
[ADR-0019](../../adr/0019-drw-2d-canvas.md) §Re-Evaluierungs-Trigger hierher deferierte AK-Schärfung für DRW-001
ein** und braucht **keine neue Grundsatz-ADR**; seine **Mapping-/AK-Entscheidung lebt in Lastenheft-AK +
`spezifikation.md` §1** (Muster [slice-032a](../done/slice-032a-drw-fundament-ak-spec.md)).
[ADR-0001](../../adr/0001-hexagonale-architektur.md) (Schichtung — der Canvas ist ein Driving-Adapter),
[ADR-0009](../../adr/0009-gui-framework-qt6.md) (Qt = Driving/UI, Regel E),
[ADR-0018](../../adr/0018-drw-2d-zeichen-daten.md) (DRW-Datenheimat — die Zeichen-**Daten** im Kern; die
Zeichen-**Aids** UI-resident),
[ADR-0020](../../adr/0020-driven-adapter-serialisieren-kern-liefert-geometrie.md) (2D-`PlanView`-Projektion
kern-seitig — die **Fang-Kandidaten** speisen sich aus derselben `PlanView`, die der Canvas ohnehin pullt).

**Autor:** Dietmar Burkard (AI-Harness-Lauf). **Datum:** 2026-07-24.

**Schnitt-Herkunft:** Entscheidungs-/Spec-Hälfte des DRW-Zeichen-Aids-Strangs (Muster 032a/025a/026a). Die
DRW-001-AK (was heißt **Fangen** benutzer-beobachtbar) braucht **prüfbare AK + ein entschiedenes Mapping**,
bevor implementiert wird (048b). **Reine Doku/Entscheidung, kein Code.**

**Bewusst NICHT Teil (benannte Grenzen / Folge):**

- **Implementierung** des Fangens (Kandidaten-Sammlung aus der `PlanView`, Bildschirm-Schwellwert, Fang-Auswahl
  in der Maus-Handhabung, `QMouseEvent`-Synthese-AK-Test) = **Folge-Impl-Slice 048b** (Muster 032b/043).
- **Raster-Fang** ([LH-FA-DRW-002](../../../../spec/lastenheft.md#modul-zeichnungsfunktionen-drw)),
  **Winkelvorgaben** ([LH-FA-DRW-003](../../../../spec/lastenheft.md#modul-zeichnungsfunktionen-drw)),
  **Bemaßung** ([LH-FA-DRW-004](../../../../spec/lastenheft.md#modul-zeichnungsfunktionen-drw)) — **eigene
  spätere Kampagnen-Slices** (je eigene AK + Impl).
- **Schnittpunkt-/Mittelpunkt-/Lot-/Tangenten-Fang** und **Fangen beim interaktiven Bauteil-Zeichnen** (es
  gibt kein Bauteil-Zeichnen auf dem Canvas) — **spätere Ausbaustufen**, nicht dieser Schnitt.
- **Sichtbare Fang-Rückmeldung** (Fang-Marker/Highlight am Cursor) — Darstellungs-Politur; v1-AK ruht auf dem
  **exakt übernommenen Endpunkt** (persistiert + exportiert), nicht auf einem Cursor-Indikator (Impl-Slice
  darf einen Marker zeichnen, die AK **fordert** ihn nicht).

---

## 1. Ziel

[LH-FA-DRW-001](../../../../spec/lastenheft.md#modul-zeichnungsfunktionen-drw) bekommt **lösungsfreie,
benutzer-beobachtbare Akzeptanzkriterien**
([MR-008](../../../../harness/conventions.md#mr-008--lastenheft-schärfung-bleibt-lösungsfrei)) und ein
**entschiedenes Mapping** (`spezifikation.md` §1), bevor implementiert wird — **innerhalb der von
[ADR-0019](../../adr/0019-drw-2d-canvas.md) Entscheidung 6 entschiedenen Aid-Heimat**. Zusätzlich wird die
**[ADR-0019](../../adr/0019-drw-2d-canvas.md)-§Re-Eval-Folgepflicht „Fang/Raster/Winkel-AK schärfen" für
DRW-001 eingelöst**.

**Die fachliche Eigenheit, die die AK prägt.** Seit [slice-043](../done/slice-043-drw-canvas-impl.md) gibt es
eine **interaktive 2D-Zeichenfläche**: der Nutzer zieht Hilfslinien mit der Maus, v1 zeichnet **frei**
(Endpunkt = geklickte Modell-mm). **Fangen** verändert genau diesen Eingabe-Schritt: liegt der Cursor **nahe
genug** an einem markanten vorhandenen Punkt, übernimmt die gezeichnete Position **exakt** diesen Punkt statt
der ungefähren Cursor-mm. Das benutzer-beobachtbare Ergebnis ist damit **interaktiv** (anders als der
Fundament-Schnitt [slice-032a](../done/slice-032a-drw-fundament-ak-spec.md), der mangels Canvas nur übers
Artefakt beobachtbar war): die gezogene Hilfslinie **rastet sichtbar ein** und trägt danach **exakt** die
Koordinaten des Fang-Ziels — nachweisbar **sofort**, **unverändert nach Speichern/Laden** und **im
2D-Grundriss-Export**. Fang-Kandidaten sind nur **sichtbare** Punkte (Ebenen-Filter,
[LH-FA-DRW-006](../../../../spec/lastenheft.md#lh-fa-drw-006)).

## 2. Definition of Done

- [ ] **Lastenheft [LH-FA-DRW-001](../../../../spec/lastenheft.md#modul-zeichnungsfunktionen-drw) von Outline
      auf AK-Niveau** (**lösungsfrei, benutzer-beobachtbar** — **kein** Bildschirm-Schwellwert-/Radius-/Pixel-/
      Kandidaten-Sammlungs-/`screenToModel`-Vokabular; das gehört in §1;
      [MR-008](../../../../harness/conventions.md#mr-008--lastenheft-schärfung-bleibt-lösungsfrei)); eigenes
      `####`-Heading mit **Inline-HTML-Anker** `lh-fa-drw-001` (Muster
      [LH-FA-DRW-005](../../../../spec/lastenheft.md#lh-fa-drw-005); Modul-Heading + Bullets DRW-002/003/004
      bleiben). Mindestens:
      **Happy:** Given eine **sichtbare** Wand-Achse (oder Hilfslinie) mit einem Endpunkt P auf der
      Zeichenfläche, when der Nutzer eine Hilfslinie zieht und Anfang **oder** Ende **in Fang-Nähe von P**
      setzt, then rastet der Punkt **exakt auf P** ein — die erzeugte Hilfslinie trägt **genau** die
      Koordinaten von P (nicht die ungefähre Cursor-Position), **sofort** sichtbar und **unverändert nach
      Speichern/Laden** sowie im 2D-Grundriss-Export.
      **Boundary:** Given der Cursor wird **außerhalb** jeder Fang-Nähe losgelassen, when die Hilfslinie
      abgeschlossen wird, then wird **nicht gefangen** — der Endpunkt ist die geklickte Position (freies
      Zeichnen wie [LH-FA-DRW-005](../../../../spec/lastenheft.md#lh-fa-drw-005) v1).
      **Negative:** Given es gibt **keinen sichtbaren fangbaren Punkt** in Reichweite (leerer Plan **oder** der
      einzige markante Punkt liegt auf einer **unsichtbaren** Ebene), when der Nutzer zeichnet, then wird
      **nicht gefangen** und es entsteht **kein zusätzlicher/verfälschter** Punkt — die Hilfslinie trägt die
      geklickte Position, das Modell bleibt konsistent. (**Vokabelsperre auch im AK-Körper**
      [[MR-008](../../../../harness/conventions.md#mr-008--lastenheft-schärfung-bleibt-lösungsfrei)]: kein
      „Kandidat"/„Kandidaten-Sammlung" im Lastenheft — das ist §1-Wie.)
      **Boundary/Entartung (Wiederverwendung der bestehenden Regel):** rastet Fangen Anfang **und** Ende auf
      **denselben** Punkt (Anfang = Ende), then greift die **bestehende** Entartungs-Ablehnung aus
      [LH-FA-DRW-005](../../../../spec/lastenheft.md#lh-fa-drw-005) (keine Hilfslinie, Modell unverändert) —
      **kein neuer Fehlerfall**.
      + Header-Nachzug **`lastenheft.md` `Version:` → 0.1.16** == oberste `lastenheft-historie.md`-Zeile
      ([MR-010](../../../../harness/conventions.md#mr-010--lastenheft-header-version--oberste-9-historie-zeile)/[MR-012](../../../../harness/conventions.md#mr-012--mr-010-invariante-folgt-der-ausgelagerten-lastenheft-historie)).
- [ ] **`spec/lastenheft-historie.md`** oberste Zeile **0.1.16** (Datum 2026-07-24, Verweis slice-048a,
      lösungsfrei; [MR-010](../../../../harness/conventions.md#mr-010--lastenheft-header-version--oberste-9-historie-zeile)/[MR-012](../../../../harness/conventions.md#mr-012--mr-010-invariante-folgt-der-ausgelagerten-lastenheft-historie)).
- [ ] **`spec/spezifikation.md` §1 neuer Block [`LH-FA-DRW-001.a`](../../../../spec/lastenheft.md#modul-zeichnungsfunktionen-drw)**
      (im **ausgeführten** §1 Bezug auf den **frisch gehobenen** Anker `#lh-fa-drw-001`, **nicht** den
      Modul-Anker — MED-2; der Plan-Link zeigt bis zur Anker-Hebung auf den Modul-Anker; Muster
      [`LH-FA-DRW-005.a`](../../../../spec/lastenheft.md#lh-fa-drw-005) → `#lh-fa-drw-005`; Mapping **ohne
      ADR-Verweis im Körper**,
      [MR-011](../../../../harness/conventions.md#mr-011--referenz-integritäts-gate-matrix-ids-spans-hostpaths)):
      Fangen quantisiert die **Eingabe-Bildschirmposition** zu Modell-mm **vor** dem bestehenden
      `addGuideLine`; **Fang-Kandidaten** = Endpunkte der **sichtbaren** 2D-`PlanView`-Segmente (Wand-Achsen je
      Geschoss) **+** Endpunkte **sichtbarer** Hilfslinien (Sichtbarkeit = Ebenen-Export-Filter); **Fang-Nähe**
      = ein **Bildschirm-Schwellwert** (Widget-Konstante; Default legt der Impl-Slice fest — **keine** Zahl im
      Lastenheft); bei **mehreren** Kandidaten in Reichweite → der **nächstgelegene**, und bei **exakt
      gleicher** Distanz eine **konkret ausgeschriebene** Tie-Break-Regel (LOW-1 — **nicht** nur „deterministisch"
      behaupten: die Regel benennen, z. B. erster in stabiler Iterations-Reihenfolge der `PlanView` /
      lexikografisch kleinste (x, y)); der **gefangene mm-Wert** ist die an `addGuideLine` übergebene Position →
      Fangen erzeugt **keine** neue Entität und **keine** Schema-/Persistenz-Änderung (eine gefangene
      Hilfslinie ist eine **gewöhnliche** Hilfslinie, deren Endpunkt zufällig exakt einem vorhandenen Punkt
      gleicht); **kein** neuer `op`, **keine** `ModelChanged`-Meldung (reine Eingabe-Quantisierung); die
      Entartungs-Ablehnung (Anfang = Ende) bleibt die bestehende
      [`E-VAL-001`](../../../../spec/spezifikation.md#4-fehler-codes-und-logging-felder)-Semantik aus dem
      DRW-Fundament (kein neuer Fehler-Code). + `spezifikation-historie.md` +
      `**Letzte Änderung:**`-Header-Nachzug.
- [ ] **`spec/architecture.md` unberührt bzw. nur `## Geschichte`-Provenance** — der Canvas pullt den
      **bestehenden** [`PlanViewPort`](../../adr/0019-drw-2d-canvas.md) bereits (Rendering); Fangen ist
      **internes Canvas-Verhalten** über derselben `PlanView` → **keine neue Naht/Port/Kante**. Falls eine
      Provenance-Zeile ergänzt wird, dann in `## Geschichte` (exclude-section), **kein** Körper-ADR-Verweis
      ([MR-011](../../../../harness/conventions.md#mr-011--referenz-integritäts-gate-matrix-ids-spans-hostpaths)).
- [ ] **[ADR-0019](../../adr/0019-drw-2d-canvas.md)-§Re-Eval-Folgepflicht** „Fang-AK (DRW-001) geschärft" im
      Closure-Commit vermerkt ([ADR-Index](../../adr/README.md) bzw. die Re-Eval-Trigger-Liste in
      [ADR-0019](../../adr/0019-drw-2d-canvas.md)) — als **teilweise erfüllt** formulieren (INFO-2: die
      Folgepflicht bündelt DRW-001/002/003; **nur DRW-001** ist hier geschärft, Raster/Winkel/Bemaßung bleiben
      offen — **nicht** „erledigt" schreiben).
- [ ] **[MR-006](../../../../harness/conventions.md#mr-006--unabhängiges-plan-review-vor-implementierungs-start)-Report**
      `docs/reviews/2026-07-24-slice-048a-plan.md` liegt vor (LOW-3; Muster
      [slice-032a](../done/slice-032a-drw-fundament-ak-spec.md)).
- [ ] **Reine Doku/Entscheidung — kein Code, keine Tests, kein Schema** (`make schema-check` **byte-unberührt**;
      Fangen ist UI-Eingabe-Quantisierung, **keine** persistierten Daten). `make gates` grün; Closure-Notiz.
      **Nicht Teil:** **048b** (Fang-Kandidaten aus `PlanView` + Bildschirm-Schwellwert + Fang-Auswahl in der
      Canvas-Maus-Handhabung + `QMouseEvent`-Synthese-AK-Test); die übrigen DRW-Aids
      (Raster/Winkel/Bemaßung/Gruppen).

## 3. Plan (vor Code)

| Datei / Komponente | Änderungs-Art | Begründung |
|---|---|---|
| `spec/lastenheft.md` | ändern | [LH-FA-DRW-001](../../../../spec/lastenheft.md#modul-zeichnungsfunktionen-drw) Outline → AK (lösungsfrei; per-ID-`####`-Heading + Inline-Anker `lh-fa-drw-001`); Header `Version:` → 0.1.16 |
| `spec/lastenheft-historie.md` | ändern | oberste Zeile 0.1.16 ([MR-010](../../../../harness/conventions.md#mr-010--lastenheft-header-version--oberste-9-historie-zeile)/[MR-012](../../../../harness/conventions.md#mr-012--mr-010-invariante-folgt-der-ausgelagerten-lastenheft-historie)) |
| `spec/spezifikation.md` | ändern | §1 [`LH-FA-DRW-001.a`](../../../../spec/lastenheft.md#modul-zeichnungsfunktionen-drw)-Mapping-Block (Kandidaten aus `PlanView`, Bildschirm-Schwellwert, nächstgelegener Tie-Break, gefangener mm-Wert → `addGuideLine`, kein `op`/Schema/Entität; Entartung = bestehende [`E-VAL-001`](../../../../spec/spezifikation.md#4-fehler-codes-und-logging-felder)-Lesart) — **ADR-frei** im Körper |
| `spec/spezifikation-historie.md` | ändern | Provenance-Zeile (slice-048a) |
| `spec/architecture.md` | ggf. ändern | nur `## Geschichte`-Provenance-Zeile (kein neuer Port/Naht) |
| [ADR-0019](../../adr/0019-drw-2d-canvas.md) / [ADR-Index](../../adr/README.md) | ändern (Closure) | §Re-Eval-Folgepflicht „Fang-AK DRW-001 geschärft" → erfüllt (Raster/Winkel/Bemaßung bleiben offen) |
| `docs/reviews/2026-07-24-slice-048a-plan.md` | neu | [MR-006](../../../../harness/conventions.md#mr-006--unabhängiges-plan-review-vor-implementierungs-start)-Report |

## 4. Trigger

- **[ADR-0019](../../adr/0019-drw-2d-canvas.md) Accepted** (2026-07-22) — die Aid-Heimat (Entscheidung 6) ist
  **entschieden**; dieser Slice schärft nur die **AK** innerhalb dieses Rahmens, **keine** neue Grundsatz-ADR
  (Abgrenzung zu [slice-041a](../done/slice-041a-drw-canvas-adr-ak.md), die die ADR **erzeugte**). Ein
  interaktiver Canvas existiert ([slice-043](../done/slice-043-drw-canvas-impl.md)) → die AK ist **interaktiv**
  beobachtbar. Nach eigenem
  [MR-006](../../../../harness/conventions.md#mr-006--unabhängiges-plan-review-vor-implementierungs-start)
  (0 HIGH) → **startbar**.

## 5. Closure-Trigger

- DoD vollständig, `make gates` grün, `make schema-check` byte-unberührt, Closure-Notiz → **slice-048b
  (DRW-001-Impl: Fang-Kandidaten aus `PlanView` + Bildschirm-Schwellwert + Fang-Auswahl + AK-Test)** wird
  startbar (eigenes
  [MR-006](../../../../harness/conventions.md#mr-006--unabhängiges-plan-review-vor-implementierungs-start)
  davor; [MR-009](../../../../harness/conventions.md#mr-009--geometrielastiges-code-review-vor-welle-closure)
  für 048b **einschlägig** — Fang-Distanz/Nächster-Punkt ist Geometrie).

## 6. Risiken und offene Punkte

- **Rest-Risiko #1 — Beobachtbarkeit „exakt gefangen" prüfbar formulieren (zentraler Review-Kandidat):** „rastet
  ein" muss **hart** beobachtbar sein, nicht gefühlt. Mitigation: die AK bindet an den **exakt übernommenen
  Endpunkt** — nach Speichern/Laden **identische** Koordinaten mit dem Fang-Ziel (nicht nur „nahe"), plus
  Erscheinen im 2D-Export. Der 048b-`QMouseEvent`-Synthese-Test prüft display-frei, dass die gezogene
  Hilfslinie **exakt** die Zielkoordinaten trägt (Surrogat-Zustand + `screenToModel`, Muster
  [slice-043](../done/slice-043-drw-canvas-impl.md)). Kein Framebuffer-Abhängen.
- **Rest-Risiko #2 — Sichtbarkeits-Kohärenz (Ebenen-Filter):** ein Kandidat auf **unsichtbarer** Ebene darf
  **nicht** fangbar sein (sonst rastet der Nutzer an etwas, das er nicht sieht → stiller Sprung). Die AK-Negative
  benennt das explizit; §1 bindet die Kandidaten-Sichtbarkeit an denselben Ebenen-Export-Filter wie
  [LH-FA-DRW-006](../../../../spec/lastenheft.md#lh-fa-drw-006).
- **Lösungsfreiheit ([MR-008](../../../../harness/conventions.md#mr-008--lastenheft-schärfung-bleibt-lösungsfrei)):**
  »rastet exakt auf einen sichtbaren Punkt in Fang-Nähe«, »außerhalb → frei«, »kein sichtbarer Kandidat → kein
  Fang« sind benutzer-beobachtbar; **Bildschirm-Schwellwert/Pixel**, Kandidaten-Sammlung aus der `PlanView`,
  nächstgelegener Tie-Break, `screenToModel`, „kein `op`" gehören in §1 bzw. 048b. **Vor dem Gate** per
  `grep` gegen Radius-/Pixel-/Port-Vokabular im Lastenheft-Diff selbst fangen (Lerneintrag 019a/026a).
- **Anker-Hebung ohne Link-Bruch:** [LH-FA-DRW-001](../../../../spec/lastenheft.md#modul-zeichnungsfunktionen-drw)
  hat heute nur den Modul-Anker; die Schärfung hebt sie auf `####`-Inline-Anker `lh-fa-drw-001` (Muster
  [LH-FA-DRW-005](../../../../spec/lastenheft.md#lh-fa-drw-005)); Modul-Heading + die Bullets DRW-002/003/004
  (Modul-anker-referenziert in [ADR-0019](../../adr/0019-drw-2d-canvas.md)) bleiben unberührt.
- **Header-Nachzug ([MR-010](../../../../harness/conventions.md#mr-010--lastenheft-header-version--oberste-9-historie-zeile)/[MR-012](../../../../harness/conventions.md#mr-012--mr-010-invariante-folgt-der-ausgelagerten-lastenheft-historie)):**
  `**Version:**` → 0.1.16 == jüngste `lastenheft-historie.md`-Zeile (vorige Schärfung 041a = 0.1.15).
  **Achtung Reihenfolge (LOW-2):** `lastenheft-historie.md` ist **chronologisch aufsteigend** (0.1.0 physisch
  oben, jüngste Zeile physisch **unten**); »oberste (jüngste) Zeile« im Sinne der Header==Historie-Invariante
  = die **letzte** Tabellenzeile. 0.1.16 daher **unten anhängen** (Muster 032a/041a), **nicht** oben einfügen.
- **Spec-Straten ADR-/Slice-frei ([MR-011](../../../../harness/conventions.md#mr-011--referenz-integritäts-gate-matrix-ids-spans-hostpaths)):**
  der §1-Block darf **keinen** ADR-Verweis und **keine** Slice-Nummer im `spezifikation.md`-Körper hinterlassen
  (Provenance nur `*-historie.md`) — vor dem Gate per `grep -E 'ADR-|slice-[0-9]{3}'` selbst fangen. Ebenso
  `architecture.md`-Körper.
- **Kein Schema-Drift:** Fangen berührt `data-model.yaml`/`schema.sql` **nicht** — `make schema-check` muss
  **byte-unberührt** bleiben (Kontrast zu 032a, das eine Kommentar-Heilung trug; hier gibt es keine).

## 7. Sub-Area-Modus-Begründung

### Sub-Area: Spec-Schreibung

- **Modus:** GF; **Dichte:** hoch — AK-Format (Happy/Boundary/Negative + Entartungs-Wiederverwendung),
  [MR-008](../../../../harness/conventions.md#mr-008--lastenheft-schärfung-bleibt-lösungsfrei) (lösungsfrei),
  [MR-010](../../../../harness/conventions.md#mr-010--lastenheft-header-version--oberste-9-historie-zeile)/012
  (Header == Historie),
  [MR-011](../../../../harness/conventions.md#mr-011--referenz-integritäts-gate-matrix-ids-spans-hostpaths)
  (Spec-Straten ADR-/Slice-frei),
  [`E-VAL-001`](../../../../spec/spezifikation.md#4-fehler-codes-und-logging-felder)-Wiederverwendung.
  **Phase-Reife:** DRW-Aid Phase 2 (Outline → AK). **Risiko:** niedrig (reine Doku;
  [ADR-0019](../../adr/0019-drw-2d-canvas.md) Entscheidung 6 trägt die Aid-Heimat). **Reconciliation:** keiner;
  Folge = 048b.

### Sub-Area: Planning-Lifecycle

- **Modus:** GF; **Dichte:** hoch ([ADR-0019](../../adr/0019-drw-2d-canvas.md)-Leitplanke: kein neuer
  Grundsatz-ADR; Muster DRW-Sub-Linien-„a"-Slices;
  [MR-006](../../../../harness/conventions.md#mr-006--unabhängiges-plan-review-vor-implementierungs-start) vor
  Impl). **Phase-Reife:** Phase 4. **Risiko:** niedrig.

## 8. Closure-Notiz

**Ausgeführt 2026-07-24** (`make docs-check` 0 Befunde, `make gates` grün, `make schema-check`
**byte-unberührt** — Fangen berührt kein Schema). Geliefert:

- **Lastenheft 0.1.16:** [`LH-FA-DRW-001`](../../../../spec/lastenheft.md#lh-fa-drw-001) Outline → AK
  (Happy/Boundary/Negative + Entartungs-Wiederverwendung, per-ID-`####`-Anker `lh-fa-drw-001`);
  Modul-Heading + Bullets DRW-002/003/004 unberührt. **Lösungsfrei** (kein Radius-/Pixel-/`PlanView`-/
  Kandidaten-Vokabular; MED-1 eingearbeitet). Header `Version:` 0.1.16 == jüngste `lastenheft-historie.md`-Zeile.
- **Spezifikation §1** neuer Block [`LH-FA-DRW-001.a`](../../../../spec/lastenheft.md#lh-fa-drw-001) (Fang-Aid-Mapping, **ADR-/Slice-frei**): Fangen =
  UI-Interaktions-Zustand des Canvas; Fang-Punkte = Endpunkte der **bestehenden** `PlanView` (Wand-Achsen +
  sichtbare Hilfslinien) → **keine** neue Naht/Port/Schicht-Kante, **kein** Schema/`op`/Entität; Auswahl =
  nächstgelegener mit **ausgeschriebenem** Tie-Break (stabile `PlanView`-Iterationsreihenfolge; LOW-1);
  unsichtbare Ebenen vor der `PlanView` gefiltert → Negative **strukturell erzwungen**; Entartung Anfang=Ende =
  bestehende [`E-VAL-001`](../../../../spec/spezifikation.md#4-fehler-codes-und-logging-felder)-Ablehnung.
  Bezug auf `#lh-fa-drw-001` (MED-2). + `spezifikation-historie.md` + `**Letzte Änderung:** 2026-07-24`.
- **[ADR-Index](../../adr/README.md)-Folgepflicht** [ADR-0019](../../adr/0019-drw-2d-canvas.md) „Fang/Raster/Winkel" von **offen** auf
  **teilweise** (DRW-001-AK erfüllt; DRW-001-Impl 048b + DRW-002/003 offen — INFO-2).
- **`architecture.md` unberührt** (kein struktureller Zuwachs; die [ADR-0019](../../adr/0019-drw-2d-canvas.md)-Geschichte benennt die
  Fang-Aids bereits).
- **Reine Doku/Entscheidung — kein Code, keine Tests, kein Schema.**

**Lerneintrag:** Der Canvas (slice-043) macht die Fang-AK **interaktiv** beobachtbar — die 032a-„kein
Canvas"-Ehrlichkeitsklausel entfällt; die harte Beobachtbarkeit ruht auf der **exakt übernommenen
Koordinate** (Round-Trip + Export), nicht auf dem Framebuffer. Weil `projectPlan` unsichtbare Ebenen **vor**
der `PlanView` filtert, ist die Sichtbarkeits-Negative **strukturell erzwungen**, nicht nur zugesichert.

**Folge:** **slice-048b** (DRW-001-Impl: Fang-Punkte aus `PlanView` + Bildschirm-Schwellwert + Fang-Auswahl in
der Canvas-Maus-Handhabung + `QMouseEvent`-Synthese-AK-Test) wird startbar — eigenes
[MR-006](../../../../harness/conventions.md#mr-006--unabhängiges-plan-review-vor-implementierungs-start) davor,
[MR-009](../../../../harness/conventions.md#mr-009--geometrielastiges-code-review-vor-welle-closure)
einschlägig (Fang-Distanz/Nächster-Punkt = Geometrie).
