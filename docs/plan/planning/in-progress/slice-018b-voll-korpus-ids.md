---
id: slice-018b
titel: Voll-Korpus-ids — Link-Hygiene aller 7 ID-Familien (Linker-Skript)
status: next
welle: harness-steering
lastenheft_refs: []
adr_refs: []
---

# Slice 018b: Voll-Korpus-`ids` (Link-Hygiene, alle 7 ID-Familien)

**Status:** next. Folge-Slice von [slice-018a](../done-archive/slice-018a-dcheck-gate-mechanik-done-archive.md)
(done). **MR-006-Plan-Review gelaufen**
([Report](../../../reviews/2026-06-15-slice-018b-plan.md) — 2 HIGH + 3 MED + LOW
**eingearbeitet**: D3-Regex behoben, Discovery neu erhoben, Cross-Stratum-Links,
Code-Span-Pflicht, Historie-`exempt-paths`). DoD offen bis `make gates` grün.

**Welle:** `harness-steering` (Quergewerk; setzt 018a fort). Bindung über **MR-006**
(Referenz-Richtung) + **MR-007** (docs-check via d-check) + **MR-011** (Referenz-
Integritäts-Gate) statt LH/ADR — Harness-Sensor-Slice.

**Autor:** Dietmar Burkard. **Datum:** 2026-06-15.

**Bezug:** slice-018a hat `ids` auf `scope.roots: [spec]` + die ADR-Familie
begrenzt; 018b weitet auf den **Voll-Korpus + alle 7 ID-Familien**
(`link-policy: always`), `done-archive/` via `scope.ignore` ausgenommen. Die
nackten/Inline-Code-Kennungen werden **verlinkt** (normativer Korpus) bzw.
**backticked** (Logs) — „gut verlinkt" als gemessenes Gate-Property, generiert von
einem einmaligen Linker-Skript, validiert von d-check.

---

## 1. Discovery-Evidenz (Probe-Läufe + Source, 2026-06-15)

Alle read-only, `.d-check.yml` danach zurückgesetzt:

- **Mengen** (2026-06-15, korrigierte D3-Regex, `done-archive/` via `scope.ignore`
  raus): **Live 718** · Logs (CHANGELOG/reviews/`*-historie.md`) **238** →
  **normativer Korpus 480** (zu verlinken). Je Familie: `LH-FA` 255 (inkl. **36 `D3`**,
  die die alte Regex `[A-Z]+` verfehlte — HIGH-1) · `ADR` 181 · `MR` 96 · `ACC` 57 ·
  `LH-QA` 46 · `OBJ` 33 · `REQ-TEC` 24 · `E-*` 26.
- **`scope.ignore` greift voll** (`done-archive/` → 0). **`exempt-paths` greift nur
  Inline-Code, nicht nackte Prosa** — Source-verifiziert: die Prosa-Prüfung
  (`internal/hexagon/core/ids.go:121` im d-check-Repo) fragt `ExemptPaths` nicht ab,
  nur die `always`-Inline-Code-Prüfung (`:95`). Probe: CHANGELOG 76 → 74.
- **`always` ist fence-aware:** IDs in ` ``` `-Blöcken werden **nicht** geprüft
  (Mini-Test: Inline + nackt flaggten, Block-ID nicht).
- **HTML-Anker werden nicht erkannt:** d-checks `anchors`-Modul prüft Fragmente
  nur gegen **ATX-Heading-Slugs** (`anchors.go` `slugsFor` → `HeadingSlugs`). Ein
  `<a id>` + `…#frag` wäre `anchor-missing`. → keine HTML-Anker, kein Tabellen-Umbau.
- **`ids` verlangt nur ID-im-Linktext** (`ids.go:154` prüft das Link-Ziel **nicht**)
  → der **Datei-Link ist ein immer-gültiger Fallback**; Anker-Präzision ist Komfort,
  vom Gate über `anchors`/`links` separat geprüft.

## 2. Entschiedene Strategie

- **Korpus:** voller Live-Korpus; `ids.scope.ignore: [docs/plan/planning/done-archive]`.
- **Familien-Taxonomie:** `ADR-\d{4}`→`docs/plan/adr/` ·
  `MR-\d{3}`→`harness/conventions.md` ·
  **`LH-(FA-[A-Z][A-Z0-9]*|QA)-\d+`** (D3-fest, HIGH-1)+`ACC-\d+`+`OBJ-\d+`
  →`spec/lastenheft.md` · `REQ-TEC-\d+`+`E-(IO|VAL|GEO)-\d+`→`spec/spezifikation.md`.
- **Cross-Stratum (MED-1):** Ziel ist immer die **kanonische Definitionsdatei der
  Familie**, unabhängig von der Fundstelle — `E-*` in `lastenheft.md` linkt nach
  `spezifikation.md`, `OBJ`/`ACC` in `spezifikation.md`/`architecture.md` nach
  `lastenheft.md` (matrix erlaubt spec→spec). Nur die *eigene* Target-Datei ist
  `inTarget`-frei.
- **Anker (Weg B):** ID mit eigenem ATX-Heading → präziser Heading-Slug; Tabellen-/
  Listen-ID → **umschließendes Kapitel**; ADR → Datei-Link. Kein HTML-Anker, kein
  Tabellen-Umbau.
- **Logs ausgenommen** (`exempt-paths` **plus** IDs zu **Inline-Code** backticken,
  weil exempt-paths nackte Prosa nicht fängt): `CHANGELOG.md`, `docs/reviews/**`,
  `spec/spezifikation-historie.md`, `spec/lastenheft-historie.md`. *(Revidierbar.)*
- **`*-results.md`** bleiben **link-pflichtig** (018a-LOW-1) → Kapitel-/Heading-Anker.
- **Slug exakt nach d-check** (`anchors.go:60` `Slugify`): nur `` ` `` und `*` als
  Markup entfernen, **`_` bleibt**, Heading-Links → Linktext, Duplikat-Suffixe
  `-1/-2`. d-check validiert das Ergebnis.

## 3. Definition of Done

- [ ] **`.d-check.yml`:** `ids` auf alle 7 Familien (Targets §2), `scope.roots: ["."]`
      + `scope.ignore: [docs/plan/planning/done-archive]`, `link-policy: always`;
      `exempt-paths: [CHANGELOG.md, "docs/reviews/**", "spec/*-historie.md"]` je Muster
      (LOW-1: `exempt-paths` befreit nur Inline-Code → Logs **zusätzlich** backticken).
- [ ] **Linker-Skript** `tools/{idlink.py}` (neu; läuft im Container wie docs-check):
      baut ID→(Ziel, Anker) aus den Definitionsdateien (d-check-`Slugify` exakt;
      Definitions-Position = ID hinter Tabellen-Pipe/Bullet, **nicht** Umbruch-Prosa);
      ersetzt die von d-check geflaggten nackten Vorkommen — **zwei Modi:**
      Logs → Inline-Code (backticken), Rest → Markdown-Link.
- [ ] **Normativer Korpus verlinkt** (480 nach Log-Ausschluss): `spec/*` (außer
      Historie), `docs/plan/adr/*`, `harness/*`, `AGENTS.md`, `docs/plan/planning/*`
      (roadmap/README/results), `README.md`, `docs/glossar.md`, `docs/user/*`,
      `tools/README.md`.
- [ ] **Logs backticked + exempt:** `CHANGELOG.md`, `docs/reviews/**`,
      `spec/*-historie.md` — IDs als Inline-Code, `exempt-paths` aktiv → 0 Befunde.
- [ ] **Edge-Cases behandelt:** Bereiche (`LH-FA-EVL-001..006` → `001` verlinkt,
      `..006` bleibt) · Suffixe (`…006.a`, angehängte Begriffe `ACC-002-Beleg`) ·
      Mehrfach-pro-Zeile · **`D3`-Modul**. **MED-3:** unter `link-policy: always` sind
      im normativen Korpus **beide** Formen link-pflichtig (nackt **und** Inline-Code);
      „nur die nackte" gilt **nur** in `exempt-paths`-Dateien. Skript-Verhalten dokumentiert.
- [ ] **`make gates` grün** (docs-check 0 Befunde über alle aktiven Module);
      arch-check/lint/test/coverage unberührt. CHANGELOG (MR-004); Closure-Notiz mit
      **Lerneintrag**.

## 4. Plan (vor Code)

| Datei / Komponente | Änderungs-Art | Begründung |
|---|---|---|
| `tools/{idlink.py}` | neu | einmaliger Linker (Generator, kein Gate) |
| `tools/Dockerfile` | ggf. ändern | idlink-Stage, falls im Container ausgeführt |
| `.d-check.yml` | ändern | 7 Familien, `scope.ignore`, `exempt-paths` |
| `spec/*`, `docs/plan/adr/*`, `harness/*`, `AGENTS.md`, `docs/plan/planning/*`, `README.md`, `docs/glossar.md`, `docs/user/*`, `tools/README.md` | ändern (generiert) | Links setzen |
| `CHANGELOG.md`, `docs/reviews/**`, `spec/*-historie.md` | ändern (generiert) | IDs → Inline-Code |
| `docs/reviews/{2026-06-15-slice-018b-plan.md}` | neu | MR-006-Report |

## 5. Trigger

- slice-018a done; Discovery abgeschlossen (Probe-Läufe + d-check-Source-Lektüre,
  2026-06-15). Anker-Weg B entschieden, Logs wie CHANGELOG.

## 6. Closure-Trigger

- DoD vollständig, `make gates` grün. **Stehende Konvention danach:** in Logs IDs
  als Inline-Code, sonst als Link — vom Gate erzwungen. Optionaler Folge-Punkt:
  MR-010-Formulierung auf die ausgelagerte `lastenheft-historie.md` ziehen.

## 7. Risiken und offene Punkte

- **Sizing (MED-2, entschieden):** **ein** Slice, aber der generierte Diff wird
  **nach Korpus-Bereich Commit-getrennt** (§2.8): (1) `spec/*`+`docs/plan/adr/*`,
  (2) `harness/*`+`AGENTS.md`+Root (`README`/`glossar`), (3) `docs/plan/planning/*`
  (roadmap/README/results)+`docs/user/*`, (4) Logs (backtick). Jeder Commit einzeln
  review-/restore-bar; reviewt wird Skript + Stichproben je Commit, nicht ~480 Einzel-Links.
- **Slug-Exaktheit** ist das Hauptrisiko: weicht der Skript-Slug von d-checks
  `Slugify` ab, flaggt `anchors`. Mitigation: `Slugify` 1:1 portieren + d-check als
  Orakel iterieren.
- **Self-Referenz:** Definitionsdateien (`lastenheft.md` def. `LH-*`) sind im Target
  → Vorkommen dort sind `inTarget`-frei (kein Selbst-Link). Vom Skript zu respektieren.
- **Dauer-Friktion:** ab 018b muss jeder künftige Eintrag im normativen Korpus seine
  IDs verlinken, in Logs backticken — bewusst akzeptiert.

## 8. Sub-Area-Modus-Begründung

### Sub-Area: Harness/Doku-Gates (`tools/`, `.d-check.yml`, gesamter Doku-Korpus)

- **Modus:** GF. Bestands-Doku, Gate auf Voll-Korpus-Link-Hygiene gehoben; der
  Linker erzeugt den Diff, `ids`/`links`/`anchors` validieren jeden Schritt. Risiko
  mittel (Masse), aber maschinell generiert + orakel-geprüft. MR-008 n/a (keine
  AK-/Inhalts-Änderung — reine Referenz-Form).

## 9. Closure-Notiz

*(folgt nach `make gates` grün.)*
