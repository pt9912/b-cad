---
id: slice-028
titel: Reine Berechnungs-Kerne verzeichnislich reklassifizieren — hexagon/services/geometry/ (a-check-Vorbereitung, Pilot-Befund 1)
status: in-progress
welle: harness-steering
lastenheft_refs: []
adr_refs: [[ADR-0001](../../adr/0001-hexagonale-architektur.md), [ADR-0011](../../adr/0011-bauteil-hosting-wandoeffnung.md), [ADR-0017](../../adr/0017-plugin-api-abi.md)]
---

# Slice 028: Reine Berechnungs-Kerne → `src/hexagon/services/{geometry}/` (Pilot-Befund 1)

**Status:** in-progress — [MR-006](../../../../harness/conventions.md#mr-006--unabhängiges-plan-review-vor-implementierungs-start)-Plan-Review
**1 HIGH / 2 MED / 1 LOW / 4 INFO — alle vor Start eingearbeitet**
(HIGH-1: die ursprüngliche Ein-Commit-Begründung war eine unzulässige
§2.8-Regel-Verengung → **Zwei-Commit-Split**; MED-1: codepaths×Freeze-
Konflikt → `d-check:ignore`-Marker, Präzedenz slice-008a; MED-2:
`architecture.md`-„Darf importieren"-Tabellenspalten mit nachziehen —
heilt eine **vorbestehende** Tabellen-Inkonsistenz; LOW-1
Self-Include-Präzisierung; INFO-1/2 Sub-Schicht-Semantik +
`volume_geometry`-Reinheits-Präzisierung). Options-Entscheidung B vom
Review **bestätigt** („B ist richtig; A zu Recht verworfen").
[Report](../../../reviews/2026-07-03-slice-028-plan.md).
**Entwurf zur Abnahme an den Projektinhaber — Implementierung erst nach
Abnahme.**

**Welle:** harness-steering (Quergewerk „a-check-Vorbereitung"; die
Gate-Umstellung selbst — a-check statt `tools/arch-check.sh`, eigener
MR-Eintrag Muster [MR-007](../../../../harness/conventions.md#mr-007--auflösung-von-mr-003-docs-check-via-d-check) —
ist ein **späterer, eigener** Slice und **nicht Teil**; ausdrücklich
**keine** `.a-check.yml` in diesem Slice).

**Bezug / Anlass:** Verifizierter a-check-Pilot-Lauf (Schwester-Tool
v0.8.0, Projektinhaber 2026-07-03) — **Befund 1:** Adapter greifen mit
**11 Includes** in die Application-Service-Schicht:
`src/adapters/geometry/stl_export_adapter.cpp` und
`src/adapters/geometry/step_export_adapter.cpp` inkludieren je
`hexagon/services/`-Header (`opening_geometry`/`roof_geometry`/
`slab_geometry`/`stair_geometry`/`wall_footprint`),
`src/adapters/persistence/sqlite_project_repository.cpp` inkludiert
`hexagon/services/stair_geometry.h` (Befund am Code verifiziert,
2026-07-03). Diese fünf Header sind **reine Berechnungs-Kerne** (nur
`hexagon/model/`-Includes + Standard-Library, totale Funktionen, kein
I/O, keine Ports) — de-facto Domänen-Funktionen in der
Application-Schicht. [ADR-0001](../../adr/0001-hexagonale-architektur.md)
(Schichtung — Regel A–E bleiben unberührt grün),
[ADR-0011](../../adr/0011-bauteil-hosting-wandoeffnung.md) (die
Berechnungs-Kerne entstanden als Bauteil-Geometrie-Muster),
[ADR-0017](../../adr/0017-plugin-api-abi.md) (Symbol-Naht-Prämisse
„model/ ist header-only" — Entscheidungs-Kriterium, s. u.).

**Maßstab (Projektinhaber-Vorgabe):** `tools/arch-check.sh` (Regeln A–E,
P1/P2) bleibt **durchgehend grün**; alle bestehenden Gates grün.
**Abnahme-Kriterium:** kein `src/adapters/**`-Include mehr auf
`hexagon/services/*` **außerhalb** des neu deklarierten Berechnungs-Orts.

**Autor:** Dietmar Burkard. **Datum:** 2026-07-03.

---

## 1. Ziel und Entscheidung

**Entscheidung (Empfehlung dieses Plans): Option B — verzeichnisbasierte
Verschiebung nach `src/hexagon/services/{geometry}/`** (neuer Unterordner;
das künftige Gate schneidet Schichten über **Verzeichnis-Globs**, daher
verzeichnisbasiert, nicht dateinamens-basiert — Projektinhaber-Vorgabe).

**Warum Option B und nicht Option A (`hexagon/model/`):**

1. **`model/` bleibt header-only.** Die fünf Kerne bringen `.cpp`-Dateien
   mit — in `model/` bräche das die dokumentierte **Symbol-Naht-Prämisse**
   aus slice-026b ([ADR-0017](../../adr/0017-plugin-api-abi.md),
   Entscheidung `ENABLE_EXPORTS`: „model/ und ports/driving/ sind
   header-only, ein Plugin braucht zur Ladezeit fast keine Kern-Symbole").
   Option B lässt die Prämisse unberührt wahr.
2. **Spec-Konvention bleibt wahr:** `spezifikation.md` §2.1 deklariert
   `model/` als „pure Werttypen" (Daten). Funktionen dort hinein
   reklassifizierten die Schicht semantisch — Option B hält die
   Model=Daten/Services=Operationen-Trennung.
3. **Die Kante wird gezielt deklarierbar:** „Adapter dürfen zusätzlich
   `hexagon/services/geometry/**` konsumieren (reine, port-freie
   Berechnungs-Kerne)" ist als Glob-Regel exakt formulierbar — für
   arch-check-Nachfolger wie für Leser.
4. **P2 unberührt:** die Plugin-Import-Grenze (nur `plugin_api`/`model`/
   `ports/driving`) bleibt wie sie ist — Plugins erhalten **keinen**
   Zugriff auf die Berechnungs-Kerne (bei Option A bekämen sie ihn
   stillschweigend mit).

**Umfang der Verschiebung (exakt die Pilot-Menge):**
`opening_geometry.{h,cpp}` · `roof_geometry.{h,cpp}` ·
`slab_geometry.{h,cpp}` · `stair_geometry.{h,cpp}` ·
`wall_footprint.{h,cpp}` — verschoben per `git mv` nach
`src/hexagon/services/{geometry}/`. **Nicht verschoben:**
`volume_geometry.{h,cpp}` (**port-frei, aber nicht model-only** —
inkludiert `slab_geometry.h` + `wall_footprint.h` und verfehlte damit
die `geometry/`-Reinheits-Invariante; Review-INFO-2. Nur service-intern
konsumiert — kein Adapter-Include; als Kohärenz-Kandidat in der
Closure-Notiz benannt [ein späterer Umzug erforderte eine erweiterte
Invariante „model/ + Geschwister-Kerne"], kein stilles Mitverschieben),
`room_detection`/`structure_edit_service`/`exchange_service` (echte
Application-Services: Ports, Zustand, Orchestrierung).

**Reinheits-Invariante (neu, prüfbar):** Dateien unter
`services/geometry/` inkludieren nur `hexagon/model/` +
Standard-Library — **keine** Ports, keine anderen Services, kein
Framework. Wird als Erweiterung der arch-check-Prüfung **nicht** in
diesem Slice verdrahtet (das übernimmt die a-check-Richtungs-Modellierung
im Folge-Slice); hier gilt sie als Review-/Text-Invariante.

## 2. Definition of Done

- [ ] **Verschiebung:** die fünf Berechnungs-Kerne per `git mv` nach
      `src/hexagon/services/{geometry}/`; `src/hexagon/CMakeLists.txt`
      Quell-Liste nachgezogen. **Commit-Schnitt nach
      [AGENTS.md §2.8](../../../../AGENTS.md) (Review-HIGH-1): Commit i =
      reiner `git mv` (nur Pfade), Commit ii = Include-/CMake-Nachzüge**
      — §2.8 ist die allgemeine Regel „git mv + Inhaltsänderung = zwei
      Commits" (das Klammer-Beispiel „Slices/Carveouts" ist kein
      Geltungsbereichs-Filter; die ursprüngliche Ein-Commit-Lesart dieses
      Plans war eine unzulässige Regel-Verengung und ist korrigiert).
- [ ] **Alle Konsumenten nachgezogen** (verifizierte Liste, 2026-07-03):
      Services (`structure_edit_service.cpp`, `volume_geometry.cpp`, je
      der **eigene** Self-Include-Pfad der fünf Kern-`.cpp` —
      Review-LOW-1: die Kerne inkludieren einander **nicht**), Adapter
      (`stl_export_adapter.cpp`, `step_export_adapter.cpp`,
      `sqlite_project_repository.cpp`), Tests
      (`test_roof_geometry`/`test_slab_geometry`/`test_stair_geometry`/
      `test_wall_footprint`/`test_openings`/`test_occ_geometry_adapter`)
      sowie **Kommentar-Pfadverweise** in
      `src/hexagon/ports/driven/geometry_kernel_port.h`,
      `src/hexagon/model/cut_prism.h`, `src/hexagon/model/slab.h`.
- [ ] **Abnahme-Kriterium erfüllt und belegt:**
      `grep -rn "hexagon/services/" src/adapters` liefert **nur noch**
      `hexagon/services/geometry/`-Treffer (Beleg in der Closure-Notiz);
      Reinheits-Invariante der `geometry/`-Dateien per grep belegt (nur
      `hexagon/model/`-Includes).
- [ ] **Verhaltens-Neutralität:** reine Verschiebung — **kein**
      Signatur-/Logik-Touch; `make test` unverändert grün (228/228; die
      Geometrie-AK-Tests inkl. Invarianten-Sonden sind das Orakel);
      [MR-009](../../../../harness/conventions.md#mr-009--geometrielastiges-code-review-vor-welle-closure)
      **n/a** (keine neue/geänderte Geometrie), unabhängiges Code-Review
      vor Closure (Move-Vollständigkeit, keine Alt-Pfad-Reste).
- [ ] **Spec-first / Doku (Review-MED-2 + INFO-1 präzisiert):**
      `spec/architecture.md` — (a) `services/geometry/` als **eigene
      Sub-Schicht** deklariert (reine, port-freie Berechnungs-Funktionen
      über Modelltypen — abweichende Semantik zur Eltern-`services/`-Zeile
      „implementiert Driving Ports, nutzt Driven Ports"); (b) die
      **„Darf importieren"-Spalten** der Geometrie-/Persistenz-Adapter-
      Zeilen um `services/geometry/**` erweitert — heute steht dort
      „model, ports/driven", was die 11 Bestands-Includes **bereits
      verletzen**: dieser Slice heilt die vorbestehende
      Tabellen-Inkonsistenz mit (positiver Nebeneffekt, im Closure
      ausweisen). Meilensteinfrei, ohne ADR-Verweis im Körper
      ([MR-011](../../../../harness/conventions.md#mr-011--referenz-integritäts-gate-matrix-ids-spans-hostpaths)).
      Kein Lastenheft-Touch, kein ADR, kein Schema.
- [ ] **done-archive-/reviews-codepaths-Nachzug (Review-MED-1):** das
      codepaths-Gate kennt — anders als das ids-Modul — **keine**
      done-archive-/reviews-Ausnahme; nach dem Move dangeln mindestens
      der gerootete Inline-Pfad in
      `done-archive/slice-023b-dach-volumen-impl.md` (Alt-Pfad
      `…/services/roof_geometry.cpp` vor dem Move) und der
      `git show`-Pfad in `docs/reviews/2026-06-19-slice-024b-code-review.md`.
      Behandlung: **`d-check:ignore`-Marker** an den Fundstellen
      (Präzedenz slice-008a; bewahrt den historischen Text — benannte,
      minimale Ausnahme vom Freeze; **keine** `.d-check.yml`-Scope-
      Änderung, die wäre eine Gate-Verengung). Vollständiger Sweep beim
      Umsetzen (weitere Treffer gleich behandeln).
- [ ] `tools/arch-check.sh` **durchgehend grün** (Regeln A–E, P1/P2 —
      die Verschiebung bleibt innerhalb `src/hexagon/`, keine Regel
      berührt); `make gates` grün; **keine** `.a-check.yml`;
      CHANGELOG; Roadmap-Quergewerk-Eintrag; Closure-Notiz mit
      Grep-Beleg + Lerneintrag.

## 3. Plan (vor Code)

| Datei / Komponente | Änderungs-Art | Begründung |
|---|---|---|
| `src/hexagon/services/{geometry}/` (5× `.h`+`.cpp` via `git mv`) | neu (Move) | Berechnungs-Kern-Ort, verzeichnisbasiert (Glob-fähig) |
| `src/hexagon/CMakeLists.txt` | ändern | Quell-Pfade der fünf `.cpp` |
| `src/hexagon/services/*.cpp` · `src/adapters/{geometry,persistence}/*.cpp` · `tests/**` | ändern | Include-Pfade der Konsumenten |
| `src/hexagon/ports/driven/geometry_kernel_port.h` · `src/hexagon/model/{cut_prism,slab}.h` | ändern | Kommentar-Pfadverweise |
| `spec/architecture.md` | ändern | Sub-Schicht-Deklaration `services/geometry/` + „Darf importieren"-Spalten der Adapter-Zeilen (Spec-first; heilt vorbestehende Tabellen-Inkonsistenz) |
| `docs/plan/planning/done-archive/slice-023b-*.md` · `docs/reviews/2026-06-19-slice-024b-code-review.md` | ändern (minimal) | `d-check:ignore`-Marker an gerooteten Alt-Pfaden (Review-MED-1, Präzedenz slice-008a) |
| `docs/plan/planning/in-progress/roadmap.md` | ändern | Quergewerk-Eintrag „Historische Trigger-Verschiebungen" (gemeinsam mit slice-029) |
| `CHANGELOG.md` | ändern (Closure) | Unreleased-Eintrag |
| `docs/reviews/{2026-07-03-slice-028-plan,2026-07-03-slice-028-code-review}.md` | neu | [MR-006](../../../../harness/conventions.md#mr-006--unabhängiges-plan-review-vor-implementierungs-start)-Report + Code-Review |

## 4. Trigger

- [MR-006](../../../../harness/conventions.md#mr-006--unabhängiges-plan-review-vor-implementierungs-start)-Review
  **und** Projektinhaber-Abnahme des Entwurfs (ausdrücklicher Auftrag
  „Entwurf zur Abnahme") — erst dann Implementierung.

## 5. Closure-Trigger

- DoD vollständig, `make gates` grün, Code-Review 0 HIGH, Closure-Notiz →
  zusammen mit slice-029 **Meldung an den Projektinhaber** („Umbauten
  durch") für den a-check-Pilot-Schnitt mit voller Richtungs-Modellierung
  (a-check-seitig dort slice-024/adapterSeg-Root-Fix).

## 6. Risiken und offene Punkte

- **Zwei-Commit-Split und Gate-Disziplin (Review-HIGH-1-Auflösung):**
  Commit i (reiner `git mv`) kompiliert **nicht** eigenständig — das ist
  akzeptiert: `make gates` ist die **Handoff**-Pflicht (AGENTS §5.7,
  „vor Handoff"), keine Pro-Commit-Pflicht; der Gate-Lauf erfolgt nach
  Commit ii auf dem Gesamtstand (dasselbe Muster wie jeder
  Lifecycle-Move + Nachzug). Die Rename-Detection ist mit dem Split
  maximal sauber (Commit i = 100 % Rename).
- **Doku-Pfad-Reste:** ältere Spec-/Historie-Texte nennen
  `roof_geometry`/`slab_geometry` als Kurz-Bezeichner (keine gerooteten
  Pfade → codepaths-neutral); volle `src/hexagon/services/…`-Pfade in
  Dokus fängt das codepaths-Gate. Kommentar-Verweise im Code fängt kein
  Gate — daher die explizite DoD-Konsumenten-Liste (verifiziert per grep).
- **Kohärenz-Rest `volume_geometry`:** bleibt bewusst in `services/`
  (kein Adapter-Konsum). Falls die a-check-Richtungs-Modellierung später
  eine strengere „Services sind adapter-unsichtbar"-Regel zieht, ist
  `volume_geometry` davon nicht betroffen; ein Umzug aus Kohärenz-Gründen
  wäre ein eigener kleiner Folge-Move (benannt, kein Scope-Creep).
- **`slab.h`-Kommentar-Kuriosum:** `model/slab.h` verweist im Kommentar
  auf `services/slab_geometry.h` — nach dem Move auf
  `services/geometry/slab_geometry.h`; reine Kommentar-Wahrheit, kein
  Include (model/ bleibt include-frei Richtung services — Regel-A-Logik
  unberührt).
- **Kein Über-Versprechen an a-check:** dieser Slice implementiert
  **keine** neue Gate-Regel (die Reinheits-Invariante wird erst mit der
  a-check-Modellierung computational); `tools/arch-check.sh` bleibt
  unverändert das geltende Gate.

## 7. Sub-Area-Modus-Begründung

### Sub-Areas: Hexagon-Kern · Geometrie-/Persistenz-Adapter · Test-Infrastruktur

- **Modus:** je GF (deklariert in
  [`harness/conventions.md`](../../../../harness/conventions.md)
  §Modus-Deklaration). Reine Struktur-Verschiebung + Include-Nachzug,
  verhaltensneutral (Orakel: unveränderte AK-/Invarianten-Tests);
  Konventions-Dichte hoch (Schichtungs-Hard-Rules), genau deshalb der
  Verzeichnis-Schnitt statt Datei-Konvention.

### Sub-Area: Spec-Schreibung

- **Modus:** GF; `architecture.md`-Deklaration ist die Spec-first-Hälfte
  (Komponenten-Sicht, meilensteinfrei, ADR-frei im Körper).
