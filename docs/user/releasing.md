# Releasing — b-cad

**Status:** Outline (Phase 2). **Letzte Änderung:** 2026-06-08.

Diese Datei trägt die Release-Strategie von b-cad. Im
Greenfield-Bootstrap ist sie als Outline angelegt (Kurs-Modul 2,
Schritt 5) und wird konkretisiert, sobald die Toolchain (CMake, Docker
DevContainer) und der erste Welle-Closure existieren.

## Release-Einheit

Ein Release von b-cad ist ein **versioniertes Desktop-Artefakt** je
Zielplattform, gebaut aus einem reproduzierbaren Container
(Docker DevContainer, REQ-TEC-009 / ADR-Folge). Versionsschema:
`Major.Minor.Patch`, gekoppelt an erreichte Meilensteine der
[Roadmap](../plan/planning/in-progress/roadmap.md).

## Release-Trigger

Ein Release wird vorbereitet, wenn eine Welle ihren Closure-Trigger
erfüllt hat (siehe Roadmap) und `make fullbuild` (geplant) grün ist.

## Release-Checkliste (Outline — wird in Phase 5 ausgebaut)

- [ ] Alle Slices der Welle in `done/` mit Closure-Notiz.
- [ ] `make gates` und `make fullbuild` grün.
- [ ] Projektdatei-Format abwärtskompatibel **oder** Migration vorhanden
      und getestet (vgl. LH-FA-BLD-002/003, ADR-0003).
- [ ] Abnahmekriterien der Welle nachgewiesen.
- [ ] Reproduzierbarer Build-Hash dokumentiert.

## Offene Punkte

- Zielplattformen (Linux / Windows / macOS) — Entscheidung als ADR.
- Signierung / Distribution — spätere Welle.
