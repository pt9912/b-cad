# Roadmap — b-cad

**Status:** Aktiv. **Letzte Änderung:** 2026-06-16.

**Format-Regel:** Reihenfolge von **Wellen**, keine Reihenfolge von
Terminen. Daten sind Schätzungen, korrigierbar. Die Roadmap entstand im
Greenfield-Bootstrap (Kurs-Modul 2, Schritt 5) — sie ist eine
Feature-Sequenz, kein Reconciliation-Plan.

---

## Aktuelle Welle

**Welle-ID:** welle-4-austausch
**Start:** 2026-06-16 (bewusste Planungs-Entscheidung nach welle-3-Closure
[`../done/welle-3-results.md`](../done/welle-3-results.md) + Accept von
[ADR-0013](../../adr/0013-ifc-bibliothek.md) — Trigger „welle-3 done + ADR zu
IFC-Bibliothek accepted" erfüllt)
**Geplantes Ende:** offen (Aufwands-Schätzung L)

**Welle-Ziel:** b-cad wird **offen austauschbar** ([OBJ-005](../../../../spec/lastenheft.md#3-projektziele)): Import/Export der
offenen Formate — **IFC** ([LH-FA-IO-001](../../../../spec/lastenheft.md#lh-fa-io-001--ifc-import)/002), **DXF** ([LH-FA-IO-003](../../../../spec/lastenheft.md#lh-fa-io-003)/004),
**STEP** ([LH-FA-IO-005](../../../../spec/lastenheft.md#lh-fa-io-005)), **STL** ([LH-FA-IO-006](../../../../spec/lastenheft.md#lh-fa-io-006)) + **PDF/PNG-Export**
([LH-FA-IO-007](../../../../spec/lastenheft.md#lh-fa-io-007)/008). Jedes Format liegt hinter einem **Driven-Adapter**
(`ModelImporterPort`/`ModelExporterPort`, angestoßen über `ExchangeModelPort`/
`ExchangeService`, `adapters/io/`) — der Kern bleibt **format-frei**
([ADR-0001](../../adr/0001-hexagonale-architektur.md)). Erfüllt **Meilenstein M4**
(„Offen austauschbar", [ACC-003](../../../../spec/lastenheft.md#7-abnahmekriterien)/[ACC-004](../../../../spec/lastenheft.md#7-abnahmekriterien)).

**Scope-Entscheidung 2026-06-16:** das **IFC-Backend** fällt zuerst (Roadmap-
Trigger) — [ADR-0013](../../adr/0013-ifc-bibliothek.md) **accepted**: ein selbst
getragener IFC-SPF-**Subset**-Codec (Option D, [ADR-0004](../../adr/0004-toolchain-dependency-pinning.md)-konform — kein
Bibliotheks-Zukauf jetzt; Re-Eval auf IfcOpenShell/web-ifc benannt). Die übrigen
Backends sind **eigene** Entscheidungen: **STEP/STL** OCC-nativ (aus
[ADR-0002](../../adr/0002-geometrie-kern-opencascade.md) ausgegliedert), **DXF**
eigene Bibliothek, **PDF/PNG** Render-/Plot-Pfad — je ein Schwester-ADR.

**Closure-Trigger** (deliverable-granular; konkrete Slices emergieren mit
[MR-006](../../../../harness/conventions.md#mr-006--unabhängiges-plan-review-vor-implementierungs-start)-Plan-Review):
- ✓ **IFC-Backend-ADR** ([ADR-0013](../../adr/0013-ifc-bibliothek.md)) accepted
  (zwei unabhängige Review-Runden, 0 HIGH).
- **IFC-Schärfung + Impl:** [LH-FA-IO-001](../../../../spec/lastenheft.md#lh-fa-io-001--ifc-import)/002-AK (lösungsfrei,
  [MR-008](../../../../harness/conventions.md#mr-008--lastenheft-schärfung-bleibt-lösungsfrei)) +
  Spec-§6/§7-Nachzug; `ExchangeService` + Importer/Exporter + Subset-Codec;
  AK-Tests + **Adapter-Pfad-Integrationstest**; neue `arch-check`-io-Regel.
- ✓ **STEP/STL-Export-Backend** ([ADR-0014](../../adr/0014-step-stl-export-backend.md), OCC-DataExchange nativ,
  geometrie-residente `ModelExporterPort`-Naht) — AK-Schärfung (slice-020a) **+ Impl
  (slice-020b: STL alle 3D-Bauteile, STEP B-Rep Wände+Decken)**; **offen:** STEP-B-Rep
  für Dächer/Treppen (Mesh→Shape-Vernähung, [MR-009](../../../../harness/conventions.md#mr-009--geometrielastiges-code-review-vor-welle-closure)).
- ✓ **DXF-Backend-ADR** ([ADR-0015](../../adr/0015-dxf-backend.md), Accepted 2026-06-17 —
  selbst getragener DXF-Subset-Codec Option D io-resident, 2D-Grundriss); **offen:**
  DXF-AK-Schärfung ([LH-FA-IO-003](../../../../spec/lastenheft.md#lh-fa-io-003)/004) + Import/Export-Impl.
- **PDF/PNG-Export** (maßstäblicher Plan, [ACC-004](../../../../spec/lastenheft.md#7-abnahmekriterien)).
- Unabhängige Welle-Verifikation + Carveout-Audit + `done/welle-4-results.md`;
  [ACC-003](../../../../spec/lastenheft.md#7-abnahmekriterien) (IFC-Export) + [ACC-004](../../../../spec/lastenheft.md#7-abnahmekriterien) (PDF) erfüllt → **Meilenstein M4**.

**Fortschritt (Stand 2026-06-17):**
- ✓ **[ADR-0013](../../adr/0013-ifc-bibliothek.md) „IFC-Bibliothek" accepted** —
  IFC-Backend = vendierter SPF-Subset-Codec (Option D); zwei unabhängige
  Review-Runden (0 HIGH, 5 MED + 4 LOW eingearbeitet). Welle-Trigger erfüllt,
  Welle **gestartet**.
- ✓ **slice-019a** — **IFC-AK-Schärfung + Spec-Mapping**: [LH-FA-IO-001](../../../../spec/lastenheft.md#lh-fa-io-001--ifc-import)/002 von
  Outline auf AK (lösungsfrei, Lastenheft 0.1.8); spez. §1 [`LH-FA-IO-001.a`](../../../../spec/lastenheft.md#lh-fa-io-001--ifc-import)
  (IFC-SPF-Subset-Mapping, IFC4-Export `IfcWall`/Import IFC4+2x3, atomar) + §6/§7-
  Nachzug ([ADR-0013](../../adr/0013-ifc-bibliothek.md)-Folgepflicht). [MR-006](../../../../harness/conventions.md#mr-006--unabhängiges-plan-review-vor-implementierungs-start) **0 HIGH**; reine Doku, gates grün (145).
- ✓ **slice-019b** — **IFC-Import lauffähig** ([LH-FA-IO-001](../../../../spec/lastenheft.md#lh-fa-io-001--ifc-import)):
  hand-gerollter IFC-SPF-Subset-Codec (`SpfReader`) + Mapping-Adapter
  (`IfcImportAdapter`) + Kern-Use-Case (`ExchangeService`, Driving
  `ExchangeModelPort` / Driven `ModelImporterPort`), end-to-end über den echten
  Pfad; nicht-IFC/kaputt → [`E-IO-003`](../../../../spec/spezifikation.md#4-fehler-codes-und-logging-felder)
  atomar; spez. §1-Zusatz Geschoss-Höhe ([MR-008](../../../../harness/conventions.md#mr-008--lastenheft-schärfung-bleibt-lösungsfrei)).
  `arch-check` A/B isolieren den Codec (Regel F gegenstandslos für Option D →
  auf externen-Lib-Re-Eval umdatiert). [MR-006](../../../../harness/conventions.md#mr-006--unabhängiges-plan-review-vor-implementierungs-start) **0 HIGH** + unabhängiges
  Code-Review **0 HIGH** (1 MED + 2 LOW eingearbeitet); `make gates` grün
  (165/165, Coverage 91,2 %). Zwei-Commit-Split.
- ✓ **slice-019c** — **IFC-Export lauffähig** ([LH-FA-IO-002](../../../../spec/lastenheft.md#lh-fa-io-002),
  [ACC-003](../../../../spec/lastenheft.md#7-abnahmekriterien)): hand-gerollter IFC-SPF-Writer (`IfcSpfWriter`, Spiegel des
  Readers) + Domänen→IFC4-Mapping (`IfcExportAdapter`) + Kern-Erweiterung
  (`ExchangeModelPort.exportModel` / Driven `ModelExporterPort`), atomar
  (Temp+Rename → [`E-IO-001`](../../../../spec/spezifikation.md#4-fehler-codes-und-logging-felder)). **Roundtrip** (Export → 019b-Import) erhält
  Anzahl + Geometrie. [MR-006](../../../../harness/conventions.md#mr-006--unabhängiges-plan-review-vor-implementierungs-start) **0 HIGH** (zwei Pässe) + unabhängiges Code-Review
  **0 HIGH**; `make gates` grün (171/171, Coverage 91,5 %). **Damit ist der
  IFC-Strang (Import + Export, [ACC-003](../../../../spec/lastenheft.md#7-abnahmekriterien)) abgeschlossen.**
- ✓ **[ADR-0014](../../adr/0014-step-stl-export-backend.md) „STEP/STL-Export-Backend" accepted** (2026-06-17) —
  OCC-DataExchange **nativ** (keine neue Dependency, [ADR-0004](../../adr/0004-toolchain-dependency-pinning.md)-konform),
  geometrie-residenter `ModelExporterPort` (Regel C); unabhängiges Text-Review
  0 HIGH. **Toolchain-Beleg erbracht** (OCC 7.9.2 DataExchange vorhanden).
- ✓ **slice-020a** — **STEP/STL-AK-Schärfung + Spec-Mapping**: [LH-FA-IO-005](../../../../spec/lastenheft.md#lh-fa-io-005)/006
  von Outline auf AK (export-only, lösungsfrei, Lastenheft 0.1.9); spez. §1
  [`LH-FA-IO-005.a`](../../../../spec/lastenheft.md#lh-fa-io-005) (OCC-DataExchange, geometrie-resident, STEP B-Rep / STL Netz,
  atomar) + §6/§7-Nachzug. [MR-006](../../../../harness/conventions.md#mr-006--unabhängiges-plan-review-vor-implementierungs-start) **0 HIGH** (MED-1/2 + LOW-3); reine Doku, gates grün.
- ✓ **slice-020b** — **STEP-/STL-Export lauffähig** ([LH-FA-IO-005](../../../../spec/lastenheft.md#lh-fa-io-005)/006): geometrie-
  residente `ModelExporterPort`-Adapter — **STL** (binär, Dreiecksnetz aller 3D-Bauteile)
  + **STEP** (B-Rep der OCC-Solid-Bauteile Wände+Decken, AP214), atomar → [`E-IO-001`](../../../../spec/spezifikation.md#4-fehler-codes-und-logging-felder);
  geteilter `occ_solids`-Builder; `ExchangeService`-Exporter-Registry; export-only-
  Import → [`E-IO-003`](../../../../spec/spezifikation.md#4-fehler-codes-und-logging-felder). **Benannte STEP-Lücke:** Dächer/Treppen (analytische Netze)
  STL-only. [MR-006](../../../../harness/conventions.md#mr-006--unabhängiges-plan-review-vor-implementierungs-start) + Code-Review/[MR-009](../../../../harness/conventions.md#mr-009--geometrielastiges-code-review-vor-welle-closure) je **0 HIGH**; Zwei-Commit-Split;
  `make gates` grün (180/180, Coverage 90,0 %).
- ✓ **[ADR-0015](../../adr/0015-dxf-backend.md) „DXF-Backend" accepted** (2026-06-17) —
  **selbst getragener DXF-Subset-Codec** (Option D, wie IFC; kein Lib-Zukauf, OCC kann
  kein DXF), **io-resident**, 2D-Grundriss (gerade Wand-Achsen je Geschoss-`LAYER`);
  Import → Default-Höhe/-Dicke (benannte Lücke). Text-Review **0 HIGH** (MED-1
  Import-Dispatch-Kern-Erweiterung + MED-2 + LOW eingearbeitet).
- ⏳ offen: STEP-B-Rep Dächer/Treppen · **DXF-AK-Schärfung + Impl** · PDF/PNG ([ACC-004](../../../../spec/lastenheft.md#7-abnahmekriterien)) ·
  Welle-4-Verifikation → `done/welle-4-results.md`.

## Nächste Wellen

| Welle | Trigger | Wichtigste Slices (geplant) | Geschätzter Aufwand |
|---|---|---|---|
| welle-5-erweiterung | welle-4 done | Plugin-System (`PLG`), UI-Themes/Docking + **2D-Zeichen-Werkzeuge `DRW`** (Bemaßung/Layer/Fangpunkte/Gruppen, aus welle-3 zurückgestellt), Mehrsprachigkeit ([`LH-QA-006`](../../../../spec/lastenheft.md#lh-qa-006--mehrsprachigkeit)) | M |

## Meilensteine

| Meilenstein | Welle(n) | Trigger | Status |
|---|---|---|---|
| M1 — Lauffähiges MVP | welle-1-mvp | [ACC-001](../../../../spec/lastenheft.md#7-abnahmekriterien)-Kern erstellbar, `make gates` grün | erreicht (2026-06-12; Viewer per Drift-Entscheidung 2026-06-11 nicht Teil des Triggers) |
| M2 — Vollständige Bauteile | welle-2-bauteile | Haus mit Türen, Fenstern, Dach vollständig | **erreicht** (2026-06-14; vier Bauteil-Familien geliefert + Decken/Fundament/Treppen, welle-2-Closure) |
| M3 — Auswertbar | welle-3-auswertung | Flächen/Volumen/Materiallisten korrekt | **erreicht** (2026-06-16; `EvaluatePort` Flächen/Volumen/Wohnfläche + Material-/Kosten-/Tür-/Fensterlisten analytisch im Kern, welle-3-Closure) |
| M4 — Offen austauschbar | welle-4-austausch | [ACC-003](../../../../spec/lastenheft.md#7-abnahmekriterien), [ACC-004](../../../../spec/lastenheft.md#7-abnahmekriterien) erfüllt | offen |
| M5 — Erweiterbar | welle-5-erweiterung | [OBJ-004](../../../../spec/lastenheft.md#3-projektziele) (Plugins) erfüllt | offen |

## Abhängigkeitsgraph

```mermaid
flowchart LR
    W1[welle-1-mvp<br/>done 2026-06-12]
    W1V[welle-1v-viewer<br/>done 2026-06-13]
    W2[welle-2-bauteile<br/>done 2026-06-14]
    W3[welle-3-auswertung<br/>done 2026-06-16]
    W4[welle-4-austausch<br/>aktiv]
    W5[welle-5-erweiterung]

    W1 --> W1V
    W1 --> W2 --> W3 --> W4 --> W5
```

## Abgeschlossene Wellen

| Welle | Zeitraum | Ergebnis | Closure-Notiz |
|---|---|---|---|
| welle-1-mvp | 2026-06-08 – 2026-06-12 | Kern-MVP als Vertrag: Projekt anlegen/speichern/laden (atomar + Crash-Recovery), Geschosse, Wände, Raum-Autoerkennung, OCC-Extrusion + Echtzeit-Benachrichtigung; 13 Slices + spike-001 in `done/`; Review + Verifikation gelaufen, Findings behoben (`330d5d0`). Sichtbarer Viewer → `welle-1v-viewer`. | [`../done/welle-1-results.md`](../done/welle-1-results.md) |
| welle-1v-viewer | 2026-06-12 – 2026-06-13 | Sichtbare Hälfte des Echtzeit-Vertrags: Qt-6-3D-Viewer (Driving Adapter) stellt das extrudierte Gebäudemodell dar und folgt committeten Änderungen — **[ACC-002](../../../../spec/lastenheft.md#7-abnahmekriterien) erfüllt** + sichtbare Hälfte [LH-FA-D3-002](../../../../spec/lastenheft.md#lh-fa-d3-002--echtzeitaktualisierung); slice-011a/011b + slice-012 (Eckenschluss WAL-006-Teilumfang) in `done/`. Unabhängige Verifikation gelaufen (keine HIGH/MEDIUM, 1 LOW); `make gates` grün am HEAD (63/63, Coverage 94,2 %). | [`../done/welle-1v-results.md`](../done/welle-1v-results.md) |
| welle-2-bauteile | 2026-06-13 – 2026-06-14 | **Alle parametrischen Bauteile** über die Wände hinaus: Türen/Fenster (automatische Wandöffnung, OCC-Boolean), Dach (Sattel/Walm/Pult), Decken/Fundament (Platten + Ausschnitte), Treppen (gerade einläufig) — je Familie Lastenheft-AK-Schärfung + Implementierung (Domäne/Geometrie/Viewer/Edit-Ops) + Persistenz; **12 Slices** in `done/`, **[ADR-0011](../../adr/0011-bauteil-hosting-wandoeffnung.md) (#6)-Leitplanke** über vier Familien. **Meilenstein M2 erreicht** + [ACC-001](../../../../spec/lastenheft.md#7-abnahmekriterien)-Bauteil-Hälfte. Unabhängige Verifikation (keine HIGH, 1 MED/1 LOW behoben) + Carveout-Audit (keine aktiven); `make gates` grün am HEAD `d7073fb` (116/116, Coverage 92,3 %). Geometrielastige Code-Reviews je Familie (013b/014b/015b je 1 HIGH gefixt, 016b keine HIGH). | [`../done/welle-2-results.md`](../done/welle-2-results.md) |
| welle-3-auswertung | 2026-06-14 – 2026-06-16 | **Gebäudemodell auswertbar** ([ADR-0012](../../adr/0012-evaluations-architektur.md) `EvaluatePort` read-only/pull, **kein** `GeometryKernelPort`/`Solid.volume_mm3`): Flächen EVL-001/003 (Shoelace-Raum-Netto + Wohnfläche), **Volumen EVL-002 analytisch im Kern** (Wand/Decke/Treppe; Dach dicke-los → benannte Lücke), **Material-System** MAT-001/002/003/005/006 (projekt-eigen über `EditStructurePort`, `restrict`-treu, NULL-sicher round-trippt über SQLite), **Listen** EVL-004/005/006 (Material-Menge=Σ Netto-Volumen, Tür-/Fensterlisten) + **Kosten MAT-006** (`Menge × cost_per_m3`); `wall_type`-Template-Fallback bewusst zurückgestellt (welle-4+). **7 Slices** (017a–017g) in `done-archive/`. **Meilenstein M3 erreicht**. Unabhängige Verifikation (0 HIGH, 1 LOW behoben) + Carveout-Audit (keine aktiven); `make gates` grün am HEAD (145/145, Coverage 92,7 %), `make schema-check` grün. | [`../done/welle-3-results.md`](../done/welle-3-results.md) |

## Historische Trigger-Verschiebungen

| Datum | Was wurde geändert? | Warum? |
|---|---|---|
| 2026-06-09 | `slice-003` in `slice-003a` (Kern, OCC-frei) + `slice-003b` (OCC-Extrusion + arch-check Regel C) geschnitten | Slice zu groß für eine Review-Sitzung (Modul 5); OCC-Teil ist build-schwer/risikobehaftet und wird isoliert. [ADR-0002](../../adr/0002-geometrie-kern-opencascade.md) dabei auf Backend-Scope verengt + accepted (slice-003-Review, Findings 1–3). |
| 2026-06-11 | `slice-009` in `slice-009a` ([ADR-0007](../../adr/0007-raumerkennung-geometrie-basis.md) + Spec-Schärfung) + `slice-009b` (Implementierung + Tests) geschnitten | Plan-Review-Findings H1/M1/M2: [ADR-0007](../../adr/0007-raumerkennung-geometrie-basis.md) trägt mehr Entscheidungsgewicht als geplant (Polygon-Basis **und** Verschachtelungs-Repräsentation), ADR-Accept ist Review-Checkpoint und gehört nicht mitten in einen Implementierungs-Slice (Präzedenz slice-007, slice-003-Split). |
| 2026-06-11 | Sichtbarer 3D-Viewer aus welle-1 in eigene Welle `welle-1v-viewer` gelöst; Welle-Ziel und Viewer-Trigger-Zeile angepasst | Scope-Entscheidung slice-010a: GUI-Grundsatz-ADR (Qt 6) fehlt noch, M1-Trigger ([ACC-001](../../../../spec/lastenheft.md#7-abnahmekriterien)-Kern + Gates) verlangt keinen Viewer; [ACC-002](../../../../spec/lastenheft.md#7-abnahmekriterien) wird in `welle-1v-viewer` erfüllt — kein stilles `done` über den Kern-Vertrag (Lastenheft-Wortlaut „sichtbar" bleibt unverändert benutzer-beobachtbar). |
| 2026-06-12 | `welle-1v-viewer` um slice-012 erweitert (Eckenschluss endpunkt-verbundener Wände, [LH-FA-WAL-006](../../../../spec/lastenheft.md#lh-fa-wal-006--wand-verbinden)-Teilumfang); slice-011b-Abnahme (DoD-4) auf den regenerierten Beleg verschoben | Abnahme-Befund des Projektinhabers am [ACC-002](../../../../spec/lastenheft.md#7-abnahmekriterien)-Beleg: Wände schließen an Außenecken nicht (fehlendes ½×½-Stärke-Quadrat, [Befund-2D](../done/acc-002-befund-2d-ecken.png)) — modell-treu gerendert, aber als Abnahme-Artefakt nicht tragfähig; WAL-006-Teilumfang wird vorgezogen statt die Grenze nur zu dokumentieren. |
| 2026-06-14 | `welle-3-auswertung` gestartet; Scope auf **MAT + EVL** (Auswertungs-Kern, M3) gesetzt, **`DRW` (Bemaßung/Layer/Fangpunkte/Raster/Hilfslinien/Gruppen) nach welle-5 zurückgestellt** | Welle-Name + M3-Trigger („Flächen/Volumen/Materiallisten korrekt") zielen auf Auswertung; `DRW` ist 2D-Zeichen-Interaktion (UX) ohne M3-Bezug und passt zu den UI-Werkzeugen von welle-5 — die Trennung hält welle-3 kohärent (Modul-5-Sizing, Auswertung ≠ 2D-Editor). |
| 2026-06-15 | **Quergewerk slice-018a/b/c** eingeschoben (Doku-Referenz-Gate, `harness-steering`): `done-archive/`-Mechanik + Regelwerk-Referenz-Richtung Spec→ADR computational (d-check `matrix`/`ids`, [MR-011](../../../../harness/conventions.md#mr-011--referenz-integritäts-gate-matrix-ids-spans-hostpaths)); **018b** weitet `ids` auf den Voll-Korpus (alle 7 ID-Familien, Linker `tools/idlink.py`), **018c** hebt Bullet-Sub-IDs per Inline-HTML-Anker (d-check v0.9.0) auf präzise Per-ID-Anker. **M3-Scope (MAT+EVL) unberührt.** | d-check-v0.8.0-Hebung stellt `matrix`/`ids`/`spans`/`hostpaths` bereit; die Referenz-Richtung war bis dahin nur inferential ([MR-006](../../../../harness/conventions.md#mr-006--unabhängiges-plan-review-vor-implementierungs-start)-Plan-Review). Quergewerk, kein welle-3-Feature — die Roadmap-Sequenz bleibt unverändert. |
