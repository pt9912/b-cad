---
id: slice-041a
titel: DRW interaktiver 2D-Canvas — Grundsatz-ADR (2D-Zeichenfläche + 2D-Lese-Naht) + AK-Schärfung
status: open
welle: welle-5-erweiterung
lastenheft_refs: [[LH-FA-DRW-005](../../../../spec/lastenheft.md#lh-fa-drw-005), [LH-FA-DRW-006](../../../../spec/lastenheft.md#lh-fa-drw-006)]
adr_refs: [[ADR-0001](../../adr/0001-hexagonale-architektur.md), [ADR-0008](../../adr/0008-aenderungs-benachrichtigung.md), [ADR-0009](../../adr/0009-gui-framework-qt6.md), [ADR-0010](../../adr/0010-headless-gl-xvfb.md), [ADR-0018](../../adr/0018-drw-2d-zeichen-daten.md)]
---

# Slice 041a: DRW interaktiver 2D-Canvas — Grundsatz-ADR + AK-Schärfung

**Status:** open (Plan **startbar** nach Review-Einarbeitung — Start auf Projektinhaber-Wort).
**[MR-006](../../../../harness/conventions.md#mr-006--unabhängiges-plan-review-vor-implementierungs-start)-Plan-Review**
2026-07-22 (Reviewer ≠ Autor): **0 HIGH / 2 MED / 2 LOW / 2 INFO → startbar**
([Report](../../../reviews/2026-07-22-slice-041a-plan.md); MED-1 [Lese-Naht-Hebung = eigener Slice vor
Canvas, im ADR festgeschrieben] + MED-2 [011a-Präzedenz; ADR stabil vor AK-Finalisierung] + LOW-1
[[ADR-0010](../../adr/0010-headless-gl-xvfb.md) = Infra, nicht Treiber] eingearbeitet; LOW-2/INFO bestätigt). **Reine Doku/Entscheidung — kein Code.**
Die neue Grundsatz-ADR durchläuft **zusätzlich** ein unabhängiges **Text-Review** (Reviewer ≠ Autor)
+ Projektinhaber-Durchsicht vor **Accepted** (Muster [ADR-0009](../../adr/0009-gui-framework-qt6.md)/[ADR-0018](../../adr/0018-drw-2d-zeichen-daten.md);
HIGH blockiert) — Architektur-Entscheidungen sind **projektinhaber-zu-wählen** (wie [ADR-0009](../../adr/0009-gui-framework-qt6.md) (a)/(b)).

**Welle:** welle-5-erweiterung (DRW-Strang, **Interaktiv-Editor-Eröffnung** — die zweite DRW-Sub-Linie
nach dem Fundament [slice-032a](../done/slice-032a-drw-fundament-ak-spec.md)/[032b](../done/slice-032b-drw-impl.md)/[032c](../done/slice-032c-drw-export.md);
**strukturelles Muster [slice-011a](../done-archive/slice-011a-viewer-gui-adr-spec.md)** — sie **verfasste
[ADR-0009](../../adr/0009-gui-framework-qt6.md) + Spec-Nachzug in EINEM Slice** (dieser Slice bündelt ADR
+ AK ebenso). **Abgrenzung zu [slice-032a](../done/slice-032a-drw-fundament-ak-spec.md):** 032a
referenzierte ein **bereits akzeptiertes** [ADR-0018](../../adr/0018-drw-2d-zeichen-daten.md) (»braucht
keine neue Grundsatz-ADR«); 041a **erzeugt** die ADR — daher gilt das 011a-Muster.

**Bezug:** [LH-FA-DRW-005](../../../../spec/lastenheft.md#lh-fa-drw-005) (Hilfslinien) —
[slice-032a](../done/slice-032a-drw-fundament-ak-spec.md) hat die AK auf Niveau gesetzt, aber die
**interaktive Erzeugung** (Hilfslinie mit der Maus setzen/ziehen) **ausdrücklich offen** gelassen
(»späterer UI-Umfang«). **Dieser Slice schärft genau diese offene Klausel** und entscheidet die
Architektur der 2D-Zeichenfläche. **[ADR-0018](../../adr/0018-drw-2d-zeichen-daten.md) §Re-Evaluierungs-
Trigger** deferiert das hierher: »Interaktiver 2D-Zeichen-Editor … eigener Beschluss an der Regel-E-
Grenze ([ADR-0009](../../adr/0009-gui-framework-qt6.md), UI-Strang): eine 2D-Zeichenfläche **und eine
Lese-/Darstellungs-Naht (2D-Query/ViewModel)** werden dann fällig.«
[ADR-0001](../../adr/0001-hexagonale-architektur.md) (Schichtung, Regel A/B/E),
[ADR-0009](../../adr/0009-gui-framework-qt6.md) (Qt = Driving/UI, Regel E; single-threaded; Observer
pullt+repaint), [ADR-0008](../../adr/0008-aenderungs-benachrichtigung.md) (Benachrichtigung; **DRW-
Mutationen melden keinen `op`** — offener Punkt), [ADR-0010](../../adr/0010-headless-gl-xvfb.md)
(Headless-GL-**Infrastruktur** via Xvfb, unter der der `QMouseEvent`-Synthese-Test läuft — den
**Bedarf** an einem Interaktions-Test kündigte [ADR-0009](../../adr/0009-gui-framework-qt6.md) (f)
»mit Interaktion relevant« an; [ADR-0010](../../adr/0010-headless-gl-xvfb.md) ist die Infra, nicht der Treiber selbst).

**Autor:** Dietmar Burkard (AI-Harness-Lauf). **Datum:** 2026-07-22.

**Schnitt-Herkunft:** Entscheidungs-/Spec-Hälfte des DRW-Interaktiv-Strangs (Muster
[slice-032a](../done/slice-032a-drw-fundament-ak-spec.md)/019a/025a): die interaktive Zeichenfläche
braucht **einen entschiedenen Architektur-Rahmen** (Widget, 2D-Lese-Naht, Koordinaten-Abbildung,
Refresh, Kommando-Naht, Fang/Raster-Heimat, Headless-Test) **und prüfbare AK**, bevor implementiert
wird. **Reine Doku/Entscheidung, kein Code.**

**Bewusst NICHT Teil (benannte Grenzen / Folge):**

- **Implementierung** der Zeichenfläche (Widget, 2D-Lese-Port, Koordinaten-Abbildung, Maus-Handhabung,
  Composition-Root-Verdrahtung, AK-Tests) = **Folge-Impl-Slice(s)** (Muster 032b/c).
- **Fang/Raster/Winkel** ([LH-FA-DRW-001](../../../../spec/lastenheft.md#modul-zeichnungsfunktionen-drw)/002/003)
  — die Zeichen-**Aids** auf dem Canvas — bleiben **eigene spätere Slices** (je eigene AK + Impl, sobald
  der Canvas steht); v1 = **freies** Zeichnen (Endpunkt = geklickte Modell-mm). Der ADR benennt ihre
  Heimat (UI-Interaktions-Zustand vs. Kern/Spec-Konstante), schärft aber ihre AK nicht.
- **Bauteile auf dem Canvas zeichnen** (Wände/Räume interaktiv), **Layer-Bedien-Panel**, **Selektion/
  Picking**, **Bemaßung** — benannte Re-Eval-Trigger, nicht dieser Schnitt.
- **Interaktive 3D-Selektion** — eigener Trigger ([ADR-0009](../../adr/0009-gui-framework-qt6.md)
  §Re-Eval, AIS/V3d), unberührt.

---

## 1. Ziel

[LH-FA-DRW-005](../../../../spec/lastenheft.md#lh-fa-drw-005) bekommt seine **interaktive Erzeugungs-AK**
(lösungsfrei, benutzer-beobachtbar, [MR-008](../../../../harness/conventions.md#mr-008--lastenheft-schärfung-bleibt-lösungsfrei)):
der Nutzer **zeichnet eine Hilfslinie mit der Maus** auf einer 2D-Grundriss-Fläche, sie **erscheint
sofort**, **überlebt Speichern/Laden** und **erscheint im 2D-Export** (die 032b/c-Beobachtbarkeit wird
so um den **fehlenden Erzeugungs-Nutzerweg** ergänzt — die durchgängig benannte Fundament-Lücke
»der Nutzer kann noch nicht zeichnen« wird geschlossen). Zusätzlich wird die **DRW-Canvas-Grundsatz-ADR**
(Datei `docs/plan/adr/{0019-drw-2d-canvas}.md`, Nummer beim Anlegen) verfasst — der Architektur-Rahmen,
den 032a/[ADR-0018](../../adr/0018-drw-2d-zeichen-daten.md) hierher deferiert haben.

**Die sieben Architektur-Fragen, die die Grundsatz-ADR entscheidet** (aus der UI-Karte, jeweils mit
Pro/Contra + benannter Empfehlung; Projektinhaber wählt wie [ADR-0009](../../adr/0009-gui-framework-qt6.md) (a)/(b)):

1. **Widget-Form:** ein **neues 2D-Canvas-Widget** (`ui/view/`, `QWidget`/`QPainter` bzw.
   2D-ortho-GL) neben dem `ViewerWidget` (Präzedenz slice-029 Richtungs-Trennung) vs. den 3D-Viewer
   erweitern. **Empfehlung:** neues Widget (der 3D-Viewer hat eine fest verdrahtete Orbit-Kamera).
2. **2D-Lese-Naht (zentral, Regel-B-kritisch):** die 2D-Projektion `plan_geometry.projectPlan`
   (Building → 2D-Segmente je Geschoss **inkl. sichtbarer Hilfslinien** + BBox) liegt in `adapters/io/`
   — die UI darf sie **nicht** importieren (Regel B, Adapter→Adapter). Optionen: **(a)** die reine,
   framework-freie Projektion in den **Kern** heben (neuer Driving-Read-Port, z. B. `PlanViewPort`;
   `plan_geometry` + die Export-Adapter ziehen dann aus dem Kern — **eine** Quelle, UI + Export teilen
   sie), **(b)** einen neuen 2D-Read-Port im Kern, der die Projektion neu implementiert (Duplikat-
   Risiko), **(c)** die UI bekommt nur `const Building&` und projiziert selbst (UI-Duplikat).
   **Empfehlung: (a)** — die Projektion ist bereits »pure, kein OCC/Qt« (nur ihr **Wohnort** ist
   export-gekoppelt); Heben in den Kern löst Regel B **und** dedupliziert. Die Export-Seite (032c,
   gerade geliefert) wird auf den Kern-Port umgestellt.
3. **Bildschirm→Modell-Abbildung:** heute existiert **nur** Modell→Bildschirm (3D-MVP); der Canvas
   braucht eine **invertierbare 2D-Sicht-Transformation** (Pan/Zoom einer mm-Ebene) mit einer
   **testbaren Naht** `screenToModel(QPoint) → Point2D` (headless prüfbar ohne Display).
4. **Refresh ohne `op`:** DRW-Mutationen melden **keinen** `op` ([ADR-0018](../../adr/0018-drw-2d-zeichen-daten.md)
   §2; `ModelChangeOp` hat kein DRW-Glied). Der Canvas **ist Treiber und Betrachter zugleich** →
   **Selbst-Refresh** nach dem eigenen erfolgreichen `addGuideLine` (repaint synchron; **kein** neuer
   `op` nötig — die 3D-Sicht braucht Hilfslinien ohnehin nicht). Alternative (DRW-`op` einführen)
   benannt + verworfen (revidierte [ADR-0018](../../adr/0018-drw-2d-zeichen-daten.md) §2 ohne Not).
5. **Kommando-Naht UI→Kern:** heute hält **keine** UI-Klasse einen mutierenden Port (nur der read-only
   `ViewModelMeshSource`). Der Canvas ist der **erste** — der mutierende `EditDrawingPort` wird über
   ein **`ui/command/`-Objekt** vermittelt (slice-029-Disziplin: Driving-Port-Include nur in
   `command/`), nicht direkt im `view/`-Widget.
6. **Fang/Raster/Winkel-Heimat:** die Aids ([LH-FA-DRW-001](../../../../spec/lastenheft.md#modul-zeichnungsfunktionen-drw)/002/003)
   quantisieren die **Eingabe** zu Modell-mm — **UI-Interaktions-Zustand** (Canvas-Widget), **nicht**
   Kern-Modell/Persistenz. Der ADR **entscheidet die Heimat** (UI-resident) und **benennt** die Aids als
   Folge-Slices; ihre AK schärft dieser Slice **nicht**.
7. **Composition-Root/Fenster + Headless-Test:** heute **ein** `setCentralWidget(viewer)`, **kein**
   Menü/Toolbar. Der Canvas braucht ein Layout (Splitter/Stacked + 3D/2D-Umschalter) + zweites
   `subscribe`/`unsubscribe`. **Headless:** simulierte `QMouseEvent` (Muster `test_viewer_widget`,
   [ADR-0010](../../adr/0010-headless-gl-xvfb.md)) + **Surrogat-Assertion** (nach Klick-Zug wächst
   `building().guide_lines` um eins mit den gemappten mm) — der Interaktions-Test, dessen **Bedarf**
   [ADR-0009](../../adr/0009-gui-framework-qt6.md) (f) ankündigte (`QMouseEvent`-Synthese unter der
   Xvfb-Infra [ADR-0010](../../adr/0010-headless-gl-xvfb.md)).

## 2. Definition of Done

- [ ] **Grundsatz-ADR** (`docs/plan/adr/{0019-drw-2d-canvas}.md`, Nummer beim Anlegen fortlaufend;
      Muster [ADR-0018](../../adr/0018-drw-2d-zeichen-daten.md)-Struktur): entscheidet die **sieben
      Fragen** (§1) je mit Pro/Contra + verglichenen Alternativen + Fitness-Function + Re-Eval-Triggern;
      **keine** bestehende Regel gelockert (Regel A/B/E bleiben; falls die 2D-Lese-Naht-Wahl (a) den
      Export auf einen Kern-Port umstellt, ist das eine **Verschärfung/Refactor**, keine Lockerung —
      [§2.6](../../../../AGENTS.md) n/a). **Der Konsequenzen-Abschnitt schreibt die Impl-Sequenz fest
      (Review-MED-1):** die 2D-Lese-Naht-Hebung (Projektion → Kern-Port) ist ein **eigener Impl-Slice
      VOR** dem Canvas-Widget-Slice (§6). **Status Proposed → unabhängiges Text-Review (0 HIGH) +
      Projektinhaber-Accept.** [ADR-Index](../../adr/README.md) + `architecture.md` §1.1/§Geschichte
      nachgezogen (Präzedenz [ADR-0009](../../adr/0009-gui-framework-qt6.md)/[ADR-0018](../../adr/0018-drw-2d-zeichen-daten.md)).
- [ ] **Sequenz innerhalb des Slice (Review-MED-2, 011a-Bündelung):** die **sieben ADR-Entscheidungen
      werden zuerst stabilisiert** (Text-Review 0 HIGH + Projektinhaber-Accept), **dann** der Lastenheft-/
      Spec-Text finalisiert — ein Text-Review-Entscheidungs-Flip (z. B. Lese-Naht a→c) löst eine
      **kontrollierte AK-Nachschärfung** aus, keine stille Inkonsistenz. (Anders als [slice-032a](../done/slice-032a-drw-fundament-ak-spec.md),
      das auf ein **fertiges** ADR aufsetzte.)
- [ ] **Lastenheft [LH-FA-DRW-005](../../../../spec/lastenheft.md#lh-fa-drw-005) interaktive Erzeugungs-AK**
      (lösungsfrei, benutzer-beobachtbar — **kein** Widget-/Port-/Koordinaten-Mechanik-Vokabular, das
      gehört in §1; [MR-008](../../../../harness/conventions.md#mr-008--lastenheft-schärfung-bleibt-lösungsfrei)):
      die 032a-Teilumfang-Klausel »interaktives Setzen offen« wird **eingelöst**. Mindestens:
      **Happy:** Given ein Projekt mit einer sichtbaren Ebene, when der Nutzer auf der 2D-Zeichenfläche
      eine Hilfslinie zieht (Anfang klicken, Ende klicken/loslassen), then erscheint sie **sofort** auf
      der Fläche, ist der Ebene zugeordnet und überlebt Speichern/Laden + 2D-Export.
      **Boundary:** Anfang = Ende (kein Zug) → **keine** Hilfslinie entsteht (Modell unverändert).
      **Negative:** ohne sichtbare/aktive Ebene → das Werkzeug erzeugt nichts bzw. der Nutzer wird auf
      die fehlende Ebene verwiesen (kein stiller Verlust; genaue UX = §1/ADR). + Header-Nachzug
      `lastenheft.md` `**Version:**` == oberste `lastenheft-historie.md`-Zeile
      ([MR-010](../../../../harness/conventions.md#mr-010--lastenheft-header-version--oberste-9-historie-zeile)/[MR-012](../../../../harness/conventions.md#mr-012--mr-010-invariante-folgt-der-ausgelagerten-lastenheft-historie)).
- [ ] **`spec/spezifikation.md` §1** [`LH-FA-DRW-005.a`](../../../../spec/spezifikation.md) um die
      **interaktive Erzeugungs-/Canvas-Naht** ergänzt (im ADR-Rahmen, **ohne** ADR-/Slice-Token im
      Körper — [MR-011](../../../../harness/conventions.md#mr-011--referenz-integritäts-gate-matrix-ids-spans-hostpaths)/[MR-014](../../../../harness/conventions.md#mr-014--referenz-richtungs-verschärfung-adr-nennt-keine-slice-d-check-v0371),
      vor dem Gate greppen): 2D-Zeichenfläche als Driving-Adapter, 2D-Lese-Naht (Kern-Port), Bildschirm→
      Modell-Abbildung, Selbst-Refresh (kein `op`), Fang/Raster als UI-Aids (Heimat benannt). **§6**
      interne Grenz-Vertragszeile 2D-Canvas (Muster DRW-Zeile 032a). **`spezifikation-historie.md`** +
      Header. **Architektur §2**-Regel-E-Nachzug (Qt-Include-Zone unverändert; der Canvas ist UI-Adapter).
- [ ] **[ADR-Index](../../adr/README.md)-Folgepflicht** der neuen ADR (AK-Schärfung + Spec-Nachzug +
      Impl-Folge-Zeilen »offen«, Muster [ADR-0018](../../adr/0018-drw-2d-zeichen-daten.md)). **CHANGELOG**
      ([MR-004](../../../../harness/conventions.md#mr-004--top-level-changelogmd-keep-a-changelog)).
- [ ] **Reine Doku/Entscheidung — kein Code, keine Tests, kein Schema.** `make gates` grün;
      `make schema-check` unberührt. Closure-Notiz. **Nicht Teil:** Impl (Widget/Port/Koordinaten/
      Maus/AK-Tests) = Folge-Slice(s); Fang/Raster/Winkel-AK = eigene Slices.

## 3. Plan (vor Code)

| Datei / Komponente | Änderungs-Art | Begründung |
|---|---|---|
| `docs/plan/adr/{0019-drw-2d-canvas}.md` | neu | DRW-Canvas-Grundsatz-ADR (7 Entscheidungen; Text-Review + Accept) |
| `docs/reviews/{2026-07-2x-adr-drw-canvas-text-review}.md` | neu | unabhängiges ADR-Text-Review (Muster [ADR-0018](../../adr/0018-drw-2d-zeichen-daten.md)-Text-Review) |
| `spec/lastenheft.md` | ändern | [LH-FA-DRW-005](../../../../spec/lastenheft.md#lh-fa-drw-005) interaktive Erzeugungs-AK (lösungsfrei); Header-Version |
| `spec/lastenheft-historie.md` | ändern | oberste Zeile ([MR-010](../../../../harness/conventions.md#mr-010--lastenheft-header-version--oberste-9-historie-zeile)/[MR-012](../../../../harness/conventions.md#mr-012--mr-010-invariante-folgt-der-ausgelagerten-lastenheft-historie)) |
| `spec/spezifikation.md` | ändern | §1 Canvas-/Lese-Naht-Mapping (ADR-/slice-frei), §6 Grenzzeile |
| `spec/spezifikation-historie.md` | ändern | Provenance-Zeile + Header |
| `spec/architecture.md` | ändern | §1.1/§2 Canvas-Driving-Adapter + `## Geschichte`-Provenance (Regel E unverändert) |
| [ADR-Index](../../adr/README.md) | ändern | neue ADR eintragen + Folgepflichten (Impl/Fang-Raster »offen«) |
| `CHANGELOG.md` | ändern | Slice-Eintrag ([MR-004](../../../../harness/conventions.md#mr-004--top-level-changelogmd-keep-a-changelog)) |
| `docs/reviews/2026-07-22-slice-041a-plan.md` | neu (erledigt) | [MR-006](../../../../harness/conventions.md#mr-006--unabhängiges-plan-review-vor-implementierungs-start)-Report (0 HIGH → startbar) |

**Kein Code/Test/Schema** (Impl = Folge-Slice; `schema-check` unberührt).

## 4. Trigger

- **DRW-Fundament 032a→b→c done ✓** (Hilfslinie/Ebene durabel + sichtbar; `EditDrawingPort` +
  `building.guide_lines`/`layers` vorhanden — der Canvas mutiert über den **bestehenden** Port).
- **[ADR-0018](../../adr/0018-drw-2d-zeichen-daten.md) §Re-Eval-Trigger ✓** (interaktiver Editor hierher
  deferiert) + **[ADR-0009](../../adr/0009-gui-framework-qt6.md) (f) ✓** (»Interaktions-Treiber wird dann
  relevant«).
- **[MR-006](../../../../harness/conventions.md#mr-006--unabhängiges-plan-review-vor-implementierungs-start)-Plan-Review
  vor Start** — gelaufen ✓ 2026-07-22, **0 HIGH → startbar** (MED-1/MED-2/LOW-1 eingearbeitet);
  **ADR-Text-Review vor Accept** (Reviewer ≠ Autor) folgt bei der ADR-Erstellung. **Keine HIGH offen.**

## 5. Closure-Trigger

- DoD vollständig, ADR **Accepted** (Text-Review 0 HIGH + Projektinhaber), `make gates` grün, Closure-
  Notiz → **Folge-Impl-Slice(s)** (2D-Canvas-Widget + Kern-2D-Lese-Port + Koordinaten-Abbildung +
  Kommando-Naht + Maus-Handhabung + Composition-Root + Headless-AK-Tests) werden startbar; danach
  Fang/Raster/Winkel als eigene Slices.

## 6. Risiken und offene Punkte

- **Rest-Risiko #1 — 2D-Lese-Naht-Refactor berührt die frische Export-Seite (HIGH-Kandidat):** Wahl (a)
  (Projektion in den Kern heben) stellt die gerade in [slice-032c](../done/slice-032c-drw-export.md)
  gelieferten Export-Adapter auf einen Kern-Port um. Mitigation: die Projektion ist **pure/framework-
  frei** (nur Umzug, keine Logik-Änderung); die 032c-Decode-Orakel bleiben grün (Verhaltens-Invariante).
  Der ADR-Text muss den Refactor als **Konsequenz** führen; ist er zu groß, kann der Impl-Slice die
  Naht **schrittweise** ziehen (erst Kern-Port neben `plan_geometry`, dann Export migrieren). **Das
  Text-Review-Auflösung (Review-MED-1): die **Entscheidung** (a) gehört in den ADR (Architektur-Wahl),
  der **Refactor** aber ist eine cross-cutting Export-Änderung, orthogonal zu den sechs UI-Entscheidungen
  → **der ADR-Konsequenzen-Abschnitt schreibt fest: die Projektions-Hebung ist ein EIGENER Impl-Slice,
  sequenziert VOR dem Canvas-Widget-Slice** (Kern-`PlanViewPort` neben `plan_geometry` → Export-Adapter
  migrieren → 032c-Decode-Orakel grün → **dann** der Canvas). Nicht in einen Canvas-Impl-Slice gefaltet.**
- **Rest-Risiko #2 — Umfang der ADR (7 Entscheidungen):** groß, aber jede ist geerdet (UI-Karte).
  Scope strikt auf **freies Zeichnen einer Hilfslinie**; Fang/Raster/Winkel + Bauteile + Selektion
  deferiert (benannt). Kein stilles Über-Versprechen.
- **Beobachtbarkeit/Lösungsfreiheit ([MR-008](../../../../harness/conventions.md#mr-008--lastenheft-schärfung-bleibt-lösungsfrei)):**
  die AK sind benutzer-beobachtbar (»zeichnet mit der Maus«, »erscheint sofort«, »überlebt Speichern/
  Laden + Export«); Widget-/Port-/Koordinaten-Mechanik gehört in §1/ADR, **nicht** ins Lastenheft.
- **Refresh-ohne-`op` (Design-Prüfstein):** Selbst-Refresh statt DRW-`op` — das Text-Review prüft, ob
  eine zweite Sicht (künftig) ohne `op` synchron bleibt; v1 hat nur den Canvas (self-driven).
- **Headless-Testbarkeit (ADR-Fitness):** der Canvas muss `screenToModel` als testbare Naht + einen
  Surrogat-Zustand (analog `ViewerScene`) exponieren, sonst ist die Maus-AK nur GL-Smoke-prüfbar —
  der ADR muss die Naht **fordern** (Fitness-Function).
- **Spec-Straten ADR-/slice-frei ([MR-011](../../../../harness/conventions.md#mr-011--referenz-integritäts-gate-matrix-ids-spans-hostpaths)/[MR-014](../../../../harness/conventions.md#mr-014--referenz-richtungs-verschärfung-adr-nennt-keine-slice-d-check-v0371)):**
  §1/§6/architecture-Nachzug ohne ADR-/Slice-Token im Körper — vor dem Gate greppen (Lerneintrag 032a/c).
- **Header-Nachzug ([MR-010](../../../../harness/conventions.md#mr-010--lastenheft-header-version--oberste-9-historie-zeile)/[MR-012](../../../../harness/conventions.md#mr-012--mr-010-invariante-folgt-der-ausgelagerten-lastenheft-historie)):**
  die Lastenheft-Schärfung zieht `**Version:**` auf die neue Historie-Zeile nach.

## 7. Sub-Area-Modus-Begründung

### Sub-Area: Spec-Schreibung `spec/`

- **Modus:** GF; **Dichte:** hoch (AK-Format, [MR-008](../../../../harness/conventions.md#mr-008--lastenheft-schärfung-bleibt-lösungsfrei)-
  Lösungsfreiheit, [MR-010](../../../../harness/conventions.md#mr-010--lastenheft-header-version--oberste-9-historie-zeile)/012,
  [MR-011](../../../../harness/conventions.md#mr-011--referenz-integritäts-gate-matrix-ids-spans-hostpaths)-ADR-Freiheit).
  **Phase-Reife:** DRW Phase 2 (interaktive Klausel). **Risiko:** niedrig (reine Doku).

### Sub-Area: Planning-Lifecycle / ADR `docs/plan/`

- **Modus:** GF; **Dichte:** hoch (Grundsatz-ADR mit Text-Review + Projektinhaber-Accept, Muster
  [ADR-0009](../../adr/0009-gui-framework-qt6.md)/[ADR-0018](../../adr/0018-drw-2d-zeichen-daten.md);
  [MR-006](../../../../harness/conventions.md#mr-006--unabhängiges-plan-review-vor-implementierungs-start)
  vor Impl). **Phase-Reife:** 4. **Risiko:** mittel — Architektur-Entscheidung mit Refactor-Konsequenz
  (Rest-Risiko #1).

## 8. Closure-Notiz

*(bei Closure ausgefüllt: ADR-Nummer + Accept-Stand, Lastenheft-Version, Spec-Nachzüge, Review-Ergebnisse
[[MR-006](../../../../harness/conventions.md#mr-006--unabhängiges-plan-review-vor-implementierungs-start)
+ ADR-Text-Review], Lerneintrag, Folge = Impl-Slice(s) + Fang/Raster/Winkel.)*
