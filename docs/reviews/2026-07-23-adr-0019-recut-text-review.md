# ADR-0019 Text-Review (Neuschnitt gegen ADR-0020) — DRW interaktiver 2D-Zeichen-Canvas

**Datum:** 2026-07-23 · **Reviewer:** unabhängiger Agent (≠ Autor), read-only ·
**Gegenstand:** `docs/plan/adr/0019-drw-2d-canvas.md` (Status Proposed, **neu geschnitten**
gegen die Accepted [ADR-0020](../plan/adr/0020-driven-adapter-serialisieren-kern-liefert-geometrie.md)) ·
Muster [ADR-0018-Text-Review](2026-07-05-adr-0018-text-review.md) (Reviewer ≠ Autor, HIGH blockiert Accept).

**Anlass:** Der Neuschnitt kehrt bewusst den zentralen **HIGH-1** des Erst-Reviews
([2026-07-22](2026-07-22-adr-drw-canvas-text-review.md)) um: die frühere ADR-0019 **autorisierte** eine
neue `.a-check.yml`-Allow-Kante `io → services_geo` (§2.6-Autorisierung). Unter der jetzt **Accepted**
ADR-0020 **entfällt** diese Kante — der Kern liefert die 2D-Projektion den Exportern als `PlanView` im
`DerivedGeometry`-Bündel; **kein** Adapter importiert `services/geometry` → §2.6 n/a.

## Verdikt

**0 HIGH / 0 MED / 1 LOW / 4 INFO → accept-fähig (0 HIGH offen).** Der Projektinhaber-Accept ist erteilt
(2026-07-23, Roadmap-Schritt 2). Die a-check-Fakten wurden maschinell gegen `.a-check.yml` verifiziert.

## Geprüft (kein Finding)

- **A. Konsistenz mit ADR-0020:** kein Rest-Widerspruch. Entscheidung 2 (Kopf/Konsum/Sequenz), Konsequenzen,
  Fitness Function und Bezug tragen die Bündel-Fassung; **keine** Stelle behauptet mehr den direkten
  Export-Aufruf von `services/geometry` oder die `io → services_geo`-Kante. PDF/PNG verbrauchen die
  `PlanView`, DXF iteriert `building.walls`/`.guide_lines` direkt + `visibleLayerIds` — deckt sich mit
  ADR-0020 §Kontext-Befund 3 + MED-1.
- **B. §2.6-Einordnung maschinell korrekt:** `.a-check.yml` — `io` hat nur `→ model`/`→ ports_driven`;
  die Kante entsteht nicht (kein Adapter importiert `services_geo`). Canvas-Read/-Write laufen über die
  **bestehende** Kante `ui_command → ports_driving`; `ui_view → ports_driving` ist real nicht erlaubt
  (bestätigt das Read-Naht-Detail). Die Kanten-Entfernung `geometry`/`persistence → services_geo` ist
  ADR-0020s Job, nicht 0019s. → **§2.6 n/a korrekt.**
- **C. architecture.md-Nachzug korrekt getrennt:** 0019 begrenzt auf §1.1 (`PlanViewPort` +
  `EditDrawingPort`), cediert §2-Tabelle + §1-Diagramm an ADR-0020 — disjunkt, kein Doppel-Nachzug.
- **D. Interne Kohärenz:** nur Entscheidung 2 + „Zu 2"-Alternativen betroffen; 1/3/4/5/6/7 unberührt gültig.
- **E. MR-014 / §2.7:** `grep 'slice-[0-9]'` über Body + `## Geschichte` → 0 Treffer. ADR→ADR-Verweis
  0019→0020 erlaubt (0020 Accepted).
- **F. Ehrlichkeit:** die Umkehr des HIGH-1 ist im Body explizit (Zeilen 44/78), nicht stillschweigend.

## LOW-1 (behoben) — `## Geschichte` dokumentierte den Neuschnitt-gegen-0020 nicht

**Befund:** Die einzige Geschichte-Zeile (Proposed) erwähnte weder den Neuschnitt gegen ADR-0020 noch die
Umkehr der internen `io → services_geo`-Autorisierungs-Annahme; die Transparenz lebte allein im Body. Die
Provenance-Rand-Tabelle ist der deklarierte Ort der Lineage.
**Fix (eingearbeitet):** Neuschnitt-/Accepted-Zeile in `## Geschichte` ergänzt (Neuschnitt gegen ADR-0020,
Kanten-Autorisierung entfällt, §2.6 n/a).

## INFO (keine Findings)

1. 0019 folgt der **verfeinerten** 0020-Entscheidung (DXF iteriert direkt), nicht deren loser Kontext-Zeile.
2. Alle Allow-Listen-Fakten in 0019 gegen `.a-check.yml` verifiziert (io-Kanten, ui_command/ui_view,
   geometry/persistence→services_geo) — maschinell exakt.
3. DXF-Direkt-Iteration bündel-unberührt: `visibleLayerIds` bleibt io-lokaler Nicht-Geometrie-Filter
   (schützt das DXF-Round-Trip-Orakel).
4. **Index-Randbeobachtung (außerhalb der Prüfdatei, mitgezogen):** `docs/plan/adr/README.md` labelte
   ADR-0020s Kern-Naht-Folge-Slice noch „ADR Proposed → Accept davor", obwohl 0020 bereits Accepted ist
   — veraltete Index-Zeile, beim Accept-Nachzug mitgezogen.

---

**Einarbeitung (Autor, 2026-07-23):** LOW-1 (Geschichte-Neuschnitt-Zeile) + INFO-4 (Index-Folge-Zeilen)
eingearbeitet. **0 HIGH offen → accept-fähig; Projektinhaber-Accept erteilt (Roadmap-Schritt 2).**
