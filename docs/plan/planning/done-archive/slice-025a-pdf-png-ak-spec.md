---
id: slice-025a
titel: PDF-/PNG-Export — AK-Schärfung [LH-FA-IO-007](../../../../spec/lastenheft.md#lh-fa-io-007)/008 & Spec-Mapping (parametrisiert auf [ADR-0016](../../adr/0016-pdf-png-backend.md))
status: done
welle: welle-4-austausch
lastenheft_refs: [[LH-FA-IO-007](../../../../spec/lastenheft.md#lh-fa-io-007), [LH-FA-IO-008](../../../../spec/lastenheft.md#lh-fa-io-008)]
adr_refs: [[ADR-0001](../../adr/0001-hexagonale-architektur.md), [ADR-0013](../../adr/0013-ifc-bibliothek.md), [ADR-0015](../../adr/0015-dxf-backend.md), [ADR-0016](../../adr/0016-pdf-png-backend.md)]
---

# Slice 025a: PDF-/PNG-Export — AK-Schärfung & Spec-Mapping

**Status:** done (2026-07-01). [MR-006](../../../../harness/conventions.md#mr-006--unabhängiges-plan-review-vor-implementierungs-start)-Plan-Review **0 HIGH / 2 LOW / 3 INFO** (LOW-1 §6 zwei Zeilen + LOW-2 §7-Chirurgie/Parität eingearbeitet); DoD vollständig, `make gates` grün, Closure-Notiz §8. [Report](../../../reviews/2026-07-01-slice-025a-plan.md). **Reine Doku/Entscheidung, kein Code.**

**Welle:** welle-4-austausch (PDF/PNG-Strang, Entscheidungs-/Spec-Hälfte;
Muster slice-019a [IFC] / slice-020a [STEP/STL] / slice-021a [DXF]).

**Bezug:** [LH-FA-IO-007](../../../../spec/lastenheft.md#lh-fa-io-007) (PDF-Export, maßstäblicher Plan, [ACC-004](../../../../spec/lastenheft.md#7-abnahmekriterien)) +
[LH-FA-IO-008](../../../../spec/lastenheft.md#lh-fa-io-008) (PNG-Export), bisher **Outline**-Stubs. **Parametrisiert auf
[ADR-0016](../../adr/0016-pdf-png-backend.md) (PDF-/PNG-Backend, accepted 2026-07-01)** — das Backend ist
**entschieden** (selbst getragener **Vektor-PDF- + Raster-PNG-Writer, Option D**, wie
IFC/DXF [ADR-0013](../../adr/0013-ifc-bibliothek.md)/[ADR-0015](../../adr/0015-dxf-backend.md); io-resident, kein OCC, kein Qt, kein
Lib-Zukauf, export-only); dieser Slice braucht **keine neue Grundsatz-ADR**, seine
**Mapping-/Mechanik-Entscheidung lebt in Lastenheft-AK + `spezifikation.md` §1**
(Muster slice-019a/020a/021a). [ADR-0001](../../adr/0001-hexagonale-architektur.md) (Kern führt; Format-Writer
adapter-lokal; Port-/Dispatch-Mechanik = Kern-Hoheit, Impl-Slice).

**Autor:** Dietmar Burkard. **Datum:** 2026-07-01.

**Schnitt-Herkunft:** Entscheidungs-/Spec-Hälfte des PDF/PNG-Strangs (Muster
019a/020a/021a). Die PDF/PNG-2D-Plan-Abbildung (was ist ein maßstäblicher Plan,
welche b-cad-Bauteile, Totalität, atomare Ablehnung) braucht **prüfbare AK + ein
entschiedenes Mapping**, bevor implementiert wird. **Reine Doku/Entscheidung, kein
Code.**

---

## 1. Ziel

Die PDF/PNG-Anforderungen [LH-FA-IO-007](../../../../spec/lastenheft.md#lh-fa-io-007) (PDF) und [LH-FA-IO-008](../../../../spec/lastenheft.md#lh-fa-io-008) (PNG)
bekommen **lösungsfreie, benutzer-beobachtbare Akzeptanzkriterien** ([MR-008](../../../../harness/conventions.md#mr-008--lastenheft-schärfung-bleibt-lösungsfrei)) und ein
**entschiedenes Mapping** (`spezifikation.md` §1), bevor implementiert wird —
**innerhalb des von [ADR-0016](../../adr/0016-pdf-png-backend.md) entschiedenen Backends** (self-rolled Writer,
io-resident, export-only, 2D-Achsen-Maßstabsplan). Zusätzlich wird die
**[ADR-0016](../../adr/0016-pdf-png-backend.md)-Folgepflicht** eingelöst: die durch den ADR-Accept **stale**
gewordenen Spec-Stellen (§7 Offene-Backends-Klausel, §6 ohne PDF/PNG-Zeile, §4
[`E-IO-001`](../../../../spec/spezifikation.md#4-fehler-codes-und-logging-felder)-Bedingung) werden nachgezogen.

**PDF/PNG sind 2D-Plan-Render + export-only.** Anders als IFC/DXF (bidirektional)
und wie STEP/STL (export-only, 020a) haben PDF/PNG **nur Export** — man liest **kein**
Gebäudemodell aus einem PDF/PNG zurück. Anders als STEP/STL (geometrie-resident,
weil OCC-nativ) sind sie **io-resident** (self-rolled, reine Byte-Serialisierung —
wie IFC/DXF). Die fachliche Eigenheit: **PDF = maßstäblicher 2D-Plan** ([ACC-004](../../../../spec/lastenheft.md#7-abnahmekriterien)),
**PNG = Rasterbild desselben Plans**.

**Reifephase-Teilumfang welle-4 (= [ADR-0016](../../adr/0016-pdf-png-backend.md)-Subset):** **Achsen-Plan** —
gerade Wand-**Achsen** je Geschoss (dieselbe 2D-Datenquelle wie DXF). Wand-Footprint/
-Dicke, Räume, Bemaßung, Schraffur, Text, Möblierung, 3D-Ansicht bleiben ausdrücklich
offen (benannte Lücke, nicht geschrieben); PNG **unkomprimiert**. **Wand-Footprint/
-Dicke** ist ein **AK-abhängiger Re-Eval-Trigger** ([ADR-0016](../../adr/0016-pdf-png-backend.md) §Trigger).

## 2. Definition of Done

- [x] **Lastenheft [LH-FA-IO-007](../../../../spec/lastenheft.md#lh-fa-io-007) + [LH-FA-IO-008](../../../../spec/lastenheft.md#lh-fa-io-008) von Outline auf AK-Niveau**
      (**lösungsfrei, benutzer-beobachtbar** — **kein** PDF-Operator/PNG-Chunk-Name/
      DEFLATE-Mechanik im Lastenheft-Text, das gehört in §1; [MR-008](../../../../harness/conventions.md#mr-008--lastenheft-schärfung-bleibt-lösungsfrei)):
      **IO-007 PDF** mindestens — **Happy:** Given ein b-cad-Modell mit Geschossen +
      geraden Wänden, when nach PDF exportiert, then entsteht eine **valide PDF-Datei**,
      die ein Standard-PDF-Leser als **maßstäblichen 2D-Grundriss** öffnet (die
      Wand-Achsen je Geschoss); **maßstäblich = benutzer-beobachtbar:** eine auf dem
      Plan gemessene Länge entspricht der Modell-Abmessung über einen **definierten,
      im Plan dokumentierten Maßstab** ([ACC-004](../../../../spec/lastenheft.md#7-abnahmekriterien)). **IO-008 PNG** mindestens —
      **Happy:** Given dasselbe Modell, when nach PNG exportiert, then entsteht eine
      **valide PNG-Datei** (Rasterbild desselben 2D-Grundrisses). **Boundary:** Given
      ein Modell ohne Geschosse/Wände (leer), when exportiert, then entsteht eine
      **gültige, (annähernd) leere** PDF-Seite / PNG-Bild **ohne Absturz**. **Negative:**
      Given ein nicht beschreibbarer Zielpfad, when exportiert, then Fehler-Code
      [`E-IO-001`](../../../../spec/spezifikation.md#4-fehler-codes-und-logging-felder), **kein Teil-Export**; der Zielpfad bleibt unverändert.
      **Teilumfang-Klausel (benutzer-beobachtbar):** der Plan zeigt **gerade Wand-Achsen
      je Geschoss** (Achsen-Plan); Wand-Umrisse mit Dicke/Footprint, Räume, Bemaßung,
      Schraffur, Text, Möblierung, 3D-Ansicht werden **nicht gezeichnet** — ausdrücklich
      offen. **Export-only-Klausel (benutzer-beobachtbar):** PDF/PNG sind **nur Export**
      (kein Import — man liest kein Modell aus einem PDF/PNG zurück). + Header-Nachzug +
      `lastenheft-historie.md` **0.1.12** (**[MR-010](../../../../harness/conventions.md#mr-010--lastenheft-header-version--oberste-9-historie-zeile)/[MR-012](../../../../harness/conventions.md#mr-012--mr-010-invariante-folgt-der-ausgelagerten-lastenheft-historie)**: Header-`Version:` == oberste
      Historie-Zeile).
- [x] **`spec/spezifikation.md` §1 neuer Block [`LH-FA-IO-007.a`](../../../../spec/lastenheft.md#lh-fa-io-007)** (Sammelblock —
      **deckt IO-007 + IO-008**, Muster [`LH-FA-IO-001.a`](../../../../spec/lastenheft.md#lh-fa-io-001--ifc-import)/[`LH-FA-IO-005.a`](../../../../spec/lastenheft.md#lh-fa-io-005)/
      [`LH-FA-IO-003.a`](../../../../spec/lastenheft.md#lh-fa-io-003)): das PDF/PNG-Mapping innerhalb des **[ADR-0016](../../adr/0016-pdf-png-backend.md)-Subsets** —
      **Schicht** io-residente self-rolled Writer (`adapters/io/`, kein OCC, kein Qt —
      `ModelExporterPort`, export-only via `ExporterMap`); der einzige Kern-Touch ist
      die **additive `ExchangeFormat::Pdf`/`Png`-Enum-Erweiterung** (Impl-Slice), **keine**
      Service-/Registry-Architektur-Änderung; **Repräsentation** PDF = maßstäblicher
      Vektor-Plan (Wand-Achsen je Geschoss, Modell-mm → Seiten-Einheit über definierten
      Maßstab, Rahmen), PNG = Raster desselben Plans; **Datenquelle** `building.storeys`
      + `building.walls` (gerade Wand-Achsen, wie DXF); **Maßstab/Seitenformat/PNG-
      Auflösung** (fester Maßstab vs. Fit-to-Page, A4/A3, eine Seite/Bild je Geschoss
      vs. kombiniert, DPI) fixiert der **Impl-Slice** dokumentiert (analog DXF-Profil-
      version 021a — der Maßstab ist im Plan dokumentiert, damit [ACC-004](../../../../spec/lastenheft.md#7-abnahmekriterien) beobachtbar
      bleibt); **atomarer Export** ([`E-IO-001`](../../../../spec/spezifikation.md#4-fehler-codes-und-logging-felder), kein Teil-Export, binär-sicher —
      PDF/PNG sind Byte-Ströme); **export-only** (kein PDF/PNG-Import-Adapter; ein
      Import-*Request* → **bestehende generische** [`E-IO-003`](../../../../spec/spezifikation.md#4-fehler-codes-und-logging-felder)-Zeile als export-only-
      Lookup-Miss, **identisch** STEP/STL, **kein** neuer Code); **Totalität** (leeres
      Modell → gültige leere Seite/Bild, kein Wurf). Mechanik, **kein** Lastenheft. +
      `spezifikation-historie.md` + `**Letzte Änderung:**`-Header.
- [x] **`spec/spezifikation.md` §6 + §7 + §4 nachgezogen ([ADR-0016](../../adr/0016-pdf-png-backend.md)-Folgepflicht):**
      **§7** ist ein **Sammel-Bullet**, das IFC/STEP-STL/DXF als **entschieden** aufzählt
      und mit „— **PDF-/PNG-Backends bleiben offen** (welle-4)" endet → **nur den
      offen-Schwanz** streichen (den decided-Enumerations-Bullet **erhalten**, er trägt
      die Provenance) und **symmetrisch** „PDF/PNG-Backend entschieden (§1
      [`LH-FA-IO-007.a`](../../../../spec/lastenheft.md#lh-fa-io-007))" ergänzen (Parität DXF/STEP-STL, 021a-Muster; Review-LOW-2);
      danach ist **kein IO-Backend mehr offen**. Die übrigen §7-Punkte
      (Performance-Raumerkennung, Zielplattformen) bleiben. **§6** hat **keine**
      PDF/PNG-Zeile → **zwei neue** Vertragszeilen **anlegen** (PDF + PNG, [ADR-0016](../../adr/0016-pdf-png-backend.md)-/
      Index-Wortlaut „zwei neue Vertragszeilen"; je self-rolled Writer io-resident,
      export-only, §1 [`LH-FA-IO-007.a`](../../../../spec/lastenheft.md#lh-fa-io-007)). Ein kombinierter „PDF / PNG"-Row
      (STEP/STL-Präzedenz) nur mit **expliziter Closure-Begründung** (Review-LOW-1). **§4**
      [`E-IO-001`](../../../../spec/spezifikation.md#4-fehler-codes-und-logging-felder)-Bedingung um „PDF-/PNG-Export" erweitern (zählt heute „IFC-/STEP-/
      STL-/DXF-Export"). ADR-Index-Folgepflicht-Zeile ([ADR-0016](../../adr/0016-pdf-png-backend.md) „AK-Schärfung +
      Spec-Nachzug") abhaken (im Closure-Commit).
- [x] **`spec/architecture.md`:** voraussichtlich **unverändert** — `ExchangeModelPort`/
      `ModelExporterPort`/`adapters/io/` sind deklariert; **optional** eine
      `## Geschichte`-Provenance-Zeile „PDF/PNG Export → [ADR-0016](../../adr/0016-pdf-png-backend.md)"
      (exclude-section, Link erlaubt) — sonst bewusst nicht geändert (Begründung in
      Closure). **`ExchangeFormat`-Enum-Erweiterung** (`Pdf`/`Png`) ist **Impl-Slice**,
      nicht hier.
- [x] **Reine Doku/Entscheidung — kein Code, keine Tests, kein ADR, kein Schema**
      ([ADR-0016](../../adr/0016-pdf-png-backend.md) deckt das Backend; Mapping ist Spec-Entscheidung). `make gates`
      grün; `make schema-check` unberührt; Closure-Notiz mit Lerneintrag. **Nicht Teil:**
      der **PDF/PNG-Impl-Slice** (self-rolled `PdfWriter`/`PngWriter` +
      `PdfExportAdapter`/`PngExportAdapter` + `ExchangeFormat::Pdf`/`Png`-Enum-Erweiterung
      + `ExporterMap`-Verdrahtung + AK-Tests mit **voll-Decode-Orakel** + Maßstabs-Sonde +
      Adapter-Pfad-Integration + `make io-smoke`-Erweiterung).

## 3. Plan (vor Code)

| Datei / Komponente | Änderungs-Art | Begründung |
|---|---|---|
| `spec/lastenheft.md` | ändern | [LH-FA-IO-007](../../../../spec/lastenheft.md#lh-fa-io-007)/008 Outline → AK (lösungsfrei, export-only, Achsen-Plan-Teilumfang, maßstäblich); Header `Version:` → 0.1.12 |
| `spec/lastenheft-historie.md` | ändern | oberste Zeile 0.1.12 ([MR-010](../../../../harness/conventions.md#mr-010--lastenheft-header-version--oberste-9-historie-zeile)/[MR-012](../../../../harness/conventions.md#mr-012--mr-010-invariante-folgt-der-ausgelagerten-lastenheft-historie)-Invariante) |
| `spec/spezifikation.md` | ändern | §1 [`LH-FA-IO-007.a`](../../../../spec/lastenheft.md#lh-fa-io-007) PDF/PNG-2D-Plan-Mapping; §6 neue PDF/PNG-Zeile(n); §7 PDF/PNG chirurgisch (Klausel wird leer); §4 [`E-IO-001`](../../../../spec/spezifikation.md#4-fehler-codes-und-logging-felder) um PDF/PNG-Export |
| `spec/spezifikation-historie.md` | ändern | Provenance-Zeile (slice-025a) |
| `spec/architecture.md` | ggf. ändern | optional `## Geschichte`-Provenance „PDF/PNG → [ADR-0016](../../adr/0016-pdf-png-backend.md)"; sonst unverändert |
| `docs/plan/adr/README.md` | ändern (Closure) | [ADR-0016](../../adr/0016-pdf-png-backend.md)-Folgepflicht „AK-Schärfung + Spec-Nachzug" → erfüllt |
| `docs/reviews/{2026-07-01-slice-025a-plan}.md` | neu | [MR-006](../../../../harness/conventions.md#mr-006--unabhängiges-plan-review-vor-implementierungs-start)-Report |

## 4. Trigger

- Keiner — **startbar** nach [MR-006](../../../../harness/conventions.md#mr-006--unabhängiges-plan-review-vor-implementierungs-start)-Review: reine Doku/Entscheidung; das
  PDF/PNG-Backend ist mit [ADR-0016](../../adr/0016-pdf-png-backend.md) (accepted) entschieden, die IO-Ports
  (`ExchangeModelPort`/`ModelExporterPort`, `ExporterMap`) sind real (slice-019c/020b/021b).

## 5. Closure-Trigger

- DoD vollständig, `make gates` grün, Closure-Notiz mit Lerneintrag →
  der **PDF/PNG-Impl-Slice** (025b) wird startbar.

## 6. Risiken und offene Punkte

- **Lösungsfreiheit des Lastenhefts ([MR-008](../../../../harness/conventions.md#mr-008--lastenheft-schärfung-bleibt-lösungsfrei)):** „valide PDF-Datei, von einem
  Standard-Leser als maßstäblicher 2D-Grundriss geöffnet" + „gemessene Länge
  entspricht der Modell-Abmessung über den dokumentierten Maßstab" sind benutzer-
  beobachtbar; **PDF-Operatoren** (`m`/`l`/`S`), **PNG-Chunks** (`IHDR`/`IDAT`),
  **stored-DEFLATE/Adler-32/CRC-32** und die **Writer-Mechanik** sind Mechanik und
  gehören in §1, **nicht** ins Lastenheft. Teilumfang benutzer-beobachtbar („Räume/
  Bemaßung werden nicht gezeichnet"), nicht über Operator-/Chunk-Listen.
- **[ACC-004](../../../../spec/lastenheft.md#7-abnahmekriterien)-Erfüllbarkeit des Achsen-Plans (Owner-LOW, [ADR-0016](../../adr/0016-pdf-png-backend.md)):** der Achsen-Plan-
  Teilumfang muss den Lastenheft-Wortlaut „maßstäblicher PDF-Plan" ([ACC-004](../../../../spec/lastenheft.md#7-abnahmekriterien),
  M4-bindend) tragen. Dieser Slice **bestätigt explizit**, dass ein maßstäblicher
  Achsen-Plan [ACC-004](../../../../spec/lastenheft.md#7-abnahmekriterien) erfüllt (der Maßstab ist die milestone-tragende Eigenschaft,
  nicht der Wand-Umriss); **Wand-Footprint/-Dicke** = **out of scope + AK-abhängiger
  Re-Eval-Trigger** ([ADR-0016](../../adr/0016-pdf-png-backend.md) §Trigger). Falls das Review dies als **nicht**
  vom Achsen-Plan erfüllbar sieht → HIGH (Scope-Nachschärfung vor Impl).
- **Maßstab als „Was" vs. „Wie" ([MR-008](../../../../harness/conventions.md#mr-008--lastenheft-schärfung-bleibt-lösungsfrei)):** dass der Plan **maßstäblich** ist und
  der Maßstab **dokumentiert** wird, ist das **Was** (Lastenheft, [ACC-004](../../../../spec/lastenheft.md#7-abnahmekriterien)-tragend);
  der **konkrete** Maßstab (z. B. 1:100), Seitenformat, DPI sind **Wie** (§1 bzw.
  Impl-Slice, analog DXF-Profilversion 021a).
- **Header-Versions-Nachzug ([MR-010](../../../../harness/conventions.md#mr-010--lastenheft-header-version--oberste-9-historie-zeile)/[MR-012](../../../../harness/conventions.md#mr-012--mr-010-invariante-folgt-der-ausgelagerten-lastenheft-historie)):** `**Version:**` der `lastenheft.md`
  auf **0.1.12** == oberste `lastenheft-historie.md`-Zeile nachziehen.
- **Spec-Stale-Nachzug ([ADR-0016](../../adr/0016-pdf-png-backend.md)-Folgepflicht):** **§7** ist ein **Sammel-Bullet**
  (IFC/STEP-STL/DXF **entschieden** + „PDF/PNG bleiben offen") → **nur den
  offen-Schwanz** streichen (Bullet erhalten) und symmetrisch „PDF/PNG entschieden
  (§1 [`LH-FA-IO-007.a`](../../../../spec/lastenheft.md#lh-fa-io-007))" ergänzen (Parität DXF, Review-LOW-2); danach **kein
  IO-Backend mehr offen**. **§6** legt **zwei** PDF/PNG-Vertragszeilen an (Review-LOW-1;
  ADR-entschieden → [MR-001](../../../../harness/conventions.md#mr-001--source-precedence-mit-eigener-spezifikations-schicht)-erlaubte ADR→Spec-Richtung). **§4** [`E-IO-001`](../../../../spec/spezifikation.md#4-fehler-codes-und-logging-felder)-Bedingung
  um „PDF-/PNG-Export".
- **Spec-Straten ADR-frei ([MR-011](../../../../harness/conventions.md#mr-011--referenz-integritäts-gate-matrix-ids-spans-hostpaths)):** die §1-/§6-Einarbeitung der frischen
  [ADR-0016](../../adr/0016-pdf-png-backend.md)-Entscheidung darf **keinen** ADR-Verweis im `spezifikation.md`-Körper
  hinterlassen (Provenance nur in `spezifikation-historie.md`) — vor dem Gate per
  `grep ADR-` selbst fangen (Muster 019a/020a/021a-Lerneintrag).
- **Subset-Treue zu [ADR-0016](../../adr/0016-pdf-png-backend.md) (kein Über-Versprechen):** die §1-Mechanik behauptet
  nur, was [ADR-0016](../../adr/0016-pdf-png-backend.md) entschied (gerade Wand-Achsen 2D je Geschoss, maßstäblich,
  export-only, self-rolled io-resident, PNG unkomprimiert). Footprint/Räume/Bemaßung/
  Schraffur/Text/3D = benannte Lücke.

## 7. Sub-Area-Modus-Begründung

### Sub-Area: Spec-Schreibung

- **Modus:** GF; **Konventionen-Dichte:** hoch — AK-Format (Happy/Boundary/Negative),
  Reifephase-/Teilumfang-Klausel, [MR-008](../../../../harness/conventions.md#mr-008--lastenheft-schärfung-bleibt-lösungsfrei) (lösungsfrei), [MR-010](../../../../harness/conventions.md#mr-010--lastenheft-header-version--oberste-9-historie-zeile)/012 (Header ==
  Historie), Fehler-Code-Konvention ([`E-IO-001`](../../../../spec/spezifikation.md#4-fehler-codes-und-logging-felder)/[`E-IO-003`](../../../../spec/spezifikation.md#4-fehler-codes-und-logging-felder) bestehen). **Phase-Reife:**
  IO Phase 2 (Outline → AK). **Evidenz-/Diskrepanz-Risiko:** niedrig (reine Doku;
  [ADR-0016](../../adr/0016-pdf-png-backend.md) trägt die Backend-Entscheidung). **Reconciliation:** keiner; Folge-Slice
  = PDF/PNG-Impl (025b).

### Sub-Area: Planning-Lifecycle

- **Modus:** GF; **Dichte:** hoch ([ADR-0016](../../adr/0016-pdf-png-backend.md)-Leitplanke: kein neuer Grundsatz-ADR;
  Reifephase-Teilumfang wie IFC/STEP-STL/DXF; [MR-006](../../../../harness/conventions.md#mr-006--unabhängiges-plan-review-vor-implementierungs-start) vor Impl). **Phase-Reife:** Phase 4.
  **Risiko:** niedrig.

## 8. Closure-Notiz

**Closure-Kriterien (beobachtbar, 2026-07-01):**

- **Lastenheft 0.1.12:** [`LH-FA-IO-007`](../../../../spec/lastenheft.md#lh-fa-io-007) (PDF) +
  [`LH-FA-IO-008`](../../../../spec/lastenheft.md#lh-fa-io-008) (PNG) von Outline auf AK-Niveau
  (**export-only, 2D-Achsen-Maßstabsplan**, lösungsfrei/benutzer-beobachtbar); **maßstäblich**
  = gemessene Plan-Länge == Modell-Abmessung über dokumentierten Maßstab ([ACC-004](../../../../spec/lastenheft.md#7-abnahmekriterien));
  **Teilumfang gerade Wand-Achsen je Geschoss** explizit; **export-only** (kein Import).
  Header 0.1.12 == oberste (jüngste) `lastenheft-historie.md`-Zeile ([MR-010](../../../../harness/conventions.md#mr-010--lastenheft-header-version--oberste-9-historie-zeile)/012).
- **`spezifikation.md` §1 [`LH-FA-IO-007.a`](../../../../spec/lastenheft.md#lh-fa-io-007)** (Sammelblock IO-007+008):
  self-rolled Vektor-PDF-/Raster-PNG-Writer **io-resident** (kein OCC, kein Qt), gerade
  Wand-Achsen je Geschoss (Datenquelle wie DXF), PDF maßstäblich (Maßstab/Seitenformat/DPI =
  Impl-Slice, dokumentiert), **export-only** (kein Import-Adapter; Import-Request → generische
  [`E-IO-003`](../../../../spec/spezifikation.md#4-fehler-codes-und-logging-felder)) / atomarer Export ([`E-IO-001`](../../../../spec/spezifikation.md#4-fehler-codes-und-logging-felder),
  binär-sicher), Subset-Lücke/Totalität. **§6** zwei neue PDF/PNG-Vertragszeilen (Review-LOW-1),
  **§7** PDF/PNG chirurgisch aus dem decided-Bullet (nur offen-Schwanz gestrichen, symmetrische
  „PDF/PNG entschieden (§1 [`LH-FA-IO-007.a`](../../../../spec/lastenheft.md#lh-fa-io-007))"-Nennung ergänzt, Review-LOW-2) →
  **alle IO-Backends entschieden**, **§4** [`E-IO-001`](../../../../spec/spezifikation.md#4-fehler-codes-und-logging-felder)-Bedingung um PDF/PNG-Export.
  `**Letzte Änderung:**` + `spezifikation-historie.md`.
- **Spec-Straten ADR-frei ([MR-011](../../../../harness/conventions.md#mr-011--referenz-integritäts-gate-matrix-ids-spans-hostpaths)):** der `spezifikation.md`-Körper trägt **keinen**
  ADR-Verweis (per `grep ADR-` selbst geprüft; Provenance nur in `*-historie.md`);
  `architecture.md` `## Geschichte` um PDF/PNG→[ADR-0016](../../adr/0016-pdf-png-backend.md)-Provenance ergänzt (exclude-section,
  Parität DXF/STEP-STL, Review-INFO-3).
- **Kein Code/Test/ADR/Schema**; `make gates` grün; ADR-Index-Folgepflicht „AK-Schärfung +
  Spec-Nachzug" ([ADR-0016](../../adr/0016-pdf-png-backend.md)) abgehakt.

**Lerneintrag (benannte Spec-Lücke + geschärfte Praxis):**

- **Benannte Spec-Lücke (Achsen-Plan vs. „maßstäblicher Plan"):** der PDF-Plan ist ein **Achsen-Plan-
  Teilumfang** (gerade Wand-Achsen je Geschoss); [ACC-004](../../../../spec/lastenheft.md#7-abnahmekriterien) „maßstäblicher Plan" ist
  über die **Maßstäblichkeit** (nicht den Wand-Umriss) erfüllt. **Wand-Footprint/-Dicke** =
  out of scope + AK-abhängiger Re-Eval-Trigger ([ADR-0016](../../adr/0016-pdf-png-backend.md) §Trigger). PNG unkomprimiert,
  3D-Screenshot benannte Lücke (viewer-resident).
- **Schwester-Backend-Symmetrie (geschärfte Praxis):** PDF/PNG ist der **dritte Option-D-Fall**
  (wie IFC/DXF: nicht-nativ, self-rolled io-resident), aber **export-only wie STEP/STL** — die
  §1-Schärfung mischt beide Muster (io-resident Schicht wie DXF-021a, Export-only-AK wie
  STEP/STL-020a). Die §7-Chirurgie schloss die **letzte** Backend-Offen-Klausel → **alle
  IO-Backends entschieden** (M4-Format-Vollständigkeit dokumentiert).
- **Was-vs-Wie beim Maßstab (Lösungsfreiheits-Grenzfall, [MR-008](../../../../harness/conventions.md#mr-008--lastenheft-schärfung-bleibt-lösungsfrei)):** „maßstäblich + dokumentierter Maßstab" ist das
  benutzer-beobachtbare **Was** (Lastenheft, [ACC-004](../../../../spec/lastenheft.md#7-abnahmekriterien)-tragend); der **konkrete** Maßstab/
  Seitenformat/DPI ist **Wie** (§1/Impl-Slice, analog DXF-Profilversion 021a) — hält die
  milestone-kritische AK prüfbar **ohne** Mechanik ins Lastenheft zu ziehen.

**Restrisiko / Nachfolge:** **PDF/PNG-Impl-Slice (025b)** wird startbar — self-rolled
`PdfWriter`/`PngWriter` + `PdfExportAdapter`/`PngExportAdapter` + additive `ExchangeFormat::Pdf`/
`Png`-Enum-Erweiterung + `ExporterMap`-Verdrahtung + AK-Tests mit **voll-Decode-Orakel**
(PDF `xref`/`trailer`/Stream-`/Length`; PNG `IDAT`-Inflate + Adler-32 + CRC-32 + Scanline-
Filterbytes) + Maßstabs-Sonde + Adapter-Pfad-Integration + `make io-smoke`-Erweiterung —
[MR-006](../../../../harness/conventions.md#mr-006--unabhängiges-plan-review-vor-implementierungs-start)-Review davor. Danach Welle-4-Verifikation + Carveout-Audit → `done/welle-4-results.md` → **M4**.
