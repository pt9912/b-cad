---
id: slice-032a
titel: DRW-Fundament — AK-Schärfung [LH-FA-DRW-005](../../../../spec/lastenheft.md#modul-zeichnungsfunktionen-drw)/006 & Spec-Mapping (parametrisiert auf [ADR-0018](../../adr/0018-drw-2d-zeichen-daten.md))
status: in-progress
welle: welle-5-erweiterung
lastenheft_refs: [[LH-FA-DRW-005](../../../../spec/lastenheft.md#modul-zeichnungsfunktionen-drw), [LH-FA-DRW-006](../../../../spec/lastenheft.md#modul-zeichnungsfunktionen-drw)]
adr_refs: [[ADR-0001](../../adr/0001-hexagonale-architektur.md), [ADR-0006](../../adr/0006-relationales-schema-design.md), [ADR-0012](../../adr/0012-evaluations-architektur.md), [ADR-0015](../../adr/0015-dxf-backend.md), [ADR-0016](../../adr/0016-pdf-png-backend.md), [ADR-0018](../../adr/0018-drw-2d-zeichen-daten.md)]
---

# Slice 032a: DRW-Fundament — AK-Schärfung & Spec-Mapping

**Status:** in-progress (angelegt 2026-07-05).
**[MR-006](../../../../harness/conventions.md#mr-006--unabhängiges-plan-review-vor-implementierungs-start)-Review:
0 HIGH / 2 MED / 4 LOW → startbar** (MED-1 aufgelöst: Export-/Persistenz-Beobachtbarkeit trägt den
Fundament-Schnitt, **kein** Canvas nötig; Ehrlichkeits-Klausel in den AK-Körper.
[Report](../../../reviews/2026-07-05-slice-032a-plan.md)). **Reine Doku/Entscheidung — kein Code, kein Schema.**

**Welle:** welle-5-erweiterung (DRW-Strang, Entscheidungs-/Spec-Hälfte — erster Schnitt
**Fundament Hilfslinien + Layer**; Muster [slice-019a](../done-archive/slice-019a-ifc-ak-spec.md)/[slice-025a](../done/slice-025a-pdf-png-ak-spec.md)/[slice-026a](../done/slice-026a-plg-ak-spec.md)
— AK-Schärfung vor Impl).

**Bezug:** [LH-FA-DRW-005](../../../../spec/lastenheft.md#modul-zeichnungsfunktionen-drw)
(Hilfslinien) + [LH-FA-DRW-006](../../../../spec/lastenheft.md#modul-zeichnungsfunktionen-drw)
(Layer), bisher **Outline**-Einzeiler (Bullets ohne AK, ohne per-ID-Anker);
[OBJ-003](../../../../spec/lastenheft.md#3-projektziele) (»2D- und 3D-Darstellung aus
demselben Datenmodell« — kein DRW-ACC existiert oder entsteht). **Parametrisiert auf
[ADR-0018](../../adr/0018-drw-2d-zeichen-daten.md) (DRW-Fundament, Proposed 2026-07-05)** —
das Fundament ist **entschieden** (Datenheimat = Domänen-Modell; Zugriffs-Naht = neuer
Driving-Port; Layer-Scope v1 = nur Zeichen-Entitäten, direkter typisierter Bezug,
Sichtbarkeit = Export-Filter; Beobachtbarkeit über Persistenz-Round-Trip + 2D-Export mit
ehrlich benannter Canvas-Grenze); dieser Slice braucht **keine neue Grundsatz-ADR**, seine
**Mapping-/AK-Entscheidung lebt in Lastenheft-AK + `spezifikation.md` §1/§4/§6** (Muster
019a/025a/026a). [ADR-0001](../../adr/0001-hexagonale-architektur.md) (Schichtung),
[ADR-0006](../../adr/0006-relationales-schema-design.md) (Schema, `layers`/`entity_layers`
forward-deklariert), [ADR-0012](../../adr/0012-evaluations-architektur.md) (Port-Fork-Präzedenz),
[ADR-0015](../../adr/0015-dxf-backend.md)/[0016](../../adr/0016-pdf-png-backend.md) (2D-Grundriss-/
Subset-Vertrag).

**Autor:** Dietmar Burkard. **Datum:** 2026-07-05.

**Schnitt-Herkunft:** Entscheidungs-/Spec-Hälfte des DRW-Strangs (Muster 019a/025a/026a).
Die DRW-005/006-AK (was heißt Hilfslinie/Layer **benutzer-beobachtbar**) brauchen
**prüfbare AK + ein entschiedenes Mapping**, bevor implementiert wird (032b/032c). **Reine
Doku/Entscheidung, kein Code.**

---

## 1. Ziel

[LH-FA-DRW-005](../../../../spec/lastenheft.md#modul-zeichnungsfunktionen-drw)/006 bekommen
**lösungsfreie, benutzer-beobachtbare Akzeptanzkriterien**
([MR-008](../../../../harness/conventions.md#mr-008--lastenheft-schärfung-bleibt-lösungsfrei))
und ein **entschiedenes Mapping** (`spezifikation.md` §1/§4/§6), bevor implementiert wird —
**innerhalb des von [ADR-0018](../../adr/0018-drw-2d-zeichen-daten.md) entschiedenen
Fundaments**. Zusätzlich wird die **[ADR-0018](../../adr/0018-drw-2d-zeichen-daten.md)-Folgepflicht
„AK-Schärfung + Spec-Nachzug" eingelöst**.

**Die fachliche Eigenheit, die die AK prägt.** Es gibt (welle-5-Ausgangslage) **keine
interaktive 2D-Zeichenfläche** (Viewer = reines 3D); die 2D-Sicht existiert nur als
**Export** (DXF/PDF/PNG, io-resident). Das benutzer-beobachtbare Ergebnis dieser Fundament-
Stufe: eine Hilfslinie **überlebt Speichern/Laden** und **erscheint im 2D-Grundriss-Export**,
und **verschwindet, wenn ihr Layer unsichtbar** geschaltet wird. Die interaktive 2D-Fläche ist
**nicht** Teil dieser Stufe (benannter UI-Strang-Re-Eval,
[ADR-0018](../../adr/0018-drw-2d-zeichen-daten.md) §Trigger).

## 2. Definition of Done

- [ ] **Lastenheft [LH-FA-DRW-005](../../../../spec/lastenheft.md#modul-zeichnungsfunktionen-drw)/006
      von Outline auf AK-Niveau** (**lösungsfrei, benutzer-beobachtbar** — **kein**
      Port-/Schema-/FK-/`E-VAL`-/Koordinaten-Mechanik-Vokabular, das gehört in §1;
      [MR-008](../../../../harness/conventions.md#mr-008--lastenheft-schärfung-bleibt-lösungsfrei));
      je Anforderung ein eigenes `####`-Heading mit **Inline-HTML-Anker** (Muster
      [LH-FA-IO-007](../../../../spec/lastenheft.md#lh-fa-io-007); Modul-Heading bleibt).
      Mindestens:
      **DRW-005 Hilfslinien — Happy:** Given ein Projekt, dessen Geschoss auf einer **sichtbaren**
      Ebene eine Hilfslinie trägt, when das Projekt gespeichert und neu geladen wird, then ist
      die Hilfslinie mit Endpunkten und Ebene unverändert vorhanden; when der Grundriss exportiert
      wird, then erscheint sie im Artefakt. **Boundary:** Hilfslinie ohne Ausdehnung (Anfang =
      Ende) → verworfen. **Negative:** Ebene auf **unsichtbar** → keine Hilfslinie dieser Ebene im
      Artefakt. **Teilumfang-Klausel (Review-MED-1, must-fix — in den AK-KÖRPER, nicht nur Plan-Risiken):**
      in dieser Ausbaustufe wird **nicht interaktiv gezeichnet**; die Beobachtung ruht auf dem
      **persistierten/exportierten Artefakt** (Muster STEP/STL »export-only«, MAT »canvas-los« — beide
      lösungsfrei im Lastenheft). Das *When* ist **artefakt-/zustandsbasiert**, **keine** Nutzer-Zeichenhandlung.
      **DRW-006 Layer — Happy:** Ebene mit Name (optional Farbe) anlegen/umbenennen/sichtbar-
      schalten → bereit/geändert; die Sichtbarkeit steuert **die Export-Sichtbarkeit ihrer Hilfslinien**
      (nicht die 3D-Viewer-Sicht) [Review-INFO]; nach Speichern/Laden unverändert. **Boundary:** Ebene ohne Namen
      → abgelehnt. **Negative:** noch genutzte Ebene löschen → abgelehnt, Modell unverändert (kein
      stiller Verlust).
      + Header-Nachzug **`lastenheft.md` `Version:` → 0.1.14** == oberste
      `lastenheft-historie.md`-Zeile
      ([MR-010](../../../../harness/conventions.md#mr-010--lastenheft-header-version--oberste-9-historie-zeile)/[MR-012](../../../../harness/conventions.md#mr-012--mr-010-invariante-folgt-der-ausgelagerten-lastenheft-historie)).
- [ ] **`spec/spezifikation.md` §1 neuer Block [`LH-FA-DRW-005.a`](../../../../spec/lastenheft.md#modul-zeichnungsfunktionen-drw)** (Sammelblock — **deckt
      DRW-005 + 006**, Muster [`LH-FA-IO-007.a`](../../../../spec/lastenheft.md#lh-fa-io-007)):
      Mapping im [ADR-0018](../../adr/0018-drw-2d-zeichen-daten.md)-Rahmen, **ohne ADR-Verweis im
      Körper** ([MR-011](../../../../harness/conventions.md#mr-011--referenz-integritäts-gate-matrix-ids-spans-hostpaths)):
      Datenheimat (pure Werttypen im Kern), Hilfslinie = 2D-Segment (Start/Ende, Geschoss),
      Entartung → abgelehnt; Zugriffs-Naht = ein Driving-Port fürs 2D-Zeichnen (gleiche
      Validierung wie Bauteil-Edits); Layer benannt + sichtbar/gesperrt + optionale Farbe,
      projekt-eindeutig, **Sichtbarkeit = Export-Filter**; Zuordnung = typisierter Bezug;
      Löschschutz referenzierter Ebene; kein interaktiver 2D-Canvas (Beobachtung über
      Round-Trip + Export); **DXF-Geschoss-LAYER unverändert** (benutzer-Layer ≠ DXF-Layer). **Round-Trip-
      Asymmetrie benennen (Review-LOW-2):** die benutzer-Layer-Zuordnung überlebt nur **nativ (SQLite)**;
      durch DXF geht sie verloren (Export flacht auf den Geschoss-`LAYER` ab, nur Sichtbarkeits-Filter).
      + `spezifikation-historie.md` + `**Letzte Änderung:**`-Header.
- [ ] **`data-model.yaml`-Kommentar-Heilung (Review-MED-2 — die stale Welle-Angaben liegen NUR hier):**
      `layers` „welle-3" (Z. 233) + `documents` „welle-3/4" (Z. 260) → **welle-5** (`entity_layers` Z. 247
      trägt **kein** Welle-Jahr → nicht anfassen). d-migrate ignoriert Kommentare → `schema.sql`/`schema-check`
      byte-unberührt.
- [ ] **`spec/spezifikation.md` §2.2 + §4 + §6 nachgezogen:** **§2.2** = **neue Prosa** für die
      forward-deklarierten Tabellen (`layers`/`guide_lines`, welle-5-Provenance) — §2.2 trägt heute **keinen**
      welle-3-Kommentar zu heilen (Review-MED-2); die `guide_lines`-Tabelle **selbsttragend** benennen
      (»aktiviert mit dem DRW-Impl-Slice«, **ohne** Slice-Nummer — sonst Slice-Token im Spec-Stratum,
      Review-LOW-3). **§4** Ablehnungs-Bedingungen via **Wiederverwendung
      [`E-VAL-001`](../../../../spec/spezifikation.md#4-fehler-codes-und-logging-felder)** (Hilfslinie ohne
      Ausdehnung, Layer-Name leer, Löschen referenzierter Ebene) — **kein neuer Fehler-Code**, aber die
      **Ablehnungs-Lesart explizit** ausschreiben („Modell unverändert", **nicht** Klemmung — dessen
      §4-Kopf-Semantik; Review-LOW-1); **§6** neue **interne Grenz-Vertragszeile 2D-Zeichnen (DRW)** (kein
      Fremdsystem — Muster Plugin-Host-Grenzzeile; Review-INFO).
- [ ] **`spec/spezifikation.md` Subset-Grenze DXF/PDF/PNG autorisiert erweitert** (Vertrags-
      erweiterung, Muster [LH-FA-ROF-006](../../../../spec/lastenheft.md#lh-fa-rof-006)/slice-023a):
      **Hilfslinien auf sichtbarem Layer werden gezeichnet** (DXF `LINE` auf Geschoss-LAYER;
      PDF/PNG als 2D-Segment). Bemaßung/Schraffur/Räume bleiben ausgeschlossen. **Kein stiller
      Drift** — projektinhaber-autorisiert (Plan-Freigabe 2026-07-05); Impl = 032c.
- [ ] **`spec/architecture.md`:** §1.1 Driving-Ports um die **DRW-Zeichen-Naht** ergänzen (Muster
      [ADR-0012](../../adr/0012-evaluations-architektur.md)→[slice-017a](../done-archive/slice-017a-auswertung-material-adr-spec.md));
      `## Geschichte`-Provenance-Zeile »2D-Zeichen-Daten → [ADR-0018](../../adr/0018-drw-2d-zeichen-daten.md)« (exclude-section). Kein
      Körper-ADR-Verweis ([MR-011](../../../../harness/conventions.md#mr-011--referenz-integritäts-gate-matrix-ids-spans-hostpaths)).
- [ ] **[ADR-Index](../../adr/README.md)-Folgepflicht** „[ADR-0018](../../adr/0018-drw-2d-zeichen-daten.md)
      AK-Schärfung + Spec-Nachzug" → erfüllt (im Closure-Commit; setzt **[ADR-0018](../../adr/0018-drw-2d-zeichen-daten.md) Accepted** voraus).
- [ ] **Reine Doku/Entscheidung — kein Code, keine Tests, kein Schema** (`guide_lines`-Tabelle +
      Persistenz + Export sind **032b/032c**). `make gates` grün; `make schema-check` unberührt;
      Closure-Notiz. **Nicht Teil:** **032b** (Modell-Typen + Driving-Port + Service +
      `data-model.yaml`/`schema.sql` + Persistenz + AK-Tests), **032c** (DXF/PDF/PNG-Export-
      Sichtbarkeit + io-smoke); die übrigen DRW-Familien (Fangpunkte/Raster/Winkel/Bemaßung/Gruppen).

## 3. Plan (vor Code)

| Datei / Komponente | Änderungs-Art | Begründung |
|---|---|---|
| `spec/lastenheft.md` | ändern | [LH-FA-DRW-005](../../../../spec/lastenheft.md#modul-zeichnungsfunktionen-drw)/006 Outline → AK (lösungsfrei; per-ID-Heading + Inline-Anker); Header `Version:` → 0.1.14 |
| `spec/lastenheft-historie.md` | ändern | oberste Zeile 0.1.14 ([MR-010](../../../../harness/conventions.md#mr-010--lastenheft-header-version--oberste-9-historie-zeile)/[MR-012](../../../../harness/conventions.md#mr-012--mr-010-invariante-folgt-der-ausgelagerten-lastenheft-historie)) |
| `spec/data-model.yaml` | ändern | Kommentar-Heilung `layers` Z. 233 / `documents` Z. 260 „welle-3/-4"→„welle-5" (Review-MED-2; `schema-check` unberührt) |
| `spec/spezifikation.md` | ändern | §1 [`LH-FA-DRW-005.a`](../../../../spec/lastenheft.md#modul-zeichnungsfunktionen-drw)-Sammelblock; **§2.2 neue Prosa** (forward-deklarierte Tabellen welle-5, `guide_lines` slice-nummer-frei); §4 [`E-VAL-001`](../../../../spec/spezifikation.md#4-fehler-codes-und-logging-felder)-Wiederverwendung (**Ablehnungs-Lesart explizit**); §6 interne Grenz-Vertragszeile; DXF/PDF/PNG-Subset-Grenze um Hilfslinien |
| `spec/spezifikation-historie.md` | ändern | Provenance-Zeile (slice-032a) |
| `spec/architecture.md` | ändern | §1.1 DRW-Driving-Port-Zeile; `## Geschichte`-Provenance »DRW → [ADR-0018](../../adr/0018-drw-2d-zeichen-daten.md)« |
| [ADR-Index](../../adr/README.md) | ändern (Closure) | [ADR-0018](../../adr/0018-drw-2d-zeichen-daten.md)-Folgepflicht „AK-Schärfung" → erfüllt |
| `docs/reviews/{2026-07-05-slice-032a-plan}.md` | neu | [MR-006](../../../../harness/conventions.md#mr-006--unabhängiges-plan-review-vor-implementierungs-start)-Report |

## 4. Trigger

- **[ADR-0018](../../adr/0018-drw-2d-zeichen-daten.md) Accepted** (heute **Proposed**) — die
  AK-Schärfung setzt das entschiedene Fundament voraus. [ADR-0018](../../adr/0018-drw-2d-zeichen-daten.md) durchläuft vor Accept ein
  **unabhängiges Text-Review** (Reviewer ≠ Autor) + Projektinhaber-Durchsicht (Muster
  [ADR-0017](../../adr/0017-plugin-api-abi.md)); HIGH blockiert. Danach + eigenes
  [MR-006](../../../../harness/conventions.md#mr-006--unabhängiges-plan-review-vor-implementierungs-start)
  dieses Slice-Plans → **startbar**.

## 5. Closure-Trigger

- DoD vollständig, `make gates` grün, Closure-Notiz → **slice-032b (DRW-Impl: Kern + Port +
  Service + Persistenz)** wird startbar (eigenes
  [MR-006](../../../../harness/conventions.md#mr-006--unabhängiges-plan-review-vor-implementierungs-start)
  davor; [MR-009](../../../../harness/conventions.md#mr-009--geometrielastiges-code-review-vor-welle-closure)
  n/a — Hilfslinie = 2 Punkte).

## 6. Risiken und offene Punkte

- **Rest-Risiko #1 — Beobachtbarkeit ohne 2D-Canvas (zentraler HIGH-Kandidat):** die AK ist ohne
  interaktive Fläche nur über Export + Persistenz beobachtbar — der Nutzer kann in dieser Stufe
  noch **nicht zeichnen**. Mitigation: Präzedenz (welle-4 IO export-only
  [ACC-004](../../../../spec/lastenheft.md#7-abnahmekriterien), welle-3 Material canvas-los), AK
  strikt aufs **sichtbare Artefakt**, interaktive Fläche als benannter UI-Strang-Trigger
  ([ADR-0018](../../adr/0018-drw-2d-zeichen-daten.md) §Trigger) — kein stilles Über-Versprechen.
  Wertet das Review die Fassung als zu dünn → **HIGH** (Scope-Nachschnitt statt weichgezeichnetes AK).
- **Rest-Risiko #2 — Subset-Grenzen-Drift:** Hilfslinien in den DXF/PDF/PNG-Zeichenumfang erweitert
  die akzeptierte Subset-Grenze (slice-021/025) — **autorisiert** geführt (Muster
  [LH-FA-ROF-006](../../../../spec/lastenheft.md#lh-fa-rof-006)) + §-nachgezogen, kein stiller Drift.
- **Lösungsfreiheit ([MR-008](../../../../harness/conventions.md#mr-008--lastenheft-schärfung-bleibt-lösungsfrei)):**
  »überlebt Speichern/Laden«, »erscheint/verschwindet je Sichtbarkeit«, »entartete Linie/leerer
  Name abgelehnt«, »referenzierte Ebene nicht löschbar« sind benutzer-beobachtbar; Port-Namen,
  Werttyp-/Koordinaten-Struktur, FK/`restrict`, [`E-VAL-001`](../../../../spec/spezifikation.md#4-fehler-codes-und-logging-felder),
  Schema gehören in §1/§4 bzw. 032b.
- **Anker-Hebung ohne Link-Bruch:** [LH-FA-DRW-005](../../../../spec/lastenheft.md#modul-zeichnungsfunktionen-drw)/006
  haben heute nur den Modul-Anker; die Schärfung hebt sie auf `####`-Inline-Anker (Muster
  [LH-FA-IO-007](../../../../spec/lastenheft.md#lh-fa-io-007)/018c), Modul-Heading bleibt.
- **Header-Nachzug ([MR-010](../../../../harness/conventions.md#mr-010--lastenheft-header-version--oberste-9-historie-zeile)/[MR-012](../../../../harness/conventions.md#mr-012--mr-010-invariante-folgt-der-ausgelagerten-lastenheft-historie)):**
  `**Version:**` → 0.1.14 == jüngste `lastenheft-historie.md`-Zeile (vorige Schärfung 026a = 0.1.13).
- **Spec-Straten ADR-frei ([MR-011](../../../../harness/conventions.md#mr-011--referenz-integritäts-gate-matrix-ids-spans-hostpaths)):**
  die §1/§4/§6-Einarbeitung darf **keinen** ADR-Verweis im `spezifikation.md`-Körper hinterlassen
  (Provenance nur `*-historie.md`) — vor dem Gate per `grep -E 'ADR-|slice-\d{3}'` selbst fangen — **auch Slice-Tokens** (Review-LOW-3: `guide_lines`-Nennung in §2.2 slice-nummer-frei, sonst [MR-014](../../../../harness/conventions.md)-adr↛slice-Klasse-Falle) (Lerneintrag
  019a/026a). Ebenso `architecture.md`-Körper.

## 7. Sub-Area-Modus-Begründung

### Sub-Area: Spec-Schreibung

- **Modus:** GF; **Dichte:** hoch — AK-Format (Happy/Boundary/Negative), Reifephase-Klausel,
  [MR-008](../../../../harness/conventions.md#mr-008--lastenheft-schärfung-bleibt-lösungsfrei)
  (lösungsfrei), [MR-010](../../../../harness/conventions.md#mr-010--lastenheft-header-version--oberste-9-historie-zeile)/012
  (Header == Historie), [MR-011](../../../../harness/conventions.md#mr-011--referenz-integritäts-gate-matrix-ids-spans-hostpaths)
  (Spec-Straten ADR-frei), [`E-VAL-001`](../../../../spec/spezifikation.md#4-fehler-codes-und-logging-felder)-Wiederverwendung.
  **Phase-Reife:** DRW Phase 2 (Outline → AK). **Risiko:** niedrig (reine Doku; [ADR-0018](../../adr/0018-drw-2d-zeichen-daten.md) trägt das
  Fundament). **Reconciliation:** keiner; Folge = 032b/032c.

### Sub-Area: Planning-Lifecycle

- **Modus:** GF; **Dichte:** hoch ([ADR-0018](../../adr/0018-drw-2d-zeichen-daten.md)-Leitplanke:
  kein neuer Grundsatz-ADR; Muster welle-4-Strang-„a"-Slices;
  [MR-006](../../../../harness/conventions.md#mr-006--unabhängiges-plan-review-vor-implementierungs-start)
  vor Impl). **Phase-Reife:** Phase 4. **Risiko:** niedrig.

## 8. Closure-Notiz

*Wird bei DoD-Erfüllung gefüllt (Closure-Kriterien beobachtbar + Lerneintrag —
Muster [slice-026a](../done/slice-026a-plg-ak-spec.md) §8).*
