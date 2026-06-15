---
id: slice-018a
titel: Doku-Referenz-Gate — done-archive + matrix/spans/hostpaths + Spec→ADR-Richtung (ids spec-scope, always)
status: done
welle: harness-steering
lastenheft_refs: []
adr_refs: []
---

# Slice 018a: Doku-Referenz-Gate (done-archive + Referenz-Richtung Spec→ADR)

**Status:** in-progress (DoD erfüllt, bereit für Closure). **MR-006-Plan-Review
gelaufen** ([Report](../../../reviews/2026-06-14-slice-018-plan.md) —
**HIGH-1/2/3 + MED/LOW eingearbeitet**; Referenz-Richtung nach Regelwerk
korrigiert, Direktion (a)). DoD erfüllt; `make gates` grün — siehe Closure-Notiz §8.

**Welle:** `harness-steering` (Quergewerk; **nicht** welle-3-M3-Scope MAT+EVL —
Roadmap-Notiz dokumentiert den Einschub). Bindung über **MR-006** (Referenz-
Richtung) + **MR-007** (docs-check via d-check) statt LH/ADR — Harness-Sensor-
Slice, Präzedenz slice-005 (Gate-Consistency-Sensor).

**Bezug:** d-check-v0.8.0-Hebung (`1055124`) stellt `matrix`/`spans`/`hostpaths`/
`ids` bereit. Dieser Slice setzt die **Regelwerk-Referenz-Richtung** computational
durch (§Stable Dependencies: „**Spec → ADR existiert im bindenden Text nicht —
auch nicht als Quellen-Spalte**") und baut die **Archiv-Mechanik** (`done-archive/`).
Der **Voll-Korpus-`ids`** (alle 7 ID-Familien, ~780 Live-Verlinkungen) ist
**slice-018b**.

**Autor:** Dietmar Burkard. **Datum:** 2026-06-14.

**Schnitt-Herkunft:** Aufteilung von slice-018 nach MR-006. **018a = Archiv-
Mechanik + Referenz-Richtung Spec→ADR (vollständig, link + nackt); 018b = Voll-
Korpus-`ids`.** Präzedenz 009a/b, 003a/b. **Sizing-Vorbehalt:** 018a ist groß
(Move + ~10 Link-Migrationen + ~72 Spec-Ref-Sanierung + §2.7 + Config + Doku) —
§6 nennt die Commit-Trennung + die Option, die Spec-Sanierung abzuspalten.

---

## 1. Ziel

Die Doku-**Referenz-Integrität** als Gate etablieren: (a) `done-archive/` für
abgeschlossene Slice-Pläne (eingefroren), damit 018b den Voll-Korpus-`ids` sauber
begrenzen kann; (b) `spans`/`hostpaths` (Hygiene, 0 Befunde); (c) **die Regelwerk-
Referenz-Richtung Spec→ADR durchsetzen** — `matrix` (`spec-straten ↛ adr`/`↛
slice` + `forbidden superseded`) fängt die **Link**-Form, `ids` (ADR-Muster, auf
`spec` gescopt, **`link-policy: always`**) fängt die **nackten + Inline-Code**-
Erwähnungen; zusammen ist *jede* Spec→ADR-Referenz erfasst. Die bestehenden ~72
Refs werden aufgelöst.

## 2. Definition of Done

- [x] **(Vor-Schritt, HIGH-3, geklärt — dokumentiert)** `exempt-paths` greift
      **nicht** (Sondierung 2026-06-14: CHANGELOG blieb geflaggt). 018b nimmt
      `done-archive/` daher per **`ids.scope.roots`** (modul-granularer Scan-
      Override) aus; Fallback `scan.ignore` ist vertretbar, weil der
      done→done-archive-Move **tiefengleich** ist (Relativ-Links bleiben gültig).
- [x] **`done-archive/` angelegt + Slices migriert:** `docs/plan/planning/{done-archive}/`
      existiert; die abgeschlossenen Slice-/Spike-Pläne (`done/slice-*.md`,
      `done/spike-*.md`) per **reinem `git mv`** (eigener Commit, AGENTS §2.8)
      dorthin. **In `done/` bleiben:** die 3 `welle-*-results.md` (lebende
      Referenz) **und** die 4 acc-002-Artefakte (LOW-1).
- [x] **Eingehende Links migriert** (~10 Dateien: `docs/reviews/*` → Pläne,
      `adr/README`, `adr/0004`, `*results.md` → Slices, `roadmap.md`,
      `planning/README.md`) auf `{done-archive}/`; **`links`-Modul grün**.
- [x] **`.d-check.yml` Module + Regeln:**
      - `modules: [links, anchors, codepaths, spans, hostpaths, matrix, ids]`.
      - **`matrix`:** `classes` spec-straten (lastenheft/spezifikation/architecture)
        · adr (`docs/plan/adr/[0-9]*.md`) · slice; `rules: spec-straten ↛ adr`
        **und** `↛ slice`; `status.forbidden: [superseded, deprecated]`;
        `exclude-sections: [Historie, Geschichte, Änderungshistorie]`.
      - **`ids` (nur für die Referenz-Richtung, NICHT der Voll-Korpus):** ein
        Muster `ADR-\d{4}` → `docs/plan/adr/`, **`scope.roots: [spec]`**,
        **`link-policy: always`** (fängt nackt + Inline-Code, Regelwerk „auch
        nicht als Quellen-Spalte"). Die übrigen 6 Familien + der Voll-Korpus
        bleiben 018b.
- [x] **~72 Spec→ADR-Referenzen aufgelöst** (Regelwerk; matrix+ids → 0 Befunde
      in `spec/`): **Körper-Referenzen entfernen/verlagern** — `architecture.md`
      hat keine Historie-Sektion → eine **Provenance-Rand-Tabelle `## Geschichte`**
      anlegen, die zu erhaltenden ADR-Bezüge dorthin (matrix/ids ausgenommen);
      `architecture.md` §6 ADR-Index-Linkliste entfällt (Vollindex in
      `adr/README.md`). **Historie-Tabellen-Refs** (`spezifikation.md` §8) →
      **verlinken** (dort erlaubt; `exclude-sections` ist matrix-only, daher
      flaggt `ids` sie → Link ist die saubere Auflösung).
- [x] **§2.7-Reconciliation:** `AGENTS.md` §2.7 („`architecture.md` referenziert
      ADRs") kollidiert mit dem Regelwerk (AGENTS #7 < Regelwerk) → korrigieren:
      ADR-Provenance lebt in `docs/plan/adr/` + Rand-Historie, **nicht** im
      Architektur-Körper. Verschärfung Richtung Regelwerk (§2.6 n/a).
- [x] **Lifecycle-Doku:** `docs/plan/planning/README.md` (Tabelle + Bedeutungen +
      Stand) um `done-archive/` ergänzt; `AGENTS.md` §2.8 falls nötig.
- [x] **Gate-/Konventions-Doku:** `harness/README.md` §Sensors + `conventions.md`
      (**neue MR-NNN** „Referenz-Integritäts-Gate") dokumentieren Module +
      done-archive-Scoping + durchgesetzte Referenz-Richtung; `AGENTS.md`
      Gates-Tabelle. **018b-Vorbereitung:** korrigierte ID-Taxonomie (HIGH-1):
      `MR-\d{3}`→`harness/conventions.md` · `LH-(FA-[A-Z]+|QA)-\d+`+`ACC-\d+`+
      `OBJ-\d+`→`spec/lastenheft.md` · `REQ-TEC-\d+`+`E-(IO|VAL|GEO)-\d+`→
      `spec/spezifikation.md` (ADR-Muster steht schon in 018a).
- [x] **Roadmap-Notiz:** „Quergewerk slice-018a/b eingeschoben (Doku-Referenz-
      Gate), M3-Scope MAT+EVL unberührt".
- [x] **`make gates` grün** (docs-check alle aktiven Module 0 Befunde; arch-check/
      lint/test/coverage unberührt); CHANGELOG (MR-004); Closure-Notiz mit
      **Lerneintrag** (HIGH-1 Taxonomie-Grep-Falle; Referenz-Richtung erst nach
      Regelwerk-Lektüre korrekt — §2.7 war undeklarierte Inkonsistenz; `always`
      statt `prose` für vollständige Erfassung; HIGH-3 exempt-Mechanik) +
      018b-Trigger.

## 3. Plan (vor Code)

| Datei / Komponente | Änderungs-Art | Begründung |
|---|---|---|
| `docs/plan/planning/{done-archive}/` | neu | eingefrorenes Slice-Archiv |
| `docs/plan/planning/done/slice-*.md`, `spike-*.md` | `git mv` → `{done-archive}/` | Archivierung (reiner Move, §2.8; results/acc-002 bleiben) |
| `docs/reviews/*.md`, `roadmap.md`, `*results.md`, `planning/README.md`, `adr/README.md`, `adr/0004-*.md` | ändern | eingehende Links migrieren |
| `spec/architecture.md`, `spec/spezifikation.md` | ändern | ~72 Spec→ADR-Refs auflösen (Körper raus / Historie verlinken; Provenance-Tabelle in architecture) |
| `.d-check.yml` | ändern | `spans`/`hostpaths`/`matrix`/`ids`(ADR, spec-scope, always) |
| `harness/README.md`, `harness/conventions.md`, `AGENTS.md` | ändern | Gate-Doku + neue MR-NNN + §2.7-Reconcile + Lifecycle |
| `CHANGELOG.md` | ändern | Slice-Eintrag (MR-004) |
| `docs/reviews/2026-06-14-slice-018-plan.md` | neu | MR-006-Report |

## 4. Trigger

- d-check v0.8.0 (`1055124`); MR-006-Review (HIGH-1/2/3 + Split); Regelwerk-
  Referenz-Richtung als maßgeblich bestätigt (Direktion a); `link-policy: always`
  entschieden (Vollständigkeit, Dogfooding-Linie).

## 5. Closure-Trigger

- DoD vollständig, `make gates` grün → **slice-018b** (Voll-Korpus-`ids`: alle 7
  ID-Familien über den Live-Korpus, **`link-policy: always`**, `done-archive/` via
  `ids.scope.roots` ausgenommen; ~780 Verlinkungen, ggf. nach Korpus-Bereich
  gesplittet). **018b-Prüfpunkt:** fasst `always` nur Inline-Code-Spans oder auch
  Fenced-Blocks (```` ``` ````)? Bei Fenced-Blocks wäre `always` zu grob → klären.

## 6. Risiken und offene Punkte

- **Sizing (zentral):** 018a bündelt reinen `git mv` (31 Pläne) + ~10 Link-
  Migrationen + ~72 bindende Spec-Ref-Sanierungen + §2.7 + Config + Doku.
  **Commit-Trennung (§2.8):** (1) reiner Move; (2) Link-Migration + Config + Doku;
  (3) Spec→ADR-Sanierung + §2.7 als eigener inhaltlicher Commit. **Falls zu groß
  für eine Review-Sitzung:** die Spec→ADR-Richtung (Config-`matrix`/`ids` + 72-Ref
  + §2.7) in einen eigenen Slice (018a2) abspalten — MR-006-Entscheidung.
- **`link-policy: always` (statt `prose`):** Regelwerk verbietet Spec→ADR „auch
  nicht als Quellen-Spalte" → Inline-Code-Form muss mitgefangen werden (heute 0
  Fälle in spec, aber zukunftssicher); konsistent mit dem Dogfooding-Beispiel.
  `prose` war nur Aufwands-Reduktion, kein Prinzip.
- **`exclude-sections` ist matrix-only:** `ids` flaggt die ADR-Refs in der
  `spezifikation.md`-§8-Historie-Provenance-Tabelle mit → dort **verlinken**
  (in der Rand-Tabelle erlaubt; matrix ausgenommen). Im Körper dagegen: entfernen.
- **§2.7-Korrektur = Hard-Rule-Eingriff** (Verschärfung Richtung Regelwerk, keine
  Gate-Lockerung; §2.6 n/a) — in der MR/Gate-Doku begründen.
- **Residual:** keiner mehr — `matrix`+`ids` fangen Link **und** nackt **und**
  Inline-Code; die frühere „~55 gate-unsichtbar"-Aussage ist mit `ids(always)`
  hinfällig.
- **`*results.md`/acc-002 bleiben link-pflichtig** (lebende Referenz, LOW-1).

## 7. Sub-Area-Modus-Begründung

### Sub-Area: Harness/Doku-Gates (`tools/`, `.d-check.yml`, `harness/`, `docs/plan/`)

- **Modus:** BF→GF-Konvergenz. Bestands-Doku, Gate auf Referenz-Integrität
  gehoben; `done-archive/` + Module = Mechanik auf GF-Standard. Risiko mittel
  (Move + Migration, aber `links`/`matrix`/`ids` validieren jeden Schritt).

### Sub-Area: Spec (`spec/*`, `AGENTS.md`)

- **Modus:** GF. `matrix`+`ids` setzen die Regelwerk-Referenz-Richtung durch → die
  ~72 Spec→ADR-Refs werden aufgelöst (Körper raus / Historie verlinken) und §2.7
  reconcilet. **Referenz-Form-Eingriff, keine AK-/Inhalts-Änderung** (Aussagen
  bleiben, nur die verbotene Abwärts-Referenz verschwindet). MR-008 n/a.

## 8. Closure-Notiz

**Closure 2026-06-15.** DoD erfüllt; `docs-check` **0 Befunde** über alle 7
Module (90 Dateien); arch-check/lint/test/coverage unberührt.

**Auflösung der DoD-Inkonsistenz (Projektinhaber-Entscheidung „Historie
auslagern").** Der `exclude-sections`-Wert bleibt **wörtlich nach DoD**
(`[Historie, Geschichte, Änderungshistorie]`); meine zwischenzeitliche
eigenmächtige Config-Korrektur ist zurückgenommen. Damit das mit b-cads
**nummerierten** Historie-Headings (`## 8. Historie` / `## 9. Historie`)
zusammenpasst, sind die **Historie-Tabellen ausgelagert** in
[`spezifikation-historie.md`](../../../../spec/spezifikation-historie.md) und
[`lastenheft-historie.md`](../../../../spec/lastenheft-historie.md) — derivative
Provenance-Dateien außerhalb der `matrix`-Spec-Straten. So bleiben ihre
ADR-Aufwärts-Verweise erlaubt verlinkt, ohne die Spec-Straten zu berühren.

**Geliefert:**
- `done-archive/` angelegt; **31** abgeschlossene Slice-/Spike-Pläne per reinem
  `git mv` archiviert (3 `*-results.md` + 4 acc-002-Artefakte bleiben in `done/`).
  ~18 eingehende Links (adr/, reviews/, Archiv-Sibling-Refs) auf `done-archive/`
  bzw. `in-progress/` migriert — `links` grün.
- `.d-check.yml`: `matrix`/`ids`/`spans`/`hostpaths` aktiviert. `matrix`
  (spec-straten ↛ adr/slice; forbidden superseded/deprecated; exclude-sections
  Historie/Geschichte) + `ids` (ADR-Muster, `scope.roots: [spec]`,
  `link-policy: always`) setzen die Referenz-Richtung Spec→ADR durch.
- **72 Spec→ADR-Befunde aufgelöst** (5 matrix-Links + 67 ids-nackt/Code):
  Körper-Referenzen in `architecture.md`/`spezifikation.md`/`lastenheft.md`
  entfernt; `architecture.md` §6 ADR-Index → `## Geschichte`-Provenance-Rand;
  §8-Historie-Tabelle **ausgelagert** (`spezifikation-historie.md`, lastenheft §9
  analog). `AGENTS.md` §2.7 reconcilet; **MR-011** neu.

**Lerneintrag (Steering-Loop):**
1. **matrix+ids = Vise:** in `spec/` ist eine ADR-Referenz nur sauber, wenn sie
   **entfernt** wird (Link → matrix-forbidden; nackt/Code → id-unlinked). Nur in
   `exclude-sections` (Historie/Geschichte) löst ein **Link** beide auf — die
   mechanische Kodierung von „Provenance nur am Dokument-Rand" (Regelwerk Regel 5).
2. **`exclude-sections` ist exakt + case-sensitiv:** b-cad nummeriert
   („8. Historie"/„9. Historie") — bare „Historie" matcht das **nicht**. Statt den
   DoD-Config-Wert zu ändern (mein erster, eigenmächtiger Reflex → vom
   Projektinhaber zurückgewiesen), ist die **Historie ausgelagert**: die
   Provenance-Tabellen verlassen die `matrix`-Straten, der DoD-Wert bleibt wörtlich.
3. **`ids` prüft auch die ausgelagerten Historie-Dateien** (sie liegen in `spec/`):
   deren ADR-Refs sind **verlinkt** (Link erfüllt `ids`); `matrix` ignoriert sie,
   weil sie keine deklarierte Stratum-Klasse sind.
4. **`always` statt `prose`:** fängt auch Inline-Code-IDs — Regelwerk „auch nicht
   als Quellen-Spalte"; heute 0 Inline-Fälle in spec, aber zukunftssicher.
5. **§2.7 war undeklarierte Inkonsistenz:** „architecture.md referenziert ADRs"
   (AGENTS #7) gegen das Regelwerk — per Source Precedence AGENTS **verschärft**,
   nicht das Regelwerk (HIGH-2-Folge).
6. **HIGH-1 Taxonomie-Grep-Falle:** „erster Treffer" ≠ „Definitionsdatei" — die
   ID→Ziel-Zuordnung für 018b ist gegen die realen Definitionsorte verifiziert.

**Folge-Slice-Trigger → slice-018b:** Voll-Korpus-`ids` über alle 7 ID-Familien
auf dem Live-Korpus (`link-policy: always`), `done-archive/` via `ids.scope.roots`
ausgenommen (HIGH-3: `exempt-paths` griff in der Sondierung nicht).
