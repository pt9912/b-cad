---
id: slice-020a
titel: STEP-/STL-Export — AK-Schärfung [LH-FA-IO-005](../../../../spec/lastenheft.md#lh-fa-io-005)/006 & Spec-Mapping (parametrisiert auf [ADR-0014](../../adr/0014-step-stl-export-backend.md))
status: done
welle: welle-4-austausch
lastenheft_refs: [[LH-FA-IO-005](../../../../spec/lastenheft.md#lh-fa-io-005), [LH-FA-IO-006](../../../../spec/lastenheft.md#lh-fa-io-006)]
adr_refs: [[ADR-0001](../../adr/0001-hexagonale-architektur.md), [ADR-0002](../../adr/0002-geometrie-kern-opencascade.md), [ADR-0014](../../adr/0014-step-stl-export-backend.md)]
---

# Slice 020a: STEP-/STL-Export — AK-Schärfung & Spec-Mapping

**Status:** done (2026-06-17). [MR-006](../../../../harness/conventions.md#mr-006--unabhängiges-plan-review-vor-implementierungs-start)-Plan-Review gelaufen
(**0 HIGH / 2 MED / 3 LOW / 2 INFO**; implementierungs-bereit, Start nicht blockiert);
DoD vollständig, `make gates` grün, Closure-Notiz §8.
[Report](../../../reviews/2026-06-17-slice-020a-plan.md). Eingearbeitet: **MED-1**
(§7 ist eine **Sammelklausel** DXF/STEP/STL/PDF/PNG — nur STEP/STL chirurgisch
entfernen, DXF/PDF/PNG bleiben offen), **MED-2** (§6 hat **keine** STEP/STL-Zeile →
**anlegen**, nicht „präzisieren"), **LOW-3** (STEP-Happy lässt Solid-vs-`Compound`
offen). LOW-1/2/INFO = Bestätigungen/Stil. Alle Vertrags-/Versions-/Export-only-
Behauptungen gegen die kanonischen Quellen verifiziert.

**Welle:** welle-4-austausch (STEP/STL-Strang, Entscheidungs-/Spec-Hälfte; Muster
slice-019a für IFC).

**Bezug:** [LH-FA-IO-005](../../../../spec/lastenheft.md#lh-fa-io-005) (STEP-Export) + [LH-FA-IO-006](../../../../spec/lastenheft.md#lh-fa-io-006) (STL-Export), bisher
**Outline**-Stubs. **Parametrisiert auf [ADR-0014](../../adr/0014-step-stl-export-backend.md) (STEP/STL-Export-Backend,
accepted 2026-06-17)** — das Backend ist **entschieden** (OCC-DataExchange nativ,
geometrie-residenter `ModelExporterPort`); dieser Slice braucht **keine neue
Grundsatz-ADR**, seine **Mapping-/Mechanik-Entscheidung lebt in Lastenheft-AK +
`spezifikation.md` §1** (Muster slice-019a). [ADR-0002](../../adr/0002-geometrie-kern-opencascade.md) (OCC liefert STEP/STL
nativ — die [ADR-0002](../../adr/0002-geometrie-kern-opencascade.md)-Ausgliederung, eingelöst durch [ADR-0014](../../adr/0014-step-stl-export-backend.md)), [ADR-0001](../../adr/0001-hexagonale-architektur.md)
(Kern führt; Format-Backend adapter-lokal; Port-Mechanik = Kern-Hoheit, Impl-Slice).

**Autor:** Dietmar Burkard. **Datum:** 2026-06-17.

**Schnitt-Herkunft:** Entscheidungs-/Spec-Hälfte des STEP/STL-Strangs (Muster
019a). Die Format-Abbildung (welche Bauteile, welche Repräsentation STEP=B-Rep /
STL=Mesh, atomare Ablehnung) braucht **prüfbare AK + ein entschiedenes Mapping**,
bevor implementiert wird (Impl-Slice). **Reine Doku/Entscheidung, kein Code.**
**Toolchain-Beleg vorab erbracht** (ADR-Index-Folgepflicht [ADR-0014](../../adr/0014-step-stl-export-backend.md): OCC 7.9.2
`libocct-data-exchange-dev` + `TKDESTEP`/`TKDESTL`/`TKRWMesh` vorhanden) — keine
[ADR-0004](../../adr/0004-toolchain-dependency-pinning.md)-Berührung, der Strang ist baubar.

---

## 1. Ziel

Die Export-Anforderungen [LH-FA-IO-005](../../../../spec/lastenheft.md#lh-fa-io-005) (STEP) und [LH-FA-IO-006](../../../../spec/lastenheft.md#lh-fa-io-006) (STL) bekommen
**lösungsfreie, benutzer-beobachtbare Akzeptanzkriterien** ([MR-008](../../../../harness/conventions.md#mr-008--lastenheft-schärfung-bleibt-lösungsfrei)) und ein
**entschiedenes Mapping** (`spezifikation.md` §1), bevor implementiert wird —
**innerhalb des von [ADR-0014](../../adr/0014-step-stl-export-backend.md) entschiedenen Backends** (OCC-DataExchange nativ,
geometrie-resident). Zusätzlich wird die **[ADR-0014](../../adr/0014-step-stl-export-backend.md)-Folgepflicht** eingelöst:
die durch den ADR-Accept **stale** gewordenen Spec-Stellen (§6 Vertragstabelle,
§7 Offene Punkte „STEP/STL-Backends offen") werden nachgezogen.

**Wichtiger Unterschied zu IFC (019a): STEP/STL ist EXPORT-ONLY.** b-cad **importiert
kein** STEP/STL (kein `ModelImporterPort` dafür). Daher gibt es **keinen
Roundtrip-zu-b-cad-AK** wie bei IFC; die Happy-AK ist „eine **valide** STEP-/STL-
Datei entsteht, die ein **Standard-Leser** als die Bauteil-Geometrie zurückliest".

**Reifephase-Teilumfang welle-4:** die **3D-fähigen Bauteile** (Wände inkl.
Wandöffnungen, Dächer, Decken/Fundament, Treppen — die Geometrie, die der Kern/
Geometrie-Adapter ohnehin baut). Nicht-3D-Aspekte (Material/Farbe/Property-Sets,
PMI, Assemblies) bleiben ausdrücklich offen (benannte Lücke; [ADR-0014](../../adr/0014-step-stl-export-backend.md) Re-Eval).

## 2. Definition of Done

- [x] **Lastenheft [LH-FA-IO-005](../../../../spec/lastenheft.md#lh-fa-io-005) + [LH-FA-IO-006](../../../../spec/lastenheft.md#lh-fa-io-006) von Outline auf AK-Niveau**
      (**lösungsfrei, benutzer-beobachtbar** — **kein** OCC/Toolkit-/Schema-Vokabular
      im Lastenheft-Text, das gehört in §1; [MR-008](../../../../harness/conventions.md#mr-008--lastenheft-schärfung-bleibt-lösungsfrei)):
      **IO-005 STEP** mindestens — **Happy:** Given ein b-cad-Modell mit 3D-Bauteilen,
      when nach STEP exportiert, then entsteht eine **valide STEP-Datei**, die ein
      Standard-CAD-/STEP-Leser zurückliest, wobei die **exportierten Bauteile als
      Volumenkörper enthalten** sind (genaue Solid-vs-`Compound`-Repräsentation = AK
      des Impl-Slice, [ADR-0014](../../adr/0014-step-stl-export-backend.md) offen — LOW-3); **Negative:** nicht
      beschreibbarer Zielpfad → [`E-IO-001`](../../../../spec/spezifikation.md#4-fehler-codes-und-logging-felder), **kein Teil-Export**, Zielpfad
      unverändert. **IO-006 STL** mindestens — **Happy:** Modell → **valide STL-Datei**
      mit dem **Dreiecksnetz** der Bauteile (nicht leer; Netz entspricht der
      3D-Darstellung); **Negative:** wie STEP ([`E-IO-001`](../../../../spec/spezifikation.md#4-fehler-codes-und-logging-felder)). **Boundary
      (beide):** leeres/3D-leeres Modell → leere-aber-gültige Datei **ohne Absturz**
      (Totalität). **Teilumfang-Klausel (benutzer-beobachtbar):** Export deckt die
      **3D-Bauteile**; Material/Farbe/Property-Sets bleiben **ausgespart** —
      ausdrücklich offen, kein stiller Vollumfang. + Header-Nachzug +
      `lastenheft-historie.md` **0.1.9** (**[MR-010](../../../../harness/conventions.md#mr-010--lastenheft-header-version--oberste-9-historie-zeile)/[MR-012](../../../../harness/conventions.md#mr-012--mr-010-invariante-folgt-der-ausgelagerten-lastenheft-historie)**: Header-`Version:` ==
      oberste Historie-Zeile).
- [x] **`spec/spezifikation.md` §1 neuer Block [`LH-FA-IO-005.a`](../../../../spec/lastenheft.md#lh-fa-io-005)** (Sammelblock —
      **deckt IO-005 STEP + IO-006 STL**, Muster [`LH-FA-IO-001.a`](../../../../spec/lastenheft.md#lh-fa-io-001--ifc-import)): das
      Export-Mapping innerhalb des **[ADR-0014](../../adr/0014-step-stl-export-backend.md)-Backends** — **STEP** schreibt die
      **B-Rep-Solids** (extrudierte/boolesch geschnittene Bauteil-Körper) über
      OCC-DataExchange (Ziel-Schema benannt: AP214 oder AP242); **STL** schreibt das
      **tessellierte Dreiecksnetz** (binär als Default). **Schicht:** der Exporter ist
      **geometrie-resident** (`src/adapters/geometry/`, OCC nur dort — Regel C) hinter
      `ModelExporterPort`, vom Composition Root je `ExchangeFormat` verdrahtet
      ([ADR-0014](../../adr/0014-step-stl-export-backend.md)). **Atomar** (Temp + Rename; nicht beschreibbarer Zielpfad →
      [`E-IO-001`](../../../../spec/spezifikation.md#4-fehler-codes-und-logging-felder), kein Teil-Export); OCC-Konvertierungs-/Schreibfehler →
      neutraler Wurf (kein OCC-Typ-Leck, [ADR-0001](../../adr/0001-hexagonale-architektur.md)). **Bauteil-Subset** = 3D-fähige
      Bauteile; **Totalität** (3D-leeres Modell → gültige, leere Datei). Mechanik,
      **kein** Lastenheft. + `spezifikation-historie.md` + `**Letzte Änderung:**`-Header.
- [x] **`spec/spezifikation.md` §6 + §7 nachgezogen ([ADR-0014](../../adr/0014-step-stl-export-backend.md)-Folgepflicht):**
      **§6** hat **keine** STEP/STL-Zeile (nur OCC/Qt/SQLite/IFC) → eine **neue**
      Vertragszeile **anlegen** (STEP/STL = OCC-DataExchange nativ, geometrie-
      residenter `ModelExporterPort`, §1 [`LH-FA-IO-005.a`](../../../../spec/lastenheft.md#lh-fa-io-005)); das Anlegen einer
      ADR-entschiedenen Vertragszeile ist die [MR-001](../../../../harness/conventions.md#mr-001--source-precedence-mit-eigener-spezifikations-schicht)-erlaubte ADR→Spec-Richtung
      (MED-2: **kein** „präzisieren" einer nicht existierenden Zeile). **§7** ist eine
      **Sammelklausel** „DXF-/STEP-/STL-/PDF-/PNG-Backends bleiben offen" (kein
      eigenständiger STEP/STL-Punkt, MED-1) → **chirurgisch** auf
      „DXF-/PDF-/PNG-Backends bleiben offen" kürzen — nur STEP/STL entfernt,
      **DXF/PDF/PNG bleiben offen** (eigene ADRs). ADR-Index-Folgepflicht-Zeile
      ([ADR-0014](../../adr/0014-step-stl-export-backend.md) „AK-Schärfung + Spec-Nachzug") abhaken (im Closure-Commit).
- [x] **`spec/architecture.md`:** voraussichtlich **unverändert** — `ModelExporterPort`
      (IO-005/006), `ExchangeModelPort`/`ExchangeService` und der §5-Export→[`E-IO-001`](../../../../spec/spezifikation.md#4-fehler-codes-und-logging-felder)-
      Eintrag sind **bereits** vorhanden; **optional** eine `## Geschichte`-Provenance-
      Zeile „Austauschformate STEP/STL → [ADR-0014](../../adr/0014-step-stl-export-backend.md)" (matrix-`exclude-sections`,
      Provenance erlaubt) — sonst bewusst nicht geändert (Begründung in Closure).
- [x] **Reine Doku/Entscheidung — kein Code, keine Tests, kein ADR** ([ADR-0014](../../adr/0014-step-stl-export-backend.md)
      deckt das Backend; Mapping ist Spec-Entscheidung). `make gates` grün;
      `make schema-check` unberührt; Closure-Notiz mit Lerneintrag. **Nicht Teil:**
      der **STEP/STL-Impl-Slice** (geometrie-residenter Exporter + `ExchangeFormat`-
      Dispatch + OCC-Toolkit-Linkage + AK-Tests [`LH-FA-IO-005`](../../../../spec/lastenheft.md#lh-fa-io-005)/`006` + Re-Read-/
      Adapter-Pfad-Integrationstest).

## 3. Plan (vor Code)

| Datei / Komponente | Änderungs-Art | Begründung |
|---|---|---|
| `spec/lastenheft.md` | ändern | [LH-FA-IO-005](../../../../spec/lastenheft.md#lh-fa-io-005)/006 Outline → AK (lösungsfrei, export-only, Teilumfang-Klausel); Header `Version:` → 0.1.9 |
| `spec/lastenheft-historie.md` | ändern | oberste Zeile 0.1.9 ([MR-010](../../../../harness/conventions.md#mr-010--lastenheft-header-version--oberste-9-historie-zeile)/[MR-012](../../../../harness/conventions.md#mr-012--mr-010-invariante-folgt-der-ausgelagerten-lastenheft-historie)-Invariante) |
| `spec/spezifikation.md` | ändern | §1 [`LH-FA-IO-005.a`](../../../../spec/lastenheft.md#lh-fa-io-005) STEP/STL-Mapping (B-Rep/Mesh, OCC-DataExchange, geometrie-resident, atomar); §6 STEP/STL-Vertragszeile entschieden; §7 STEP/STL-Offene-Punkte streichen |
| `spec/spezifikation-historie.md` | ändern | Provenance-Zeile (slice-020a) |
| `spec/architecture.md` | ggf. ändern | optional `## Geschichte`-Provenance „STEP/STL → [ADR-0014](../../adr/0014-step-stl-export-backend.md)"; sonst unverändert |
| `docs/plan/adr/README.md` | ändern (Closure) | [ADR-0014](../../adr/0014-step-stl-export-backend.md)-Folgepflicht „AK-Schärfung + Spec-Nachzug" → erfüllt |
| `docs/reviews/{2026-06-17-slice-020a-plan}.md` | neu | [MR-006](../../../../harness/conventions.md#mr-006--unabhängiges-plan-review-vor-implementierungs-start)-Report |

## 4. Trigger

- Keiner — **startbar** nach [MR-006](../../../../harness/conventions.md#mr-006--unabhängiges-plan-review-vor-implementierungs-start)-Review: reine Doku/Entscheidung; das
  STEP/STL-Backend ist mit [ADR-0014](../../adr/0014-step-stl-export-backend.md) (accepted) entschieden, der Toolchain-Beleg
  ist erbracht, die IO-Ports (`ExchangeModelPort`/`ModelExporterPort`) sind
  deklariert.

## 5. Closure-Trigger

- DoD vollständig, `make gates` grün, Closure-Notiz mit Lerneintrag →
  der **STEP/STL-Impl-Slice** wird startbar.

## 6. Risiken und offene Punkte

- **Lösungsfreiheit des Lastenhefts ([MR-008](../../../../harness/conventions.md#mr-008--lastenheft-schärfung-bleibt-lösungsfrei)):** „valide STEP-/STL-Datei, von einem
  Standard-Leser zurückgelesen" ist benutzer-beobachtbar; **OCC**, `STEPControl_Writer`,
  AP214/242, `StlAPI`, Toolkit-Namen sind Mechanik und gehören in §1, **nicht** ins
  Lastenheft. Der Teilumfang ist benutzer-beobachtbar zu formulieren („Material/
  Farbe werden nicht exportiert"), nicht über Toolkit-Listen.
- **Export-only-AK (kein Roundtrip-zu-b-cad):** anders als IFC (019a) gibt es keinen
  b-cad-STEP/STL-Import → die Happy-AK darf **keinen** Roundtrip-zu-b-cad behaupten.
  Beobachtbarkeit über „valide Datei, von einem Standard-Leser/Tool zurücklesbar"
  (das **Re-Read-Orakel** im Impl-Test ist OCC-`STEPControl_Reader` — Mechanik, §1/
  Impl, nicht Lastenheft).
- **Header-Versions-Nachzug ([MR-010](../../../../harness/conventions.md#mr-010--lastenheft-header-version--oberste-9-historie-zeile)/[MR-012](../../../../harness/conventions.md#mr-012--mr-010-invariante-folgt-der-ausgelagerten-lastenheft-historie)):** `**Version:**` der `lastenheft.md`
  auf **0.1.9** == oberste `lastenheft-historie.md`-Zeile nachziehen. Im Review als
  Quellen-Konsistenz-Linse prüfen.
- **Spec-Stale-Nachzug ([ADR-0014](../../adr/0014-step-stl-export-backend.md)-Folgepflicht):** §6/§7 müssen mit diesem Slice auf
  den entschiedenen Stand (sonst bleibt die höherrangige Spec nach dem ADR-Accept
  stale). **§6 legt eine *neue* STEP/STL-Vertragszeile an** (MED-2: §6 hat bisher
  keine; ADR-entschieden → [MR-001](../../../../harness/conventions.md#mr-001--source-precedence-mit-eigener-spezifikations-schicht)-erlaubte ADR→Spec-Richtung). **§7 ist eine
  Sammelklausel** „DXF-/STEP-/STL-/PDF-/PNG-Backends offen" (MED-1: kein eigener
  STEP/STL-Punkt) → nur STEP/STL **chirurgisch** entfernen; **DXF/PDF/PNG bleiben
  offen** (kein eigenes ADR — nicht mit-streichen!).
- **Spec-Straten ADR-frei ([MR-011](../../../../harness/conventions.md#mr-011--referenz-integritäts-gate-matrix-ids-spans-hostpaths)):** die §1-/§6-Einarbeitung der frischen
  [ADR-0014](../../adr/0014-step-stl-export-backend.md)-Entscheidung darf **keinen** ADR-Verweis im `spezifikation.md`-Körper
  hinterlassen (matrix `spec-straten ↛ adr`) — Provenance nur in `spezifikation-historie.md`.
  Vor dem Gate per `grep ADR-` selbst fangen (Muster 019a-Lerneintrag).
- **Subset-Treue zu [ADR-0014](../../adr/0014-step-stl-export-backend.md) (kein Über-Versprechen):** die §1-Mechanik behauptet
  nur, was [ADR-0014](../../adr/0014-step-stl-export-backend.md) entschied (B-Rep-Solids/Mesh der 3D-Bauteile, geometrie-
  resident, atomar). Material/PMI/Assemblies sind benannte Lücke.

## 7. Sub-Area-Modus-Begründung

### Sub-Area: Spec-Schreibung

- **Modus:** GF; **Konventionen-Dichte:** hoch — AK-Format (Happy/Boundary/Negative),
  Reifephase-/Teilumfang-Klausel, [MR-008](../../../../harness/conventions.md#mr-008--lastenheft-schärfung-bleibt-lösungsfrei) (lösungsfrei), [MR-010](../../../../harness/conventions.md#mr-010--lastenheft-header-version--oberste-9-historie-zeile)/012 (Header==
  Historie), Fehler-Code-Konvention ([`E-IO-001`](../../../../spec/spezifikation.md#4-fehler-codes-und-logging-felder) besteht). **Phase-Reife:** IO
  Phase 2 (Outline → AK). **Evidenz-/Diskrepanz-Risiko:** niedrig (reine Doku;
  [ADR-0014](../../adr/0014-step-stl-export-backend.md) trägt die Backend-Entscheidung, Toolchain belegt). **Reconciliation:**
  keiner; Folge-Slice = STEP/STL-Impl.

### Sub-Area: Planning-Lifecycle

- **Modus:** GF; **Dichte:** hoch ([ADR-0014](../../adr/0014-step-stl-export-backend.md)-Leitplanke: kein neuer Grundsatz-ADR;
  Reifephase-Teilumfang wie IFC/ROF; [MR-006](../../../../harness/conventions.md#mr-006--unabhängiges-plan-review-vor-implementierungs-start) vor Impl). **Phase-Reife:** Phase 4.
  **Risiko:** niedrig.

## 8. Closure-Notiz

**Closure-Kriterien (beobachtbar, 2026-06-17):**

- **Lastenheft 0.1.9:** [`LH-FA-IO-005`](../../../../spec/lastenheft.md#lh-fa-io-005) (STEP) +
  [`LH-FA-IO-006`](../../../../spec/lastenheft.md#lh-fa-io-006) (STL) von Outline auf AK-Niveau (Happy/Boundary/
  Negative, **lösungsfrei/benutzer-beobachtbar**, **export-only** — kein Roundtrip-
  zu-b-cad behauptet); **Teilumfang 3D-Bauteile** explizit (Material/Farbe/
  Property-Sets ausgespart, offen). Header 0.1.9 == oberste `lastenheft-historie.md`-
  Zeile ([MR-010](../../../../harness/conventions.md#mr-010--lastenheft-header-version--oberste-9-historie-zeile)/012).
- **`spezifikation.md` §1 [`LH-FA-IO-005.a`](../../../../spec/lastenheft.md#lh-fa-io-005)** (Sammelblock STEP+STL):
  OCC-DataExchange nativ, **geometrie-residenter** `ModelExporterPort` (Regel C),
  STEP B-Rep (AP214/242) / STL Netz (binär), atomar ([`E-IO-001`](../../../../spec/spezifikation.md#4-fehler-codes-und-logging-felder), kein
  Teil-Export), Bauteil-Subset/Totalität. **§6** neue STEP/STL-Vertragszeile (MED-2:
  §6 hatte keine), **§7** STEP/STL **chirurgisch** aus der Sammelklausel entfernt —
  **DXF/PDF/PNG bleiben offen** (MED-1). `**Letzte Änderung:**` + `spezifikation-historie.md`
  nachgezogen.
- **Spec-Straten ADR-frei ([MR-011](../../../../harness/conventions.md#mr-011--referenz-integritäts-gate-matrix-ids-spans-hostpaths)):** der `spezifikation.md`-Körper trägt **keinen**
  ADR-Verweis (Provenance nur in `*-historie.md`); `architecture.md` `## Geschichte`
  um eine STEP/STL→[ADR-0014](../../adr/0014-step-stl-export-backend.md)-Provenance-Zeile ergänzt (exclude-section, Link erlaubt).
- **`architecture.md`** sonst unverändert (`ModelExporterPort`/§5-Export-Zeile lagen
  vor); ADR-Index-Folgepflicht „AK-Schärfung + Spec-Nachzug" abgehakt.
- **Kein Code/Test/ADR/Schema**; `make gates` grün (docs-check unter d-check v0.11.0);
  `make schema-check` unberührt.

**Lerneintrag (benannte Spec-Lücke + geschärfte Praxis):**

- **Benannte Spec-Lücke:** STEP/STL exportieren nur die **3D-fähigen Bauteile**;
  Material/Farbe/Property-Sets/PMI/Assemblies sind als Re-Eval-Trigger benannt
  ([ADR-0014](../../adr/0014-step-stl-export-backend.md): XDE/AP242) — kein stiller Vollumfang.
- **Export-only-Schärfung (geschärfte Praxis):** anders als IFC (019a) hat b-cad
  **keinen** STEP/STL-Import → die Happy-AK darf **keinen** Roundtrip-zu-b-cad
  behaupten; Beobachtbarkeit über „valide Datei, von einem Standard-Leser
  zurückgelesen" (Re-Read-Orakel = Impl-Mechanik, nicht Lastenheft). Das [MR-006](../../../../harness/conventions.md#mr-006--unabhängiges-plan-review-vor-implementierungs-start)-
  Review fing zwei Quellen-Genauigkeitsfehler (MED-1 §7-Sammelklausel statt
  eigener Zeile; MED-2 §6 ohne STEP/STL-Zeile) **vor** dem Edit ab — verhinderte
  eine Über-Löschung von DXF/PDF/PNG.
- **`ids`-Linkpflicht in `planning/` (Werkzeug-Disziplin):** vier bare ADR-Mentions
  im Plan wurden vom `docs-check` (`ids`, `planning/` nicht exempt) gefangen und
  verlinkt — `docs/reviews/**` ist unter d-check v0.11.0 exempt, `planning/` nicht.

**Closure-Trigger erfüllt → STEP/STL-Impl-Slice wird startbar** (geometrie-residenter
Exporter + `ExchangeFormat`-Dispatch + OCC-Toolkit-Linkage + AK-/Re-Read-Tests).
