# Text-Review — ADR-0018 „DRW-Fundament — 2D-Zeichen-Daten"

**Datum:** 2026-07-05 · **Reviewer:** unabhängig (≠ Autor), read-only · **Status geprüft:** Proposed.
**Prüfumfang:** ADR gegen AGENTS/harness (MR-006/008/009/011/014), `data-model.yaml`, `lastenheft.md`
§DRW, `spezifikation.md` §4, ADR-0006/0012 (Volltext), realen Quellcode (`ports/**`,
`structure_edit_service.cpp`, `adapters/ui/`, `adapters/io/plan_geometry.*`).

**Gesamturteil: solide, überdurchschnittlich sorgfältig, akzept-fähig nach Einarbeitung. Kein
HIGH-Blocker.** Jede am Code prüfbare tragende Behauptung stimmt. Die offenen Fragen sind bewusst an
slice-032a (MR-006) delegiert und dort ehrlich benannt. 2 MED vor Accept bzw. spätestens im
slice-032a-MR-006 scharf stellen.

## Verifizierte Behauptungen (alle bestätigt)

- Material meldet **keinen** `op` (Präzedenz „kein neuer op") — `model_changed_port.h` + `structure_edit_service.cpp`. ✓
- Material via `EditStructurePort` (Bauteil-Eigenschaft) — `edit_structure_port.h:154–186`. ✓
- `ViewModelPort` strikt 3D-Tessellation (nur WallMesh/…{TriangleMesh}). ✓
- GUI-Viewer reines 3D, **kein** 2D-Canvas — `viewer_widget.cpp` (paintGL/GL), kein QGraphicsScene/View. ✓
- 2D nur io-resident in `plan_geometry.cpp` (in 025b/c geteilt). ✓
- `layers` vor-deklariert, deckt LH-FA-DRW-006 wörtlich (name/visible/locked/color_hex + projekt-eindeutig) — `data-model.yaml:232–243` inkl. `uq_layers_project_name`. ✓ exakt
- `entity_layers` = polymorph, kein FK auf entity_id — `data-model.yaml:255`. ✓
- `EvaluatePort` als eigener Port (Präzedenz Zeichen-Port-Fork) — ADR-0012 §Entscheidung 1. ✓
- LH-FA-DRW-005/006 reine Outline-Einzeiler — `lastenheft.md:585–591`. ✓
- E-VAL-001 existiert — `spezifikation.md:893`. ✓

## Findings

- **MED-1 — Beobachtbarkeit hat in v1 keinen Nutzer-Erzeugungsweg (zentrales Rest-Risiko).** Die AK
  „angelegte Hilfslinie überlebt Speichern/Laden + erscheint im Export" hat als Vorbedingung „Nutzer legt
  Hilfslinie an" — in v1 über **keine** GUI erreichbar (kein Canvas, kein Menü-Weg; die Driving-Naht wird
  real nur von Tests/Plugin-Host getrieben). Faktisch reduziert sich die AK auf „falls eine Hilfslinie im
  Modell existiert → Round-Trip + Export"; MR-008 verlangt aber benutzer-beobachtbare Given/When/Then. Die
  ADR benennt das ehrlich (Z. 51/76/85) + liefert einen sauberen Ausweg (vorgezogener minimaler 2D-Canvas,
  kein weichgezeichnetes AK). **→ slice-032a-MR-006 muss explizit entscheiden**, ob „benutzer-beobachtbar"
  ohne Erzeugungs-Nutzerweg tragfähig ist, oder ob ein minimaler Erzeugungs-Weg (Menüaktion „Hilfslinie bei
  X" ohne Canvas, oder vorgezogener Canvas) in den Fundament-Schnitt gehört. **Der Punkt, an dem der Slice
  fallen kann.**
- **MED-2 — E-VAL-001-Wiederverwendung (3 Ablehnungs-Fälle) vs. dessen Klemm-Semantik.** E-VAL-001 trägt in
  §4 „auf Grenzwert **geklemmt**" (`spezifikation.md:893`); die drei DRW-Fälle (entartete Hilfslinie Länge 0,
  leerer Layer-Name, Löschen referenzierter Ebene) sind **harte Ablehnungen** („Modell unverändert"), nicht
  Klemmung. Verteidigbar (Spec nutzt E-VAL-001 bereits dual, Z. 137/345), aber der §4-Nachzug in slice-032a
  **muss die Ablehnungs-Lesart** für die drei Fälle explizit machen (per-Fall-§1-Klausel oder §4-Ergänzung).
- **LOW-1 — Nummern-Inkonsistenz bei der `entity_layers`-Ausnahme:** Z. 20 „#1/#5-Ausnahme", Z. 47 „Ausnahme
  #6". Beide gegen ADR-0006 korrekt (§6 = die Abweichung von #1/#5), aber gemischt widersprüchlich lesend.
  **→ vereinheitlichen** („ADR-0006 §6, die bewusste Abweichung von #1/#5"). **(ADR-Ebene.)**
- **LOW-2 — Ausgelieferter „Layer"-Umfang dünn:** in v1 wirkt „Layer" nur als **Export-Schreibfilter** für
  Hilfslinien; Bauteil-Layer + Editor-Sichtbarkeit deferiert. Legitim, aber die LH-FA-DRW-006-AK darf nicht
  mehr versprechen als „benannte, projekt-eindeutige Ebene mit Sichtbarkeit, die die Export-Sichtbarkeit
  ihrer Hilfslinien steuert". **→ slice-032a.**
- **LOW-3 — Layer-Zuordnung überlebt nur nativ (SQLite), nicht durch DXF:** Hilfslinien exportieren als LINE
  auf den **Geschoss**-LAYER; die Benutzer-Layer-Zuordnung geht verloren (nur Sichtbarkeits-Filter). Round-
  Trip-Asymmetrie sauber benennen (kein Defekt, Erwartungs-Fallstrick). **→ slice-032a/Spec.**
- **INFO — `data-model.yaml`-Kommentare „welle-3" stale:** `layers` (Z. 233), `documents` (Z. 260) → jetzt
  welle-5. Die ADR-Folgepflicht „§2.2-Kommentar-Heilung" zielt auf `spezifikation.md` §2.2 — **auch** die
  `data-model.yaml`-Inline-Kommentare mitnehmen.

## Mechanik / Gate

- **MR-014 (adr ↛ slice, ab ADR-0018 scharf): ✓** — erste ADR unter der Disziplin; kein `slice-\d{3}`-Token,
  keine `planning/`-Links im Körper (nur generische „AK-Schärfungs-Slice/Impl-Slice"). Reales Fallstrick-Risiko
  korrekt vermieden.
- docs-check (links/anchors/codepaths) stichprobenartig grün; Immutabilität: Proposed → iterierbar; ADR-Index
  trägt den Eintrag; Format schwester-ADR-konform (0012/0016/0017).

## Stärken

Faktentreue (jede Präzedenz code-belegt); Ehrlichkeit am schwächsten Punkt (Canvas-Lücke mit hartem Ausweg,
kein weichgezeichnetes AK); saubere Was/Wie-Trennung (MR-008, AK an slice-032a delegiert); Scope-Disziplin
(5 DRW-Familien + Bemaßung + interaktiver Editor je als Re-Eval-Trigger ausgelagert).
