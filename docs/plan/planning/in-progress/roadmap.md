# Roadmap — b-cad

**Status:** Aktiv. **Letzte Änderung:** 2026-06-08.

**Format-Regel:** Reihenfolge von **Wellen**, keine Reihenfolge von
Terminen. Daten sind Schätzungen, korrigierbar. Die Roadmap entstand im
Greenfield-Bootstrap (Kurs-Modul 2, Schritt 5) — sie ist eine
Feature-Sequenz, kein Reconciliation-Plan.

---

## Aktuelle Welle

**Welle-ID:** welle-1-mvp
**Start:** 2026-06-08
**Geplantes Ende:** offen (Schätzung folgt mit der Slice-Zerlegung)

**Welle-Ziel:** Ein lauffähiges b-cad, mit dem sich ein
Einfamilienhaus im Sinne von ACC-001 in Grundzügen erstellen lässt —
Projekt anlegen/speichern/laden, Geschosse, Wände (mit Stärke/Höhe),
automatische Raumerkennung, 3D-Extrusion in Echtzeit.

**Closure-Trigger** (jeder Trigger verweist auf ≥1 Slice mit DoD; „Slice
folgt" = Slice noch in der Welle zu schneiden):
- slice-001 done — Build-Skelett & DevContainer (`make build` grün).
- slice-002 done — Code-Gates real & gepromotet (`make gates` grün).
- slice-003 done — Wände mit Grenzwert-Verhalten (`LH-FA-WAL-001/002/003`) + 3D-Extrusion (`LH-FA-D3-001`).
- Raum-Autoerkennung (`LH-FA-ROM-001`) und 3D-Echtzeit (`LH-FA-D3-002`) — *Slice folgt*.
- ACC-005 speichern/laden (`LH-FA-BLD-002/003`, atomar, LH-QA-005) — *Slice folgt*.
- slice-004 done — reproduzierbare, gepinnte Toolchain (ADR-0004): Migration 26.04/node24, Digest+Snapshot. (Toolchain-Härtung; gatet nicht die Feature-Funktion, aber den reproduzierbaren MVP-Build.)
- Closure-Notiz in `done/welle-1-results.md`.

## Nächste Wellen

| Welle | Trigger | Wichtigste Slices (geplant) | Geschätzter Aufwand |
|---|---|---|---|
| welle-2-bauteile | welle-1 done | Türen/Fenster mit Wandöffnung (`DOR`,`WIN`), Treppen (`STR`), Decken/Dach (`SLB`,`ROF`) | L |
| welle-3-auswertung | welle-2 done | Material (`MAT`), Auswertungen (`EVL`), Bemaßung/Layer (`DRW`) | M |
| welle-4-austausch | welle-3 done + ADR zu IFC-Bibliothek accepted | IFC/DXF/STEP/STL-Adapter (`IO`), PDF/PNG-Export | L |
| welle-5-erweiterung | welle-4 done | Plugin-System (`PLG`), UI-Themes/Docking (`UI`), Mehrsprachigkeit (`LH-QA-006`) | M |

## Meilensteine

| Meilenstein | Welle(n) | Trigger | Status |
|---|---|---|---|
| M1 — Lauffähiges MVP | welle-1-mvp | ACC-001-Kern erstellbar, `make gates` grün | offen |
| M2 — Vollständige Bauteile | welle-2-bauteile | Haus mit Türen, Fenstern, Dach vollständig | offen |
| M3 — Auswertbar | welle-3-auswertung | Flächen/Volumen/Materiallisten korrekt | offen |
| M4 — Offen austauschbar | welle-4-austausch | ACC-003, ACC-004 erfüllt | offen |
| M5 — Erweiterbar | welle-5-erweiterung | OBJ-004 (Plugins) erfüllt | offen |

## Abhängigkeitsgraph

```mermaid
flowchart LR
    W1[welle-1-mvp<br/>aktiv]
    W2[welle-2-bauteile]
    W3[welle-3-auswertung]
    W4[welle-4-austausch]
    W5[welle-5-erweiterung]

    W1 --> W2 --> W3 --> W4 --> W5
```

## Abgeschlossene Wellen

(noch keine — Greenfield-Bootstrap soeben abgeschlossen)

## Historische Trigger-Verschiebungen

(noch keine)
