---
id: slice-021a
titel: DXF-Import/-Export — AK-Schärfung [LH-FA-IO-003](../../../../spec/lastenheft.md#lh-fa-io-003)/004 & Spec-Mapping (parametrisiert auf [ADR-0015](../../adr/0015-dxf-backend.md))
status: done
welle: welle-4-austausch
lastenheft_refs: [[LH-FA-IO-003](../../../../spec/lastenheft.md#lh-fa-io-003), [LH-FA-IO-004](../../../../spec/lastenheft.md#lh-fa-io-004)]
adr_refs: [[ADR-0001](../../adr/0001-hexagonale-architektur.md), [ADR-0013](../../adr/0013-ifc-bibliothek.md), [ADR-0015](../../adr/0015-dxf-backend.md)]
---

# Slice 021a: DXF-Import/-Export — AK-Schärfung & Spec-Mapping

**Status:** done (2026-06-17). [MR-006](../../../../harness/conventions.md#mr-006--unabhängiges-plan-review-vor-implementierungs-start)-Plan-Review **0 HIGH**;
DoD vollständig, `make gates` grün (docs-check 126/0), Closure-Notiz §8.
(Plan-Review **0 HIGH / 2 MED / 3 LOW / 3 INFO**; Start nicht blockiert.)
[Report](../../../reviews/2026-06-17-slice-021a-plan.md). Eingearbeitet in §1 (DoD-2):
**MED-1** (Import nutzt die **bestehende generische** [`E-IO-003`](../../../../spec/spezifikation.md#4-fehler-codes-und-logging-felder)-Zeile, kein neuer Code),
**MED-2** (atomarer Import mechanisch gepinnt — **vollständiges In-Memory-Modell zuerst**,
IFC-Grade), **LOW-1** (Ziel-DXF-Profil: ASCII, `LINE` versions-robust, Profilversion =
Impl-Slice). LOW-2/3 + INFO = Bestätigungen/Stil. Alle Vertrags-/Versions-/§4-§6-§7-
Behauptungen gegen die kanonischen Quellen verifiziert.

**Welle:** welle-4-austausch (DXF-Strang, Entscheidungs-/Spec-Hälfte; Muster
slice-019a [IFC] / slice-020a [STEP/STL]).

**Bezug:** [LH-FA-IO-003](../../../../spec/lastenheft.md#lh-fa-io-003) (DXF-Import) + [LH-FA-IO-004](../../../../spec/lastenheft.md#lh-fa-io-004) (DXF-Export), bisher
**Outline**-Stubs. **Parametrisiert auf [ADR-0015](../../adr/0015-dxf-backend.md) (DXF-Backend, accepted
2026-06-17)** — das Backend ist **entschieden** (selbst getragener DXF-SPF-artiger
**Subset-Codec im IO-Adapter, Option D**, wie IFC [ADR-0013](../../adr/0013-ifc-bibliothek.md); io-resident,
kein OCC, kein Lib-Zukauf); dieser Slice braucht **keine neue Grundsatz-ADR**, seine
**Mapping-/Mechanik-Entscheidung lebt in Lastenheft-AK + `spezifikation.md` §1**
(Muster slice-019a/020a). [ADR-0013](../../adr/0013-ifc-bibliothek.md) (Option-D-Präzedenz, io-resident),
[ADR-0001](../../adr/0001-hexagonale-architektur.md) (Kern führt; Format-Codec adapter-lokal; Port-/Dispatch-Mechanik =
Kern-Hoheit, Impl-Slice).

**Autor:** Dietmar Burkard. **Datum:** 2026-06-17.

**Schnitt-Herkunft:** Entscheidungs-/Spec-Hälfte des DXF-Strangs (Muster 019a/020a).
Die DXF-2D-Abbildung (welche Entitäten, welche b-cad-Bauteile, Import-Defaults,
atomare Ablehnung) braucht **prüfbare AK + ein entschiedenes Mapping**, bevor
implementiert wird. **Reine Doku/Entscheidung, kein Code.**

---

## 1. Ziel

Die DXF-Anforderungen [LH-FA-IO-003](../../../../spec/lastenheft.md#lh-fa-io-003) (Import) und [LH-FA-IO-004](../../../../spec/lastenheft.md#lh-fa-io-004) (Export) bekommen
**lösungsfreie, benutzer-beobachtbare Akzeptanzkriterien** ([MR-008](../../../../harness/conventions.md#mr-008--lastenheft-schärfung-bleibt-lösungsfrei)) und ein
**entschiedenes Mapping** (`spezifikation.md` §1), bevor implementiert wird —
**innerhalb des von [ADR-0015](../../adr/0015-dxf-backend.md) entschiedenen Backends** (Subset-Codec, io-resident,
2D-Grundriss). Zusätzlich wird die **[ADR-0015](../../adr/0015-dxf-backend.md)-Folgepflicht** eingelöst: die durch
den ADR-Accept **stale** gewordenen Spec-Stellen (§7 Offene-Backends-Klausel, §6
ohne DXF-Zeile, §4 [`E-IO-001`](../../../../spec/spezifikation.md#4-fehler-codes-und-logging-felder)-Bedingung) werden nachgezogen.

**DXF ist 2D + bidirektional.** Anders als STEP/STL (export-only, 020a) hat DXF
**Import UND Export** (wie IFC, 019a) — aber als **2D-Grundriss** (kein 3D). Die
fachliche Eigenheit: DXF trägt **keine Höhe/Dicke** → importierte Wände bekommen
**Default-Höhe/-Dicke** (benannte Lücke, analog IFC-Geschoss-Höhe slice-019b).

**Reifephase-Teilumfang welle-4 (= [ADR-0015](../../adr/0015-dxf-backend.md)-Subset):** **gerade Wände als
2D-Achsen, je Geschoss getrennt**. Räume, Bemaßung, Schraffur, Blöcke, Text,
Bögen/Kreise, 3D, beliebige Geometrie bleiben ausdrücklich offen (benannte Lücke,
beim Import übersprungen / beim Export nicht geschrieben).

## 2. Definition of Done

- [x] **Lastenheft [LH-FA-IO-003](../../../../spec/lastenheft.md#lh-fa-io-003) + [LH-FA-IO-004](../../../../spec/lastenheft.md#lh-fa-io-004) von Outline auf AK-Niveau**
      (**lösungsfrei, benutzer-beobachtbar** — **kein** DXF-Entitätsname/Gruppencode/
      Codec-Mechanik im Lastenheft-Text, das gehört in §1; [MR-008](../../../../harness/conventions.md#mr-008--lastenheft-schärfung-bleibt-lösungsfrei)):
      **IO-004 Export** mindestens — **Happy:** Given ein b-cad-Modell mit Geschossen
      + geraden Wänden, when nach DXF exportiert, then entsteht eine **valide DXF-Datei**,
      die ein Standard-CAD-/DXF-Leser als **2D-Grundriss** zurückliest (die Wand-Achsen
      je Geschoss). **IO-003 Import** mindestens — **Happy:** valide DXF mit 2D-Linien/
      Polylinien → **entsprechende gerade Wände entstehen, Anzahl stimmt mit der Quelle
      überein**; da DXF **keine Höhe/Dicke** trägt, erhalten sie **Default-Höhe/-Dicke**
      (benannte Lücke). **Roundtrip (beobachtbar):** Export → Re-Import erhält die
      **Wand-Achsen-Anzahl** je Geschoss (Treue = Anzahl + Achs-Lage, **nicht** Höhe/
      Dicke). **Boundary:** leere/strukturlose DXF → leeres Modell ohne Absturz (Import);
      3D-leeres Modell → gültige (leere) DXF (Export). **Negative:** Import nicht-DXF/
      kaputt → [`E-IO-003`](../../../../spec/spezifikation.md#4-fehler-codes-und-logging-felder), **kein Teil-Import**; Export nicht beschreibbarer Zielpfad →
      [`E-IO-001`](../../../../spec/spezifikation.md#4-fehler-codes-und-logging-felder), **kein Teil-Export**. **Teilumfang-Klausel (benutzer-beobachtbar):**
      Import/Export decken **gerade Wände als 2D-Achsen je Geschoss**; weitere Inhalte
      (Räume, Bemaßung, Schraffur, Blöcke, Text, Bögen, 3D) werden **übersprungen bzw.
      nicht geschrieben** — ausdrücklich offen. + Header-Nachzug + `lastenheft-historie.md`
      **0.1.10** (**[MR-010](../../../../harness/conventions.md#mr-010--lastenheft-header-version--oberste-9-historie-zeile)/[MR-012](../../../../harness/conventions.md#mr-012--mr-010-invariante-folgt-der-ausgelagerten-lastenheft-historie)**: Header-`Version:` == oberste Historie-Zeile).
- [x] **`spec/spezifikation.md` §1 neuer Block [`LH-FA-IO-003.a`](../../../../spec/lastenheft.md#lh-fa-io-003)** (Sammelblock —
      **deckt IO-003 + IO-004**, Muster [`LH-FA-IO-001.a`](../../../../spec/lastenheft.md#lh-fa-io-001--ifc-import)/[`LH-FA-IO-005.a`](../../../../spec/lastenheft.md#lh-fa-io-005)): das
      DXF-Mapping innerhalb des **[ADR-0015](../../adr/0015-dxf-backend.md)-Subsets** — **Encoding** DXF-ASCII
      (Gruppencode-Paare, `ENTITIES`-Sektion); **Subset** gerade Wand-**Achsen** als
      `LINE`/`LWPOLYLINE`, **je Geschoss auf DXF-`LAYER`**; **Schicht** io-residenter
      Subset-Codec (`adapters/io/`, kein OCC — `ModelImporterPort`/`ModelExporterPort`,
      symmetrischer Reader+Writer, Muster IFC-SPF); **Import-Defaults** (DXF trägt keine
      Höhe/Dicke → Default-Werte); **Ziel-DXF-Profil** (ASCII-DXF; `LINE` für gerade
      Achsen ist versions-robust, `LWPOLYLINE` erst R14+ — die exakte Profilversion
      fixiert der Impl-Slice nach Lesbarkeit, MED/LOW-1 des Reviews); **atomarer Import**
      (**vollständiges In-Memory-Modell zuerst**, Übergabe erst nach fehlerfreiem Parse —
      Muster IFC-Import; nicht-DXF/kaputt → die **bestehende generische**
      [`E-IO-003`](../../../../spec/spezifikation.md#4-fehler-codes-und-logging-felder)-Zeile, **kein** neuer Code, kein Teil-Import); **atomarer Export**
      ([`E-IO-001`](../../../../spec/spezifikation.md#4-fehler-codes-und-logging-felder), kein Teil-Export); **Totalität** (leere/strukturlose DXF →
      leeres Modell, kein Wurf). Mechanik, **kein** Lastenheft. + `spezifikation-historie.md`
      + `**Letzte Änderung:**`-Header.
- [x] **`spec/spezifikation.md` §6 + §7 + §4 nachgezogen ([ADR-0015](../../adr/0015-dxf-backend.md)-Folgepflicht):**
      **§7** ist eine **Sammelklausel** „DXF-/PDF-/PNG-Backends bleiben offen" → DXF
      **chirurgisch** entfernen, „PDF-/PNG-Backends bleiben offen" erhalten (PDF/PNG
      **kein** eigenes ADR → bleiben offen). **§6** hat **keine** DXF-Zeile → **neue**
      Vertragszeile **anlegen** (DXF = self-rolled Subset-Codec io-resident, §1
      [`LH-FA-IO-003.a`](../../../../spec/lastenheft.md#lh-fa-io-003)). **§4** [`E-IO-001`](../../../../spec/spezifikation.md#4-fehler-codes-und-logging-felder)-Bedingung um „DXF-Export" erweitern
      (zählt heute nur „IFC-Export" — STEP/STL trägt dieselbe latente Lücke, **parallel
      mitziehen**; [ADR-0015](../../adr/0015-dxf-backend.md)-Review-LOW-2). ADR-Index-Folgepflicht-Zeile ([ADR-0015](../../adr/0015-dxf-backend.md)
      „AK-Schärfung + Spec-Nachzug") abhaken (im Closure-Commit).
- [x] **`spec/architecture.md`:** voraussichtlich **unverändert** — `ExchangeModelPort`/
      `ModelImporterPort`/`ModelExporterPort`/`adapters/io/` sind deklariert; **optional**
      eine `## Geschichte`-Provenance-Zeile „DXF Import+Export → [ADR-0015](../../adr/0015-dxf-backend.md)"
      (exclude-section, Link erlaubt) — sonst bewusst nicht geändert (Begründung in
      Closure). **Import-Dispatch-Kern-Erweiterung** (Importer-Registry, [ADR-0015](../../adr/0015-dxf-backend.md)-Review-
      MED-1) ist **Impl-Slice**, nicht hier.
- [x] **Reine Doku/Entscheidung — kein Code, keine Tests, kein ADR** ([ADR-0015](../../adr/0015-dxf-backend.md)
      deckt das Backend; Mapping ist Spec-Entscheidung). `make gates` grün;
      `make schema-check` unberührt; Closure-Notiz mit Lerneintrag. **Nicht Teil:**
      der **DXF-Impl-Slice** (io-residenter DXF-Subset-Codec Reader+Writer +
      `ModelImporterPort`/`ModelExporterPort` + `ExchangeFormat::Dxf`-Dispatch **+
      Import-Dispatch-Kern-Erweiterung** + AK-/Roundtrip-Tests + Adapter-Pfad-Integration).

## 3. Plan (vor Code)

| Datei / Komponente | Änderungs-Art | Begründung |
|---|---|---|
| `spec/lastenheft.md` | ändern | [LH-FA-IO-003](../../../../spec/lastenheft.md#lh-fa-io-003)/004 Outline → AK (lösungsfrei, bidirektional, Teilumfang-Klausel); Header `Version:` → 0.1.10 |
| `spec/lastenheft-historie.md` | ändern | oberste Zeile 0.1.10 ([MR-010](../../../../harness/conventions.md#mr-010--lastenheft-header-version--oberste-9-historie-zeile)/[MR-012](../../../../harness/conventions.md#mr-012--mr-010-invariante-folgt-der-ausgelagerten-lastenheft-historie)-Invariante) |
| `spec/spezifikation.md` | ändern | §1 [`LH-FA-IO-003.a`](../../../../spec/lastenheft.md#lh-fa-io-003) DXF-2D-Mapping; §6 neue DXF-Zeile; §7 DXF chirurgisch (PDF/PNG bleiben); §4 [`E-IO-001`](../../../../spec/spezifikation.md#4-fehler-codes-und-logging-felder) um DXF-Export |
| `spec/spezifikation-historie.md` | ändern | Provenance-Zeile (slice-021a) |
| `spec/architecture.md` | ggf. ändern | optional `## Geschichte`-Provenance „DXF → [ADR-0015](../../adr/0015-dxf-backend.md)"; sonst unverändert |
| `docs/plan/adr/README.md` | ändern (Closure) | [ADR-0015](../../adr/0015-dxf-backend.md)-Folgepflicht „AK-Schärfung + Spec-Nachzug" → erfüllt |
| `docs/reviews/{2026-06-17-slice-021a-plan}.md` | neu | [MR-006](../../../../harness/conventions.md#mr-006--unabhängiges-plan-review-vor-implementierungs-start)-Report |

## 4. Trigger

- Keiner — **startbar** nach [MR-006](../../../../harness/conventions.md#mr-006--unabhängiges-plan-review-vor-implementierungs-start)-Review: reine Doku/Entscheidung; das
  DXF-Backend ist mit [ADR-0015](../../adr/0015-dxf-backend.md) (accepted) entschieden, die IO-Ports
  (`ExchangeModelPort`/Importer/Exporter) sind deklariert.

## 5. Closure-Trigger

- DoD vollständig, `make gates` grün, Closure-Notiz mit Lerneintrag →
  der **DXF-Impl-Slice** wird startbar.

## 6. Risiken und offene Punkte

- **Lösungsfreiheit des Lastenhefts ([MR-008](../../../../harness/conventions.md#mr-008--lastenheft-schärfung-bleibt-lösungsfrei)):** „valide DXF-Datei, von einem Standard-
  Leser als 2D-Grundriss zurückgelesen" + „DXF mit Linien → gerade Wände, Anzahl
  stimmt" sind benutzer-beobachtbar; **DXF-Entitätsnamen** (`LINE`/`LWPOLYLINE`/
  `LAYER`), **Gruppencodes** und die **Codec-/Layer-Mechanik** sind Mechanik und
  gehören in §1, **nicht** ins Lastenheft. Teilumfang benutzer-beobachtbar
  („Schraffuren/Blöcke werden beim Import übersprungen"), nicht über Entitäts-Listen.
- **Import-Defaults (benannte Lücke, [MR-008](../../../../harness/conventions.md#mr-008--lastenheft-schärfung-bleibt-lösungsfrei)-tauglich):** DXF trägt keine Höhe/Dicke
  → importierte Wände bekommen Default-Höhe/-Dicke; im Lastenheft als beobachtbare
  Lücke fassen („importierte Wände erhalten Standard-Höhe/-Dicke"), die genauen
  Default-Werte/Mechanik in §1 (analog IFC-Geschoss-Höhe 019b).
- **Header-Versions-Nachzug ([MR-010](../../../../harness/conventions.md#mr-010--lastenheft-header-version--oberste-9-historie-zeile)/[MR-012](../../../../harness/conventions.md#mr-012--mr-010-invariante-folgt-der-ausgelagerten-lastenheft-historie)):** `**Version:**` der `lastenheft.md`
  auf **0.1.10** == oberste `lastenheft-historie.md`-Zeile nachziehen.
- **Spec-Stale-Nachzug ([ADR-0015](../../adr/0015-dxf-backend.md)-Folgepflicht):** **§7** ist eine **Sammelklausel**
  (DXF/PDF/PNG) → **nur DXF** chirurgisch entfernen, **PDF/PNG bleiben offen** (kein
  eigenes ADR — nicht mit-streichen! Muster slice-020a-MED-1). **§6** legt eine **neue**
  DXF-Vertragszeile an (ADR-entschieden → [MR-001](../../../../harness/conventions.md#mr-001--source-precedence-mit-eigener-spezifikations-schicht)-erlaubte ADR→Spec-Richtung). **§4**
  [`E-IO-001`](../../../../spec/spezifikation.md#4-fehler-codes-und-logging-felder)-Bedingung um „DXF-Export" (+ STEP/STL-Parität, [ADR-0015](../../adr/0015-dxf-backend.md)-Review-LOW-2).
- **Spec-Straten ADR-frei ([MR-011](../../../../harness/conventions.md#mr-011--referenz-integritäts-gate-matrix-ids-spans-hostpaths)):** die §1-/§6-Einarbeitung der frischen
  [ADR-0015](../../adr/0015-dxf-backend.md)-Entscheidung darf **keinen** ADR-Verweis im `spezifikation.md`-Körper
  hinterlassen (Provenance nur in `spezifikation-historie.md`) — vor dem Gate per
  `grep ADR-` selbst fangen (Muster 019a/020a-Lerneintrag).
- **Subset-Treue zu [ADR-0015](../../adr/0015-dxf-backend.md) (kein Über-Versprechen):** die §1-Mechanik behauptet
  nur, was [ADR-0015](../../adr/0015-dxf-backend.md) entschied (gerade Wand-Achsen 2D, Geschoss-Layer, Default-Höhe/
  -Dicke beim Import). Räume/Bemaßung/Schraffur/Blöcke/3D = benannte Lücke.

## 7. Sub-Area-Modus-Begründung

### Sub-Area: Spec-Schreibung

- **Modus:** GF; **Konventionen-Dichte:** hoch — AK-Format (Happy/Boundary/Negative),
  Reifephase-/Teilumfang-Klausel, [MR-008](../../../../harness/conventions.md#mr-008--lastenheft-schärfung-bleibt-lösungsfrei) (lösungsfrei), [MR-010](../../../../harness/conventions.md#mr-010--lastenheft-header-version--oberste-9-historie-zeile)/012 (Header==
  Historie), Fehler-Code-Konvention ([`E-IO-001`](../../../../spec/spezifikation.md#4-fehler-codes-und-logging-felder)/[`E-IO-003`](../../../../spec/spezifikation.md#4-fehler-codes-und-logging-felder) bestehen). **Phase-Reife:**
  IO Phase 2 (Outline → AK). **Evidenz-/Diskrepanz-Risiko:** niedrig (reine Doku;
  [ADR-0015](../../adr/0015-dxf-backend.md) trägt die Backend-Entscheidung). **Reconciliation:** keiner; Folge-Slice
  = DXF-Impl.

### Sub-Area: Planning-Lifecycle

- **Modus:** GF; **Dichte:** hoch ([ADR-0015](../../adr/0015-dxf-backend.md)-Leitplanke: kein neuer Grundsatz-ADR;
  Reifephase-Teilumfang wie IFC/STEP-STL; [MR-006](../../../../harness/conventions.md#mr-006--unabhängiges-plan-review-vor-implementierungs-start) vor Impl). **Phase-Reife:** Phase 4.
  **Risiko:** niedrig.

## 8. Closure-Notiz

**Closure-Kriterien (beobachtbar, 2026-06-17):**

- **Lastenheft 0.1.10:** [`LH-FA-IO-003`](../../../../spec/lastenheft.md#lh-fa-io-003) (Import) +
  [`LH-FA-IO-004`](../../../../spec/lastenheft.md#lh-fa-io-004) (Export) von Outline auf AK-Niveau (**bidirektional,
  2D-Grundriss**, lösungsfrei/benutzer-beobachtbar); **Teilumfang gerade Wände als
  2D-Achsen je Geschoss** explizit; **Import-Defaults** (Standard-Höhe/-Dicke, DXF
  trägt keine — benannte Lücke); **Roundtrip = Achsen-Anzahl, nicht Höhe/Dicke**.
  Header 0.1.10 == oberste `lastenheft-historie.md`-Zeile ([MR-010](../../../../harness/conventions.md#mr-010--lastenheft-header-version--oberste-9-historie-zeile)/012).
- **`spezifikation.md` §1 [`LH-FA-IO-003.a`](../../../../spec/lastenheft.md#lh-fa-io-003)** (Sammelblock IO-003+004):
  ASCII-DXF-Subset-Codec **io-resident** (kein OCC), gerade Wand-Achsen je Geschoss-
  `LAYER`, Import-Defaults, **atomarer Import** (In-Memory-zuerst, generische
  [`E-IO-003`](../../../../spec/spezifikation.md#4-fehler-codes-und-logging-felder)) / **Export** ([`E-IO-001`](../../../../spec/spezifikation.md#4-fehler-codes-und-logging-felder)), Subset-Lücke/Totalität. **§6** neue
  DXF-Vertragszeile, **§7** DXF chirurgisch aus der Offene-Backends-Klausel (**PDF/PNG
  bleiben offen**), **§4** [`E-IO-001`](../../../../spec/spezifikation.md#4-fehler-codes-und-logging-felder)-Bedingung um STEP/STL/DXF-Export erweitert
  (latente ADR-0014-Lücke parallel geschlossen). `**Letzte Änderung:**` + `spezifikation-historie.md`.
- **Spec-Straten ADR-frei ([MR-011](../../../../harness/conventions.md#mr-011--referenz-integritäts-gate-matrix-ids-spans-hostpaths)):** der `spezifikation.md`-Körper trägt **keinen**
  ADR-Verweis (per `grep ADR-` selbst geprüft; Provenance nur in `*-historie.md`);
  `architecture.md` `## Geschichte` um DXF→[ADR-0015](../../adr/0015-dxf-backend.md)-Provenance ergänzt (exclude-section).
- **Kein Code/Test/ADR/Schema**; `make gates` grün (docs-check 126/0); ADR-Index-
  Folgepflicht „AK-Schärfung + Spec-Nachzug" ([ADR-0015](../../adr/0015-dxf-backend.md)) abgehakt.

**Lerneintrag (benannte Spec-Lücke + geschärfte Praxis):**

- **Benannte Spec-Lücke (DXF-2D-Verlust):** DXF trägt **keine** Höhe/Dicke → importierte
  Wände bekommen **Default-Werte**, der Roundtrip erhält nur Achsen-Anzahl + -Lage, nicht
  Höhe/Dicke. Räume/Bemaßung/Schraffur/Blöcke/Text/Bögen/3D = benannte Lücke (Skip/nicht
  geschrieben). Muster der IFC-Import-Geschoss-Höhe (transient/Default).
- **Schwester-Backend-Symmetrie (geschärfte Praxis):** DXF ist der **zweite Option-D-Fall**
  (wie IFC: nicht-nativ, Text, self-rolled Subset-Codec io-resident) — die §1-/§6-/§7-
  Schärfung folgt 1:1 dem IFC-Muster (019a), während STEP/STL (020a) wegen OCC-Nativität
  abweicht. Die §7-Chirurgie (nur DXF aus der Sammelklausel, **PDF/PNG bleiben offen**) ist
  die wiederholte slice-020a-MED-1-Lehre.
- **Latente Schwester-Lücke mitgeschlossen (neuer-Sensor-nah):** die §4-[`E-IO-001`](../../../../spec/spezifikation.md#4-fehler-codes-und-logging-felder)-
  Bedingung zählte nur „IFC-Export"; bei der DXF-Erweiterung wurde die **STEP/STL-Parität**
  ([ADR-0014](../../adr/0014-step-stl-export-backend.md) ließ sie offen, [ADR-0015](../../adr/0015-dxf-backend.md)-Review-LOW-2) gleich mitgezogen — eine im Schwester-
  ADR übersehene Doku-Lücke beim nächsten Nachzug geschlossen.

**Restrisiko / Nachfolge:** **DXF-Impl-Slice** (io-residenter DXF-Subset-Codec Reader+Writer
+ `ModelImporterPort`/`ModelExporterPort` + `ExchangeFormat::Dxf`-Dispatch **+ Import-
Dispatch-Kern-Erweiterung** [ADR-0015](../../adr/0015-dxf-backend.md)-Review-MED-1 + AK-/Roundtrip-Tests) wird startbar —
[MR-006](../../../../harness/conventions.md#mr-006--unabhängiges-plan-review-vor-implementierungs-start)-Review davor. Danach Dächer/Treppen-STEP-B-Rep · PDF/PNG ([ACC-004](../../../../spec/lastenheft.md#7-abnahmekriterien)) · Welle-4-Verifikation → M4.
