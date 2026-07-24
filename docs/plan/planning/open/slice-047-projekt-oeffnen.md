---
id: slice-047
titel: Projekt aus SQLite öffnen (Laden verdrahten) — Projekt-Laden, macht Export-Quelle real
status: open
welle: welle-5-erweiterung
lastenheft_refs: [[LH-FA-BLD-003](../../../../spec/lastenheft.md#lh-fa-bld-003--projekt-laden)]
adr_refs: [[ADR-0003](../../adr/0003-persistenz-sqlite.md), [ADR-0006](../../adr/0006-relationales-schema-design.md)]
---

# Slice 047: Projekt aus SQLite öffnen (Laden verdrahten)

**Status:** open — **Skelett** ([MR-020](../../../../harness/conventions.md#mr-020--adr-folgepflicht-sichtbarkeit-closure-disziplin)(3):
Scope-Reservierung + Vorgänger-Verweis, klar als Skelett markiert; **[MR-006](../../../../harness/conventions.md#mr-006--unabhängiges-plan-review-vor-implementierungs-start)-Plan-Review
+ Detail-Schnitt folgen beim Start**).

**Welle:** welle-5-erweiterung. **Anlass:** das [MR-006](../../../../harness/conventions.md#mr-006--unabhängiges-plan-review-vor-implementierungs-start)-Review
von [`slice-046`](../done/slice-046a-export-provenance.md) deckte auf: der `SqliteProjectRepository` (Save/Load) existiert im
Adapter, ist aber **nirgends in `main.cpp` verdrahtet** — b-cad kann **kein gespeichertes Projekt öffnen** (jeder
Export/GUI-Start baut die in-memory [ACC-001](../../../../spec/lastenheft.md#7-abnahmekriterien)-Demo). Das ist eine
**echte Anforderungs-Lücke** ([LH-FA-BLD-003](../../../../spec/lastenheft.md#lh-fa-bld-003--projekt-laden) „Projekt
laden").

## 1. Ziel (Skelett)

Den vorhandenen SQLite-**Load** in den Composition-Root verdrahten: **`--open <pfad.bcad>`** (CLI) — optional GUI
„Datei → Öffnen" —, sodass das geladene Modell der exportierte/bearbeitete Zustand wird. **Zugleich** den geladenen
**Dateipfad (Basename)** verfügbar machen, damit der Export-Provenance-Vertrag aus
[`slice-046`](../done/slice-046a-export-provenance.md) die **„Quelle" real** füllt (statt `(ungespeichert)`).

## 2. Reservierter Scope / bekannte Prüfsteine (beim Start schärfen)

- **`main.cpp` verdrahtet `SqliteProjectRepository::load`** (existiert) hinter einem `--open <pfad>`-Flag; das
  Ergebnis-`Building` ersetzt die Demo als Export-/Bearbeitungs-Modell.
- **Atomarität/Fehler:** nicht-existente/defekte/schema-fremde Datei → definierter Fehler (kein Crash, kein
  Teil-Zustand); Muster der bestehenden `E-IO`-Fehlercodes.
- **Geladenen Basename tracken** → in den [`slice-046`](../done/slice-046a-export-provenance.md)-`ExportProvenance` reichen
  (macht dessen `(ungespeichert)`-Fallback zur Ausnahme). **Reihenfolge:** nach 046 (der Provenance-Vertrag muss
  stehen); die 046-Quelle-Mechanik ist bereits fallback-fähig gebaut.
- **AK:** ein gespeichertes Projekt öffnen → Modell entspricht dem gespeicherten Stand (Geschosse/Wände/…);
  anschließender Export trägt die **echte Quelle** (Basename); Roundtrip Save→Open erhält den Stand.

## 3. Trigger

- [`slice-046`](../done/slice-046a-export-provenance.md) (Provenance-Vertrag) done — der `source`-Eingang steht.
- **[MR-006](../../../../harness/conventions.md#mr-006--unabhängiges-plan-review-vor-implementierungs-start)-Plan-Review vor Start** (Reviewer ≠ Autor), HIGH blockiert.
