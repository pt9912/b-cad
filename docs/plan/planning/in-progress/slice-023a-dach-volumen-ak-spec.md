---
id: slice-023a
titel: Dach-Volumen — AK-Schärfung (LH-FA-ROF Dachdicke/Volumenkörper) & Spec-Geometrie
status: done
welle: welle-4-austausch
lastenheft_refs: [[LH-FA-ROF-001](../../../../spec/lastenheft.md#lh-fa-rof-001--satteldach), [LH-FA-ROF-002](../../../../spec/lastenheft.md#lh-fa-rof-002--walmdach), [LH-FA-ROF-003](../../../../spec/lastenheft.md#lh-fa-rof-003--pultdach), [LH-FA-IO-005](../../../../spec/lastenheft.md#lh-fa-io-005)]
adr_refs: [[ADR-0001](../../adr/0001-hexagonale-architektur.md), [ADR-0011](../../adr/0011-bauteil-hosting-wandoeffnung.md), [ADR-0012](../../adr/0012-evaluations-architektur.md), [ADR-0014](../../adr/0014-step-stl-export-backend.md)]
---

# Slice 023a: Dach-Volumen — AK-Schärfung & Spec-Geometrie

**Status:** in-progress. **Vor Start:** [MR-006](../../../../harness/conventions.md#mr-006--unabhängiges-plan-review-vor-implementierungs-start)-Plan-Review (Reviewer ≠ Autor) — **HIGHs blockieren den Start**.

**Welle:** welle-4-austausch (Dach-Volumen-Initiative, Entscheidungs-/Spec-Hälfte; Muster der
welle-2-Bauteil-Familien 014a/015a/016a). Voraussetzung für STEP-B-Rep der Dächer (Folge-Slice 024).

**Bezug:** [LH-FA-ROF-001](../../../../spec/lastenheft.md#lh-fa-rof-001--satteldach)..003 (Dach,
heute **dicke-loses Flächenmodell**), [LH-FA-IO-005](../../../../spec/lastenheft.md#lh-fa-io-005)
(STEP-Export — verspricht Dächer **bereits als „Volumenkörper"**). **Regelwerk-Einordnung:** die
Ergänzung einer **Dachdicke** ist eine **neue Lastenheft-Anforderung** (Vertragserweiterung,
Projektinhaber-autorisiert) und **löst zugleich eine Lastenheft-Inkonsistenz** auf
(ROF liefert Flächen, IO-005 verspricht Volumenkörper). **Kein neuer ADR** — Modell-Erweiterung
über das in §1 deklarierte **Bauteil-Erweiterungs-Muster (#6, [ADR-0011](../../adr/0011-bauteil-hosting-wandoeffnung.md))**, die Geometrie wird normativ in
[`spec/spezifikation.md` §1](../../../../spec/spezifikation.md) festgelegt; [ADR-0012](../../adr/0012-evaluations-architektur.md)
(EVL liest **kein** `Solid.volume_mm3` — das Dach-Volumen bleibt analytisch im Kern),
[ADR-0014](../../adr/0014-step-stl-export-backend.md) (STEP-Folge 024), [ADR-0001](../../adr/0001-hexagonale-architektur.md)
(Geometrie analytisch im Kern, framework-frei).

**Autor:** Dietmar Burkard (AI-Harness-Lauf). **Datum:** 2026-06-18.

**Schnitt-Herkunft:** Entscheidungs-/Spec-Hälfte der Dach-Volumen-Initiative (Muster 014a/015a/016a).
Die Dach-als-Volumenkörper-Semantik (Dicke, Slab-Geometrie, Klemmung) braucht **prüfbare AK + eine
entschiedene Geometrie**, bevor implementiert wird (023b). **Reine Doku/Entscheidung, kein Code.**

**Plan-Review ([MR-006](../../../../harness/conventions.md#mr-006--unabhängiges-plan-review-vor-implementierungs-start), Reviewer ≠ Autor):**
**0 HIGH / 2 MED / 2 LOW / 1 INFO — Start nicht blockiert**
([Report](../../../reviews/2026-06-18-slice-023a-plan.md)). **Regelwerk-Einordnung bestätigt:**
ROF↔[LH-FA-IO-005](../../../../spec/lastenheft.md#lh-fa-io-005)-Inkonsistenz real (ROF=Fläche, IO-005=Volumenkörper),
Vertragserweiterung/Change-Request korrekt, **kein neuer ADR** ([ADR-0011](../../adr/0011-bauteil-hosting-wandoeffnung.md)-#6-Muster),
0.1.10→0.1.11. Eingearbeitet: **MED-1** ([ADR-0011](../../adr/0011-bauteil-hosting-wandoeffnung.md)-Anker
ergänzt), **MED-2** (§1 pinnt die Volumen-Formel **analytisch im Kern**, ohne `Solid.volume_mm3` —
[ADR-0012](../../adr/0012-evaluations-architektur.md) #4), **LOW-1** (Dicke-Grenzfall
[`E-VAL-001`](../../../../spec/spezifikation.md#4-fehler-codes-und-logging-felder)), **LOW-2**
(`architecture.md` ADR-frei). INFO = [MR-011](../../../../harness/conventions.md#mr-011--referenz-integritäts-gate-matrix-ids-spans-hostpaths)-Vorwegnahme korrekt.

---

## 1. Ziel

Das Dach bekommt eine **Dicke** und wird damit ein **Volumenkörper** (statt der heutigen dicke-losen
Fläche). Das (a) erfüllt das in [LH-FA-IO-005](../../../../spec/lastenheft.md#lh-fa-io-005) bereits gegebene
Versprechen „Dächer als Volumenkörper", (b) ermöglicht das **EVL-Dach-Volumen** (die in welle-3-results §6
zurückgestellte Lücke) und (c) macht das Dach **STEP-B-Rep-fähig** (Folge-Slice 024). Dieser Slice
**schärft das Lastenheft** (neue Dachdicke-Anforderung, lösungsfrei/benutzer-beobachtbar,
[MR-008](../../../../harness/conventions.md#mr-008--lastenheft-schärfung-bleibt-lösungsfrei)) und **entscheidet die
Geometrie-Mechanik** (`spec/spezifikation.md` §1: geschlossener Schräg-Slab) — bevor 023b implementiert.

## 2. Definition of Done

- [x] **DoD-1 — Lastenheft: neue AK „Dachdicke / Dach als Volumenkörper".** Eine **neue
      `LH-FA-ROF`-Anforderung** (ID beim Spec-Schreiben nach [MR-002](../../../../harness/conventions.md#mr-002--id-schema-für-b-cad)
      vergeben — die nächste freie `LH-FA-ROF`-Nummer) auf **AK-Niveau**, **lösungsfrei/benutzer-beobachtbar**
      ([MR-008](../../../../harness/conventions.md#mr-008--lastenheft-schärfung-bleibt-lösungsfrei) — **keine** Slab-/Offset-/
      OCC-Mechanik im Lastenheft-Text): **Happy** — Given ein Dach, when es erzeugt/dargestellt wird,
      then ist es ein **Volumenkörper mit einer Dicke** (nicht eine dünne Fläche); ohne Angabe gilt eine
      **Standard-Dicke**. **Boundary** — Dicke am Grenzwert akzeptiert (Bereich §3). **Negative/Total** —
      degenerierter Grundriss → kein Dach, kein Absturz (wie [LH-FA-ROF-001](../../../../spec/lastenheft.md#lh-fa-rof-001--satteldach));
      eine **Null-/degenerierte Dicke** wird geklemmt (Standard-Dicke / Bereich §3,
      [`E-VAL-001`](../../../../spec/spezifikation.md#4-fehler-codes-und-logging-felder)) — kein Absturz (LOW-1).
      **Konsistenz:** der Wortlaut macht ROF↔[LH-FA-IO-005](../../../../spec/lastenheft.md#lh-fa-io-005) („Volumenkörper")
      widerspruchsfrei. + Header `**Version:**` → **0.1.11** == oberste `lastenheft-historie.md`-Zeile
      ([MR-010](../../../../harness/conventions.md#mr-010--lastenheft-header-version--oberste-9-historie-zeile)/[MR-012](../../../../harness/conventions.md#mr-012--mr-010-invariante-folgt-der-ausgelagerten-lastenheft-historie)).
- [x] **DoD-2 — `spec/spezifikation.md` §1 [`LH-FA-ROF-001.a`](../../../../spec/spezifikation.md) erweitert
      (Geometrie-Mechanik).** Das Dach wird ein **geschlossener Schräg-Slab** der Dicke `d`: die
      bestehende(n) geneigte(n) **Oberfläche(n)** + eine **parallele Unterseite** (Dicke-Offset) +
      **geschlossene Trauf-/Giebel-Ränder** → eine **wasserdichte, außen-orientierte Hülle** (alle drei
      Typen Sattel/Walm/Pult). **Offset-Richtung entschieden** (vertikal nach unten **oder** schräg-normal
      — §1 fixiert; Default-Empfehlung: vertikal, parametrisch tragbar). **Folge:** das Netz ist nun
      **geschlossen** → EVL-Volumen **analytisch im Kern** berechenbar (`volume_geometry`, **ohne**
      `Solid.volume_mm3`-Lesen — [ADR-0012](../../adr/0012-evaluations-architektur.md) #4-Reinheit,
      welle-3-Plan-Fang; **§1 pinnt die Volumen-Formel analytisch**, MED-2) **und** STEP-B-Rep-vernähbar (024). Mechanik, **kein** Lastenheft;
      Spec-Körper **ADR-frei** ([MR-011](../../../../harness/conventions.md#mr-011--referenz-integritäts-gate-matrix-ids-spans-hostpaths)).
      + `spezifikation-historie.md` + `**Letzte Änderung:**`-Header.
- [x] **DoD-3 — `spec/spezifikation.md` §3 Dach-Dicke-Konstanten.** `ROOF_THICKNESS_MIN_MM` /
      `_MAX_MM` / `DEFAULT_ROOF_THICKNESS_MM` als §3-Wertebereich dokumentiert (Quelle der Wahrheit für
      `constants.h`; Muster Slab `SLAB_THICKNESS_*`); Klemmung über [`E-VAL-001`](../../../../spec/spezifikation.md#4-fehler-codes-und-logging-felder) (§3). Die `constants.h`-
      **Code**-Ergänzung ist **023b**, nicht hier.
- [x] **DoD-4 — Wirkung benannt, reine Doku.** **EVL-Volumen** (das Dach trägt künftig bei — die
      welle-3-results-§6-Lücke; Impl 023b), **Persistenz** (`roofs.thickness_mm`-Spalte — 023c),
      **STEP-B-Rep** (024) als Folge-Slices benannt. **Kein Code/Test/Schema/ADR.** `make gates` grün;
      `make schema-check` unberührt; Closure-Notiz mit Lerneintrag. **Nicht Teil:** die Geometrie-Impl
      (023b, geometrie-schwer → [MR-009](../../../../harness/conventions.md#mr-009--geometrielastiges-code-review-vor-welle-closure)).

## 3. Plan (vor Code)

| Datei / Komponente | Änderungs-Art | Begründung |
|---|---|---|
| `spec/lastenheft.md` | ändern | neue `LH-FA-ROF`-Dachdicke-AK (lösungsfrei, Volumenkörper); Header `Version:` → 0.1.11 |
| `spec/lastenheft-historie.md` | ändern | oberste Zeile 0.1.11 ([MR-010](../../../../harness/conventions.md#mr-010--lastenheft-header-version--oberste-9-historie-zeile)/[MR-012](../../../../harness/conventions.md#mr-012--mr-010-invariante-folgt-der-ausgelagerten-lastenheft-historie)) |
| `spec/spezifikation.md` | ändern | §1 [`LH-FA-ROF-001.a`](../../../../spec/spezifikation.md) Slab-Geometrie; §3 Dach-Dicke-Konstanten |
| `spec/spezifikation-historie.md` | ändern | Provenance-Zeile (slice-023a) |
| `spec/architecture.md` | ggf. ändern | vsl. unverändert (Roof bleibt ein Bauteil; optional `## Geschichte`-Provenance) |
| `docs/reviews/{2026-06-18-slice-023a-plan}.md` | neu | [MR-006](../../../../harness/conventions.md#mr-006--unabhängiges-plan-review-vor-implementierungs-start)-Report |

## 4. Trigger

- Keiner — **startbar** nach [MR-006](../../../../harness/conventions.md#mr-006--unabhängiges-plan-review-vor-implementierungs-start)-Review:
  reine Doku/Entscheidung; Projektinhaber hat die Vertragserweiterung autorisiert.

## 5. Closure-Trigger

- DoD vollständig, `make gates` grün, Closure-Notiz → **023b** (Dach-Volumen-Impl) wird startbar.

## 6. Risiken und offene Punkte

- **Vertragserweiterung (Regelwerk):** eine **neue** `LH-FA-ROF`-Anforderung wächst den Vertrag —
  Projektinhaber-Entscheid (gegeben: „Dach-Volumen zuerst, Regelwerk konform"). Der Wortlaut bleibt
  **lösungsfrei** ([MR-008](../../../../harness/conventions.md#mr-008--lastenheft-schärfung-bleibt-lösungsfrei)): „Dach ist
  ein Volumenkörper mit Dicke" ist benutzer-beobachtbar; Slab-/Offset-/Vernähungs-Mechanik gehört in §1.
- **ROF↔IO-005-Konsistenz:** der neue Wortlaut darf die bestehende IO-005-„Volumenkörper"-Zusage
  **bestätigen**, nicht doppeln/widersprechen — die Inkonsistenz wird **aufgelöst**, keine zweite
  Volumenkörper-Definition.
- **Offset-Richtung (§1-Entscheidung, MED-Kandidat):** vertikaler vs. schräg-normaler Dicke-Offset —
  beeinflusst Geometrie + EVL-Volumen-Formel (023b). §1 **fixiert** es eindeutig; **vertikal** ist
  einfacher und parametrisch tragbar, **schräg-normal** realistischer. Nicht offen lassen.
- **Geschlossenheit ist die Voraussetzung für 024:** der §1-Slab muss **wasserdicht + außen-orientiert**
  sein (Walm-Dach mit Giebel-Walmen + Unterseite ist der heikle Fall) — die **Geometrie-Korrektheit**
  prüft 023b ([MR-009](../../../../harness/conventions.md#mr-009--geometrielastiges-code-review-vor-welle-closure) + Invarianten-Tests), nicht dieser Doku-Slice; §1 muss die Geschlossenheit aber
  **normativ fordern**.
- **[MR-010](../../../../harness/conventions.md#mr-010--lastenheft-header-version--oberste-9-historie-zeile)/012-Header-Nachzug:** `lastenheft.md` `**Version:**` → 0.1.11 == oberste `lastenheft-historie.md`-Zeile.
- **Spec-Straten ADR-frei ([MR-011](../../../../harness/conventions.md#mr-011--referenz-integritäts-gate-matrix-ids-spans-hostpaths)):** §1-/§3-Einarbeitung ohne ADR-Verweis im
  `spezifikation.md`-Körper (Provenance nur in `spezifikation-historie.md`) — vor dem Gate per
  `grep ADR-` selbst fangen (Muster 019a/020a/021a).
- **Kein Über-Versprechen:** nur die Dach-**Geometrie** wird Volumenkörper; Dämmaufbau/Mehrschicht/
  Material-Schichten bleiben offen (Teilumfang wie bisher).

## 7. Sub-Area-Modus-Begründung

### Sub-Area: Spec-Schreibung (`spec/`)

- **Modus:** GF; **Konventionen-Dichte:** hoch — AK-Format (Happy/Boundary/Negative),
  [MR-008](../../../../harness/conventions.md#mr-008--lastenheft-schärfung-bleibt-lösungsfrei) (lösungsfrei),
  [MR-010](../../../../harness/conventions.md#mr-010--lastenheft-header-version--oberste-9-historie-zeile)/012 (Header==Historie),
  §3-Konstanten-Konvention ([`E-VAL-001`](../../../../spec/spezifikation.md#4-fehler-codes-und-logging-felder)-Klemmung), [MR-011](../../../../harness/conventions.md#mr-011--referenz-integritäts-gate-matrix-ids-spans-hostpaths)
  (Spec ADR-frei). **Phase-Reife:** ROF Phase 4→Vertragserweiterung. **Evidenz-/Diskrepanz-Risiko:**
  niedrig (reine Doku). **Reconciliation:** keiner; Folge-Slice 023b.

### Sub-Area: Planning-Lifecycle (`docs/plan/`)

- **Modus:** GF; **Dichte:** hoch ([MR-006](../../../../harness/conventions.md#mr-006--unabhängiges-plan-review-vor-implementierungs-start)
  vor Impl; Initiative-Schnitt 023a/b/c→024). **Phase-Reife:** Phase 4. **Risiko:** niedrig.

## 8. Closure-Notiz

**Closure-Kriterien (beobachtbar, 2026-06-18):**

- **Lastenheft 0.1.11:** neue Anforderung [LH-FA-ROF-006](../../../../spec/lastenheft.md#lh-fa-rof-006)
  (Dachdicke / Dach als Volumenkörper) auf AK-Niveau (lösungsfrei/benutzer-beobachtbar — Volumenkörper
  mit Dicke, Standard-Dicke, Grenzwert-Klemmung, Totalität); löst die ROF↔[LH-FA-IO-005](../../../../spec/lastenheft.md#lh-fa-io-005)-
  Inkonsistenz auf. Header 0.1.11 == jüngste `lastenheft-historie.md`-Zeile
  ([MR-010](../../../../harness/conventions.md#mr-010--lastenheft-header-version--oberste-9-historie-zeile)/[MR-012](../../../../harness/conventions.md#mr-012--mr-010-invariante-folgt-der-ausgelagerten-lastenheft-historie)).
- **§1 [`LH-FA-ROF-001.a`](../../../../spec/lastenheft.md#lh-fa-rof-001--satteldach):** geschlossener Schräg-Slab der Dicke `d` (Oberseite + **vertikal** um `d`
  versetzte parallele Unterseite + geschlossene Trauf-/Giebel-Ränder, wasserdicht/außen-orientiert,
  alle 3 Typen); **Offset-Richtung = vertikal** (entschieden); Dach-Volumen **analytisch im Kern**
  (ohne `Solid.volume_mm3`). §3 `ROOF_THICKNESS_MIN/MAX/DEFAULT` (50/500/200 mm). Letzte Änderung +
  `spezifikation-historie.md`.
- **Spec-Straten ADR-frei ([MR-011](../../../../harness/conventions.md#mr-011--referenz-integritäts-gate-matrix-ids-spans-hostpaths)):**
  `lastenheft.md` + §1-Körper tragen **keinen** `ADR-NNNN`-Verweis (per `grep ADR-` selbst geprüft;
  Provenance nur in `spezifikation-historie.md`, [ADR-0011](../../adr/0011-bauteil-hosting-wandoeffnung.md)/[ADR-0012](../../adr/0012-evaluations-architektur.md) verlinkt).
- **Kein Code/Test/Schema/ADR.** `make gates` grün; `make schema-check` unberührt.

**Lerneintrag:**

- **Vertragserweiterung vs. Reifephase-Schärfung (Regelwerk):** ROF-001..005 waren bereits Voll-AK
  (Flächenmodell) — die Dachdicke ist eine **neue** Anforderung (Vertrags-Wachstum/Change Request,
  Projektinhaber-autorisiert), **nicht** eine Outline→AK-Schärfung. Zugleich löst sie die latente
  ROF↔IO-005-Inkonsistenz (IO-005 versprach Volumenkörper, ROF lieferte Flächen).
- **Offset-Richtung entschieden (nicht offen gelassen):** vertikaler Dicke-Offset; schräg-normal als
  späterer Ausbau benannt — verhindert eine 023b-blockierende offene §1-Frage.
- **Analytisches Dach-Volumen (welle-3-Plan-Fang-Reflex):** §1 pinnt die Volumen-Formel analytisch im
  Kern, **ohne** `Solid.volume_mm3` — die [ADR-0012](../../adr/0012-evaluations-architektur.md)-#4-Reinheit
  (welle-3 kritischer Plan-Fang) vorab gewahrt.

**Restrisiko / Nachfolge:** **023b** (Geometrie-Impl — geschlossener Slab alle 3 Typen,
`StructureEditService` Default/Clamp, Viewer folgt, EVL-Dach-Volumen; geometrie-schwer →
[MR-009](../../../../harness/conventions.md#mr-009--geometrielastiges-code-review-vor-welle-closure)) · **023c**
(Persistenz `roofs.thickness_mm`) · **024** (Dächer+Treppen STEP-B-Rep). Der **Walm-Dach-Slab**
(Giebel-Walme + Unterseite) ist der geometrisch heikelste 023b-Fall.
