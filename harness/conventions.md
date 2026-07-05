# Harness-Konventionen — b-cad

Diese Datei deklariert die *repo-lokalen* Strukturregeln von **b-cad**
gegenüber der adoptierten Harnesskonvention (Baseline). Sie ist
konformitätsbringend für *Form*-Fragen, nicht autoritativ über Inhalt;
bei Konflikt mit einer kanonischen Quelle gilt die kanonische Quelle
([Source Precedence](README.md#source-precedence)).

## Purpose

Der Default-Ort für:

- **Adaptionen** ggü. der Baseline (mit Begründung und Auflösungs-Trigger).
- **ID-Schema-Deklaration** — welches Präfix-Schema b-cad nutzt
  (deklariert in [MR-002](#mr-002--id-schema-für-b-cad); *vergeben*
  werden IDs beim Spec-/ADR-Schreiben, nie ad hoc —
  [`AGENTS.md` §4](../AGENTS.md)).
- **Zusatzklassen-Deklarationen** für Sensors-Bindung-Klassen, die über
  die vier kanonischen (ADR, Carveout, Schwelle, Reproduzierbarkeit)
  hinausgehen.
- **Modus-Deklarationen** pro Sub-Area (Greenfield / Brownfield / Hybrid)
  inklusive Konvergenz-Auftrag bei BF.

## Baseline

- **Konvention:** AI-Harness-Kurs (Harness Engineering für Coding
  Agents) — externes, benachbartes Repo *ai-harness-course*. Sein
  Template-Set und seine Grundlagen-Konventionen sind die Form-Quelle.
  Bewusst **kein** Markdown-Link, da Fremd-Repo (ein repo-relativer
  Pfad würde aus b-cad herausführen).
- **Stand:** Template-Set 2026-06.
- **Datum der Adoption:** 2026-06-08.
- **Bootstrap-Modus:** Greenfield (neues Repo, Doku führt, Code folgt).
  Bootstrap-Lebenszyklus nach Kurs-Modul 2, Worked Example 1.

## Adoptierte Konventions-Quellen

- **Extern (Lehrmaterial):** AI-Harness-Kurs (benachbartes Repo
  *ai-harness-course*), insbesondere dessen Grundlagen-Konventionen und
  -Klassifikation. Als externe Nennung geführt, nicht als In-Repo-Link.
- **Extern (Agenten-Destillat):**
  [`agents-regelwerk.md`](https://raw.githubusercontent.com/pt9912/ai-harness-course/main/kurs/de/agents-regelwerk.md)
  im GitHub-Repo
  [`pt9912/ai-harness-course`](https://github.com/pt9912/ai-harness-course)
  — das operative Kurs-Regelwerk, das ein Code-Agent statt des vollen
  Lehrmaterials liest. Derivativ ohne eigene Normativität — bei
  Konflikt gilt das Lehrmaterial.
- **In-Repo (verkörperte Form):** Die ausgefüllten Artefakte unter
  [`../spec/`](../spec/), [`../docs/plan/`](../docs/plan/) und
  [`README.md`](README.md) selbst sind die in b-cad verkörperte
  Konvention.

## Repo-Klasse

**Anwendungs-/Produkt-Repo** (Desktop-CAD für Wohngebäude). Keine
Safety/Control-Klasse (b-cad steuert keine physische Anlage) und keine
Policy/Compliance-Klasse. Hard Rules werden moderat gesetzt — schärfer
nur an den Stellen, an denen *Datenverlust am Gebäudemodell* droht
(Persistenz, Undo/Redo, Crash-Recovery, atomare Schreiboperationen).

## Adaptions-Block

Einträge sind chronologisch nummeriert und nach Aufnahme **inhaltlich
unveränderlich** — Korrekturen oder Aufhebungen entstehen als neuer
`MR-<NNN>`-Eintrag mit Verweis auf den abgelösten Eintrag (dieselbe
Immutability-Disziplin wie bei Accepted-ADRs,
[`AGENTS.md` §2.5](../AGENTS.md)).

### MR-000 — Baseline-Aussage

- **Datum:** 2026-06-08
- **Geltungsbereich:** gesamtes Repo
- **Adaption:** *Keine inhaltlichen Adaptionen ggü. Baseline-Default
  für Verzeichniskonvention, Lifecycle-Regeln (`open → next →
  in-progress → done`), Carveout-Disziplin und ID-Schema-Mechanik.*
  Konkrete ID-Präfixe für b-cad siehe MR-002.
- **Begründung:** Initial-Setzung beim Greenfield-Bootstrap. Spätere
  Adaptionen werden als `MR-<NNN>` nachgetragen.
- **Auflösungs-Trigger:** permanent.

### MR-001 — Source Precedence mit eigener Spezifikations-Schicht

- **Datum:** 2026-06-08
- **Geltungsbereich:** [`harness/README.md` §Source precedence](README.md#source-precedence)
- **Adaption:** Die Source-Precedence-Tabelle führt
  [`spec/spezifikation.md`](../spec/spezifikation.md) als eigenen
  **Rang 2** zwischen Lastenheft (Rang 1) und Architektur (Rang 3).
  Der Kurs-Default setzt zwei Spec-Ränge (`lastenheft` →
  `architecture`); b-cad nutzt drei.
- **Begründung:** b-cad verwendet die Spec-Stratifizierung explizit mit
  drei Spec-Dateien — Lastenheft (vertraglich, *was*), Spezifikation
  (technisch fortschreibbar: Geometrie-Toleranzen, Default-Wertebereiche,
  Fehler-Codes, OTel-Spans), Architektur (Komponenten/Schichten). Damit
  die ADR-Schärfungs-Regel ("ADR darf Spezifikation schärfen, nicht
  Lastenheft") strukturell sichtbar ist, muss die Spezifikation eigener
  Rang sein.
- **Auflösungs-Trigger:** permanent.

### MR-002 — ID-Schema für b-cad

- **Datum:** 2026-06-08
- **Geltungsbereich:** `spec/`, `docs/plan/`, Commits, PRs
- **Adaption:** b-cad verwendet von Beginn an *Bereichskürzel* in den
  funktionalen Anforderungs-IDs (kein zweistelliges Welle-1-Schema wie
  im Kurs-Beispiel). Schema:

  | Artefakt | Schema | Beispiel |
  |---|---|---|
  | Funktionale Anforderung | `LH-FA-<BEREICH>-<NNN>` | [`LH-FA-WAL-002`](../spec/lastenheft.md#lh-fa-wal-002--wandstärke-definieren) |
  | Nichtfunktionale Anforderung | `LH-QA-<NNN>` | [`LH-QA-001`](../spec/lastenheft.md#lh-qa-001--performance-projektöffnung) |
  | Technische Rahmenbedingung | `REQ-TEC-<NNN>` | [`REQ-TEC-003`](../spec/spezifikation.md#9-technische-rahmenbedingungen-req-tec) |
  | Projektziel | `OBJ-<NNN>` | [`OBJ-002`](../spec/lastenheft.md#3-projektziele) |
  | Benutzerrolle | `ROLE-<NNN>` | `ROLE-002` |
  | Abnahmekriterium | `ACC-<NNN>` | [`ACC-001`](../spec/lastenheft.md#7-abnahmekriterien) |
  | Architektur-Entscheidung | `ADR-<NNNN>` | [`ADR-0001`](../docs/plan/adr/0001-hexagonale-architektur.md) |
  | Carveout | `CO-<NNN>` | `CO-001` |
  | Slice | `slice-<NNN>` | `slice-001` |
  | Welle | `welle-<NN>-<titel>` | `welle-1-mvp` |
  | Konventions-Adaption | `MR-<NNN>` | `MR-002` |

  Die `<BEREICH>`-Kürzel entsprechen den Lastenheft-Modulen:
  `BLD` (Gebäude/Projekt) · `FLR` (Geschosse) · `WAL` (Wände) ·
  `ROM` (Räume) · `DOR` (Türen) · `WIN` (Fenster) · `STR` (Treppen) ·
  `ROF` (Dach) · `SLB` (Decken) · `FND` (Fundament) · `D3` (3D) ·
  `DRW` (Zeichnung) · `MAT` (Material) · `EVL` (Auswertung) ·
  `IO` (Import/Export) · `UI` (Oberfläche) · `PLG` (Plugins).
- **Begründung:** b-cad hat von Anfang an klar abgegrenzte fachliche
  Module mit je eigener Anforderungs-Familie. Ein Bereichskürzel ab
  ID-001 vermeidet die spätere Schema-Migration, vor der das
  Kurs-Beispiel warnt.
- **Hinweis `D3`:** Das 3D-Modul nutzt `D3` (nicht `3D`), weil eine ID
  nicht mit einer Ziffer beginnen soll (stabile Sortierung, sauberes
  Grep).
- **Auflösungs-Trigger:** permanent.

Die `REQ-TEC-<NNN>` sind die technischen Rahmenbedingungen aus dem
Lastenheft-Ursprung (C++20, Qt 6, OpenCascade, CMake, GoogleTest, OTel,
SQLite, Shared-Library-Plugins, Docker DevContainer). Sie sind
**fortschreibbar** und leben daher in
[`spec/spezifikation.md` §Technische Rahmenbedingungen](../spec/spezifikation.md#9-technische-rahmenbedingungen-req-tec),
nicht im vertraglichen Lastenheft; ADRs (z. B. [ADR-0002](../docs/plan/adr/0002-geometrie-kern-opencascade.md)/0003) schärfen
sie.

### MR-003 — docs-check als vendored Doku-Sensor

- **Datum:** 2026-06-08
- **Geltungsbereich:** `tools/`, `Makefile`, [`harness/README.md` §Sensors](README.md#sensors-feedback-gates)
- **Adaption:** b-cad übernimmt `tools/docs-check.js` **unverändert** aus <!-- d-check:ignore (historisch: geloescht mit MR-007) -->
  dem Baseline-Kurs (`ai-harness-course`, `tools/docs-check.js`) als <!-- d-check:ignore (historisch: Kurs-Quelle, dort zum Rest-Sensor geschrumpft) -->
  eigenes, self-contained Werkzeug mit zweckgebundenem `tools/Dockerfile`
  (nur `docs-check`, ohne den `alignment-check` des Kurses). Aufruf über
  `make docs-check` / `make gates` im Container.
- **Begründung:** `docs-check` ist ein **Doku-Sensor** — er prüft
  Artefakte, die im Greenfield-Bootstrap bereits existieren (die ganze
  Spec/Plan-Lieferung). Damit ist er b-cads **erstes reales Gate**,
  ohne Code zu erfordern (Kurs zeigt das mit `verify-closure-notes`,
  Modul 11, und listet `docs-check` als Gate, Modul 13). Eigenes
  Vendoring statt Kurs-Container, weil b-cad self-contained und
  reproduzierbar sein muss (Modul 14; Hard Rule AGENTS.md §2.3) und
  nicht an ein Fremd-Repo koppeln darf.
- **Folgepflicht:** Source-Drift gegen den Kurs beim Update nachziehen
  (Entropy Management).
- **Auflösungs-Trigger:** permanent (solange b-cad Markdown-Doku
  trägt). *(Aufgelöst mit
  [MR-007](#mr-007--auflösung-von-mr-003-docs-check-via-d-check),
  2026-06-12 — die Quelle selbst ist zum Kurs-Rest-Sensor geschrumpft,
  die Kopie war verwaist.)*

### MR-007 — Auflösung von MR-003: docs-check via d-check

- **Datum:** 2026-06-12
- **Geltungsbereich:** `tools/`, `Makefile`,
  [`harness/README.md` §Sensors](README.md#sensors-feedback-gates),
  [`.d-check.yml`](../.d-check.yml)
- **Adaption:** Der vendorte Kurs-Validator (MR-003) ist abgelöst:
  `make docs-check` läuft über das digest-gepinnte Container-Image
  [d-check](https://github.com/pt9912/d-check) (v0.8.0) — weiterhin
  als Build-Stage ohne Bind-Mount (`tools/Dockerfile`: `FROM d-check`
  + `COPY . .` + `RUN`), die Hard Rule bleibt erfüllt. Konfiguration
  deklarativ in [`.d-check.yml`](../.d-check.yml) (Module `links`,
  `anchors`, `codepaths` mit b-cad-eigenen Wurzel-Präfixen — die
  Kurs-Kopie prüfte nur `lab/`/`kurs/`/`tools/`, zwei davon existieren
  hier nicht). Vergleichslauf 2026-06-12: Alt 0 Befunde (45 Dateien),
  d-check 8 echte Mehr-Befunde aus der erweiterten
  Inline-Code-Pfad-Abdeckung (2 Pfad-Fixes, 6 begründete
  `d-check:ignore`-Marker an geplanten/fremden/historischen Zielen),
  final beidseitig 0. Die MR-003-Folgepflicht (Source-Drift nachziehen)
  entfällt — die Kurs-Quelle ist ihrerseits auf d-check migriert und
  zum Modul-Nummern-Rest-Sensor geschrumpft.
- **Begründung:** Eine verwaiste vendorte Kopie ist kein Sensor mehr,
  sondern Drift-Risiko (Entropy Management, Modul 15); ein
  digest-gepinntes Image ist genauso reproduzierbar wie Vendoring
  (Modul 14), ohne Fremd-Repo-Kopplung zur Laufzeit.
- **Auflösungs-Trigger:** permanent (Zielzustand).

### MR-004 — Top-level CHANGELOG.md (Keep a Changelog)

- **Datum:** 2026-06-08
- **Geltungsbereich:** [`CHANGELOG.md`](../CHANGELOG.md)
- **Adaption:** b-cad führt ein repo-weites `CHANGELOG.md` im Format
  [Keep a Changelog](https://keepachangelog.com/en/1.1.0/) mit
  [Semantic Versioning](https://semver.org/). Der Kurs sieht das
  **nicht** vor — er hält Änderungs-Historie verteilt (Spec-§Historie,
  ADR-Geschichte, Slice-Closure-Notizen, `welle-N-results.md`).
- **Begründung:** ausdrückliche Projektentscheidung für ein einzelnes,
  release-orientiertes Überblicks-Artefakt zusätzlich zur verteilten
  Historie.
- **Drift-Disziplin (gegen das Duplikations-Risiko, vor dem der Kurs
  warnt):** `CHANGELOG.md` ist **kuratiert und grobkörnig** — ein
  Eintrag pro Slice/Welle/Release unter `## [Unreleased]`, **keine**
  Spiegelung jedes Commits und **keine** Wiederholung der
  Akzeptanzkriterien. Die feinkörnige, auditierbare Wahrheit bleibt in
  den kanonischen Quellen; das CHANGELOG verweist auf sie, ersetzt sie
  nicht.
- **Auflösungs-Trigger:** permanent.

### MR-005 — Harness-Hook-Härtung (Rückport aus d-check)

- **Datum:** 2026-06-10
- **Geltungsbereich:** [`tools/harness/working-tree-hash.sh`](../tools/harness/working-tree-hash.sh),
  `.claude/`, [`Makefile`](../Makefile) (`gates`-Target)
- **Adaption:** Übernahme der im Schwester-Repo d-check (dortiges
  `MR-005`) entwickelten Härtungen der ursprünglich von b-cad
  stammenden Gate-Nachweis-Mechanik:
  (a) Working-Tree-Hash **inhaltsbasiert** (sha256 über getrackte +
  untracked Dateiinhalte) statt diff-basiert — der Gate-Nachweis gilt
  über Commits hinweg, ein Commit *ohne* Gate-Lauf macht den Stop-Hook
  nicht mehr grün (Restlücke: frischer Klon/gelöschter `.harness`-State
  mit cleanem Tree; CI als Netz).
  (b) PreToolUse-Guard: fail-closed ohne `node`; im Pass-Fall keine
  Ausgabe statt `approve` (das übersprang das Permission-System);
  Prüfung nur an Befehlsposition (keine False-Positives durch
  Commit-Messages/Argumente); rekursive Prüfung von
  `bash/sh -c`-Strings inkl. Flag-Bündeln (`-lc`/`-ec`/`-cx`).
  (c) Stop-Hook: Schleifen-Schutz via `stop_hook_active`.
  (d) `record-gates` läuft im `gates`-**Rezept** statt als letzter
  Prerequisite — unter `make -j` entstand der Nachweis sonst trotz
  roter Gates.
  (e) Hook-Pfade in `.claude/settings.json` über
  `$CLAUDE_PROJECT_DIR` statt cwd-relativ.
- **Begründung:** Review-Befunde aus dem d-check-Bootstrap
  (drei HIGHs: fail-open, Permission-Bypass, `-j`-Race; zwei
  R2/R3-Beobachtungen: Commit-Bypass, Sub-Shell-/Flag-Bündel-Umgehung).
- **Auflösungs-Trigger:** permanent.

### MR-006 — Unabhängiges Plan-Review vor Implementierungs-Start

- **Datum:** 2026-06-12
- **Geltungsbereich:** Planning-Lifecycle
  ([`docs/plan/planning/`](../docs/plan/planning/)),
  [`AGENTS.md` §5](../AGENTS.md) (Workflow-Schritt 5)
- **Adaption:** Vor dem Implementierungs-Start eines Slice wird der
  Slice-Plan einem **unabhängigen Plan-Review** unterzogen (Reviewer
  ≠ Plan-Autor; bei AI-Sessions: getrennter Agent/Session ohne
  Autoren-Kontext). Findings werden vor dem Start eingearbeitet;
  **HIGH-Findings blockieren den Start.** Das Review prüft
  mindestens: (a) Quellen-Konsistenz — IDs, Pfade, Zitate,
  Vertrags-Behauptungen gegen die kanonischen Quellen; (b)
  Plan-Qualität — DoD-Beobachtbarkeit, Schnitt/Sitzungs-Umfang,
  Lösungsfreiheit der Ebenen, reale Sensor-Deckung jeder DoD-Zeile.
- **Begründung:** Drittes Vorkommen der Praxis mit jeweils
  substanziellen Funden (Kurs-Regel: zweimal kategorisieren, dreimal
  Regel; Zähler-Herkunft
  [`welle-1-results.md` §5](../docs/plan/planning/done/welle-1-results.md)):
  slice-009 (HIGH H1 → ADR-Scope + Split), slice-010 (HIGH F1 →
  Lastenheft-Grenzverletzung abgefangen), slice-011 (Review
  2026-06-12: 3 HIGH — Pflicht-Lücke Headless-Strategie, behauptete
  Sensor-Deckung ohne Sensor, [ACC-002](../spec/lastenheft.md#7-abnahmekriterien)-Beleg ohne Erzeugungsweg).
- **Auflösungs-Trigger:** permanent.

### MR-008 — Lastenheft-Schärfung bleibt lösungsfrei

- **Datum:** 2026-06-13
- **Geltungsbereich:** [`../spec/lastenheft.md`](../spec/lastenheft.md)
  (Reifephase-Schärfung Outline → AK),
  [`../spec/spezifikation.md`](../spec/spezifikation.md),
  [`../docs/plan/adr/`](../docs/plan/adr/), [`AGENTS.md` §4](../AGENTS.md)
- **Adaption:** Wenn ein Slice eine Lastenheft-Anforderung von **Outline
  auf AK-Niveau** schärft (Reifephase-Klausel), bleiben die
  Akzeptanzkriterien **lösungsfrei und benutzer-beobachtbar**
  (Given/When/Then über *sichtbare* Ergebnisse; vertragliche
  Wertebereiche/Grenzwerte sind das **Was** und gehören ins Lastenheft).
  Die **Lösungsmechanik** — Algorithmen, Konstruktions-Verfahren, Ports,
  Daten-/Schnitt-Strukturen, Fehler-Code-/`op`-Vokabular, Formeln —
  ist das **Wie** und lebt in
  [`spec/spezifikation.md`](../spec/spezifikation.md) (technisch,
  fortschreibbar) bzw. in ADRs, **nie im Lastenheft-Text.**
- **Abgrenzung zu [MR-001](#mr-001--source-precedence-mit-eigener-spezifikations-schicht):**
  MR-001/Spec-Stratifizierung regelt die *ADR→Spec/Lastenheft*-Richtung
  („ADR darf Spezifikation schärfen, nicht Lastenheft"). MR-008 schärft
  das auf die **Slice-interne Outline→AK-Praxis**: auch ein Slice, der
  das Lastenheft legitim *reift*, schreibt keine Lösung hinein.
- **Begründung:** Viertes+ Vorkommen mit jeweils substanziellen
  Plan-Review-Funden (Kurs-Regel: 2× kategorisieren, 3× Regel — hier
  überfällig): slice-009a (Review-009 M4), slice-010a (Review-010
  F1/F2 — Lastenheft-Grenzverletzung abgefangen), slice-012, slice-013a,
  slice-014a (Plan-Review INFO-1: „überfällig"). Zähler-Herkunft
  [`welle-1v-results.md` §5](../docs/plan/planning/done/welle-1v-results.md).
- **Sensor:** das unabhängige Plan-Review (MR-006, Linse
  Plan-Qualität/Quellen-Konsistenz) prüft die Lösungsfreiheit der
  Lastenheft-AK je Schärfungs-Slice; ein computational Gate existiert
  dafür (noch) nicht.
- **Auflösungs-Trigger:** permanent.

### MR-009 — Geometrielastiges Code-Review vor Welle-Closure

- **Datum:** 2026-06-14
- **Geltungsbereich:** Planning-Lifecycle
  ([`docs/plan/planning/`](../docs/plan/planning/)),
  [`AGENTS.md` §5](../AGENTS.md) (Workflow)
- **Adaption:** Ein **geometrieschwerer Implementierungs-Slice** (neue
  Bauteil-/Solid-Geometrie im Kern oder Geometrie-Adapter) wird **vor der
  Welle-Closure** einem **unabhängigen Code-Review** (Reviewer ≠ Autor;
  getrennter Agent/Session ohne Autoren-Kontext) unterzogen, das die
  **Geometrie-Korrektheit gegen die Spezifikation** prüft — mindestens:
  Orientierung/Winding (Außennormalen), Bündigkeit/Spaltfreiheit benachbarter
  Körper, Höhen-/Maß-Exaktheit, Totalität bei Degeneration. **HIGH-Findings
  blockieren die Closure** und werden vorher behoben. Ergänzend: die AK-Tests
  des Slice sollen **geometrische Invarianten** sondieren (geschlossener-Körper-/
  Bündigkeits-Sonden), nicht nur Bounding-Box/Dreiecks-Anzahl — die Korrektheit
  gehört in die Tests, nicht nur in den Code.
- **Begründung:** Viertes Vorkommen mit je substanziellem Befund (Kurs-Regel:
  2× kategorisieren, 3× Regel — hier überfällig): slice-013b (HIGH: lateraler
  Cutter-Überstand nur in z), slice-014b (HIGH: Dachflächen-Orientierung),
  slice-015b (HIGH: OCC-Cutout-Boolean ungetestet), slice-016b (kein HIGH, aber
  Test-Orakel-Lücken: invertierte Normalen/Stufen-Spalt wären durchgerutscht).
  Durchgängig: **grünes Gate ≠ Geometrie-Korrektheit/Spec-Treue.** Zähler-Herkunft
  [`welle-2-results.md` §5](../docs/plan/planning/done/welle-2-results.md) (#2/#5).
- **Sensor:** das unabhängige Code-Review (inferential feedback); seit slice-016b
  zusätzlich computational gehärtet (geometrische Invarianten-Sonden in den
  AK-Tests, z. B. Divergenzsatz-Summe der Außennormalen).
- **Auflösungs-Trigger:** permanent.

### MR-010 — Lastenheft-Header-Version == oberste §9-Historie-Zeile

- **Datum:** 2026-06-14
- **Geltungsbereich:** [`../spec/lastenheft.md`](../spec/lastenheft.md),
  MR-006-Review-Linse (Quellen-Konsistenz)
- **Adaption:** Schärft ein Slice das Lastenheft (Reifephase-Klausel,
  Outline → AK), **zieht er den Header `**Version:**` auf die Version der neu
  ergänzten §9-Historie-Zeile nach.** Invariante: der Header-`Version:`-Wert ==
  oberste (jüngste) §9-Historie-Zeile.
- **Begründung:** Drittes Vorkommen (3×-Regel): slice-013a/014a/015a ergänzten
  je eine §9-Historie-Zeile (0.1.3/0.1.4/0.1.5), zogen aber den Header **nicht**
  nach — er blieb seit slice-012 auf 0.1.2, während die Historie auf 0.1.5
  wuchs; erst slice-016a fiel auf den Drift und korrigierte (→ 0.1.6). Eine über
  drei Schärfungs-Slices **stille** Inkonsistenz, vom docs-check (d-check prüft
  keine Feld-Gleichheit) nicht gefangen. Zähler-Herkunft
  [`welle-2-results.md` §5](../docs/plan/planning/done/welle-2-results.md) (#6).
- **Sensor:** interim die MR-006-Plan-Review-Linse (Quellen-Konsistenz) prüft den
  Header-Nachzug je Schärfungs-Slice. **Computational Promotion-Ziel:** eine
  d-check-/`make`-Regel „Header-`Version:` == oberste §9-Historie-Zeile" (fängt
  die Klasse fail-closed); bis dahin inferential.
- **Auflösungs-Trigger:** permanent (Konvention); die computational-Sensor-
  Promotion bleibt offener Tooling-Kandidat.
  *(Referenz nachgezogen mit [MR-012](#mr-012--mr-010-invariante-folgt-der-ausgelagerten-lastenheft-historie),
  2026-06-15 — die §9-Historie ist seit slice-018a nach
  [`lastenheft-historie.md`](../spec/lastenheft-historie.md) ausgelagert; die
  Invariante bezieht sich auf deren oberste Zeile.)*

### MR-011 — Referenz-Integritäts-Gate (matrix, ids, spans, hostpaths)

- **Datum:** 2026-06-15
- **Geltungsbereich:** [`.d-check.yml`](../.d-check.yml), `spec/`,
  [`AGENTS.md` §2.7](../AGENTS.md), `docs/plan/planning/done-archive/`,
  [`harness/README.md` §Sensors](README.md#sensors-feedback-gates)
- **Adaption:** Das `docs-check`-Gate (MR-007, d-check ≥ v0.8.0) führt vier
  weitere Module. **`matrix`** und **`ids`** setzen die **Regelwerk-Referenz-
  Richtung** computational durch (Stable Dependencies Principle, §Referenz-
  Richtung: „Spec → ADR existiert im bindenden Text nicht — auch nicht als
  Quellen-Spalte"); **`spans`**/**`hostpaths`** sind Markdown-Span- und
  Host-Pfad-Hygiene (0 Befunde).
  - `matrix`: Klassen `spec-straten` (lastenheft/spezifikation/architecture)
    · `adr` · `slice`; Regeln `spec-straten ↛ adr` **und** `↛ slice`;
    `status.forbidden: [superseded, deprecated]`; Provenance-Ausnahme über
    `exclude-sections` (`Historie` / `8. Historie` / `9. Historie` /
    `Geschichte`). Fängt die **Link**-Form.
  - `ids`: ADR-Muster auf `docs/plan/adr/`, `scope.roots: [spec]`,
    `link-policy: always` — fängt die **nackten + Inline-Code**-Erwähnungen.
    Zusammen erfasst matrix+ids **jede** Spec→ADR-Referenz. Der Voll-Korpus
    (alle 7 ID-Familien über den Live-Korpus) bleibt **slice-018b**.
- **§2.7-Reconciliation:** [`AGENTS.md` §2.7](../AGENTS.md) sagte zuvor
  „`architecture.md` referenziert ADRs" — eine **undeklarierte Inkonsistenz**
  ggü. dem Regelwerk (AGENTS rangiert darunter, [Source Precedence](README.md#source-precedence)).
  §2.7 ist auf die Regelwerk-Richtung **verschärft** (ADR-Provenance nur in
  den ADRs aufwärts + Rand-`## Geschichte`); die ~72 Spec→ADR-Körper-
  Referenzen sind aufgelöst (Körper entfernt, Historie-Tabellen ausgelagert).
  Verschärfung, keine Gate-Lockerung — §2.6 n/a.
- **Historie-Auslagerung (Vise-Auflösung):** in `spec/` ist eine ADR-Referenz
  nur sauber, wenn sie **entfernt** wird (Link → matrix-forbidden; nackt →
  id-unlinked); nur in `exclude-sections` löst ein Link beide. Da b-cads
  Historie-Headings **nummeriert** sind (`## 8. Historie` / `## 9. Historie`)
  und der `exclude-sections`-Wert **wörtlich nach DoD** bleibt
  (`[Historie, Geschichte, Änderungshistorie]`), sind die **Historie-Tabellen
  ausgelagert** nach [`spec/spezifikation-historie.md`](../spec/spezifikation-historie.md)
  und [`spec/lastenheft-historie.md`](../spec/lastenheft-historie.md) —
  derivative Provenance-Dateien **außerhalb der `matrix`-Spec-Straten** (keine
  eigene Anforderung, nicht normativ zitierbar). `exclude-sections` deckt damit
  nur noch `architecture.md ## Geschichte` ab.
- **`done/` (Lifecycle-Endzustand):** ein abgeschlossener Slice-/Spike-Plan wandert
  per reinem `git mv` (AGENTS §2.8) aus `in-progress/` nach `docs/plan/planning/done/`
  — dort **bleibt er im Live-`ids`-Scan** (gate-geprüft), neben den `*-results.md`- und
  acc-002-Artefakten. Das ist der reguläre Endzustand (`open → next → in-progress → done`).
- **`done-archive/` (kein Lifecycle-Schritt, eingefrorener Alt-Bestand):** hält die **früh
  abgeschlossenen Pläne bis `slice-024`**, deren `ids`-Links **nicht rückwirkend** an die seit
  der d-check-Einführung (slice-018a,
  [MR-011](#mr-011--referenz-integritäts-gate-matrix-ids-spans-hostpaths)) verschärfte
  Linkpflicht angepasst werden. `.d-check.yml` nimmt den Pfad per `ids.scope.ignore` aus dem
  Live-`ids`-Scan. **Ab `slice-025`** gehen abgeschlossene Pläne in den Lifecycle-Endzustand
  `done/` (Live-Scan, gate-geprüft) — sie erfüllen die Linkpflicht und brauchen den Freeze nicht.
- **Begründung:** Die d-check-v0.8.0-Hebung stellt `matrix`/`ids`/`spans`/
  `hostpaths` bereit; die Referenz-Richtung war bis dahin nur inferential
  (Plan-Review, MR-006). Computational feedforward (Modul 9/13) schließt die
  Klasse fail-closed.
- **Auflösungs-Trigger:** permanent (Zielzustand); der Voll-Korpus-`ids`
  (slice-018b) ist Folge-Slice.

### MR-012 — MR-010-Invariante folgt der ausgelagerten Lastenheft-Historie

- **Datum:** 2026-06-15
- **Geltungsbereich:** [MR-010](#mr-010--lastenheft-header-version--oberste-9-historie-zeile),
  [`../spec/lastenheft.md`](../spec/lastenheft.md),
  [`../spec/lastenheft-historie.md`](../spec/lastenheft-historie.md)
- **Adaption:** slice-018a (MR-011) hat die Lastenheft-Historie aus `## 9. Historie`
  nach [`lastenheft-historie.md`](../spec/lastenheft-historie.md) ausgelagert — §9
  ist nur noch ein Pointer-Stub. MR-010s Invariante „Header-`Version:` == oberste
  §9-Historie-Zeile" bezieht sich daher ab jetzt auf die **oberste (jüngste) Zeile
  in `lastenheft-historie.md`**. Substanz unverändert (Header == jüngster
  Historie-Eintrag); nur der Fundort der Historie-Tabelle hat sich verschoben.
- **Begründung:** MR-010 ist nach Aufnahme inhaltlich unveränderlich
  ([AGENTS.md §2.5](../AGENTS.md)); die durch MR-011 erzwungene Historie-Auslagerung
  macht MR-010s wörtliche „§9"-Referenz stale. Nachzug per neuem Eintrag (statt
  In-Place-Edit), Lineage-Pointer an MR-010 — analog MR-003 → MR-007.
- **Sensor:** unverändert die MR-006-Plan-Review-Linse (Quellen-Konsistenz); das
  computational Promotion-Ziel aus MR-010 prüft den Header künftig gegen die oberste
  Zeile von `lastenheft-historie.md`.
- **Auflösungs-Trigger:** permanent (erbt MR-010).

### MR-013 — arch-check via a-check

- **Datum:** 2026-07-04
- **Geltungsbereich:** [`tools/arch-check.sh`](../tools/arch-check.sh),
  [`.a-check.yml`](../.a-check.yml), [`a-check.mk`](../a-check.mk),
  [`Makefile`](../Makefile), [`tools/gate-consistency.sh`](../tools/gate-consistency.sh),
  [`harness/README.md` §Sensors](README.md#sensors-feedback-gates),
  [`AGENTS.md` §3](../AGENTS.md)
- **Adaption:** Das **primäre Architektur-Gate** ist vom hand-gerollten
  `tools/arch-check.sh` auf das externe, digest-gepinnte Image **a-check**
  (`ghcr.io/pt9912/a-check`, v0.9.0 `@sha256:0378211f…`) umgestellt — deklarative
  Config in `.a-check.yml`, Gate-Snippet in `a-check.mk` (`include a-check.mk`,
  `make a-check` als `gates`-Member). a-check trägt jetzt: Kern-Reinheit (vormals
  Regel A), laterale Adapter (B), Tech-Kapselung OCC-`.hxx`/`sqlite3`/Qt/
  `dlfcn.h`-**Include** (C/D/E + P1-Include-Teil) — inkl. des `plugins/`-Baums —
  sowie **neu** den Schicht-Kanten-Graph und die **driving/driven-Richtung**, die
  `arch-check.sh` nie prüfte. `tools/arch-check.sh` ist auf den **P-Rest**
  geschrumpft: (P1) das `dlopen`/`dlsym`/`dlclose`-**Funktionsaufruf**-Muster
  (a-check prüft nur Include-/Import-**Kanten**, keine Aufrufe) und (P2) die feine
  Quote-vs-Angle-Import-Allowlist für `plugins/`+`src/plugin_api/` — beides
  strukturell außerhalb dessen, was ein kanten-basierter Prüfer sieht.
- **Direkter Präzedenzfall [MR-007](#mr-007--auflösung-von-mr-003-docs-check-via-d-check):**
  derselbe Formwechsel (hand-gerollter/vendorter Gate → externes digest-gepinntes
  Image + deklarative `.<tool>.yml`-Config) wurde bereits für `docs-check`
  (d-check) als MR-Adaption verankert. **Kein ADR:** es wird **keine**
  Architekturregel gelockert ([`AGENTS.md` §2.6](../AGENTS.md) verlangt ADRs nur
  für Lockerungen) — die Regeln bleiben erhalten und werden **strenger** (Kanten +
  Richtung neu); nur der **Durchsetzungs-Mechanismus** wechselt. Die
  Architektur-ADRs [ADR-0001](../docs/plan/adr/0001-hexagonale-architektur.md)/[ADR-0002](../docs/plan/adr/0002-geometrie-kern-opencascade.md)/[ADR-0003](../docs/plan/adr/0003-persistenz-sqlite.md)/[ADR-0009](../docs/plan/adr/0009-gui-framework-qt6.md)/[ADR-0017](../docs/plan/adr/0017-plugin-api-abi.md)
  bleiben unverändert gültig; ihre Sensors-Bindung wandert von `arch-check` auf
  `a-check` (+ P-Rest). Roadmap-Vorgabe 2026-07-03 („eigener Folge-Slice Muster
  MR-007").
- **Bind-Mount-Abweichung (dokumentiert):** anders als die COPY-Stage-Gates läuft
  a-check per `docker run --network none -v $(CURDIR):/src:ro` — read-only
  Bind-Mount. Reproduzierbarkeit sitzt auf Image-Ebene (`@sha256`,
  [ADR-0004](../docs/plan/adr/0004-toolchain-dependency-pinning.md)-Prinzip, Muster
  d-migrate/d-check), Hermetik via `--network none`; `:ro` verhindert
  Repo-Mutation. a-check ist der **erste Bind-Mount-Member im `gates`-Aggregat** —
  der Makefile-Kopf trägt die Ausnahme.
- **gate-consistency-Include-Awareness:** [`tools/gate-consistency.sh`](../tools/gate-consistency.sh)
  scannt zusätzlich `a-check.mk` (der `a-check:`-Target lebt im Include, nicht im
  `Makefile`), sonst Fehlalarm „halluziniertes Gate".
- **Selbst-Pin-Lag:** `a-check --print-mk` aus dem v0.9.0-Image druckt bauartbedingt
  noch den v0.8.0-Pin; maßgeblich ist das byte-genau eingebettete `a-check.mk`
  (v0.9.0-Digest). Pin-Hebung ist ein bewusster Commit
  ([ADR-0004](../docs/plan/adr/0004-toolchain-dependency-pinning.md)-Prinzip).
- **Folgepflicht:** Source-Drift gegen a-check beim Update nachziehen (Entropy
  Management, Modul 15); der a-check-Version-Hub ist ein bewusster Commit.
- **Auflösungs-Trigger:** permanent (Zielzustand).

### MR-014 — Referenz-Richtungs-Verschärfung: ADR nennt keine Slice (d-check v0.37.1)

- **Datum:** 2026-07-05
- **Geltungsbereich:** [`.d-check.yml`](../.d-check.yml), [`d-check.mk`](../d-check.mk),
  [`Makefile`](../Makefile), `docs/plan/adr/`, `spec/` (Spec-Straten),
  [`harness/README.md` §Sensors](README.md#sensors-feedback-gates), [`AGENTS.md` §3](../AGENTS.md)
- **Adaption:** Das mit [MR-011](#mr-011--referenz-integritäts-gate-matrix-ids-spans-hostpaths)
  verankerte Referenz-Integritäts-Gate (`matrix`) wird um die Kante **`adr → slice`** erweitert:
  ein **ADR-Körper nennt keine `slice`-Kennung** — Stable Dependencies / no-downward (die stabile
  Entscheidungsschicht verweist nicht auf volatile Slices; die Folgepflicht-Zuordnung lebt im
  **mutablen** [ADR-Index](../docs/plan/adr/README.md), nicht im immutablen ADR-Body). Umgesetzt
  über `token: 'slice-\d{3}'` auf der `slice`-Klasse (Klartext-Token-Erkennung, nicht nur
  Markdown-Links) — das schärft zugleich die bestehende Kante `spec-straten → slice` auf
  Klartext-Token. docs-check ist dafür von der `tools/Dockerfile`-FROM-Stage (COPY) auf
  **`d-check.mk`** umgestellt (`include`, netzloser read-only Bind-Mount, `DCHECK_DIGEST`
  **v0.37.1**; Muster `a-check.mk`/slice-030, zweiter Bind-Mount-Member; `tools/Dockerfile`
  entfernt, codepaths-`ignore-refs`-Tombstone). Die `token`/`exempt-paths`/`status-provenance`-
  Features gibt es ab d-check v0.31; Pin-Sprung-Fallout auf b-cads Doku = **0** gemessen.
  - **Grandfathering:** die vor der Verschärfung Accepted-ADRs (0001–0017, immutabel
    [AGENTS.md §2.5](../AGENTS.md)) sind per `exempt-paths`-Globs ganz ausgenommen (sie nennen
    Slices als Verifikations-Zeiger im Körper); ab dem DRW-Grundsatz-ADR (0018) gilt die Disziplin.
  - **Ausnahme-Mittel:** ein legitimer Verifikations-Zeiger auf eine Slice wird per Zeilen-Marker
    `<!-- d-check:status-provenance -->` deklariert (matrix-scoped; die `ids`-Linkpflicht bleibt
    unberührt). Die vorbestehenden Spec-Straten-Slice-Tokens sind so remediert: reine
    Reifephase-Provenance markiert, substanzielle Fachverweise **selbsttragend umformuliert**,
    `architecture.md` slice-frei ([AGENTS.md §2.7](../AGENTS.md)).
- **Lineage:** MR-014 **erweitert** [MR-011](#mr-011--referenz-integritäts-gate-matrix-ids-spans-hostpaths)
  (dort `spec-straten ↛ adr/slice` in Link-Form) um die Klartext-Token-Ebene und die
  `adr ↛ slice`-Kante; der d-check-Live-Pin lebt seit
  [MR-007](#mr-007--auflösung-von-mr-003-docs-check-via-d-check) im `tools/Dockerfile`.
- **Kein ADR (Verschärfung, kein [AGENTS.md §2.6](../AGENTS.md)-Fall):** es wird **keine** Regel
  gelockert — eine neue Matrix-Kante ist eine **Restriktion**, die Regeln werden strenger
  (dieselbe Argumentation wie [MR-011](#mr-011--referenz-integritäts-gate-matrix-ids-spans-hostpaths)/[MR-013](#mr-013--arch-check-via-a-check)).
- **Direkter Präzedenzfall [MR-007](#mr-007--auflösung-von-mr-003-docs-check-via-d-check)/[MR-013](#mr-013--arch-check-via-a-check):**
  externes digest-gepinntes Tool-Image + deklarative `.<tool>.yml`-Config; die Digest-Hebung ist ein
  bewusster Commit mit Begründung ([ADR-0004](../docs/plan/adr/0004-toolchain-dependency-pinning.md)-Prinzip).
- **Auflösungs-Trigger:** permanent (Zielzustand).

### MR-015 — Commit-Traceability-Gate (d-check-Modul `commits`)

- **Datum:** 2026-07-05
- **Geltungsbereich:** [`.d-check.yml`](../.d-check.yml), [`AGENTS.md` §4/§3](../AGENTS.md),
  [`harness/README.md` §Traceability rules + §Sensors](README.md#traceability-rules)
- **Adaption:** Die §4-Traceability-Regel („jeder Commit nennt eine Kennung") wird von Konvention
  zu **Gate**: das d-check-Modul `commits` prüft, dass **jede Commit-Message einer Range** eine
  `id-patterns`-Kennung trägt (`slice-\d{3}` · `ADR-\d{4}` · `MR-\d{3}` · `LH-(FA-…|QA)-\d+`;
  `exempt-pattern` = Merge-/Revert-Betreffe). Lauf über **`make doc-commits RANGE=<base>..<head>`**
  (git-Range) bzw. `--commit-msg` (lokaler `commit-msg`-Hook, opt-in DX) — **CI-only-Sensor**
  (Muster [`make schema-check`](README.md#sensors-feedback-gates)/`io-smoke`), **nicht** in
  `make gates` (git-abhängig; `commits` ist nicht im `modules:`-Set → `docs-check`/`gates`
  unberührt).
  - **Kennungs-Set breiter als der frühere Wortlaut** (nur `LH-/ADR-`): die gelebte Praxis ist
    `slice-`-dominiert. **Steering-/Prozess-Slices sind legitim anforderungsfrei**
    (`lastenheft_refs`/`adr_refs` leer) — `slice-*` ist ihr Traceability-Anker. **Beide**
    Regel-Fundstellen ([`AGENTS.md` §4](../AGENTS.md) **und** `harness/README.md`
    [§Traceability rules](README.md#traceability-rules)) sind **symmetrisch** um `slice-*`/`MR-*`
    + die Gate-Bindung ergänzt — kein Regel↔Gate-Drift.
- **Kein ADR (Verschärfung, kein [AGENTS.md §2.6](../AGENTS.md)-Fall):** eine unenforcte Konvention
  wird enforced (netto strenger); die Kennungs-Verbreiterung bildet die **reale Praxis** ab und
  lockert keine Schwelle. Tool-native Ablösung des `trace-check`-Skript-Musters (a-check dogfoodet
  `commits`); dieselbe d-check.mk-Verteilform wie
  [MR-014](#mr-014--referenz-richtungs-verschärfung-adr-nennt-keine-slice-d-check-v0371).
- **Auflösungs-Trigger:** permanent (Zielzustand); die reale CI-Verdrahtung folgt mit der ersten
  `.github/workflows/`-Datei (heute dokumentarisch in der CI-Befehlsliste, Muster `schema-check`).

### MR-016 — ADR-Immutabilitäts-Gate (d-check-Modul `vcs`)

- **Datum:** 2026-07-05
- **Geltungsbereich:** [`.d-check.yml`](../.d-check.yml), [`AGENTS.md` §2.5/§3](../AGENTS.md),
  [`docs/plan/adr/README.md` §Konventionen](../docs/plan/adr/README.md),
  [`harness/README.md` §Sensors](README.md#sensors-feedback-gates)
- **Adaption:** Die §2.5-Regel („`Accepted`-ADRs immutabel") wird von Konvention zu **Gate**: das
  d-check-Modul `vcs` prüft per git-Diff, dass der **Core** einer immutablen ADR (`immutable-when:
  '^\*\*Status:\*\* Accepted'`, `paths: docs/plan/adr/[0-9]*.md`) über eine Commit-Range unverändert
  bleibt (`core-drift-vcs`); die mutable `## Geschichte`-Provenance zählt nicht zum Core
  (`exclude-sections`). Lauf über **`make doc-immutable RANGE=<base>..<head>`** bzw. `STAGED=1`
  (lokaler pre-commit) — **CI-only-Sensor** (Muster [`make doc-commits`](README.md#sensors-feedback-gates)/`schema-check`),
  **nicht** in `make gates` (git-abhängig; `vcs` ist nicht im `modules:`-Set → `docs-check`/`gates`
  unberührt).
  - **`head-allow` = §2.5-Supersede-Status-Übergang:** erlaubt `Accepted` (unverändert) oder
    `Superseded by ADR-NNNN`, sonst `core-drift-vcs`. **Praxis-Nuance:** b-cad führt den Supersede-/
    Erfüllungsstatus im **mutablen [ADR-Index](../docs/plan/adr/README.md)** (nicht im Body); eine
    reale Supersession ist eine **gekoppelte Operation** (das `matrix.status.forbidden`-Gate flaggt
    Inbound-Links auf ein superseded Ziel).
  - **Zwei Regel-Fundstellen (Review-H1):** [`AGENTS.md` §2.5](../AGENTS.md) **und**
    [`docs/plan/adr/README.md` §Konventionen](../docs/plan/adr/README.md) (Source-Precedence-höher) —
    **beide** tragen den Gate-Bindungs-Hinweis; der ADR-README-Wortlaut wurde mit §2.5 abgeglichen
    („Korrekturen" statt „Schärfungen").
- **Kein ADR (Verschärfung, kein [AGENTS.md §2.6](../AGENTS.md)-Fall):** eine unenforcte Konvention
  wird enforced (netto strenger). Tool-native Ablösung des `adr-check`-Skript-Musters (a-check dogfoodet
  `vcs`); dieselbe d-check.mk-Verteilform wie
  [MR-015](#mr-015--commit-traceability-gate-d-check-modul-commits).
- **`immutable`-Alternative (hermetisch, Marker) erwogen + verworfen:** würde einen `make gates`-Member
  erlauben, verlangt aber 16 sha256-Marker (je Accepted-ADR) + Pflege und modelliert den Supersede-
  Übergang nicht nativ; `vcs` erkennt Accepted-ADRs automatisch über die Status-Zeile. (slice-035)
- **Auflösungs-Trigger:** permanent (Zielzustand); reale CI-Verdrahtung mit der ersten
  `.github/workflows/`-Datei.

## Zusatzklassen-Deklaration für Sensors-Bindung

b-cad nutzt neben den vier kanonischen Bindung-Klassen (ADR · Carveout ·
Schwelle · Reproduzierbarkeit) **eine** Zusatzklasse:

| Klasse | Form | Bedeutung | Beispiel (geplant) |
|---|---|---|---|
| LH-Bindung | `LH-FA-<…>` · `LH-QA-<…>` | **Sensor (Gate *oder* CI-only)** prüft direkt eine Anforderung aus `spec/lastenheft.md` | [`make io-smoke`](README.md#sensors-feedback-gates) (real, CI-only) auf [`LH-FA-IO-001`](../spec/lastenheft.md#lh-fa-io-001--ifc-import)…[`006`](../spec/lastenheft.md#lh-fa-io-006); [`LH-FA-WAL-002`](../spec/lastenheft.md#lh-fa-wal-002--wandstärke-definieren) als Bindung eines künftigen Wandstärken-Grenzwert-Tests |

Die Klasse ist **erstmals real gebunden** durch den **CI-only-Sensor**
[`make io-smoke`](README.md#sensors-feedback-gates) ([LH-FA-IO-001](../spec/lastenheft.md#lh-fa-io-001--ifc-import) …
[LH-FA-IO-006](../spec/lastenheft.md#lh-fa-io-006) — headless Binary-Smoke der IO-Austauschpfade).
**Geltung:** die LH-Bindung gilt für **Sensoren — Gate *oder* CI-only**; `make io-smoke`
ist **nicht** in `make gates` (CI-Befehlsliste, Muster `make schema-check`). Erster
*Gate*-Kandidat dieser Klasse bleibt das geplante `make coverage-gate-critical`
([LH-QA-005](../spec/lastenheft.md#lh-qa-005--crash-recovery)); mit seiner Promotion aus dem
"Nicht behauptet"-Block der Sensors-Tabelle in
[`README.md`](README.md#sensors-feedback-gates) trägt erstmals ein **Gate** diese Bindung.

## Modus-Deklaration pro Sub-Area

b-cad startet als **reines Greenfield**: Doku führt, Code wird daran
gemessen. Die Sub-Areas sind einzeln deklariert, damit (a) die
Granularitäts-Disziplin sichtbar ist und (b) eine künftig nach
Brownfield kippende Sub-Area (z. B. ein importierter Alt-Geometrie-Port)
einen dokumentierten Platz hat. Jede Zeile weist die erfüllten
Inklusions-Achsen aus (Schwelle ≥ 2 von 3: 1 Konventions-Härte ·
2 Inventur-Linie · 3 Struktureller Cluster).

| Sub-Area | Pfad-Cluster | Erfüllte Inklusions-Achsen | Modus |
|---|---|---|---|
| Spec-Schreibung | `spec/` | 1 (AK-Format-Standard, Geometrie-Toleranz-Konvention) · 2 (Spec↔Code abgleichbar) · 3 (`spec/`) → **3/3** | Greenfield |
| Konventionen & Harness-Doku | `harness/` | 1 (Heimat der `MR-NNN`) · 2 (Doku-Konsistenz-Linie) · 3 (`harness/`) → **3/3** | Greenfield |
| Planning-Lifecycle | `docs/plan/` | 1 (Slice-/ADR-/Carveout-Konvention) · 2 (`open`→`done`-Inventur) · 3 (`docs/plan/`) → **3/3** | Greenfield |
| Domänen-Modell + Ports + Services (Hexagon-Kern) | `src/hexagon/` (geplant) | 1 (Pure-Domain-Regel, Port-Schnittstellen-Konvention, kein Adapter-Import) · 2 (Typen/Services↔Spec abgleichbar) · 3 (`src/hexagon/`) → **3/3** | Greenfield |
| Geometrie-Kern-Adapter | `src/adapters/geometry/` (geplant) | 1 (OCC-Adapter-Konvention hinter `GeometryKernelPort`) · 2 (Geometrie-Determinismus-Inventur) · 3 (`src/adapters/geometry/`) → **3/3** | Greenfield |
| Persistenz-Adapter | `src/adapters/persistence/` (geplant) | 1 (SQLite-Schema-/Migrations-Konvention, Atomarität) · 2 (Schema↔Modell-Drift) · 3 (`src/adapters/persistence/`) → **3/3** | Greenfield |
| GUI-Adapter | `src/adapters/ui/` (geplant) | 1 (Qt-Adapter-Konvention; Driving-Ports **und** driven Beobachter-Implementierung — verzeichnislich getrennt in `view/` [driven] + `command/` [driving], slice-029) · 2 (UI↔Port-Richtungs-Inventur) · 3 (`src/adapters/ui/`) → **3/3** | Greenfield |
| Plugin-Host | `src/adapters/plugin/`, `plugins/` (geplant) | 1 (Plugin-API-/Lifecycle-/Sandbox-Konvention) · 2 (ABI-/API-Vertrags-Inventur) · 3 (`src/adapters/plugin/`) → **3/3** | Greenfield |
| Import/Export-Adapter | `src/adapters/io/` (geplant) | 1 (Format-Adapter-Konvention je IFC/DXF/STEP/STL hinter Importer/Exporter-Port) · 2 (Round-Trip-Inventur) · 3 (`src/adapters/io/`) → **3/3** | Greenfield |
| Build & Toolchain | `Makefile`, `tools/`, `CMakeLists.txt`, `src/*/CMakeLists.txt`, `.devcontainer/` (existieren seit slice-001: docs-check- + build-Gate) | 1 (CMake-/Container-/docs-check-Konvention, MR-003, [ADR-0001](../docs/plan/adr/0001-hexagonale-architektur.md)) · 2 (Reproduzierbarkeits-Inventur) · 3 (Build-Datei-Familie) → **3/3** | Greenfield |
| Test-Infrastruktur | `tests/` (geplant) | 1 (GoogleTest-/Determinismus-Konvention) · 2 (Test-ohne-`LH`-ID als Diskrepanz) · 3 (`tests/`) → **3/3** | Greenfield |

**Sub-Area-Aspirantinnen** (< 2 Achsen, bewusst *nicht* als Sub-Area
geführt, bis sie eine eigene Konvention *und* Inventur-Linie tragen):

- `docs/user/` — bislang nur Achse 3; keine eigenständige Inventur-Linie
  (hängt an der Spec). Kippt mit eigenem Doku-Style-Standard + eigener
  Drift-Linie.

**Konvergenz-Hinweis (Greenfield).** Kein Konvergenz-Auftrag nötig — GF
*ist* der Steady-State-Zielmodus. Sollte ein Bestands-Port (z. B.
übernommener Geometrie-Code) eingeführt werden, bekommt er eine eigene
BF-Sub-Area-Zeile mit Graduation-Bedingung.

## Glossar (optional)

| Begriff | Bedeutung in b-cad |
|---|---|
| Gebäudemodell | das durchgängige, parametrische Datenmodell, aus dem 2D- und 3D-Darstellung abgeleitet werden ([OBJ-003](../spec/lastenheft.md#3-projektziele)). |
| Geometrie-Kern | OpenCascade (OCC), hinter einem Port gekapselt — die Domäne kennt nur den Port, nicht OCC. |
| Raum-Autoerkennung | Ableitung geschlossener Raumpolygone aus Wandzügen ([`LH-FA-ROM-001`](../spec/lastenheft.md#lh-fa-rom-001--raum-automatisch-erkennen)). |
| Slice | kleinste lieferbare Einheit eines b-cad-Features mit eigenem Plan und eigener DoD. |
