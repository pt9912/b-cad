# ADR-0010: Headless-GL via Xvfb + Mesa/llvmpipe (präzisiert ADR-0009 (f))

**Status:** Accepted

**Datum:** 2026-06-12

**Autor:** Dietmar Burkard (Implementierungs-Befund slice-011b,
ausgearbeitet im AI-Harness-Lauf)

**Bezug:** ADR-0009 (f) (Headless-/Testbarkeits-Strategie — wird im
Mechanik-Detail präzisiert, nicht in der Substanz geändert), ADR-0004
(Toolchain-Pinning), LH-FA-D3-002, ACC-002

---

## Kontext

ADR-0009 (f) legte fest, dass Qt-gebundene Tests und der
ACC-002-Beleg „mit `QT_QPA_PLATFORM=offscreen` im bestehenden
Container" laufen. **Implementierungs-Befund (slice-011b):** Die
Ubuntu-26.04-Qt-offscreen-Plattform trägt **kein OpenGL** — sie ist
ohne EGL-Integration gebaut; `QOpenGLContext::create()` schlägt fehl,
`QOpenGLWidget` meldet „not supported on this platform". Das
Szenen-Surrogat und alle nicht-GL-Tests sind davon unberührt; betroffen
sind nur der GL-Widget-Smoke-Test und der Beleg-Renderweg.

Da ADR-0009 `Accepted` und immutable ist, wird die faktisch falsche
Mechanik-Detailangabe per Supersedes-Präzisierung korrigiert statt
still umgangen (AGENTS §2.5).

## Entscheidung

**Headless-GL läuft über Xvfb (virtueller X-Server) + xcb-QPA +
Mesa/llvmpipe (Software-Rasterizer).** Konkret:

1. `xvfb` wird Toolchain-Paket (deps-Stage; ADR-0004:
   `make versions`-Beleg + `harness/toolchain-versions.txt`
   regeneriert).
2. Die `test`-/`coverage-check`-Stages führen `ctest` unter
   `xvfb-run -a` aus; `make acc-002-beleg` rendert unter
   `xvfb-run -a`.
3. Tests setzen die QPA-Plattform **nicht selbst** — die Wahl gehört
   dem Harness (Dockerfile-Stage), sonst überschriebe ein Test die
   Umgebung (Befund des ersten Fehlversuchs).
4. Alle übrigen (f)-Entscheidungen — Szenen-Surrogat, Beleg-Target
   außerhalb `gates`, Coverage-Umgang — gelten unverändert
   (ADR-0009).

## Verglichene Alternativen

- **offscreen-QPA (ADR-0009-Wortlaut):** trägt kein GL — durch Befund
  widerlegt.
- **EGL-surfaceless/eglfs/headless-Wayland:** scheitert an derselben
  Plugin-Ausstattung bzw. zöge einen Compositor in die Toolchain.
- **QPainter-Software-Fallback im Widget:** zweiter Render-Pfad nur
  für Tests — der Beleg zeigte dann nicht den echten Render-Code
  (Honesty-Verstoß im Abnahme-Artefakt).
- **GL-Smoke streichen:** Beleg-Erzeugungsweg bliebe ungetestet,
  Widget-Code unabgedeckt (Coverage-Risiko aus W2-P16 träte ein).

## Konsequenzen

- Toolchain wächst um `xvfb` (+ X-Server-Abhängigkeiten) — gepinnt
  über den bestehenden apt-Snapshot; Beleg via `make versions`.
- Der GL-Smoke-Test rendert echte Pixel (llvmpipe) — der
  ACC-002-Beleg-Weg ist damit getestet, nicht nur behauptet.
- Kein Render-Pfad-Doppel: Widget-Code ist derselbe im Test, im
  Beleg und am echten Display.

## Fitness Function

| Tooling | Regel | Make-Target |
|---|---|---|
| GL-Smoke (slice-011b) | `grabFramebuffer` liefert Nicht-Hintergrund-Pixel unter Xvfb | `make test` |
| Toolchain-Beleg | `xvfb` erscheint in `make versions` == `harness/toolchain-versions.txt` | `make versions` |

## Re-Evaluierungs-Trigger

- Ubuntu-/Qt-Paket liefert offscreen-QPA mit EGL/GL → zurück auf
  offscreen (Xvfb entfällt), neue Präzisierungs-ADR.
- Wechsel der Render-API (QRhi/Vulkan, ADR-0009-Trigger) → Headless-
  Strategie mitbewerten.

## Geschichte

| Datum | Ereignis | Verweis |
|---|---|---|
| 2026-06-12 | Proposed + Accepted — Implementierungs-Befund slice-011b (offscreen-QPA ohne GL), Xvfb-Probe grün | slice-011b |
