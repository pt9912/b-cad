# Harness-Konventionen — b-cad

Diese Datei deklariert die *repo-lokalen* Strukturregeln von **b-cad**
gegenüber der adoptierten Harnesskonvention (Baseline). Sie ist
konformitätsbringend für *Form*-Fragen, nicht autoritativ über Inhalt;
bei Konflikt mit einer kanonischen Quelle gilt die kanonische Quelle
([Source Precedence](README.md#source-precedence)).

## Purpose

Der Default-Ort für:

- **Adaptionen** ggü. der Baseline (mit Begründung und Auflösungs-Trigger).
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
  | Funktionale Anforderung | `LH-FA-<BEREICH>-<NNN>` | `LH-FA-WAL-002` |
  | Nichtfunktionale Anforderung | `LH-QA-<NNN>` | `LH-QA-001` |
  | Technische Rahmenbedingung | `REQ-TEC-<NNN>` | `REQ-TEC-003` |
  | Projektziel | `OBJ-<NNN>` | `OBJ-002` |
  | Benutzerrolle | `ROLE-<NNN>` | `ROLE-002` |
  | Abnahmekriterium | `ACC-<NNN>` | `ACC-001` |
  | Architektur-Entscheidung | `ADR-<NNNN>` | `ADR-0001` |
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
nicht im vertraglichen Lastenheft; ADRs (z. B. ADR-0002/0003) schärfen
sie.

### MR-003 — docs-check als vendored Doku-Sensor

- **Datum:** 2026-06-08
- **Geltungsbereich:** `tools/`, `Makefile`, [`harness/README.md` §Sensors](README.md#sensors-feedback-gates)
- **Adaption:** b-cad übernimmt `tools/docs-check.js` **unverändert** aus
  dem Baseline-Kurs (`ai-harness-course`, `tools/docs-check.js`) als
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
- **Auflösungs-Trigger:** permanent (solange b-cad Markdown-Doku trägt).

## Zusatzklassen-Deklaration für Sensors-Bindung

b-cad nutzt neben den vier kanonischen Bindung-Klassen (ADR · Carveout ·
Schwelle · Reproduzierbarkeit) **eine** Zusatzklasse:

| Klasse | Form | Bedeutung | Beispiel (geplant) |
|---|---|---|---|
| LH-Bindung | `LH-FA-<…>` · `LH-QA-<…>` | Gate prüft direkt eine Anforderung aus `spec/lastenheft.md` | `LH-QA-003` als Bindung eines künftigen Undo/Redo-Tiefen-Tests; `LH-FA-WAL-002` als Bindung eines Wandstärken-Grenzwert-Tests |

Solange kein Makefile existiert, ist diese Klasse nur *deklariert*, noch
nicht *gebunden* — die Sensors-Tabelle in
[`README.md`](README.md#sensors-feedback-gates) führt die Gates im
"Nicht behauptet"-Block, bis der erste Code-Slice sie real einführt
(Promotion-Trigger, Kurs-Modul 2).

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
| GUI-Adapter | `src/adapters/ui/` (geplant) | 1 (Qt-Adapter-Konvention, nur Driving-Ports) · 2 (UI↔Driving-Port-Inventur) · 3 (`src/adapters/ui/`) → **3/3** | Greenfield |
| Plugin-Host | `src/adapters/plugin/`, `plugins/` (geplant) | 1 (Plugin-API-/Lifecycle-/Sandbox-Konvention) · 2 (ABI-/API-Vertrags-Inventur) · 3 (`src/adapters/plugin/`) → **3/3** | Greenfield |
| Import/Export-Adapter | `src/adapters/io/` (geplant) | 1 (Format-Adapter-Konvention je IFC/DXF/STEP/STL hinter Importer/Exporter-Port) · 2 (Round-Trip-Inventur) · 3 (`src/adapters/io/`) → **3/3** | Greenfield |
| Build & Toolchain | `Makefile`, `tools/`, `CMakeLists.txt`, `src/*/CMakeLists.txt`, `.devcontainer/` (existieren seit slice-001: docs-check- + build-Gate) | 1 (CMake-/Container-/docs-check-Konvention, MR-003, ADR-0001) · 2 (Reproduzierbarkeits-Inventur) · 3 (Build-Datei-Familie) → **3/3** | Greenfield |
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
| Gebäudemodell | das durchgängige, parametrische Datenmodell, aus dem 2D- und 3D-Darstellung abgeleitet werden (OBJ-003). |
| Geometrie-Kern | OpenCascade (OCC), hinter einem Port gekapselt — die Domäne kennt nur den Port, nicht OCC. |
| Raum-Autoerkennung | Ableitung geschlossener Raumpolygone aus Wandzügen (`LH-FA-ROM-001`). |
| Slice | kleinste lieferbare Einheit eines b-cad-Features mit eigenem Plan und eigener DoD. |
