# Unabhängiges Text-Review — ADR-0016 (PDF-/PNG-Export-Backend)

**Datum:** 2026-07-01
**Artefakt:** [`docs/plan/adr/0016-pdf-png-backend.md`](../plan/adr/0016-pdf-png-backend.md) (Status bei Review: Proposed)
**Reviewer:** unabhängig (Reviewer ≠ Autor, ohne Entstehungs-Kontext — MR-006-Linse auf eine ADR, Muster ADR-0013/0014/0015 vor Accept)
**Modus:** read-only; adversariale Verifikation jeder tragenden Behauptung gegen die realen Artefakte (Lastenheft/Spezifikation/conventions/Schwester-ADRs/echter Code).

## Verdikt

**0 HIGH.** Accept-fähig nach Einarbeitung der Findings. Der Entscheidungskern
(Option D, io-resident, kein Qt, `ExporterMap`-only, keine neue Dependency)
trägt gegen die Quellen. Findings sind Präzisions-/Vollständigkeits-Feinschliff.

## Verifikations-Log (tragende Behauptungen — alle bestätigt)

- **Lastenheft:** `LH-FA-IO-007` (Anker), `LH-FA-IO-008` (Anker), `ACC-004 → LH-FA-IO-007`; **kein ACC bindet PNG** → ADR-Aussage korrekt.
- **Roadmap:** `M4 = ACC-003 + ACC-004`.
- **Spezifikation §4:** `E-IO-001`-Zeile zählt heute „IFC-/STEP-/STL-/DXF-Export" auf → ADR-Nachzug-Ansage (um PDF/PNG erweitern) korrekt. **§6:** keine PDF/PNG-Zeile. **§7:** „PDF-/PNG-Backends bleiben offen (welle-4)" → nach chirurgischer Entfernung wird die Offen-Klausel leer.
- **conventions:** MR-006/MR-008/MR-009 — Anker + Bedeutung korrekt zitiert.
- **arch-check** (`tools/arch-check.sh`): Regel A (Kern framework-frei), B (kein Adapter→Adapter), C (OCC nur geometry/), E (Qt nur `ui/`+`main.cpp`) existieren wortgetreu; ein header-freier io-Codec wird durch A+B isoliert (keine neue Regel).
- **Code:** `ExchangeFormat = { Ifc, Step, Stl, Dxf }` (Pdf/Png sind neue Einträge); `ExporterMap`/`ImporterMap`-Registries existieren (`ImporterMap` = slice-021b); `ModelExporterPort::write(Building&, path&)` + Atomaritäts-/E-IO-001-Vertrag; `atomicWrite` schreibt rohe Bytes (`::write` + `fsync` + `rename`) → byte-treu/binär-sicher; DXF-Datenquelle = `building.storeys` + `building.walls` (`wall.start/end`) → PDF/PNG teilen dieselbe 2D-Quelle; `main.cpp` verdrahtet je Format einen `ExporterMap`-Eintrag.
- **Schwester-ADRs:** 0013/0014/0015 gliedern PDF/PNG je als „eigenes ADR" aus; Gegenfolie 0014 (geometrie-resident nur weil OCC-nativ, *driven* Adapter unter Regel C) korrekt; ADR-0009 Regel E begrenzt Qt real auf `ui/`+`main.cpp`; ADR-0010 existiert.
- **io-smoke:** existiert (CI-only, kein Gate) → ADR-Erweiterungs-Ansage trägt.

## Findings (schärfstes zuerst) + Disposition

### MED-1 — „E-IO-003 nicht anwendbar" widerspricht dem realen ExchangeService-Verhalten
**Fund:** `ExchangeService::importModel` wirft bei einem Import-Lookup-Miss explizit `E-IO-003` (`event=import_rejected`) — der bewusst gebaute export-only-Vertrag (STEP/STL, slice-021b-MED-1). Da PDF/PNG als `ExchangeFormat`-Werte hinzukommen, ergäbe `importModel(path, Pdf/Png)` **dasselbe** `E-IO-003`. Die blanke Aussage „nicht anwendbar" ist gegen den Code zu stark.
**Disposition: eingearbeitet.** Kontext + Entscheidung #4 auf die adapter-zentrische, STEP/STL-identische Lesart umformuliert (Import-*Request* → export-only-Lookup-Miss `E-IO-003`; kein PDF/PNG-Import-Adapter/Parse-Pfad).

### LOW-1 — „Regel F gegenstandslos" ADR-0013 zugeschrieben
**Fund:** ADR-0013 *proponierte* eine Regel F (nie gebaut); die Reframe „gegenstandslos" ist eine ADR-0015-Prägung. Doppel-Zuschreibung an 0013 historisch unscharf.
**Disposition: eingearbeitet.** Beide Vorkommen: „ADR-0015-Prägung; header-freier Codec = ADR-0013-Muster".

### LOW-2 — OBJ-005 enumeriert PDF/PNG nicht
**Fund:** OBJ-005 = „IFC, DXF, STEP, STL"; PDF/PNG stehen nicht darin (Ausgabe-Formate). OBJ-005 ist kontextueller M4-Rahmen, nicht die direkte Anforderungsquelle.
**Disposition: eingearbeitet.** Bezug kennzeichnet OBJ-005 als kontextuellen M4-Rahmen; direkte Bindung = LH-FA-IO-007/008 + ACC-004.

### INFO-1 — ExporterMap-Provenance
`ExporterMap` eingeführt slice-020b (Registry), 021b ergänzte Dxf + `ImporterMap`. **Disposition: eingearbeitet** („eingeführt slice-020b").

### INFO-4 — Konsequenzen-„bleiben gültig"-Liste ließ ADR-0010 aus
**Disposition: eingearbeitet** (0010 ergänzt).

### INFO-2 — Subset vs. ACC-004
Der Achsen-Subset muss im Schärfungs-Slice gegen „maßstäblicher PDF-Plan" (ACC-004, M4-bindend) tragen; als benannte Lücke + Re-Eval-Trigger korrekt geführt, Totalität/Maßstab lösungsfrei delegiert (MR-008-treu). **Kein Handlungsbedarf** — Hinweis an den Schärfungs-Slice: ACC-004-Erfüllbarkeit des Achsen-Subsets explizit bestätigen.

### INFO-3 — MR-009-„n/a" plausibel
PDF/PNG = 2D-Projektion bestehender Wand-Achsen, keine neue Solid-Geometrie → „n/a" trägt; Rest-Korrektheit über Maßstabs-Sonde + Re-Read-Orakel + MR-006/Code-Review im Impl-Slice abgedeckt. **Kein Handlungsbedarf.**

## Positive Befunde
Die angesagten Spec-Nachzüge (§1/§6/§7/§4) beschreiben den realen Stand korrekt.
io-smoke-Erweiterung, Re-Read-/Maßstabs-Sonde-Orakel und Adapter-Pfad-
Integrationstest (welle-3-Lehre slice-015b) sind vollständig als Folgepflichten
benannt.

## Ein-Satz-Verdikt
**Accept-fähig: ja** — 0 HIGH; MED-1 + LOW/INFO eingearbeitet, Entscheidungskern
unberührt. Accept bleibt Projektinhaber-Entscheidung.

## Nachtrag — Projektinhaber-Durchsicht (zweite Runde, 2026-07-01)

Nach dem unabhängigen Text-Review hat der Projektinhaber eine eigene Durchsicht
vorgenommen; 3 weitere Findings, alle eingearbeitet:

- **MED (Datei-Orakel zu schwach):** Nur PDF-Content-Stream + PNG-`IHDR` zu
  re-parsen kann kaputte PDF-`xref`-/Stream-Längen, defekte PNG-`IDAT`-zlib/
  DEFLATE-Daten, CRC-/Adler-Fehler oder fehlende Scanline-Filterbytes übersehen.
  Weil die ADR **selbst geschriebene** Writer wählt, fordert die Fitness Function
  jetzt ein **vollständiges Struktur-/Decode-Orakel** (PDF `xref`/`trailer`/
  Stream-`/Length` + PNG voll-`IDAT`-Inflate + Adler-32 + je-Chunk-CRC-32 +
  Scanline-Filterbytes → Pixel-Rekonstruktion), optional ein externer
  Standard-Reader als Zweit-Orakel. → Fitness Function „Export-AK" + Konsequenzen.
- **LOW (Kern-Umbau):** „Kein Kern-Umbau" war zu absolut — die Umsetzung erweitert
  mindestens das `ExchangeFormat`-Enum (`src/hexagon/ports/driving/exchange_model_port.h`).
  Präzisiert: **keine Service-/Registry-Architektur-Änderung**, der einzige
  Kern-Touch ist die **additive** `ExchangeFormat`-Enum-Erweiterung um `Pdf`/`Png`
  + Exporter-Verdrahtung. → Entscheidung #2 + Option-D-Pro + Konsequenzen.
- **LOW (Achsen-Plan-Teilumfang):** „Wand-Achsen je Geschoss" vs. Lastenheft-Wortlaut
  „maßstäblicher PDF-Plan" (`ACC-004`) explizit als **Achsen-Plan-Teilumfang**
  benannt; Wand-Footprint/-Dicke ist **out of scope** und ein **AK-abhängiger
  Re-Eval-Trigger**. → Repräsentation #3 + neuer Re-Eval-Trigger.
