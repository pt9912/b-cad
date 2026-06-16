---
id: slice-019a
titel: IFC-Import/-Export — AK-Schärfung [LH-FA-IO-001](../../../../spec/lastenheft.md#lh-fa-io-001--ifc-import)/002 & Spec-Mapping (Subset, parametrisiert auf [ADR-0013](../../adr/0013-ifc-bibliothek.md))
status: in-progress
welle: welle-4-austausch
lastenheft_refs: [[LH-FA-IO-001](../../../../spec/lastenheft.md#lh-fa-io-001--ifc-import), [LH-FA-IO-002](../../../../spec/lastenheft.md#lh-fa-io-002)]
adr_refs: [[ADR-0001](../../adr/0001-hexagonale-architektur.md), [ADR-0002](../../adr/0002-geometrie-kern-opencascade.md), [ADR-0013](../../adr/0013-ifc-bibliothek.md)]
---

# Slice 019a: IFC-Import/-Export — AK-Schärfung & Spec-Mapping

**Status:** in-progress (2026-06-16). [MR-006](../../../../harness/conventions.md#mr-006--unabhängiges-plan-review-vor-implementierungs-start)-Plan-Review gelaufen
(**0 HIGH / 0 MED / 3 LOW**; implementierungs-bereit, Start nicht blockiert) —
LOW eingearbeitet.

**Plan-Review ([MR-006](../../../../harness/conventions.md#mr-006--unabhängiges-plan-review-vor-implementierungs-start)):**
[Report](../../../reviews/2026-06-16-slice-019a-plan.md) — **keine HIGH/MED**
(alle Vertrags-Behauptungen gegen die kanonischen Quellen verifiziert; Subset-
Treue zu [ADR-0013](../../adr/0013-ifc-bibliothek.md), [MR-008](../../../../harness/conventions.md#mr-008--lastenheft-schärfung-bleibt-lösungsfrei)-Lösungsfreiheit, [MR-010](../../../../harness/conventions.md#mr-010--lastenheft-header-version--oberste-9-historie-zeile)/012-Header-Nachzug,
§6/§7-„präzisiert"-Richtung bestätigt). Eingearbeitet: LOW-1 (`spezifikation.md`
`**Letzte Änderung:**`-Header beim Edit nachziehen — kein Gate), LOW-2 (Boundary-AK
für IO-001 als **Zuwachs** ausgewiesen), LOW-3 ([`LH-FA-IO-001.a`](../../../../spec/lastenheft.md#lh-fa-io-001--ifc-import) ist ein
**Sammelblock** und deckt den Export IO-002 mit unter dem Import-Anker). INFO-1
(auto-verlinkte Frontmatter-IDs = `ids`-Gate-Artefakt, kein Defekt), INFO-2 (Split
nicht erzwungen — in einer Sitzung review-bar).

**Welle:** welle-4-austausch (erster Slice; erster der IFC-Familie).

**Bezug:** [LH-FA-IO-001](../../../../spec/lastenheft.md#lh-fa-io-001--ifc-import) (IFC-Import, im Lastenheft mit knapper Happy/Negative-AK),
[LH-FA-IO-002](../../../../spec/lastenheft.md#lh-fa-io-002) (IFC-Export, bisher **Outline**-Stub). **Parametrisiert auf
[ADR-0013](../../adr/0013-ifc-bibliothek.md) (IFC-Bibliothek, accepted 2026-06-16)** — das IFC-Backend ist
**entschieden** (selbst getragener IFC-SPF-Subset-Codec im IO-Adapter, Option D);
dieser Slice braucht **keine neue Grundsatz-ADR**, seine **Mapping-/Mechanik-
Entscheidung lebt in Lastenheft-AK + `spezifikation.md` §1** (Muster slice-014a/
slice-013a). [ADR-0002](../../adr/0002-geometrie-kern-opencascade.md) (OCC = STEP/STL, **nicht** IFC — disqualifiziert, [ADR-0013](../../adr/0013-ifc-bibliothek.md)
§Alternativen C), [ADR-0001](../../adr/0001-hexagonale-architektur.md) (Kern führt; Format-Backend adapter-lokal; Port-
Signatur = Kern-Hoheit, slice-019b).

**Autor:** Dietmar Burkard. **Datum:** 2026-06-16.

**Schnitt-Herkunft:** Entscheidungs-/Spec-Hälfte des IFC-Strangs (Muster
013a/014a/017a): die IFC-Subset-Abbildung (welche Entitäten, welche b-cad-
Bauteile, atomare Ablehnung) braucht **prüfbare AK + ein entschiedenes
Mapping**, bevor implementiert wird (slice-019b). **Reine Doku/Entscheidung,
kein Code.**

---

## 1. Ziel

Die IFC-Anforderungen [LH-FA-IO-001](../../../../spec/lastenheft.md#lh-fa-io-001--ifc-import) (Import) und [LH-FA-IO-002](../../../../spec/lastenheft.md#lh-fa-io-002) (Export) bekommen
**lösungsfreie, benutzer-beobachtbare Akzeptanzkriterien** ([MR-008](../../../../harness/conventions.md#mr-008--lastenheft-schärfung-bleibt-lösungsfrei)) und ein
**entschiedenes Mapping** (`spezifikation.md` §1), bevor implementiert wird —
**innerhalb des von [ADR-0013](../../adr/0013-ifc-bibliothek.md) entschiedenen Subsets**. Zusätzlich wird die
[ADR-0013](../../adr/0013-ifc-bibliothek.md)-**Folgepflicht** eingelöst: die durch den ADR-Accept **stale**
gewordenen Spec-Stellen (§6 Vertragstabelle „IFC Schema-Version offen", §7
Offene Punkte) werden nachgezogen.

**Reifephase-Teilumfang welle-4 (= [ADR-0013](../../adr/0013-ifc-bibliothek.md)-Subset):** die **räumliche Struktur**
(Projekt/Gelände/Gebäude/Geschosse) und **gerade, achsen-getragene Wände**.
Türen/Fenster/Dach/Decken/Treppen, beliebige Geometrie und Property-Sets bleiben
ausdrücklich offen (benannte Lücke, Muster ROF-Rechteck-Teilumfang / WAL-006).

## 2. Definition of Done

- [ ] **Lastenheft [LH-FA-IO-001](../../../../spec/lastenheft.md#lh-fa-io-001--ifc-import) + [LH-FA-IO-002](../../../../spec/lastenheft.md#lh-fa-io-002) auf AK-Niveau geschärft**
      (**lösungsfrei, benutzer-beobachtbar** — **keine** IFC-Entitätsnamen/
      Encoding/Mapping-Mechanik im Lastenheft-Text, die gehören in §1; [MR-008](../../../../harness/conventions.md#mr-008--lastenheft-schärfung-bleibt-lösungsfrei)):
      je Happy/Boundary/Negative über *sichtbare* Ergebnisse.
      **IO-001 Import** mindestens: valide IFC-Datei mit Geschossen + Wänden →
      entsprechende b-cad-Geschosse/Wände entstehen, **Anzahl stimmt mit der
      Quelle überein** (bestehende Happy-AK erhalten/geschärft); **Boundary
      (neu — IO-001 trug bisher nur Happy + Negative, LOW-2):** leere/strukturlose
      IFC-Datei → leeres/teil-leeres Modell ohne Absturz;
      **Negative:** nicht-IFC-Datei → [`E-IO-003`](../../../../spec/spezifikation.md#4-fehler-codes-und-logging-felder), **kein Teil-Import** (bestehende
      Negative-AK). **IO-002 Export** mindestens: Happy (Modell → IFC-Datei, die
      re-importiert dieselbe Geschoss-/Wand-Anzahl ergibt — **Roundtrip
      beobachtbar**); Negative (nicht beschreibbarer Zielpfad → [`E-IO-001`](../../../../spec/spezifikation.md#4-fehler-codes-und-logging-felder), kein
      Teil-Export). **Teilumfang-Klausel (benutzer-beobachtbar):** Import/Export
      decken **Geschosse + gerade Wände**; weitere Bauteile (Türen/Fenster/Dach/
      Decken/Treppen) werden **übersprungen bzw. nicht geschrieben** — ausdrücklich
      offen, kein stiller Vollumfang. + Header-Nachzug + `lastenheft-historie.md`
      0.1.8 (**[MR-010](../../../../harness/conventions.md#mr-010--lastenheft-header-version--oberste-9-historie-zeile)/[MR-012](../../../../harness/conventions.md#mr-012--mr-010-invariante-folgt-der-ausgelagerten-lastenheft-historie)**: Header-`Version:` == oberste Historie-Zeile).
- [ ] **`spec/spezifikation.md` §1 neuer Block [`LH-FA-IO-001.a`](../../../../spec/lastenheft.md#lh-fa-io-001--ifc-import)** (Sammelblock —
      **deckt IO-001 + IO-002**, Muster [`LH-FA-ROF-001.a`](../../../../spec/lastenheft.md#lh-fa-rof-001--satteldach)): das IFC-Mapping
      innerhalb des **[ADR-0013](../../adr/0013-ifc-bibliothek.md)-Subsets** — **Encoding** IFC-SPF (ISO 10303-21);
      **Entitäts-Subset** räumliche Komposition `IfcProject`→`IfcSite` (optional)
      →`IfcBuilding`→`IfcBuildingStorey` (über `IfcRelAggregates`) + **gerade
      Wände** (Achs-Repräsentation + Dicke via `IfcMaterialLayerSetUsage`/
      `IfcMaterialLayerSet` + Höhe); **Export** schreibt **IFC4** (`IfcWall` +
      `IfcMaterialLayerSetUsage`, **nicht** das in IFC4 deprecated
      `IfcWallStandardCase`), **Import** akzeptiert **IFC4 und IFC2x3** (beide
      Wand-Formen); **atomarer Import** (vollständiges In-Memory-Modell, jeder
      Parse-/Format-/unbekannte-Pflicht-Entität-Fehler → [`E-IO-003`](../../../../spec/spezifikation.md#4-fehler-codes-und-logging-felder), **kein
      Teil-Import**); **Totalität** (degenerierte/leere Datei → leeres Modell,
      kein Wurf). Mechanik, **kein** Lastenheft. + `spezifikation-historie.md` +
      `**Letzte Änderung:**`-Header der `spezifikation.md` nachziehen (LOW-1, kein Gate).
- [ ] **`spec/spezifikation.md` §6 + §7 nachgezogen ([ADR-0013](../../adr/0013-ifc-bibliothek.md)-Folgepflicht):**
      §6-Vertragstabelle IFC-Zeile „Schema-Version offen (ADR-Folge)" → auf den
      **entschiedenen Stand** (IFC4-Export / IFC2x3+4-Import-Subset, [ADR-0013](../../adr/0013-ifc-bibliothek.md));
      §7-Offene-Punkte-Zeile „IFC-Schema-Version und -Bibliothek" **gestrichen**
      (entschieden). ADR-Index-Folgepflicht-Zeile abhaken (im Closure-Commit).
- [ ] **`spec/architecture.md`:** voraussichtlich **unverändert** — `ExchangeModelPort`/
      `ModelImporterPort`/`ModelExporterPort`/`ExchangeService`/`adapters/io/` +
      der IFC-Import-Use-Case (§4) sind **bereits deklariert**; falls die §1-Spec-
      Entscheidung doch eine Doku-Präzisierung impliziert, mit Begründung — sonst
      bewusst nicht geändert (Port-Signatur ist slice-019b; Lösungsfreiheit der
      Ebenen).
- [ ] **Reine Doku/Entscheidung — kein Code, keine Tests, kein ADR** ([ADR-0013](../../adr/0013-ifc-bibliothek.md)
      deckt das Backend; das Mapping ist eine Spec-Entscheidung). `make gates`
      grün; `make schema-check` unberührt (kein Schema-Bezug); Closure-Notiz mit
      Lerneintrag. **Nicht Teil dieser DoD:** slice-019b (IFC-Impl: `ExchangeService`
      + `ModelImporterPort`/`ModelExporterPort` + SPF-Subset-Codec + AK-Tests
      [`LH-FA-IO-001`](../../../../spec/lastenheft.md#lh-fa-io-001--ifc-import)/`002` **+ Adapter-Pfad-Integrationstest** + neue
      `arch-check`-io-Regel) und die Welle-Closure.

## 3. Plan (vor Code)

| Datei / Komponente | Änderungs-Art | Begründung |
|---|---|---|
| `spec/lastenheft.md` | ändern | [LH-FA-IO-001](../../../../spec/lastenheft.md#lh-fa-io-001--ifc-import) schärfen + [LH-FA-IO-002](../../../../spec/lastenheft.md#lh-fa-io-002) Outline → AK (lösungsfrei, Teilumfang-Klausel); Header `Version:` → 0.1.8 |
| `spec/lastenheft-historie.md` | ändern | oberste Zeile 0.1.8 ([MR-010](../../../../harness/conventions.md#mr-010--lastenheft-header-version--oberste-9-historie-zeile)/[MR-012](../../../../harness/conventions.md#mr-012--mr-010-invariante-folgt-der-ausgelagerten-lastenheft-historie)-Invariante) |
| `spec/spezifikation.md` | ändern | §1 [`LH-FA-IO-001.a`](../../../../spec/lastenheft.md#lh-fa-io-001--ifc-import) IFC-Subset-Mapping; §6 IFC-Vertragszeile auf entschiedenen Stand; §7 IFC-Offene-Punkte-Zeile streichen |
| `spec/spezifikation-historie.md` | ändern | Provenance-Zeile (slice-019a) |
| `docs/plan/adr/README.md` | ändern (Closure) | [ADR-0013](../../adr/0013-ifc-bibliothek.md)-Folgepflicht „Spec-§6/§7-Nachzug" → erfüllt |
| `docs/reviews/{2026-06-16-slice-019a-plan}.md` | neu | [MR-006](../../../../harness/conventions.md#mr-006--unabhängiges-plan-review-vor-implementierungs-start)-Report |

## 4. Trigger

- Keiner — **sofort startbar** nach [MR-006](../../../../harness/conventions.md#mr-006--unabhängiges-plan-review-vor-implementierungs-start)-Review: reine Doku/Entscheidung; das
  IFC-Backend ist mit [ADR-0013](../../adr/0013-ifc-bibliothek.md) (accepted) entschieden, die IO-Ports
  (`ExchangeModelPort`/Importer/Exporter) sind in `architecture.md` deklariert.

## 5. Closure-Trigger

- DoD vollständig, `make gates` grün, Closure-Notiz mit Lerneintrag →
  **slice-019b** (IFC-Implementierung) wird startbar.

## 6. Risiken und offene Punkte

- **Lösungsfreiheit des Lastenhefts ([MR-008](../../../../harness/conventions.md#mr-008--lastenheft-schärfung-bleibt-lösungsfrei), Wiederholungsfall):** „eine IFC-Datei
  mit Wänden/Geschossen → Bauteile entstehen, Anzahl stimmt" ist
  benutzer-beobachtbar; **IFC-Entitätsnamen** (`IfcWallStandardCase`,
  `IfcBuildingStorey`), das **SPF-Encoding** und die **Mapping-Regel** sind
  Mechanik und gehören in §1, **nicht** ins Lastenheft. Der Teilumfang ist
  benutzer-beobachtbar zu formulieren („Türen/Fenster werden beim Import
  übersprungen"), nicht über Entitäts-Listen.
- **Header-Versions-Nachzug ([MR-010](../../../../harness/conventions.md#mr-010--lastenheft-header-version--oberste-9-historie-zeile)/[MR-012](../../../../harness/conventions.md#mr-012--mr-010-invariante-folgt-der-ausgelagerten-lastenheft-historie)):** der `**Version:**`-Header der
  `lastenheft.md` muss auf **0.1.8** == oberste Zeile der `lastenheft-historie.md`
  nachgezogen werden (die Klasse, die 013a/014a/015a still verfehlten). Im Review
  als Quellen-Konsistenz-Linse prüfen.
- **Spec-Stale-Nachzug ([ADR-0013](../../adr/0013-ifc-bibliothek.md)-Folgepflicht):** §6/§7 müssen mit diesem Slice
  auf den entschiedenen Stand — sonst bleibt die höherrangige Spec nach dem
  ADR-Accept stale (genau der im [ADR-0013](../../adr/0013-ifc-bibliothek.md)-Review monierte Punkt). Die §6-Zeile
  **präzisiert** (ADR → Spec-Richtung erlaubt, [MR-001](../../../../harness/conventions.md#mr-001--source-precedence-mit-eigener-spezifikations-schicht)/[MR-011](../../../../harness/conventions.md#mr-011--referenz-integritäts-gate-matrix-ids-spans-hostpaths)), erweitert nicht.
- **Subset-Treue zu [ADR-0013](../../adr/0013-ifc-bibliothek.md) (kein Über-Versprechen):** die §1-Mechanik darf
  **nicht** mehr behaupten als [ADR-0013](../../adr/0013-ifc-bibliothek.md) entschied (nur Geschosse + gerade Wände;
  Export IFC4 `IfcWall`, **nicht** deprecated `IfcWallStandardCase`). Jede
  weitergehende Entität ist benannte Lücke.
- **`IfcSite` optional / Beziehungs-Mechanik:** [ADR-0013](../../adr/0013-ifc-bibliothek.md) delegierte die exakten
  Beziehungs-/Platzierungs-Mechaniken (`IfcRelAggregates` vs.
  `IfcRelContainedInSpatialStructure`, optionale `IfcSite`, `IfcUnitAssignment`)
  **in genau diese Spec-Schärfung** — §1 muss sie präzise fassen (Determinismus/
  Totalität), ohne in slice-019b-Port-Mechanik vorzugreifen.
- **Port-Mechanik nicht vorab fixiert (Lösungsfreiheit der Ebenen):** ob der
  `ModelImporterPort` Roh-Entitäten oder fertige Domain-Bauteile liefert,
  entscheidet **019b** — dieser Slice spezifiziert nur das Mapping (Präzedenz
  014a/013a).
- **Split-Punkt:** kippt die Sitzung, ist DoD-1 (lösungsfreie Lastenheft-AK)
  **vorab schließbar** (hängt nicht von der §1-Mechanik ab) → DoD-2/3 (§1-Mapping
  + §6/§7-Nachzug) als zweiter Schritt.

## 7. Sub-Area-Modus-Begründung

### Sub-Area: Spec-Schreibung

- **Modus:** GF; **Konventionen-Dichte:** hoch — AK-Format (Happy/Boundary/
  Negative), Reifephase-/Teilumfang-Klausel, [MR-008](../../../../harness/conventions.md#mr-008--lastenheft-schärfung-bleibt-lösungsfrei) (lösungsfrei), [MR-010](../../../../harness/conventions.md#mr-010--lastenheft-header-version--oberste-9-historie-zeile)/012
  (Header==Historie), Wertebereich-/Fehler-Code-Konvention ([`E-IO-001`](../../../../spec/spezifikation.md#4-fehler-codes-und-logging-felder)/[`E-IO-003`](../../../../spec/spezifikation.md#4-fehler-codes-und-logging-felder)
  bestehen). **Phase-Reife:** IO Phase 2 (Outline → AK). **Evidenz-/Diskrepanz-
  Risiko:** niedrig (reine Doku; [ADR-0013](../../adr/0013-ifc-bibliothek.md) trägt die Backend-Entscheidung).
  **Reconciliation-Aufwand:** keiner; Folge-Slice 019b (Impl).

### Sub-Area: Planning-Lifecycle

- **Modus:** GF; **Dichte:** hoch ([ADR-0013](../../adr/0013-ifc-bibliothek.md)-Leitplanke: kein neuer Grundsatz-ADR;
  Reifephase-Teilumfang dokumentiert wie ROF/WAL-006; [MR-006](../../../../harness/conventions.md#mr-006--unabhängiges-plan-review-vor-implementierungs-start) vor Impl).
  **Phase-Reife:** Phase 4 (Lifecycle steht, je Slice geübt). **Risiko:** niedrig.

## 8. Closure-Notiz

*(wird bei `done` ausgefüllt — Closure-Kriterien beobachtbar, Lerneintrag in
einer der drei Formen: geschärfte Regel · neuer Sensor · benannte Spec-Lücke.)*
