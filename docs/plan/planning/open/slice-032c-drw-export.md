---
id: slice-032c
titel: DRW-Export — Hilfslinien im 2D-Grundriss (DXF/PDF/PNG) mit Ebenen-Sichtbarkeits-Filter
status: open
welle: welle-5-erweiterung
lastenheft_refs: [[LH-FA-DRW-005](../../../../spec/lastenheft.md#lh-fa-drw-005), [LH-FA-DRW-006](../../../../spec/lastenheft.md#lh-fa-drw-006), [LH-FA-IO-004](../../../../spec/lastenheft.md#lh-fa-io-004), [LH-FA-IO-007](../../../../spec/lastenheft.md#lh-fa-io-007), [LH-FA-IO-008](../../../../spec/lastenheft.md#lh-fa-io-008)]
adr_refs: [[ADR-0001](../../adr/0001-hexagonale-architektur.md), [ADR-0015](../../adr/0015-dxf-backend.md), [ADR-0016](../../adr/0016-pdf-png-backend.md), [ADR-0018](../../adr/0018-drw-2d-zeichen-daten.md)]
---

# Slice 032c: DRW-Export — Hilfslinien im 2D-Grundriss (DXF/PDF/PNG), Ebenen-Sichtbarkeits-Filter

**Status:** open (Plan **startbar** nach Review-Einarbeitung — Impl-Start auf Projektinhaber-Wort).
**[MR-006](../../../../harness/conventions.md#mr-006--unabhängiges-plan-review-vor-implementierungs-start)-Plan-Review**
2026-07-22 (Reviewer ≠ Autor, read-only): **0 HIGH / 1 MED / 1 LOW / 3 INFO → startbar**
([Report](../../../reviews/2026-07-22-slice-032c-plan.md)). **M1 eingearbeitet** (AK-Umetikettierung:
invisible→absent ist **DRW-005-Negative + DRW-006-Happy-Sichtbarkeits-Klausel**, **nicht** DRW-006-Negative
[= Löschschutz, schon in 032b geschlossen]); L1 ([LH-FA-IO-008](../../../../spec/lastenheft.md#lh-fa-io-008) ins Frontmatter); I1–I3 bestätigt.
**[MR-009](../../../../harness/conventions.md#mr-009--geometrielastiges-code-review-vor-welle-closure) n/a**
(Hilfslinie = 2-Punkt-`LINE`/-Segment, **keine** neue Solid-Geometrie); Export-Latte:
zusätzliches unabhängiges **Code-Review vor Welle-Closure** (Decode-Orakel-/self-rolled-Writer-
Klasse, Muster [slice-025b](../done/slice-025b-pdf-export-impl.md)/[slice-025c](../done/slice-025c-png-export-impl.md)).

**Welle:** welle-5-erweiterung (DRW-Strang, **Export-Hälfte** des Fundaments; Nachfolge
[slice-032b](../done/slice-032b-drw-impl.md) [Impl/Persistenz]; schließt die DRW-Fundament-Sequenz
032a→b→c).

**Bezug:** [LH-FA-DRW-005](../../../../spec/lastenheft.md#lh-fa-drw-005) (Hilfslinien) /
[LH-FA-DRW-006](../../../../spec/lastenheft.md#lh-fa-drw-006) (Layer/Ebenen) — die **Export-Teilklauseln**
der bereits (in [slice-032a](../done/slice-032a-drw-fundament-ak-spec.md)) auf AK-Niveau gesetzten
Anforderungen: **DRW-005-Happy** (Export) **„when der 2D-Grundriss exportiert wird, then erscheint sie im
Artefakt"** und **DRW-005-Negative** (unsichtbare Ebene → keine Hilfslinie dieser Ebene im Artefakt),
getragen von der **DRW-006-Happy-Sichtbarkeits-Klausel** (die Ebenen-Sichtbarkeit steuert die
Export-Sichtbarkeit ihrer Hilfslinien). **NICHT** DRW-006-Negative — das ist der Löschschutz einer
referenzierten Ebene und wurde bereits in [slice-032b](../done/slice-032b-drw-impl.md) geschlossen. Die 2D-Grundriss-Formate
[LH-FA-IO-004](../../../../spec/lastenheft.md#lh-fa-io-004) (DXF) /
[LH-FA-IO-007](../../../../spec/lastenheft.md#lh-fa-io-007)/008 (PDF/PNG) sind der Träger.
**[ADR-0018](../../adr/0018-drw-2d-zeichen-daten.md)** hat die **Subset-Erweiterung autorisiert**
(§Entscheidung 5: Hilfslinien auf sichtbarer Ebene in den 2D-Zeichenumfang; DXF `LINE` auf Geschoss-
`LAYER`, PDF/PNG als 2D-Segment) — **kein** stiller Drift.
[ADR-0015](../../adr/0015-dxf-backend.md) (DXF-Subset-Codec, Geschoss-`LAYER`),
[ADR-0016](../../adr/0016-pdf-png-backend.md) (PDF-Vektor-/PNG-Raster-Writer, io-resident),
[ADR-0001](../../adr/0001-hexagonale-architektur.md) (Export io-resident, Regel A/B).

**Autor:** Dietmar Burkard (AI-Harness-Lauf). **Datum:** 2026-07-22.

**Schnitt-Herkunft:** Export-Hälfte des DRW-Fundaments — macht die in
[slice-032b](../done/slice-032b-drw-impl.md) durable Hilfslinie **sichtbar im 2D-Artefakt** und
schließt damit die von 032b **bewusst offen gelassene** Export-AK. Beobachtbar über **Decode-Orakel**
je Format (Hilfslinie erscheint bei sichtbarer Ebene / fehlt bei unsichtbarer).

**Bewusst NICHT Teil (benannte Grenzen / Folge):**

- **Distinkte Hilfslinien-Darstellung** (eigene Farbe/Linientyp/DXF-Layer-Name) — [ADR-0018](../../adr/0018-drw-2d-zeichen-daten.md) zeichnet
  die Hilfslinie als **schlichtes 2D-Segment** auf dem Geschoss-`LAYER` (PDF/PNG in der Geschoss-Farbe
  wie Wände). Eine eigene Benutzer-Ebene→DXF-Layer-Abbildung ist ein benannter Re-Eval
  ([ADR-0018](../../adr/0018-drw-2d-zeichen-daten.md) §Re-Evaluierungs-Trigger).
- **DXF-Re-Import der Hilfslinie als Hilfslinie** — der DXF-Import liest eine `LINE` auf `STOREY_n`
  als **Wand-Achse** (Round-Trip-Asymmetrie, in [slice-032a](../done/slice-032a-drw-fundament-ak-spec.md)
  §1 benannt); 032c ändert den **Import nicht**. Das Decode-Orakel nutzt daher den **rohen `DxfReader`**
  (zählt `LINE`-Entitäten), **nicht** den Wand-Round-Trip.
- **Bemaßung/Schraffur/Räume** bleiben aus dem Subset ausgeschlossen ([ADR-0015](../../adr/0015-dxf-backend.md)/[0016](../../adr/0016-pdf-png-backend.md)/[0018](../../adr/0018-drw-2d-zeichen-daten.md) unverändert).
- **Bauteil-Layer / `entity_layers` / interaktiver 2D-Canvas** — benannte Re-Eval-Trigger.

---

## 1. Ziel

Der 2D-Grundriss-Export zeichnet jede **Hilfslinie auf sichtbarer Ebene** als 2D-Segment; eine
Hilfslinie auf **unsichtbarer Ebene** wird **nicht** gezeichnet (Export-Filter). Damit werden die
Export-Teilklauseln von [LH-FA-DRW-005](../../../../spec/lastenheft.md#lh-fa-drw-005)/006 **benutzer-
beobachtbar** — der Nutzer öffnet den exportierten Grundriss (DXF/PDF/PNG) und sieht die Hilfslinie
bzw. sieht sie nach Unsichtbarschalten der Ebene nicht mehr.

**Design-Entscheidung (zentral, [MR-006](../../../../harness/conventions.md#mr-006--unabhängiges-plan-review-vor-implementierungs-start)-Prüfstein) —
der Sichtbarkeits-Filter ist eine *eine* geteilte Quelle in `plan_geometry`; die Zeichnung teilt sich
in zwei Pfade nach der bestehenden Export-Architektur:**

- **Sichtbarkeits-Filter (single source):** ein Helfer `visibleLayerIds(const Building&)` (Menge der
  `LayerId` mit `layer.visible == true`) in `plan_geometry.h` — format-agnostisch, von **beiden**
  Pfaden genutzt (kein Drift).
- **PDF/PNG (via `plan_geometry`):** `projectPlan(building)` faltet die **sichtbaren** Hilfslinien-
  Segmente je Geschoss in `StoreyPlan.segments` **und** in die geteilte **Bounding-Box** (heute
  wand-only) — PDF/PNG rendern sie unverändert mit (schlichtes Segment, [ADR-0018](../../adr/0018-drw-2d-zeichen-daten.md)).
- **DXF (direkt aus `building`, Bestandsstil):** `dxf_export_adapter.cpp::writeEntities` erhält einen
  zweiten, filter-gedeckten Loop über `building.guide_lines` → `LINE` auf `layerName(gl.storey_id)`
  (`STOREY_n`). **DXF wird NICHT durch `plan_geometry` geroutet** — DXF hat **kein** Skalierung/keine
  BBox (Filter genügt), und ein Reroute verlangte `storey_id`/Kind auf `PlanSegment` (größerer Umbau
  ohne Nutzen). **Die DXF-`LAYER`-Tabelle (`writeLayerTable`) bleibt unverändert** — Hilfslinien reiten
  den bestehenden Geschoss-`LAYER`, der Benutzer-`visible`-Filter ist **kein** neuer DXF-Layer
  ([ADR-0018](../../adr/0018-drw-2d-zeichen-daten.md) §3: zwei „Layer"-Begriffe getrennt).

**Korrektheits-Kern (Code-Review-Primärziel):** (a) **Filter-Konsistenz** — dieselbe
`visibleLayerIds`-Quelle in beiden Pfaden (ein unsichtbarer-Layer-Leck in nur einem Format wäre ein
Defekt); (b) **BBox-Vollständigkeit** — eine Hilfslinie außerhalb der Wand-BBox muss in PDF/PNG
**auf die Seite passen** (BBox muss sichtbare Hilfslinien einschließen), sonst wird sie abgeschnitten;
(c) **Geschoss-Zuordnung** — die Hilfslinie erscheint auf **ihrem** Geschoss (DXF `STOREY_n`, PDF/PNG
je Geschoss-Seite/Palette).

## 2. Definition of Done

- [ ] **`plan_geometry.{h,cpp}`:** Helfer `visibleLayerIds(const model::Building&) → std::unordered_set<int>`
      (LayerId-Rohwerte der sichtbaren Ebenen). `projectPlan` faltet je Geschoss die Hilfslinien mit
      **sichtbarer** Ebene und `guide.storey_id == storey.id` als `PlanSegment` (aus `guide.segment`) in
      `StoreyPlan.segments` **und** in die BBox (`min/max_x/y`, `has_geometry`) — **nach** dem Wand-Loop,
      damit ein hilfslinien-only-Geschoss eine gültige BBox erhält.
- [ ] **`dxf_export_adapter.cpp`:** in `writeEntities` ein zweiter Loop über `building.guide_lines`,
      gefiltert per `visibleLayerIds`, → `LINE` (Gruppencode 8 = `layerName(gl.storey_id)`; 10/20/30 +
      11/21/31 aus `guide.segment.start/end`, z = 0). **`writeLayerTable` unverändert** (kein neuer
      Layer). Unsichtbare Ebene → kein `LINE`.
- [ ] **CLI-Demo** (`src/main.cpp`, `buildAcc001KernDemo`): eine **sichtbare** Ebene + eine Hilfslinie
      auf einem bestehenden Geschoss über `service.addLayer(...)`/`addGuideLine(...)` — damit
      `make io-smoke` den Zeichen-Pfad **end-to-end** übt (2D-Export mit Hilfslinien-Inhalt). **[ACC-002](../../../../spec/lastenheft.md#7-abnahmekriterien)-
      Beleg unberührt** (Hilfslinien sind **2D-export-only**, **nicht** im 3D-`ViewModelPort` —
      [ADR-0018](../../adr/0018-drw-2d-zeichen-daten.md) §2; der headless-3D-Render ändert sich nicht).
- [ ] **Decode-Orakel-AK-Tests** (je Format ein **Erscheint** [DRW-005-Happy] + ein **Fehlt**
      [DRW-005-Negative, getragen von der DRW-006-Happy-Sichtbarkeits-Klausel]; LH-id im Suite-Namen —
      `LH_FA_DRW_005` für Erscheinen/Fehlen, `LH_FA_DRW_006` für die Sichtbarkeits-Klausel; Muster der
      bestehenden Export-Decode-Orakel):
      **DXF** (roher `DxfReader::parse` auf die exportierte Datei, **nicht** Wand-Round-Trip):
      sichtbare Ebene → genau die erwartete Zahl `LINE`-Entitäten (Wände + 1 Hilfslinie) auf `STOREY_n`,
      mit den Hilfslinien-Koordinaten; unsichtbare Ebene → nur die Wand-`LINE`s (Hilfslinie fehlt).
      **PDF** (`countOccurrences(page, " l\n")` je Geschoss-Seite): sichtbar → Segment-Zahl inkl.
      Hilfslinie; unsichtbar → unverändert. **PNG** (unabhängiger Decoder → Tinte/Pixel): sichtbare
      Hilfslinie außerhalb der Wände → Tinte an erwarteter Stelle; unsichtbar → keine zusätzliche Tinte.
      **`plan_geometry`-Sonde:** `projectPlan` liefert die Hilfslinie nur bei sichtbarer Ebene + BBox
      schließt sie ein (falls ein `test_plan_geometry` existiert; sonst über die PDF/PNG-Orakel gedeckt).
      **Distinkte Koordinaten** (Hilfslinie ≠ Wand-Koordinaten, x ≠ y) — sonst fängt das Orakel keinen
      Vertausch (Lehre [slice-032b](../done/slice-032b-drw-impl.md)-Review-MED-1).
- [ ] **`spec/spezifikation.md` §1** [`LH-FA-DRW-005.a`](../../../../spec/spezifikation.md) um
      „Export-Sichtbarkeit **aktiv**: sichtbare Hilfslinie im DXF/PDF/PNG-Grundriss, unsichtbare Ebene →
      Export-Filter" ergänzt (Impl-Stand; ADR-/slice-token-frei im Körper,
      [MR-011](../../../../harness/conventions.md#mr-011--referenz-integritäts-gate-matrix-ids-spans-hostpaths)/[MR-014](../../../../harness/conventions.md#mr-014--referenz-richtungs-verschärfung-adr-nennt-keine-slice-d-check-v0371),
      vor dem Gate selbst greppen). **`spezifikation-historie.md`** Provenance-Zeile +
      `**Letzte Änderung:**`-Header. **Lastenheft unberührt** (AK liegen seit 032a vor, kein
      [MR-010](../../../../harness/conventions.md#mr-010--lastenheft-header-version--oberste-9-historie-zeile)).
- [ ] **[ADR-Index](../../adr/README.md)-Folgepflicht:** die [ADR-0018](../../adr/0018-drw-2d-zeichen-daten.md)-
      **Export-Zeile** → erfüllt. **CHANGELOG** ([MR-004](../../../../harness/conventions.md#mr-004--top-level-changelogmd-keep-a-changelog)).
- [ ] **Gates:** engster Sensor zuerst (`make test`), dann **`make gates` grün** (a-check/arch-check
      unverändert — **keine** neue Regel, io-resident Codecs header-frei;
      [ADR-0015](../../adr/0015-dxf-backend.md)/[0016](../../adr/0016-pdf-png-backend.md) „Regel F
      gegenstandslos"); zusätzlich **`make io-smoke`** grün (CLI-Demo mit Hilfslinie exportiert je Format,
      exit 0 + nicht-leer). **`make schema-check` unberührt** (kein Schema-Edit — reine Export-Seite).
      **Unabhängiges Code-Review vor Closure** (Decode-Orakel-Latte) ohne offene HIGH. Closure-Notiz.
- [ ] **Lifecycle:** beim Start `git mv open/ → in-progress/` **+ Ruhe-Sentinel** entfernen (selber
      Commit, [MR-017](../../../../harness/conventions.md#mr-017--planning-lifecycle-gate-d-check-modul-planning),
      [AGENTS §2.8](../../../../AGENTS.md)); bei Closure `git mv → done/` (+ Sentinel zurück).

## 3. Plan (vor Code)

| Datei / Komponente | Änderungs-Art | Begründung |
|---|---|---|
| `src/adapters/io/plan_geometry.h` | ändern | `visibleLayerIds`-Helfer (single-source-Filter) |
| `src/adapters/io/plan_geometry.cpp` | ändern | sichtbare Hilfslinien-Segmente in `projectPlan` (Segmente + BBox) |
| `src/adapters/io/dxf_export_adapter.cpp` | ändern | zweiter `writeEntities`-Loop über `guide_lines` (gefiltert, `STOREY_n`) |
| `src/main.cpp` | ändern | CLI-Demo: sichtbare Ebene + Hilfslinie (io-smoke-Übung) |
| `tests/adapters/test_dxf_export.cpp` | ändern | DXF-Decode-Orakel Erscheint/Fehlt (roher `DxfReader`) |
| `tests/adapters/test_pdf_export.cpp` | ändern | PDF-Decode-Orakel Erscheint/Fehlt (` l\n`-Zählung) |
| `tests/adapters/test_png_export.cpp` | ändern | PNG-Decode-Orakel Erscheint/Fehlt (Tinte/Pixel) |
| `spec/spezifikation.md` | ändern | §1 Export-Sichtbarkeit aktiv (ADR-/slice-frei) |
| `spec/spezifikation-historie.md` | ändern | Provenance-Zeile + Header |
| [ADR-Index](../../adr/README.md) | ändern (Closure) | [ADR-0018](../../adr/0018-drw-2d-zeichen-daten.md) Export-Zeile erfüllt |
| `CHANGELOG.md` | ändern | Slice-Eintrag ([MR-004](../../../../harness/conventions.md#mr-004--top-level-changelogmd-keep-a-changelog)) |
| `tools/io-smoke.sh` | ggf. ändern | nur falls eine DRW-spezifische Zusatz-Assertion sinnvoll (sonst deckt der Demo-Inhalt den Pfad) |
| `docs/reviews/2026-07-22-slice-032c-plan.md` | neu (erledigt) | [MR-006](../../../../harness/conventions.md#mr-006--unabhängiges-plan-review-vor-implementierungs-start)-Report (0 HIGH → startbar) |
| `docs/reviews/{2026-07-2x-slice-032c-code-review}.md` | neu | Code-Review-Report (Decode-Orakel-Latte) |

**Kein neues Source-/Test-File nötig** (alle Änderungen in bestehenden `io/`-Dateien + Export-Tests →
kein CMake-Edit). **Kein Schema-Edit** (reine Export-Seite; `schema-check` unberührt).

## 4. Trigger

- **[slice-032b](../done/slice-032b-drw-impl.md) done ✓** (Hilfslinie/Ebene durabel im Modell + Persistenz;
  `EditDrawingPort` am `StructureEditService`; `building.guide_lines`/`layers` vorhanden).
- **[ADR-0018](../../adr/0018-drw-2d-zeichen-daten.md) §5 Subset-Erweiterung ✓** (Hilfslinien autorisiert
  im 2D-Zeichenumfang) + [slice-032a](../done/slice-032a-drw-fundament-ak-spec.md) Spec-Subset-Grenze.
- **[MR-006](../../../../harness/conventions.md#mr-006--unabhängiges-plan-review-vor-implementierungs-start)-Plan-Review
  (Reviewer ≠ Autor) — gelaufen ✓ 2026-07-22, 0 HIGH / 1 MED / 1 LOW / 3 INFO → startbar**
  ([Report](../../../reviews/2026-07-22-slice-032c-plan.md); M1/L1 eingearbeitet). **Keine HIGH offen.**

## 5. Closure-Trigger

- DoD vollständig, `make gates` + `make io-smoke` grün, **Code-Review ohne offene HIGH**, Closure-Notiz →
  **DRW-Fundament (032a→b→c) komplett** (Hilfslinie durabel **und** sichtbar). Nächste DRW-Familien
  (Fangpunkte/Raster/Winkel/Bemaßung/Gruppen) bleiben eigene spätere Slices.

## 6. Risiken und offene Punkte

- **Filter-Drift zwischen den Pfaden (HIGH-Klasse):** PDF/PNG filtern in `plan_geometry`, DXF im
  Adapter — beide **müssen** dieselbe `visibleLayerIds`-Quelle nutzen. Ein Format, das den Filter
  vergisst, leakt unsichtbare Hilfslinien. Mitigation: **ein** Helfer, je Format ein **Fehlt**-Orakel
  (DRW-005-Negative, getragen von der DRW-006-Happy-Sichtbarkeits-Klausel) — das Review prüft, dass alle drei Formate filtern.
- **BBox-Abschnitt (MED, PDF/PNG):** die Wand-only-BBox würde eine Hilfslinie außerhalb der Wände
  **abschneiden**. `projectPlan` muss sichtbare Hilfslinien in die BBox aufnehmen. Ein Test mit einer
  Hilfslinie außerhalb der Wand-Ausdehnung belegt das (PNG-Tinte an der erwarteten Kante).
- **DXF-Round-Trip-Asymmetrie (benannt, kein Defekt):** eine exportierte Hilfslinien-`LINE` re-importiert
  als **Wand** (kein Hilfslinien-Import). Das Decode-Orakel nutzt den **rohen `DxfReader`** (zählt
  `LINE`s direkt), **nicht** den Wand-Round-Trip — sonst falsch-positive Wand-Zahl. In
  [slice-032a](../done/slice-032a-drw-fundament-ak-spec.md) §1 als Asymmetrie bereits benannt.
- **Orakel-Blindheit (Lehre 032b-MED-1):** distinkte Hilfslinien-Koordinaten (≠ Wände, x ≠ y), sonst
  fängt das Erscheint-Orakel keinen Koordinaten-/Geschoss-Vertausch. In die Test-Werte einbauen.
- **CLI-Demo ↔ [ACC-002](../../../../spec/lastenheft.md#7-abnahmekriterien) (MED):** die Demo bekommt eine Hilfslinie für io-smoke; da Hilfslinien
  **2D-export-only** sind (nicht im 3D-`ViewModelPort`), bleibt der **[ACC-002](../../../../spec/lastenheft.md#7-abnahmekriterien)-3D-Beleg unverändert** —
  im Plan-Review verifizieren (kein 3D-Render-Drift).
- **Keine neue Gate-Regel ([ADR-0015](../../adr/0015-dxf-backend.md)/[0016](../../adr/0016-pdf-png-backend.md)/[ADR-0018](../../adr/0018-drw-2d-zeichen-daten.md) §6):**
  reine `io/`-Änderungen, header-frei → Regel A+B genügen; `a-check`/`arch-check` unverändert
  (Selbst-Check: grün ohne Config-Edit).
- **Spec-Straten ADR-/slice-frei ([MR-011](../../../../harness/conventions.md#mr-011--referenz-integritäts-gate-matrix-ids-spans-hostpaths)/[MR-014](../../../../harness/conventions.md#mr-014--referenz-richtungs-verschärfung-adr-nennt-keine-slice-d-check-v0371)):**
  §1-Nachzug ohne ADR-/Slice-Token im Körper — vor dem Gate greppen.

## 7. Sub-Area-Modus-Begründung

### Sub-Area: Import/Export-Adapter `src/adapters/io/`

- **Modus:** GF; **Dichte:** hoch (2D-Grundriss-Subset-Konvention, self-rolled-Writer, Decode-Orakel,
  atomarer Export, Regel A+B). **Phase-Reife:** 4 (Export-Muster über IFC/DXF/STEP/STL/PDF/PNG etabliert).
  **Risiko:** mittel — Export-Sichtbarkeit trägt die Filter-Drift-/BBox-Klasse → **Code-Review-Latte**
  (Muster [025b](../done/slice-025b-pdf-export-impl.md)/[025c](../done/slice-025c-png-export-impl.md)).

### Sub-Area: Test-Infrastruktur `tests/`

- **Modus:** GF; **Dichte:** hoch (Decode-Orakel je Format, Erscheint/Fehlt-Paar, distinkte Werte).
  **Risiko:** niedrig.

### Sub-Area: Spec-Schreibung `spec/`

- **Modus:** GF; **Dichte:** niedrig (§1 Export-Sichtbarkeits-Notiz, ADR-/slice-frei). **Risiko:** niedrig.

### Sub-Area: Build & Toolchain (`src/main.cpp`, `tools/io-smoke.sh`)

- **Modus:** GF; **Dichte:** mittel (CLI-Demo-/io-smoke-Konvention). **Risiko:** niedrig (Demo-Zuwachs,
  [ACC-002](../../../../spec/lastenheft.md#7-abnahmekriterien)-neutral).

## 8. Closure-Notiz

*(bei Closure ausgefüllt: gelieferte Dateien, Test-Zahl, `make gates`/`make io-smoke`-Ergebnis,
Review-Ergebnisse [[MR-006](../../../../harness/conventions.md#mr-006--unabhängiges-plan-review-vor-implementierungs-start) + Code-Review], Lerneintrag, DRW-Fundament-Abschluss 032a→b→c.)*
