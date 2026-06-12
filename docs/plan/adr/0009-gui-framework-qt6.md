# ADR-0009: GUI-Framework-Bindung Qt 6 (Driving Adapter) — Widgets, Tessellation über Port, Regel E

**Status:** Accepted

**Datum:** 2026-06-12

**Autor:** Dietmar Burkard (Entscheidungen (a)/(b) explizit gewählt;
ausgearbeitet im AI-Harness-Lauf slice-011a)

**Bezug:** REQ-TEC-002 (Qt 6 gesetzt), ACC-002, LH-FA-D3-001/002
(sichtbare Hälfte), OBJ-003, ADR-0001 (Adapter-Grenzen), ADR-0002
(OCC nur als Port-Backend), ADR-0008 (Observer-Port,
Push-Notify/Pull-State)

---

## Kontext

`welle-1v-viewer` liefert die sichtbare Hälfte des Echtzeit-Vertrags:
ein 3D-Fenster, das das extrudierte Gebäudemodell darstellt und
committeten Änderungen ohne Benutzer-Refresh folgt (ACC-002,
LH-FA-D3-002). REQ-TEC-002 setzt Qt 6 als GUI-Framework; offen war
die **Bindungsform**: UI-Technologie, Weg der Geometrie ins Fenster,
Sensor-Durchsetzung der Qt-Grenze, Threading/Observer-Verdrahtung,
Composition-Root-Anbindung und die Headless-/Testbarkeits-Strategie
(Entscheidungs-Pflichten (a)–(f) aus slice-011a, geschärft durch die
Plan-Reviews [R1/W2](../../reviews/2026-06-12-slice-011-plan.md)).

Bindende Vorgaben (hier *nicht* neu entschieden): Qt nur im
UI-Adapter, Kern framework-frei (ADR-0001, `architecture.md` §2);
GUI ruft OCC/SQLite nie direkt (arch-check Regel B/C/D); der
Benachrichtigungs-Vertrag ist ADR-0008.

## Entscheidung

1. **(a) UI-Technologie: Qt Widgets.** Hauptfenster und 3D-Viewport
   als klassische C++-Widgets (`QMainWindow`,
   `QOpenGLWidget`/`QRhiWidget`-Familie). Kein QML/Qt Quick: eine
   Sprachebene, lint-/gate-freundlich, headless via offscreen-QPA
   erprobt, Desktop-Docking-Ökosystem stärker.
2. **(b) Geometrie-Weg: Tessellation über Port — kein OCC in der
   GUI.** Der Kern liefert der Darstellung **framework-freie
   Dreiecksnetze** (pure Werttypen: Vertices, Normalen, Indizes je
   `element_id`) über den **`ViewModelPort` (driving)**; intern
   erzeugt der Geometrie-Adapter die Netze per OCC-Tessellation
   hinter dem `GeometryKernelPort` (driven). Damit bleiben
   `architecture.md` §2 (GUI-OCC-Verbot), Regel B (kein
   Adapter→Adapter) und Regel C (`*.hxx` nur `adapters/geometry/`)
   **unverändert gültig** — keine Constraint-Revision. Das UI
   rendert die Netze mit Qt-Bordmitteln (OpenGL/QRhi; Kamera/Shading
   in welle-1v minimal: orbitierbare Perspektive, flat/Phong).
3. **(c) Sensor-Form: arch-check Regel E.** Qt-Includes
   (`<Q…>`/`Qt…`-Header) sind nur unter `src/adapters/ui/` und in
   der Composition Root `src/main.cpp` zulässig. Eine
   Plugin-Ausnahme entsteht erst mit dem Plugin-System (LH-FA-PLG-*,
   Re-Evaluierungs-Trigger). Umsetzung als **Folgepflicht in
   slice-011b** (Muster Regel C/slice-003b).
4. **(d) Threading und Observer-Verdrahtung: single-threaded am
   Qt-Event-Loop.** Alle Kern-Aufrufe (Mutationen über Driving
   Ports, Pull-Queries) laufen im UI-Thread; der synchrone
   ADR-0008-Callback erreicht den Viewer damit im selben Thread.
   Der Viewer-Callback **pullt nur und plant ein Repaint**
   (`QWidget::update()` ist queued) — er löst nie Mutationen aus;
   das ADR-0008-Re-Entranz-Verbot ist damit strukturell eingehalten,
   nicht nur vertraglich. **Import-Regel:** Der UI-Adapter darf
   `ports/driven/` importieren, **ausschließlich um
   Beobachter-Schnittstellen zu implementieren** (`ModelChangedPort`)
   — `architecture.md` §2 (Import-Spalte GUI-Adapter) und §1.2
   (Port-Tabelle) werden entsprechend nachgezogen; das ist eine
   Präzisierung der Tabelle, keine Richtungs-Umkehr (die Abhängigkeit
   zeigt weiter nach innen auf eine Kern-Schnittstelle).
5. **(e) Composition Root:** `src/main.cpp` erzeugt `QApplication`,
   Kern-Services und Adapter, injiziert die Driving-Port-Referenzen
   (`EditStructurePort`, `DetectRoomsPort`, `ViewModelPort`, …) in
   den Viewer (Konstruktor-Injektion) und verdrahtet den
   ADR-0008-Lebenszyklus: `subscribe` nach Konstruktion,
   `unsubscribe` vor Zerstörung; `main` besitzt den Viewer, der
   Service hält nur die nicht-besitzende Beobachter-Referenz
   (ADR-0008 #5).
6. **(f) Headless-/Testbarkeits-Strategie:**
   - **„Dargestellt"-Surrogat:** der **Szenen-Zustand des Viewers**
     (die gehaltenen Netze je `element_id` + Zähler wirksamer
     Szenen-Updates) — pure Daten, ohne Display prüfbar. AK-Tests
     treiben den echten Kern, lassen den Viewer-Szenen-Teil
     mithören und prüfen den Szenen-Endzustand (Happy/Boundary/
     Negative inkl. Idempotenz bei Mehrfach-Meldung).
   - **Qt-gebundene Tests** laufen mit `QT_QPA_PLATFORM=offscreen`
     im bestehenden Container; ein eigener e2e-GUI-Treiber ist in
     welle-1v **nicht** nötig (`tests/e2e/` bleibt leer —
     Begründung: das Surrogat plus offscreen deckt die AK; ein
     Treiber wird mit Interaktion/Selektion relevant).
   - **ACC-002-Beleg:** make-Target `acc-002-beleg` rendert offscreen
     ein ACC-001-Kern-Projekt und schreibt
     `docs/plan/planning/done/acc-002-beleg.png` + Begleit-`.md`
     (Projekt, Soll-Merkmale, Kommando, Commit-Hash, Datum,
     Abnahme-Satz). **Manueller Abnahme-Schritt des Projektinhabers,
     kein Gate** — das Target wird nicht in `make gates` aggregiert
     und erhält keinen Gate-Status (gate-consistency-konform
     dokumentiert).
   - **Coverage-Umgang:** Szenen-/Adapter-Logik ist über die
     offscreen-Tests normal abdeckbar; dünn bleibt nur der
     GL-Draw-Pfad. Erwartung: Gesamt-Coverage bleibt über der
     70-%-Schwelle (Ist 93,8 % — Puffer). Sollte das reißen, ist
     der legale Weg eine **ADR + Carveout** (AGENTS §2.6), keine
     stille Filter-Anpassung.

## Verglichene Alternativen

### Zu (a): Qt Quick / QML

- Pro: deklaratives UI, GPU-Compositing, Touch/Animation.
- Contra: zweite Sprachebene (QML) ohne Lint-/Gate-Deckung im Repo,
  C++/QML-Grenze als neue Fehlerklasse, Docking/komplexe
  Desktop-Widgets schwächer, headless-Tests aufwendiger. Für ein
  Desktop-Planungswerkzeug ohne Touch-Anforderung kein Gegenwert.

### Zu (b): OCC-Visualisierung (AIS/V3d) im Qt-Fenster

- Pro: Shading, Selektion, Highlight fertig; CAD-üblich.
- Contra: UI-Adapter müsste OCC-Typen sehen und `TopoDS_Shape`s aus
  dem Geometrie-Adapter beziehen — Revision von `architecture.md` §2
  (GUI-OCC-Verbot), Regel B (Adapter→Adapter) und Regel C in einem
  Schritt; headless-Tests und Coverage deutlich schwerer; das
  „dargestellt"-Surrogat müsste aus AIS-Interna rekonstruiert
  werden. Für welle-1v (Darstellung ohne Selektion) unverhältnismäßig
  — als Re-Evaluierungs-Trigger geführt statt jetzt erkauft.

### Zu (d): Worker-Thread fürs Rendern / Queued-Observer

- Pro: Mutationspfad nie durch Repaint gebremst.
- Contra: welle-1v ist single-threaded (ADR-0008 Option C bewusst
  verworfen); `QWidget::update()` entkoppelt das Repaint bereits vom
  Callback. Nebenläufigkeit bleibt ADR-0008-Trigger.

## Konsequenzen

- Positiv: keine einzige bestehende Architektur-Regel wird
  revidiert; das Headless-Surrogat fällt aus der
  Tessellation-Entscheidung natürlich ab; AK-Tests bleiben
  Port-/Werttyp-Tests wie im restlichen Repo.
- `ViewModelPort` wird der erste in Code gegossene Driving Port der
  Darstellungs-Seite (Netz-Query je Element/Geschoss); der
  `GeometryKernelPort` wächst um eine Tessellations-Query
  (Welle-1v-Umfang: Dreiecksnetz je Solid).
- Folgepflichten (slice-011b): arch-check **Regel E**;
  Gate-Doku-Nachzug (`harness/README.md` §Sensors, `AGENTS.md` §3);
  `acc-002-beleg`-Target außerhalb `gates`.
- Kamera/Shading sind selbst gebaut und bewusst minimal — jede
  Erweiterung (Selektion, Schnitt-Ansichten LH-FA-D3-004..006)
  läuft über den Re-Evaluierungs-Trigger, nicht über stilles
  Wachstum im Adapter.

## Fitness Function

| Tooling | Regel | Make-Target |
|---|---|---|
| arch-check **Regel E** (slice-011b) | Qt-Includes nur `src/adapters/ui/` + `src/main.cpp` | `make arch-check` |
| AK-Tests (slice-011b) | Szenen-Surrogat folgt committeten Mutationen (Happy/Boundary inkl. Idempotenz/Negative), display-frei | `make test` |
| Beleg-Target | rendert offscreen, schreibt Beleg-Artefakte; **kein Gate**, nicht in `gates` | `make acc-002-beleg` |

## Re-Evaluierungs-Trigger

- **Selektion/Picking im Viewport** (Bauteile anklicken, welle-2
  Türen/Fenster platzieren) → AIS/V3d (Alternative zu (b)) neu
  bewerten — dann inkl. der nötigen Constraint-Revision als
  Supersedes-ADR.
- **Touch-/Fluid-UI-Anforderung** (neue LH-FA-UI-*) → (a) Qt Quick
  neu bewerten.
- **Mehr-Fenster/Nebenläufigkeit** (LH-FA-UI-004) → zusammen mit
  ADR-0008 Option C (Queue) neu bewerten.
- **Plugin-System** (LH-FA-PLG-*) → Plugin-Ausnahme der Regel E
  definieren (Sandbox-Grenze).
- **Render-/Latenz-Budget wird Anforderung** (neue `LH-QA-<NNN>`)
  → Tessellations-Granularität und Repaint-Pfad gegen Budget messen.

## Geschichte

| Datum | Ereignis | Verweis |
|---|---|---|
| 2026-06-12 | Proposed (Entscheidungs-Pflichten a–f aus slice-011a, geschärft durch Plan-Reviews R1/W2) | slice-011a |
| 2026-06-12 | Accepted — (a)/(b) explizit durch Projektinhaber gewählt; spez. §1 + architecture.md nachgezogen | slice-011a |
