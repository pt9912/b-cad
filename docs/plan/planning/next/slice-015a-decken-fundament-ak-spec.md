---
id: slice-015a
titel: Decken + Fundament — AK-Schärfung LH-FA-SLB/FND-* & Spec-Geometrie
status: done
welle: welle-2-bauteile
lastenheft_refs: [LH-FA-SLB-001, LH-FA-SLB-002, LH-FA-SLB-003, LH-FA-FND-001, LH-FA-FND-002, LH-FA-FND-003]
adr_refs: [ADR-0001, ADR-0002, ADR-0006, ADR-0011]
---

# Slice 015a: Decken + Fundament — AK-Schärfung & Spec-Geometrie

**Status:** done (2026-06-13). MR-006-Plan-Review gelaufen („GO mit
Auflagen", HIGH-1 nicht-blockierend + MED/LOW eingearbeitet); DoD
vollständig, `make gates` grün. Closure-Notiz §8.

**Plan-Review (MR-006):**
[Report](../../../reviews/2026-06-13-slice-015a-plan.md) — „GO mit
Auflagen". HIGH-1: „kein neues Primitiv" war zu stark — die `base_z`-/
Port-Frage ist offen (ADR-0001-Kern-Hoheit, 015b). Eingearbeitet:
abgeschwächt auf „gleiches Geometrie-Vokabular, Port-Mechanik 015b";
§1 nennt `base_z`-Herkunft je `slab_type` (LOW-2); §1 begründet die
Extrusion eigenständig, nicht per D3-001.a-Analogie (MED-2; das Dach
geht analytisch); strikt keine Mechanik im Lastenheft (MED-1/MR-008).

**Welle:** welle-2-bauteile (siebter Slice; erster der Platten-Familie).

**Bezug:** LH-FA-SLB-001..003 (Decken) + LH-FA-FND-001..003 (Fundament),
im Lastenheft bisher **Outline**. **Parametrisiert auf ADR-0011 (#6)** —
neuer Bauteil-Typ, **keine neue Grundsatz-ADR**; Geometrie-Entscheidung
in Lastenheft-AK + `spezifikation.md` §1 (Muster slice-012/014a).
ADR-0002 (OCC-Boolean für Ausschnitte), ADR-0006 (`slabs`-Schema **liegt
vor** — `slab_type`, `thickness_mm`, `polygon_json`; **eine** Tabelle für
Decken UND Fundament/Bodenplatte, Diskriminator `slab_type`), ADR-0001.

**Autor:** Dietmar Burkard. **Datum:** 2026-06-13.

**Schnitt-Herkunft:** Entscheidungs-/Spec-Hälfte des Platten-Strangs
(Muster 013a/014a). Reine Doku/Entscheidung, kein Code.

---

## 1. Ziel

Decken (LH-FA-SLB-*) und Fundament/Bodenplatte (LH-FA-FND-*) — beide
**horizontale Platten** — bekommen lösungsfreie AK und einen
entschiedenen Geometrie-Algorithmus, bevor implementiert wird
(slice-015b). Kernbeobachtung: eine Platte ist ein **Grundriss-Polygon
× Dicke an einer Aufstandshöhe**, mit optionalen Ausschnitten
(LH-FA-SLB-003) — sie nutzt **dasselbe Geometrie-Vokabular** wie
Wand/Öffnung (Footprint-Extrusion + `model::CutPrism`). **Offen (HIGH-1):**
ob der bestehende `extrudeFootprint` (extrudiert ab z=0) **unverändert**
trägt oder um eine **Aufstandshöhe `base_z`** erweitert wird, ist eine
Port-Signatur-Frage (**ADR-0001-Kern-Hoheit**, Muster ADR-0011 #2) und
gehört zu **015b** — nicht trivial „kein neues Primitiv". §1 legt nur die
Geometrie fest, nicht die Port-Mechanik.

## 2. Definition of Done

- [x] **Lastenheft SLB-001..003 + FND-001..003 von Outline auf AK-Niveau
      geschärft** (Reifephase-Klausel; **lösungsfrei, benutzer-
      beobachtbar** — keine Geometrie-Mechanik im Lastenheft-Text,
      MR-008): je Happy/Boundary/Negative. Mindestens:
      **Decke** (SLB-001: horizontale Platte über einem Grundriss; in 3D
      sichtbar); **Deckendicke** (SLB-002: parametrisch, Bereich §3);
      **Deckenausschnitt** (SLB-003: eine Aussparung in der Decke — z. B.
      für Treppe/Schacht — ist als Loch sichtbar); **Fundament**
      (FND-001: Platte am Gebäude-Aufstand); **Fundamenttiefe**
      (FND-002: parametrisch); **Bodenplatte** (FND-003: Fundament-Platte
      auf Geländehöhe). **Teilumfang-Klausel falls nötig** (z. B.
      rechteckige Ausschnitte). Boundary: Dicke/Tiefe am Grenzwert
      akzeptiert, außerhalb geklemmt (`E-VAL-001`). Negative:
      degenerierter/leerer Grundriss → keine Platte, kein Absturz. +
      §9-Historie.
- [x] **`spec/spezifikation.md` §1 präzisiert** (Sammelblock
      `LH-FA-SLB-001.a`, deckt SLB+FND): eine Platte = **Grundriss-Polygon
      extrudiert um die Dicke an der Aufstandshöhe `base_z`** — die
      Extrusion wird **eigenständig für die Platte begründet**, nicht per
      D3-001.a-Sammelanalogie (MED-2: das Dach geht analytisch, nicht über
      den Port). **`base_z`-Herkunft je `slab_type` (LOW-2, da das
      `slabs`-Schema kein base_z trägt):** Decke = Geschoss-Oberkante (aus
      `storey_id`), Bodenplatte = 0, Fundament = unter Gelände (Tiefe nach
      unten). Ausschnitte (SLB-003) als **boolesche Subtraktion** von
      Schnitt-Prismen (Wiederverwendung `model::CutPrism`, ADR-0011/
      slice-013b; ADR-0002 liefert das Boolean-Backend — **nicht** die
      base_z-Frage, LOW-1). Totalität (degeneriertes Polygon → keine
      Platte, Query total) + Fehler-Code-Wiederverwendung
      (`E-VAL-001`/`E-GEO-002`). **Die Port-Mechanik (ob `extrudeFootprint`
      um `base_z` erweitert wird) ist 015b** (Lösungsfreiheit der Ebenen).
      + §8-Historie.
- [x] **`spec/spezifikation.md` §3 Konstanten:** `SLAB_THICKNESS_MIN/MAX_MM`,
      `FOUNDATION_DEPTH_MIN/MAX_MM` (oder gemeinsame Platten-Dicke) +
      Default(s) bei Anlage — konsistent mit dem `slabs`-Schema
      (`thickness_mm`).
- [x] **Reine Doku/Entscheidung — kein Code, kein ADR** (ADR-0011 #6).
      `make gates` grün; Closure-Notiz mit Lerneintrag. **Nicht Teil:**
      slice-015b (Domänen-`Slab`-Typ, Edit-Ops, ViewModel/Viewer,
      `SlabChanged`-`op`, AK-Tests) + slice-015c (Persistenz `slabs`-
      Tabelle) + die Welle-Closure.

## 3. Plan (vor Code)

| Datei / Komponente | Änderungs-Art | Begründung |
|---|---|---|
| `spec/lastenheft.md` | ändern | SLB-001..003 + FND-001..003 Outline → AK; §9-Historie |
| `spec/spezifikation.md` | ändern | §1 `LH-FA-SLB-001.a` Platten-Geometrie (Polygon×Dicke, Ausschnitt-Boolean, Decke/Fundament-Typ); §3 Dicke-/Tiefe-Konstanten; §8 |
| `spec/architecture.md` | ggf. ändern | nur falls die Spec-Entscheidung die `GeometryKernelPort`-Verantwortung erweitert (sonst Begründung in der Closure; Signatur ist 015b) |
| `docs/reviews/{2026-06-13-slice-015a-plan}.md` | neu | MR-006-Report |

## 4. Trigger

- Keiner — **sofort startbar** nach MR-006-Review: reine Doku; OCC-Boolean
  (ADR-0002) und `slabs`-Schema (ADR-0006) liegen vor.

## 5. Closure-Trigger

- DoD vollständig, `make gates` grün, Closure-Notiz → **slice-015b**
  (Platten-Implementierung) startbar.

## 6. Risiken und offene Punkte

- **Lösungsfreiheit Lastenheft (MR-008):** „Aussparung in der Decke" ist
  benutzer-beobachtbar; „boolesche Subtraktion"/Schnitt-Prismen sind
  Mechanik → §1, nicht Lastenheft. MR-008 ist seit 2026-06-13
  festgeschrieben — hier strikt anwenden.
- **Decke + Fundament zusammen:** beide sind Platten; eine gemeinsame
  Domäne (`Slab` mit Typ-Diskriminator, analog `slabs.slab_type`) hält
  den Code klein. Falls FND-Semantik (unter Gelände, Tiefe nach unten)
  divergiert, kann der b-Slice sie trennen — die AK bleiben getrennt
  benannt (SLB-* / FND-*).
- **Grundriss-Polygon vs. Rechteck:** Platten tragen ein **Polygon**
  (`slabs.polygon_json`), nicht nur ein Rechteck — die bestehende
  Footprint-Extrusion verarbeitet einfache Polygone bereits (Wände). Ein
  Teilumfang (rechteckig) ist möglich, aber nicht nötig; im Review
  entscheiden, ob welle-2 das volle Polygon trägt.
- **Aufstandshöhe `base_z`:** Decken/Dach sitzen auf Geschoss-Höhe; die
  bestehende `extrudeFootprint` extrudiert ab z=0. Ob der Port um eine
  `base_z` erweitert wird (berührt dann auch die Wand-Aufrufe) oder die
  Platte anders platziert wird, ist eine **015b**-Entscheidung — §1 hält
  nur „an der Aufstandshöhe" fest (lösungsfrei auf Port-Ebene).
- **„Lösung schärft nie das Lastenheft" jetzt Regel (MR-008):** dieser
  Slice ist der erste Schärfungs-Slice **nach** der Festschreibung — die
  MR-006-Linse prüft die Einhaltung als Konvention, nicht mehr als
  Zähler-Kandidat.

## 7. Sub-Area-Modus-Begründung

### Sub-Area: Spec-Schreibung

- **Modus:** GF; Dichte hoch (AK-Format, Reifephase-Klausel, MR-001/
  MR-008, Wertebereich-Konvention); Phase-Reife: SLB/FND Phase 2
  (Outline → AK); Risiko niedrig (reine Doku).

### Sub-Area: Planning-Lifecycle

- **Modus:** GF; Dichte hoch (ADR-0011-Leitplanke: kein neuer
  Grundsatz-ADR; Reuse der Geometrie-Infrastruktur dokumentiert); Risiko
  niedrig.

## 8. Closure-Notiz

**Closure-Kriterien (beobachtbar):**

- **Lastenheft 0.1.5:** LH-FA-SLB-001..003 (Decken: erzeugen, Dicke
  100–500 mm, Ausschnitte) + LH-FA-FND-001..003 (Fundament erzeugen,
  Tiefe 200–2000 mm, Bodenplatte) von Outline auf AK-Niveau, lösungsfrei/
  benutzer-beobachtbar (**MR-008 strikt eingehalten** — keine Mechanik
  im Lastenheft-Text); §9-Historie.
- **`spec/spezifikation.md` §1 `LH-FA-SLB-001.a`** (Sammelblock SLB+FND):
  Platte = Polygon × Dicke an `base_z`; `base_z`-Herkunft je `slab_type`
  (Decke = Geschoss-OK, Bodenplatte = 0, Fundament = unter Gelände);
  Ausschnitte als Boolean/`CutPrism` (ADR-0002-Backend); Totalität;
  **Port-base_z-Frage explizit an 015b** (ADR-0001-Kern-Hoheit, nicht
  trivial — HIGH-1); Extrusion eigenständig begründet (nicht
  D3-001.a-Analogie, MED-2). **§3:** Decken-/Fundament-Dicke-Bereiche +
  Defaults; §8-Historie.
- **Kein ADR, kein Code** (ADR-0011 #6); `architecture.md` nicht geändert
  (Port-Signatur ist 015b). `make gates` grün.

**Lerneintrag:**

- **MR-008 zum ersten Mal als Konvention angewandt:** das Plan-Review
  prüfte die Lastenheft-Lösungsfreiheit nicht mehr als Zähler-Kandidat,
  sondern als festgeschriebene Regel — Bestätigung der Festschreibung.
- **Reuse-Versprechen ehrlich halten:** „gleiches Geometrie-Vokabular"
  ist tragfähig, „kein neues Primitiv" wäre zu stark gewesen — die
  `base_z`-/Port-Frage ist eine echte 015b-Entscheidung
  (ADR-0001-Hoheit), kein Trivial-Reuse (HIGH-1 des Reviews). Schema
  trägt kein base_z → §1 muss die Höhen-Herkunft je Typ nennen.

**Restrisiko / Nachfolge:** slice-015b (Platten-Implementierung: Domänen-
`Slab`-Typ, Edit-Ops, ViewModel/Viewer, `SlabChanged`-`op`, Ausschnitt-
Boolean, AK-Tests) — die **base_z-/`extrudeFootprint`-Port-Entscheidung**
ist dort der Kern-Punkt. Dann slice-015c (Persistenz `slabs`-Tabelle).
Danach STR (Treppen), dann Welle-2-Closure.
