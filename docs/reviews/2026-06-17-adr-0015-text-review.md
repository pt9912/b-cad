# Unabhängiges Text-Review — ADR-0015 (DXF-Import/-Export-Backend)

## Kopf

- **Review-Art:** Unabhängiges ADR-Text-Review **vor Accept** (Reviewer ≠ Autor, kein Autoren-Kontext). Gegenstand: ADR-Prosa (kein Diff). Muster ADR-0013/0014.
- **Gegenstand:** `docs/plan/adr/0015-dxf-backend.md` (Status Proposed) — DXF-Backend, Schwester-ADR zu ADR-0013 (IFC, Option D) + ADR-0014 (STEP/STL, OCC-nativ).
- **Datum:** 2026-06-17.
- **Quellen:** `spec/lastenheft.md` (IO-003/004 Outline, OBJ-005), `0013-ifc-bibliothek.md` (Option-D-Präzedenz), `0014-step-stl-export-backend.md` (DXF-Ausgliederung, „Regel F gegenstandslos"), `README.md` §Folgepflichten, `tools/arch-check.sh` + `src/adapters/io/` (header-freier Codec → arch-check grün), `spec/spezifikation.md` §4/§6/§7, `.devcontainer/Dockerfile` (kein DXF-Translator), `harness/conventions.md` (MR-008), slice-019b (Import-Höhen-Default-Präzedenz).

**Verdikt: 0 HIGH / 2 MED / 4 LOW / 2 INFO. Accept nicht blockiert.**

## Verifiziert-korrekt (tragende Behauptungen)

- **IO-003/004 bare Outline** (`lastenheft.md:771-772`); **§6 ohne DXF-Zeile** (`:751-757` → Nachzug = anlegen); **§7-Sammelklausel „DXF-/PDF-/PNG-Backends bleiben offen"** existiert verbatim (`spezifikation.md:769` → chirurgisch DXF entfernen, PDF/PNG bleiben — slice-020a-MED-1-Lehre); **ADR-0014 benennt DXF als „weder OCC- noch IFC-nativ → eigenes ADR"** (`0014:48,212`).
- **„Regel F gegenstandslos" / Regel A+B isolieren** — empirisch belegt: `src/adapters/io/` (IFC-Codec) trägt **null** externe Header, `arch-check` exit 0 inkl. io/; deckt sich mit README-ADR-0013-Folgepflicht (`:44`). Wiederverwendung präzise.
- **OCC kann kein DXF** — `.devcontainer/Dockerfile` hat keinen DXF/DWG-Translator (wie IFC). **slice-019b-Höhen-Default-Präzedenz** stimmt (DXF-Import → Default-Höhe/-Dicke, benannte Lücke). **MR-008-Deferral** der exakten AK korrekt.

## Findings

### MED-1 — Import-Dispatch ist **kein** Registry (Asymmetrie zur Exporter-Map) → **eingearbeitet**
`ExchangeService` hält export-seitig `ExporterMap` (Map-Dispatch), import-seitig aber **einen einzigen** fest verdrahteten `ifc_importer_` + `switch` (Step/Stl werfen). DXF-Import ist daher **keine** „ein-Map-Eintrag-mehr"-Sache, sondern eine **Kern-Erweiterung des Import-Dispatch** (Importer-Registry o. zweite Referenz). **Fix:** Entscheidung #2 umformuliert — Export = Map-Eintrag (ohne Kern-Änderung), **Import = Kern-Erweiterung, Form = Impl-Slice** (ADR-0001-Kern-Hoheit), keine falsche Symmetrie.

### MED-2 — Re-Eval-Trigger vage („nötig") unter dem ADR-0013-MED-2-Standard → **eingearbeitet**
ADR-0013 wurde auf einen **beobachtbaren Beleg-Artefakt** geschärft; ADR-0015 regredierte. **Fix:** Trigger #1 beobachtbar — **reale DXF scheitert am AK** / **AK-Schärfung bindet eine benannte Lücke**; DXF-Lib-Adoption muss ihre **ADR-0004-Konformität selbst belegen** (`apt-get install -s`-Probe, spike-001/ADR-0014), sonst ADR-0004-Berührung.

### LOW-1 — „kann kein DXF" ADR-0002 zugeschrieben (erwähnt DXF nie) → **eingearbeitet** (Bezug: eigene Feststellung, s. Option O).
### LOW-2 — §4-`E-IO-001`-Bedingung zählt nur „IFC-Export" → **eingearbeitet** (Spec-Nachzug erweitert §4 um DXF-Export, STEP/STL-Parität).
### LOW-3 — folgt aus MED-1 (keine eigene Änderung). 
### LOW-4 — `OBJ-005`-Anker → `#3-projektziele` ist Schwester-ADR-Konvention (ADR-0013/0014) → **kein Fix**.

### INFO-1 — Entscheidungs-Qualität: Option D für DXF korrekt
DXF unterscheidet sich nicht von IFC in einer Weise, die die Antwort ändert — der schmale 2D-Achsen-Subset ist **einfacher** als IFC-SPF, das Verhältnismäßigkeits-Argument **stärker**. libdxfrw-Verwerfung (ADR-0004 vcpkg/Conan/Source + ADR-0005 Lizenz) = dieselbe validierte Logik wie ADR-0013. Asymmetrie zu STEP/STL (legitim OCC-nativ, weil OCC STEP/STL **kann**) korrekt gezogen.

### INFO-2 — Scope/2D-Ehrlichkeit/Folgepflichten solide
Beide Richtungen (IO-003 Import + IO-004 Export) adressiert; 3D→2D-Verlust (Höhe/Dicke → Default) ehrlich als Lücke benannt + slice-019b-gespiegelt; Achse-vs-Footprint korrekt an die AK delegiert; Lücken (Räume/DIMENSION/HATCH/BLOCK/TEXT/ARC/3D) als Skip/nicht-geschrieben benannt; Folge-Aufgaben (AK-Schärfung + §1 + §6/§7 + Impl) vollständig; ADR-Index-Folgepflicht-Zeilen auf Accept benannt.

## Ergebnis
**Keine HIGH-Findings — Accept nicht blockiert.** MED-1 (Import-Registry-Asymmetrie) + MED-2 (beobachtbarer Re-Eval-Trigger) + LOW-1/LOW-2 vor Accept eingearbeitet. Die Option-D-Entscheidung, die io-residente Schicht, die „Regel F gegenstandslos"-Wiederverwendung (arch-check-belegt) und die §6/§7-Chirurgie sind korrekt und konsistent zu ADR-0013/0014.
