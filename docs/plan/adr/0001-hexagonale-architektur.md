# ADR-0001: Hexagonale Architektur (Ports & Adapters)

**Status:** Accepted

**Datum:** 2026-06-08

**Autor:** Dietmar Burkard

**Bezug:** OBJ-002, OBJ-003, OBJ-004, OBJ-005, LH-FA-BLD-*, LH-FA-IO-*, LH-FA-PLG-*

---

## Kontext

b-cad hat mehrere externe Abhängigkeiten, die sich über den
Lebenszyklus ändern oder austauschbar bleiben müssen: der
Geometrie-Kern (OpenCascade, ADR-0002), die Persistenz (SQLite,
ADR-0003), das GUI-Framework (Qt) und mehrere Austauschformate
(IFC/DXF/STEP/STL). Gleichzeitig verlangt OBJ-003 ein **durchgängiges
Datenmodell**, aus dem 2D- und 3D-Sicht abgeleitet werden, und OBJ-004
**Erweiterbarkeit durch Plugins**. Das Datenmodell und die
Anwendungslogik dürfen von keiner dieser konkreten Technologien direkt
abhängen.

## Entscheidung

Wir wählen eine **hexagonale Architektur (Ports & Adapters)**.

Der **Kern (Hexagon)** — Domain-Modell, Driving Ports, Driven Ports,
Services — ist framework-frei. Konkrete Technologien (Qt, OpenCascade,
SQLite, Format-Bibliotheken) sind **Adapter** hinter Ports. Ein einziger
**Composition Root** (`main`) verdrahtet sie. Die Abhängigkeitsrichtung
zeigt immer nach innen: Adapter hängen am Kern, nie umgekehrt.

## Verglichene Alternativen

### Option A — Klassische Schichtenarchitektur (UI → Logik → DB)

- Pro: schnell, vertraut.
- Contra: verkabelt die Logik direkt mit OCC/SQLite/Qt; Geometrie-Kern-
  oder Persistenz-Wechsel teuer; GUI und Kern schwer ohne Framework
  testbar.

### Option B — Monolithisch um OpenCascade herum

- Pro: minimaler Adaptions-Overhead, volle OCC-Mächtigkeit überall.
- Contra: OCC-Typen durchdringen das gesamte Programm; OBJ-003
  (eigenes Datenmodell) und OBJ-004 (Plugins gegen stabile API) kaum
  haltbar; Tests brauchen OCC.

### Option C — Hexagonale Architektur (gewählt)

- Pro: Geometrie-/Persistenz-/Format-Wechsel bleibt auf den jeweiligen
  Adapter begrenzt; Kern ohne Qt/OCC/SQLite testbar (Port-Doubles);
  Plugins sind ein weiterer Driving Adapter; Abhängigkeitsrichtung im
  Build erzwingbar.
- Contra: mehr Boilerplate (Ports + Adapter); anfänglicher
  Mehraufwand; Lernkurve „was ist Kern, was ist Adapter".

## Konsequenzen

- Positiv: Ein Wechsel des Geometrie-Kerns berührt nur
  `src/adapters/geometry/`, der Persistenz nur
  `src/adapters/persistence/`. Plugins docken über Driving Ports an.
- Negativ: Jeder neue Use-Case braucht einen Port; jede neue
  Infrastruktur einen Adapter. Über-Abstraktion ist ein Risiko —
  Gegenmittel: pro Port zunächst genau eine Implementierung.
- Folgepflicht: Architekturtest als Fitness Function (siehe unten);
  Verzeichnis- und CMake-Target-Struktur gemäß
  [`spec/architecture.md` §2](../../../spec/architecture.md#2-schichten-und-constraints).

## Fitness Function

| Tooling | Regel | Make-Target (geplant) |
|---|---|---|
| CMake-Target-Trennung | `bcad_hexagon` hat **keine** Abhängigkeit auf `bcad_adapters` oder externe Bibliotheken (Qt/OCC/SQLite). Import aus `adapters/` in `hexagon/` ⇒ Link-Fehler. | `make build` |
| Statischer Architekturtest | kein `#include` aus `adapters/` in `hexagon/`; GUI-Adapter importiert keinen Driven Adapter direkt | `make arch-check` |

Solange kein Makefile existiert (Greenfield-Bootstrap), ist die
Build-Trennung die *strukturelle* Durchsetzung; `make arch-check` wird
im ersten Code-Slice real (Promotion-Trigger, vgl.
[`harness/README.md` §Sensors](../../../harness/README.md#sensors-feedback-gates)).

## Re-Evaluierungs-Trigger

- Wenn der Architekturtest mehr als ~20 explizite Ausnahmen bräuchte.
- Wenn b-cad zu einem Client/Server- oder Mehrbenutzer-Modell migriert
  (aktuell out-of-scope, Lastenheft §6).

## Geschichte

| Datum | Ereignis | Verweis |
|---|---|---|
| 2026-06-08 | Proposed (Greenfield-Bootstrap, aus Architektur-Outline) | Kurs-Modul 2, Schritt 8 |
| 2026-06-08 | Accepted (Bootstrap-Ende: erster ADR akzeptiert) | spec/architecture.md |
