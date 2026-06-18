---
id: slice-023b
titel: Dach-Volumen Impl — geschlossener Schräg-Slab (roofMesh) + Dicke + EVL-Dach-Volumen
status: done
welle: welle-4-austausch
lastenheft_refs: [[LH-FA-ROF-006](../../../../spec/lastenheft.md#lh-fa-rof-006), [LH-FA-EVL-002](../../../../spec/lastenheft.md#lh-fa-evl-002--volumenberechnung)]
adr_refs: [[ADR-0001](../../adr/0001-hexagonale-architektur.md), [ADR-0011](../../adr/0011-bauteil-hosting-wandoeffnung.md), [ADR-0012](../../adr/0012-evaluations-architektur.md)]
---

# Slice 023b: Dach-Volumen Impl — geschlossener Schräg-Slab + Dicke

**Status:** in-progress. **Vor Start:** [MR-006](../../../../harness/conventions.md#mr-006--unabhängiges-plan-review-vor-implementierungs-start)-Plan-Review (HIGHs blockieren Start). **Geometrieschwer →** zusätzlich [MR-009](../../../../harness/conventions.md#mr-009--geometrielastiges-code-review-vor-welle-closure)-Code-Review **vor Welle-Closure** (HIGHs blockieren Closure).

**Welle:** welle-4-austausch (Dach-Volumen-Initiative, Geometrie-Hälfte; Muster slice-014b
[Dach-Impl], 015b/016b). Setzt [slice-023a](../done-archive/slice-023a-dach-volumen-ak-spec.md)
(AK + §1-Geometrie) um.

**Bezug:** [LH-FA-ROF-006](../../../../spec/lastenheft.md#lh-fa-rof-006) (Dachdicke / Volumenkörper),
`spec/spezifikation.md` §1 [`LH-FA-ROF-001.a`](../../../../spec/lastenheft.md#lh-fa-rof-001--satteldach) (geschlossener Schräg-Slab, vertikaler Offset),
§3 `ROOF_THICKNESS_*`; [LH-FA-EVL-002](../../../../spec/lastenheft.md#lh-fa-evl-002--volumenberechnung) (Netto-Volumen —
das Dach trägt künftig bei). [ADR-0001](../../adr/0001-hexagonale-architektur.md) (Dach analytisch im
Kern, framework-frei — **kein OCC**, Muster `roof_geometry`/`stair_geometry`),
[ADR-0012](../../adr/0012-evaluations-architektur.md) (EVL-Volumen analytisch, **kein**
`Solid.volume_mm3`-Lesen), [ADR-0011](../../adr/0011-bauteil-hosting-wandoeffnung.md) (#6 Bauteil-Muster).

**Autor:** Dietmar Burkard (AI-Harness-Lauf). **Datum:** 2026-06-18.

**Schnitt-Herkunft:** Geometrie-Hälfte der Dach-Volumen-Initiative (Muster 014b). Persistenz
(`roofs.thickness_mm`) ist **023c** (eigene Sitzung, Schema/d-migrate); STEP-B-Rep der Dächer
ist **024**.

**Plan-Review ([MR-006](../../../../harness/conventions.md#mr-006--unabhängiges-plan-review-vor-implementierungs-start), Reviewer ≠ Autor):**
**1 HIGH / 4 MED / 2 LOW / 2 INFO** ([Report](../../../reviews/2026-06-18-slice-023b-plan.md)). Der
**HIGH war mechanisch** (toter `#lh-fa-evl-002`-Anker → `#lh-fa-evl-002--volumenberechnung` korrigiert,
4×), **kein Geometrie-Fehler** — der Reviewer **bestätigte** den Slab-Ansatz, den vertikalen Offset,
kein-OCC und die **EVL-Formel `bx·ty·d` als korrekt** (keine Oberseiten-Fläche·d-Verwechslung).
Eingearbeitet: **MED-1** (Walm-Zeltdach-**Apex**-Invariante, DoD-5), **MED-2/4** (realer Helfer
**`evaluateParam`** + neuer `setRoofThickness`-Setter, DoD-3 — `clampToRange` existiert nicht),
**MED-3** (`VolumeReport.roofs_m3` + `total_m3` + Totalität, DoD-4), **LOW-1** (**µm-Raster**-Kanten-
Kanonisierung, DoD-2), **LOW-2** (Orientierung = **signiertes Volumen > 0**, nicht ΣArea·n≈0, DoD-5).
INFO: EVL-**023d**-Split bei Sizing-Bedarf. **HIGH behoben → Start frei.**

---

## 1. Ziel

`roofMesh` produziert statt der heutigen **offenen** Dachfläche einen **geschlossenen,
wasserdichten Schräg-Slab** der Dicke `d` (alle drei Typen Sattel/Walm/Pult); `Roof` bekommt
ein `thickness_mm`-Feld (Default/Klemmung im `StructureEditService`); der Viewer folgt
automatisch; das **EVL-Netto-Volumen** ([LH-FA-EVL-002](../../../../spec/lastenheft.md#lh-fa-evl-002--volumenberechnung))
schließt die welle-3-Dach-Lücke (Dach-Volumen analytisch im Kern). Damit ist das Dach ein
echter Volumenkörper — Voraussetzung für STEP-B-Rep (024).

## 2. Definition of Done

- [x] **DoD-1 — Domäne: `Roof.thickness_mm` + Konstanten.** `roof.h` um `thickness_mm` (Muster
      `Slab`); `constants.h` `kRoofThicknessMinMm`/`kRoofThicknessMaxMm`/`kDefaultRoofThicknessMm`
      (50/500/200 mm, == §3). **§3-Quelle der Wahrheit unverändert** (023a); hier nur der Code-Spiegel.
- [x] **DoD-2 — `roof_geometry`: geschlossener Schräg-Slab (alle 3 Typen).** `roofMesh` baut die
      **Oberseite** (bestehende geneigte Flächen) **+ Unterseite** (jede Oberseiten-Fläche um `d`
      **vertikal nach unten** versetzt, **umgekehrte Wicklung** → Normalen nach unten) **+ Seitenwände
      entlang der Rand-Kanten** (Trauf-/Giebel-/Walm-Rand; eine vertikale Wand der Höhe `d` je Rand-Kante,
      **außen-orientiert**). **Rand-Kante = Kante, die von genau EINER Oberseiten-Fläche genutzt wird**
      (Flat-Shading-Netz ohne geteilte Vertices → **koordinaten-kanonische** Kanten-Zählung — gerundete
      Endpunkte auf ein **µm-Raster** (Muster slice-016b, nicht nur „Toleranz"); LOW-1;
      Grat/First/Hip sind innen [2 Flächen] → keine Wand). Ergebnis: **geschlossene Mannigfaltigkeit**
      (jede Kante von **genau 2** Flächen). **Totalität:** degenerierter Grundriss / nicht-positive
      Neigung / nicht-positive Dicke → **leeres** Netz (kein Wurf, wie heute).
- [x] **DoD-3 — `StructureEditService`: Default + Klemmung (MED-2/MED-4).** **Anlage:** `addRoof`
      setzt `thickness_mm` = `kDefaultRoofThicknessMm`, in den Bereich geklemmt (Anlage-`std::clamp`,
      Muster der übrigen Roof-Parameter). **Editieren:** neuer Setter `setRoofThickness` über den
      **realen** Helfer **`evaluateParam(value, Range, current)`** → `ParamStatus::Clamped`-Rückmeldung
      (der Helfer heißt `evaluateParam`, **nicht** `clampToRange`; `ParamStatus` lebt nur in Settern),
      Bereich `[ROOF_THICKNESS_MIN, MAX]`
      ([`E-VAL-001`](../../../../spec/spezifikation.md#4-fehler-codes-und-logging-felder)).
      `RoofChanged`-`op` unverändert (Dicke ist eine Dach-Mutation).
- [x] **DoD-4 — EVL-Dach-Volumen ([LH-FA-EVL-002](../../../../spec/lastenheft.md#lh-fa-evl-002--volumenberechnung)).** Der
      Volumen-Pfad (`volume_geometry` / `EvaluatePort`) ergänzt das Dach: **Netto-Volumen analytisch im
      Kern** = **`bx · ty · d`** (projizierte Trauf-Grundfläche × Dicke, der vertikale Slab — **exakt**,
      kein „bzw."-Hedge), **ohne** `Solid.volume_mm3` ([ADR-0012](../../adr/0012-evaluations-architektur.md)).
      `VolumeReport` bekommt ein **neues `roofs_m3`-Feld** + `total_m3`-Nachzug + Kommentar-Korrektur
      (MED-3; stale „Dach ausgenommen"-Kommentare nachgezogen, INFO-2). **Totalität:** Dicke ≤ 0 /
      degeneriert → Dach-Volumen **0** (getestet). **Sizing:** falls 023b zu groß ([MR-006](../../../../harness/conventions.md#mr-006--unabhängiges-plan-review-vor-implementierungs-start)/INFO-1), EVL
      als Folge-Slice **023d** ausgliedern (§6).
- [x] **DoD-5 — Tests inkl. geometrischer Invarianten ([MR-009](../../../../harness/conventions.md#mr-009--geometrielastiges-code-review-vor-welle-closure)-Orakel).** Die 7 bestehenden
      `test_roof_geometry`-Tests auf den **geschlossenen Slab** umgestellt (Flächen-Anzahl;
      „AlleFlaechenZeigenNachOben" → **Oberseite oben / Unterseite unten / Seiten außen**). **Neue
      Invarianten je Typ (Sattel/Walm/Pult):** **wasserdicht** (jede Kante von **genau 2** Flächen —
      Mannigfaltigkeits-Sonde, koordinaten-kanonisch); **außen-orientiert** = **signiertes Volumen > 0**
      (Divergenzsatz; **ΣArea·n ≈ 0 prüft nur Geschlossenheit, NICHT Orientierung** — LOW-2), zugleich
      **Volumen > 0 endlich**. **Walm-Zeltdach-Sonderfall** (First = Punkt, kollabierte Trapeze): **eigene**
      Invariante — Apex-/Trapez-/Wand-Kanten alle genau 2 Flächen (MED-1, heikelster Wasserdichtheits-Fall,
      sonst rutscht ein Apex-Loch durch). `StructureEditService`-Test (Dicke Default/Klemmung-`Clamped`).
      EVL-Test (Dach trägt `bx·ty·d` bei; Dicke ≤ 0 → 0). **Nicht** nur Bounding-Box/Flächen-Anzahl
      (013b/014b/016b-Lehre).
- [x] **DoD-6 — Gates grün + Reviews + Persistenz-Gap benannt.** `make gates` grün (Invarianten-Tests,
      `arch-check` — Dach bleibt OCC-frei im Kern, coverage); `make schema-check` **unberührt**
      (Persistenz = 023c). **Unabhängiges [MR-009](../../../../harness/conventions.md#mr-009--geometrielastiges-code-review-vor-welle-closure)-Code-Review
      0 HIGH** (Wasserdichtheit/Orientierung des Slab, bes. **Walm**). `CHANGELOG.md` + `roadmap.md`.
      **Benannte Lücke:** `thickness_mm` ist bis **023c** **nicht persistiert** → geladene Dächer erhalten
      die Default-Dicke (Round-Trip-Treue der Dicke erst 023c) — explizit in Closure.

## 3. Plan (vor Code)

| Datei / Komponente | Änderungs-Art | Begründung |
|---|---|---|
| `src/hexagon/model/roof.h` | ändern | `thickness_mm`-Feld |
| `src/hexagon/model/constants.h` | ändern | `kRoofThickness{Min,Max}Mm` + `kDefaultRoofThicknessMm` (== §3) |
| `src/hexagon/services/roof_geometry.cpp` | ändern | geschlossener Slab: Oberseite + versetzte Unterseite + Rand-Seitenwände (koordinaten-kanonische Rand-Kanten) |
| `src/hexagon/services/structure_edit_service.cpp` | ändern | Dach-Anlage Default-Dicke + `setRoofThickness` via `evaluateParam` |
| `src/hexagon/services/volume_geometry.cpp` (+ `VolumeReport.roofs_m3`) | ändern | Dach-Netto-Volumen `bx·ty·d` analytisch ([`LH-FA-EVL-002`](../../../../spec/lastenheft.md#lh-fa-evl-002--volumenberechnung)) |
| `tests/hexagon/test_roof_geometry.cpp` | ändern | Slab-Umstellung + **Wasserdichtheits-/Orientierungs-/Volumen-Invarianten** je Typ |
| `tests/hexagon/test_structure_edit_service.cpp` | ändern (ggf.) | Dicke Default/Klemmung |
| `tests/hexagon/test_evaluate_volume.cpp` | ändern | Dach trägt Volumen bei |
| `docs/plan/adr/README.md` | ggf. (Closure) | falls EVL die [ADR-0012](../../adr/0012-evaluations-architektur.md)-Lücke „Dach" schließt → Index-Notiz |
| `CHANGELOG.md` · `roadmap.md` | ändern (Closure) | 023b done; Dach-Volumen-Initiative-Fortschritt |
| `docs/reviews/{2026-06-18-slice-023b-plan}.md` | neu | [MR-006](../../../../harness/conventions.md#mr-006--unabhängiges-plan-review-vor-implementierungs-start)-Report |
| `docs/reviews/{2026-06-18-slice-023b-code-review}.md` | neu (Closure) | [MR-009](../../../../harness/conventions.md#mr-009--geometrielastiges-code-review-vor-welle-closure)-Report |

**Kein** neuer ADR ([ADR-0011](../../adr/0011-bauteil-hosting-wandoeffnung.md)-#6-Muster, [ADR-0012](../../adr/0012-evaluations-architektur.md)-EVL); **kein OCC** (Dach analytisch im Kern,
[ADR-0001](../../adr/0001-hexagonale-architektur.md)) → `arch-check` Regel A/C unberührt.

## 4. Trigger

- Keiner — **startbar** nach [MR-006](../../../../harness/conventions.md#mr-006--unabhängiges-plan-review-vor-implementierungs-start)-Review.
  Die AK + §1-Geometrie (023a) sind entschieden; `roof_geometry`/`StructureEditService`/Volumen-Pfad existieren.

## 5. Closure-Trigger

- DoD vollständig, `make gates` grün, **[MR-009](../../../../harness/conventions.md#mr-009--geometrielastiges-code-review-vor-welle-closure) 0 HIGH**, Closure-Notiz → **023c** (Persistenz) wird
  startbar; danach **024** (STEP-B-Rep Dächer+Treppen, jetzt sind Dächer Solids).

## 6. Risiken und offene Punkte

- **Rand-Kanten-Erkennung (PRIMÄR, [MR-009](../../../../harness/conventions.md#mr-009--geometrielastiges-code-review-vor-welle-closure)):** das Flat-Shading-Netz hat **keine geteilten Vertices** →
  „von genau einer Fläche genutzte Kante" muss **koordinaten-kanonisch** bestimmt werden (Kante als
  sortiertes, gerundetes Endpunkt-Paar; Toleranz `kGeometryToleranceMm`). Ein Rundungs-/Toleranz-Fehler
  erzeugt eine Loch-/Doppel-Wand → **nicht wasserdicht**. Mitigation: Wasserdichtheits-Invariante (jede
  Kante genau 2 Flächen) ist der fail-closed Wächter (DoD-5).
- **Seitenwand-Orientierung:** eine Seitenwand muss **nach außen** zeigen (nicht ins Slab-Innere). Aus
  der Rand-Kante + der zugehörigen Oberseiten-Flächennormale ableitbar (Außenrichtung = von der Fläche
  weg, horizontal). Orientierungs-Invariante = **signiertes Volumen > 0** (Divergenzsatz; ΣArea·n≈0
  ist nur Geschlossenheit, LOW-2) fängt eine Inversion.
- **Walm (heikelster Fall):** mehr Rand-Kanten (Walm-Dreiecke an den Giebeln + Trapeze); der „nahezu
  quadratisch → Zeltdach"-Sonderfall (First = Punkt) muss geschlossen bleiben. Eigene Walm-Invarianten.
- **Pult/Sattel:** weniger Rand-Kanten; Sattel-Giebel ist offen (heute) → die Seitenwände schließen ihn
  als Slab-Dicke (physikalisch korrekt). Pult = ein Quad → Quader-artiger Slab.
- **Vertikale vs. schräg-normale Dicke:** §1 (023a) entschied **vertikal** — die Unterseite ist die um
  `d` in `-z` versetzte Oberseite (gleiche xy). Real-Dachdicke wäre normal zur Fläche; vertikal ist
  parametrisch tragbar (023a-Entscheidung) und einfacher wasserdicht. **Nicht** abweichen.
- **EVL-Volumen-Formel:** Slab-Volumen = **`bx·ty·d`** (projizierte Trauf-Grundfläche × `d`) — **nicht**
  die geneigte Oberseiten-Fläche · `d` (die ist um 1/cos größer). Bei vertikalem Offset ist die
  vertikale Dicke `d` über der projizierten Grundfläche das Material-Volumen. Formel **per Hand**
  bestätigt (014b/017c-Lehre); **analytisch**, kein OCC.
- **Persistenz-Gap (benannt):** `thickness_mm` bis 023c nicht in `roofs` → geladene Dächer = Default-Dicke
  (Round-Trip-Treue der Dicke erst 023c). `make schema-check` unberührt (keine Schema-Änderung hier).
- **Schnitt-Größe:** Domäne + Geometrie + EditService + EVL + Tests in einer Sitzung. Falls zu groß
  ([MR-006](../../../../harness/conventions.md#mr-006--unabhängiges-plan-review-vor-implementierungs-start)-Urteil): **EVL → 023d** ausgliedern, 023b = reine Geometrie + Dicke + EditService.
- **Bestehende Tests brechen bewusst:** die 7 Roof-Tests prüfen die offene Fläche — sie werden auf den
  Slab umgestellt (nicht gelöscht); „AlleFlaechenZeigenNachOben" wird zu typ-spezifischen Orientierungs-
  Invarianten.

## 7. Sub-Area-Modus-Begründung

### Sub-Area: Domänen-Modell + Ports + Services (Hexagon-Kern)

- **Modus:** GF; **Konventionen-Dichte:** hoch — Pure-Domain (analytische Geometrie im Kern, **kein OCC**,
  Muster `roof_geometry`/`stair_geometry`), `evaluateParam`/[`E-VAL-001`](../../../../spec/spezifikation.md#4-fehler-codes-und-logging-felder), EVL analytisch
  ([ADR-0012](../../adr/0012-evaluations-architektur.md), kein `Solid.volume_mm3`). **Phase-Reife:** Phase 4.
  **Evidenz-/Diskrepanz-Risiko:** **mittel-hoch** (Wasserdichtheit/Orientierung des Slab) → [MR-009](../../../../harness/conventions.md#mr-009--geometrielastiges-code-review-vor-welle-closure) +
  Invarianten-Tests. **Reconciliation:** schließt die 023a-Geometrie + welle-3-EVL-Dach-Lücke.

### Sub-Area: Test-Infrastruktur

- **Modus:** GF; **Dichte:** hoch (geometrische Invarianten-Sonden statt Bounding-Box, 016b-Lehre).
  **Phase-Reife:** Phase 4. **Risiko:** niedrig.

## 8. Closure-Notiz

**Closure-Kriterien (beobachtbar, 2026-06-18):**

- **Geometrie (DoD-1/2):** `roofMesh` baut einen geschlossenen Schräg-Slab — Oberseite + um `d`
  **vertikal** versetzte Unterseite (umgekehrte Wicklung) + Seitenwände entlang koordinaten-kanonisch
  (µm) erkannter Rand-Kanten (von genau 1 Oberseiten-Fläche). `Roof.thickness_mm` + `kRoofThickness*`
  (50/500/200). **EVL in 023b belassen** (nicht nach 023d gesplittet — der Schnitt blieb tragbar).
- **EditService (DoD-3):** `addRoof` defaultet/klemmt die Dicke (Anlage-clamp wie pitch/overhang); neuer
  `setRoofThickness`-Setter via `evaluateParam` (`ParamStatus::Clamped` — das AK-„Hinweis"). **MED-2
  revidiert:** Roof-Setter existieren (`setRoofPitch`) → Setter ergänzt, nicht nur Anlage-clamp.
- **EVL (DoD-4):** `roofNetVolumeMm3 = bx·ty·d`, `VolumeReport.roofs_m3` + `total_m3`; analytisch im Kern,
  kein `Solid.volume_mm3`. Die welle-3 „Dach ausgenommen"-Lücke ist geschlossen.
- **Invarianten-Tests (DoD-5):** je Typ **wasserdicht** (jede Kante genau 2 Flächen) + **außen-orientiert**
  (signiertes Volumen > 0) + **Volumen == `bx·ty·d`** (stärkste Sonde, [MR-009](../../../../harness/conventions.md#mr-009--geometrielastiges-code-review-vor-welle-closure)-LOW-1 — fängt lokale
  Inversionen) + Walm-**Zeltdach-Apex** wasserdicht + Dicke-/Grundriss-Totalität.
- **Gates (DoD-6):** `make gates` grün (204/204, Coverage 90,2 %, arch-check — Dach OCC-frei im Kern,
  lint 0, docs-check 0); `make schema-check` unberührt.
- **Reviews:** [MR-006](../../../../harness/conventions.md#mr-006--unabhängiges-plan-review-vor-implementierungs-start) 0 HIGH (HIGH-1 = mechanischer Anker, behoben)
  + **[MR-009](../../../../harness/conventions.md#mr-009--geometrielastiges-code-review-vor-welle-closure) 0 HIGH** (Reviewer hat den Algorithmus über 15 Parametrierungen
  **repliziert**: wasserdicht alle Typen + Zeltdach, signiertes Volumen **bit-exakt** `bx·ty·d`).

**Lerneintrag:**

- **„Thicken-downward"-Slab (eine Algorithmik, alle 3 Typen):** Oberseite + reversed Unterseite +
  Seitenwände entlang der Rand-Kanten (Kante von genau 1 Oberseiten-Fläche; Flat-Shading ohne geteilte
  Vertices → **µm-kanonische** Kanten-Erkennung). Robuster als per-Typ-Konstruktion; der Walm-**Zeltdach**-
  Apex schließt automatisch (kollabierte Trapeze von `appendTriangle` verworfen, Apex-Kanten innen).
- **`Volumen == bx·ty·d` als Geometrie-Orakel:** für einen vertikalen Slab ist das signierte Volumen
  **exakt** projizierte Grundfläche · Dicke — eine Invariante, die jede Flächen-Inversion/Loch fängt
  ([MR-009](../../../../harness/conventions.md#mr-009--geometrielastiges-code-review-vor-welle-closure)-LOW-1; stärker als Geschlossenheit + Gesamt-Vorzeichen allein).
- **Persistenz-Gap (benannt):** `thickness_mm` ist bis 023c **nicht** in `roofs` → geladene Dächer erhalten
  die Default-Dicke (Round-Trip-Treue der Dicke erst 023c); `schema-check` unberührt.

**Restrisiko / Nachfolge:** **023c** (Persistenz `roofs.thickness_mm` — data-model.yaml + schema.sql via
d-migrate, Round-Trip) · **024** (Dächer+Treppen STEP-B-Rep — jetzt sind Dächer geschlossene Solids).
[MR-009](../../../../harness/conventions.md#mr-009--geometrielastiges-code-review-vor-welle-closure)-LOW-2 (Degeneracy-Schwelle flächen- statt toleranz-basiert) als benannter Mini-Cleanup.
