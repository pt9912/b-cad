---
id: slice-038
titel: Harness-Gate-Fahrplan-Abschluss — `--trace` advisory Report, `--require-complete` verschoben
status: done
welle: welle-5-erweiterung (Quergewerk harness-steering)
lastenheft_refs: []  # reines Prozess-/Doku-Steering, keine LH-Anforderung
adr_refs: []         # kein ADR
---

# Slice 038: `--trace`-Report + Harness-Gate-Fahrplan-Abschluss

**Status:** **done** (ausgeführt 2026-07-05; angelegt evidence-first, Muster
[slice-037](../done/slice-037-tracked-fresh-clone.md)).
**[MR-006](../../../../harness/conventions.md#mr-006--unabhängiges-plan-review-vor-implementierungs-start)-Review:
1 HIGH behoben** (H1 — die Ursache war falsch zugeschrieben; + 2 MED / 4 LOW eingearbeitet;
[Report](../../../reviews/2026-07-05-slice-038-plan.md)).

**Welle:** welle-5-erweiterung — **Quergewerk `harness-steering`** (kein Wellen-Feature).
**DRW-Scope (032a/b/c) unberührt** — pausiert.

**Bezug:** der **letzte** Punkt des d-check-Modul-Fahrplans ([`roadmap.md`](../in-progress/roadmap.md)
§Harness-Gate-Fahrplan): **`--trace`** (Requirements-Traceability-Matrix, `make doc-trace`,
DC-FA-CLI-009) — **advisory Report**. Dieser Slice ist ein **leichtes Doku-/Entscheidungs-Steering**
(kein Gate, keine `.d-check.yml`-Änderung): er ordnet `--trace`/`--require-complete` evidenzbasiert ein
und **schließt den Fahrplan**.

**Autor:** Dietmar Burkard. **Datum:** 2026-07-05.

**Schnitt-Herkunft:** Fahrplan-Abschluss nach den Gate-Adoptionen (`matrix adr→slice`/033,
`commits`/034, `vcs`/035, `planning`/036, `tracked`/037). `--trace` ist **report-only**; die Gate-
Variante `--require-complete` ist **verschoben** (nicht verworfen). **Reine Doku-Entscheidung, kein
Code, kein Gate.**

---

## 1. Ziel

Den Fahrplan-Rest `--trace` **evidenzbasiert einordnen**: `make doc-trace` bleibt ein **verfügbarer
advisory Report** (Exit 0 auch bei Waisen), der die Anforderungsliste als **Backlog** trägt; die Gate-
Variante `--require-complete` wird **auf den Vollständigkeits-Meilenstein verschoben** (nicht in
`make gates`/CI jetzt). Damit ist der d-check-Modul-Fahrplan **abgeschlossen**.

**Evidenz (gemessen + im [MR-006](../../../../harness/conventions.md#mr-006--unabhängiges-plan-review-vor-implementierungs-start) gegengeprüft):**
- `make doc-trace` → **8 ok / 44 WAISE / 52 Anforderungen** in der Matrix.
- **Ursache der WAISEN (korrigiert — NICHT der done-archive-Freeze):**
  1. **`--trace`-Slice-Token ist `slice-\d{3}` (suffix-blind).** b-cad setzte welle-2/3/4 in **31
     Buchstaben-Sub-Slices** (013a…024b) um → für `--trace` **unsichtbar**, **egal in welchem Ordner**.
     Beleg: `slice-007`/`012`/`022`/`033` (glatt nummeriert) **zählen** — obwohl 007 in `done-archive/`
     liegt (`--trace` scannt done-archive; `ids.scope.ignore` gilt nur fürs `ids`-Modul). Deshalb ROF-001
     (slice-014a/b, Suffix) = WAISE, IO-001 (slice-**022**, glatt) = ok. **Der Freeze ist irrelevant.**
  2. **Truncierte Matrix-Universe:** nur `###`-definierte Anforderungen erscheinen; **DRW** (7), **UI** (6),
     **PLG** (4), **IO-002…008** (`####`-/Outline-Form) fehlen ganz → 52 statt ~87 LH-Ids.
  3. **Kleiner Rest echter Nicht-Referenzen** (weder implementiert-aber-suffix-blind noch getrackt):
     `QA-006` (Mehrsprachigkeit, welle-5, **dokumentiert zurückgestellt**) sowie die weichen NFR
     `QA-001`/`QA-002`/`QA-004` (z. B. Speicherverbrauch < 2 GB) — **abnahme-verifiziert**, nicht
     slice-getragen.
- **Substanz-Check:** die großen WAISE-Familien (ROF/SLB/STR/DOR/WIN/EVL/MAT/FND) sind **real
  implementiert** (Sub-Slices mit Exakt-Id: 013a/b, 014a/b, 015a/b, 016a/b, 017c–g, 023a/b …) → **keine**
  actionable Defekte. „Nicht gaten" ist richtig — die frühere *Ursachen*-Zuschreibung (Freeze) war es nicht.

## 2. Definition of Done

- [x] **`harness/conventions.md` neuer Eintrag [MR-019](../../../../harness/conventions.md)**
      (Doku-Entscheidung; Muster [MR-018](../../../../harness/conventions.md)): dokumentiert
      (1) `make doc-trace` (`--trace`, DC-FA-CLI-009) als **verfügbaren advisory Report** — aber ein
      **Live-Korpus-Teilausschnitt**: nur `###`-Anforderungen (52 von ~87; DRW/UI/PLG/IO-Outlines fehlen)
      und nur **glatte `slice-\d{3}`-Ids** (b-cads 31 Buchstaben-Sub-Slices sind unsichtbar) → **kein
      Vollständigkeits-Instrument**, ausdrücklich **nicht** Folge des done-archive-Freeze;
      (2) **`--require-complete` verschoben** (nicht verworfen; Muster [`versions`](../in-progress/roadmap.md)):
      es ist ein **Meilenstein-Schalter** — umzulegen am Punkt der **Vollständigkeits-Zusage** (die Welle,
      die die letzten Anforderungen schließt; dort muss die Waisen-Liste leer sein), **nicht** in
      `make gates`/CI und **nicht** an jedem Zwischen-Wellen-Abschluss (b-cad = **Voll-Spec** → Waisen sind
      **build-out-lang**, nicht transient; der Regelwerk-Hinweis „GF erlaubt *transiente* Waisen" trifft den
      inkrementellen Stil, nicht den Voll-Spec-Fall). **Zusätzliche Bindungs-Voraussetzung** (nicht nur
      Timing): am Meilenstein müssten **Suffix-Blindheit + truncierte Universe** behandelt sein (glatte-Slice-
      /`###`-Traceability oder ein `--trace`-Fix), da `--require-complete` **keine** exempt/allow-list hat
      (verifiziert) → sonst rot an Nicht-Defekten;
      (3) **echte Backlog-Posten benannt** (nicht im Sammel-„gewollte Waisen" versteckt): `QA-006`
      (welle-5), `QA-002` (Speicherverbrauch, abnahme-verifiziert / offene Lücke);
      (4) **Fahrplan-Abschluss** (033–037 Gate-Adoptionen live + `--trace`-Report; `versions` verschoben,
      `diagrams`/`external` passt nicht).
- [x] **Roadmap-Fahrplan-Nachzug:** [`roadmap.md`](../in-progress/roadmap.md) §Harness-Gate-Fahrplan
      `--trace` → **verfügbar (advisory Report)**, `--require-complete` → **verschoben (Vollständigkeits-
      Meilenstein)** + **Abschluss-Notiz** + Quergewerk-Zeile §Historische Trigger-Verschiebungen. **Ohne**
      den (falschen) Freeze-Mechanismus.
- [x] **`CHANGELOG.md`** (grobkörnig, [MR-004](../../../../harness/conventions.md#mr-004--top-level-changelogmd-keep-a-changelog)).
- [x] **Kein Gate, keine `.d-check.yml`-Änderung, kein `modules:`-Eintrag** (report-only). `make gates`
      unberührt; `make doc-trace` als Report belegt (8 ok / 44 WAISE).
- [x] **Nicht Teil:** die WAISEN „reparieren" (die suffix-blinden sind Nicht-Defekte; `QA-002` ist eine
      benannte Abnahme-/Backlog-Frage, kein Slice-Ziel hier); die DRW-Arbeit; `diagrams`/`external`/`versions`.

## 3. Plan (vor Ausführung)

| Datei / Komponente | Änderungs-Art | Begründung |
|---|---|---|
| `harness/conventions.md` | ändern | neuer Eintrag **[MR-019](../../../../harness/conventions.md)** (`--trace` advisory + Teilausschnitt; `--require-complete` verschoben; Fahrplan-Abschluss) |
| `docs/plan/planning/in-progress/roadmap.md` | ändern | Fahrplan `--trace`→verfügbar / `--require-complete`→verschoben; Abschluss-Notiz; Quergewerk-Eintrag |
| `CHANGELOG.md` | ändern | `[Unreleased]`-Eintrag |
| `docs/reviews/{2026-07-05-slice-038-plan}.md` | neu | [MR-006](../../../../harness/conventions.md#mr-006--unabhängiges-plan-review-vor-implementierungs-start)-Report |

## 4. Trigger

- **[MR-006](../../../../harness/conventions.md#mr-006--unabhängiges-plan-review-vor-implementierungs-start)-Review
  durchlaufen** (1 HIGH behoben — Ursache umbasiert; + 2 MED / 4 LOW) → **startbar.**

## 5. Closure-Trigger

- DoD vollständig, `make gates` grün (unberührt), Closure-Notiz → der **Harness-Gate-Fahrplan ist
  abgeschlossen**; weitere Modul-Adoptionen nur bei Substrat-/Meilenstein-Änderung (`versions` bei erstem
  Release; `--require-complete` an der Vollständigkeits-Zusage).

## 6. Risiken und offene Punkte

- **WAISE-Ursache korrekt benennen (Review-H1, zentral):** die 44 WAISE sind **kein** Freeze-Effekt (der
  Freeze wurde reproduziert-irrelevant: glatte done-archive-Slices zählen). Ursache = **`slice-\d{3}`-Suffix-
  Blindheit** (31 Buchstaben-Sub-Slices unsichtbar) **+ truncierte Matrix** (nur `###`). [MR-019](../../../../harness/conventions.md)/Roadmap
  dürfen den Freeze-Mechanismus **nicht** wiederholen — sonst brennt sich ein falscher Kausal-Grund in die
  near-immutable MR-Familie ein (ein Maintainer erwöge „Freeze aufweichen" als wirkungslosen Hebel).
- **`--require-complete` verschoben, nicht verworfen (Nutzer-Korrektur):** kein Vollständigkeits-Zwang
  jetzt, sondern ein **Meilenstein-Schalter** (Punkt der Vollständigkeits-Zusage). **Opt-in-Grund** genau
  dieser: b-cad ist Voll-Spec → build-out-lange (nicht transiente) Waisen. Bis dahin `--trace` (advisory) =
  Backlog. **Doppelte Bindungs-Voraussetzung am Meilenstein:** Timing **und** Werkzeug (Suffix-/Truncation-
  Behandlung, kein exempt-list) — beides ehrlich in [MR-019](../../../../harness/conventions.md).
- **Echte Nicht-Defekte trennen (Review-M2):** `QA-006` = dokumentiertes welle-5-Deferral; `QA-002`
  (Speicherverbrauch) = un-referenzierte weiche NFR → als **Abnahme-Frage** benannt, nicht als „gewollte
  Waise" verschwiegen. (Die früheren Beispiele MAT-004/WAL-006/DRW waren falsch: WAL-006 = ok, MAT-004/DRW
  nicht in der Matrix.)
- **H1-Sweep (Review-L1):** keine vorbestehende Anforderungs-**Vollständigkeits**-Regel; `AGENTS §4`/README
  §Traceability = **Commit**-Traceability (anderes Konzept, [MR-015](../../../../harness/conventions.md)).
  Neue Negativ-Doku, eine Heimat ([MR-019](../../../../harness/conventions.md)).
- **Kein Scope-Creep:** kein Gate, kein Code, keine `.d-check.yml`-Änderung; DRW unberührt.

## 7. Sub-Area-Modus-Begründung

### Sub-Area: Konventionen & Harness-Doku

- **Modus:** GF; **Dichte:** hoch — `MR-NNN`, Fahrplan-Führung. **Phase-Reife:** Phase 4. **Risiko:**
  niedrig (reine Doku/Entscheidung, evidenzbasiert + gegengeprüft; kein Gate, kein Code).

## 8. Closure-Notiz

**Ausgeführt 2026-07-05** — DoD vollständig; `make gates` grün (unberührt — kein Gate); `make doc-trace`
als advisory Report belegt (8 ok / 44 WAISE).

**Beobachtbare Closure-Kriterien:**
- `--trace` (`make doc-trace`) als **advisory Report** dokumentiert ([MR-019](../../../../harness/conventions.md)),
  ausdrücklich **Live-Korpus-Teilausschnitt** (suffix-blind + truncated), kein Vollständigkeits-Instrument.
- `--require-complete` **verschoben** (Vollständigkeits-Meilenstein), nicht verworfen — Fahrplan + [MR-019](../../../../harness/conventions.md).
- **Harness-Gate-Fahrplan abgeschlossen:** 033–037 Gates live, `--trace` Report, `--require-complete`/`versions`
  verschoben, `diagrams`/`external` passt nicht.

**Lerneintrag:** Der [MR-006](../../../../harness/conventions.md#mr-006--unabhängiges-plan-review-vor-implementierungs-start)-Review fing einen **echten Kausal-Fehler**: meine done-archive-Freeze-Erklärung der
44 WAISE war **empirisch falsch** (reproduziert widerlegt — `--trace` scannt done-archive; die glatten 007/022
zählen). Wahre Ursache: `--trace`-Slice-Token `slice-\d{3}` ist **suffix-blind**. Lehre: **eine plausible
Kausal-Erklärung vor dem Festschreiben empirisch prüfen** (der Freeze *klang* passend, war es aber nicht) —
besonders vor near-immutablen MR-Einträgen. Zweite Lehre (Nutzer-Korrektur): `--require-complete` ist ein
**Meilenstein-Schalter**, kein Jetzt-Zwang — verschoben, nicht verworfen (Muster `versions`).

**Folge:** die d-check-Modul-Adoptions-Linie ist durch. Außerhalb des Fahrplans: DRW ([ADR-0018](../../adr/0018-drw-2d-zeichen-daten.md)-Text-Review);
`versions`/`--require-complete` bei Substrat-/Meilenstein-Änderung.
