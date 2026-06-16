# ADR-Text-Review (vor Accept) — ADR-0013 IFC-Bibliothek

**Review-Art:** ADR-Text-Review, vor `Accepted` (Projektinhaber-Disziplin, Muster `ADR-0012`-Geschichte).
**Gegenstand:** [`docs/plan/adr/0013-ifc-bibliothek.md`](../plan/adr/0013-ifc-bibliothek.md) (Status: Proposed, 2026-06-16).
**Reviewer:** unabhängiger Agent (Reviewer ≠ Autor; ohne Autoren-Kontext, `MR-006`-Disziplin auf eine ADR angewandt).
**Eingangs-Kontext (gelesen):**

- ADR selbst: `docs/plan/adr/0013-ifc-bibliothek.md`
- Referenzierte ADRs: `0001-hexagonale-architektur.md`, `0002-geometrie-kern-opencascade.md`, `0004-toolchain-dependency-pinning.md`, `0005-drittanbieter-lizenz-attribution.md`; Benchmark `0012-evaluations-architektur.md`; ADR-Index `docs/plan/adr/README.md`
- Spec: `spec/lastenheft.md` (§ Modul IO: `LH-FA-IO-001`..008, `ACC-003`/004, `OBJ-005`); `spec/spezifikation.md` §4 (`E-IO-001`..003); `spec/architecture.md` (§1.1/§1.2 Ports, §1.3 Schicht-Import-Tabelle, §4 IO-001-Sequenz, §5 Fehlermodell)
- Harness: `harness/conventions.md` (`MR-006`/008/009/011); `AGENTS.md` §2 (Hard Rules 2.1/2.5/2.6/2.9)
- Regelwerk: `/tmp/lab-regelwerk/modul-04-architektur-adrs.md`, `/tmp/lab-regelwerk/modul-10-review-harness.md`
- Code-Belege: `tools/arch-check.sh` (Regeln A/C/D/E), `src/adapters/io/`, `src/adapters/persistence/` (JSON-Ser/De `footprint_json`/`polygon_json`)
- Externe Faktenprüfung: unabhängige Web-Recherche zu IFC/ISO/OCC/IfcOpenShell/web-ifc/IfcWallStandardCase (Primärquellen ISO, buildingSMART, Open Cascade, GitHub)

---

## Findings

### MEDIUM-1 — IfcWallStandardCase ist seit IFC4 deprecated; ADR benennt das nicht
- **kategorie:** MEDIUM
- **quelle:** Faktenlage (buildingSMART IFC4-Schema)
- **pfad:** `0013-ifc-bibliothek.md` Entscheidung #4 (Z. 101–108), Z. 104/107
- **befund:** Die ADR wählt `IfcWallStandardCase` als Wand-Export-Repräsentation für das Ziel-Schema IFC4 und stellt es als in IFC4 **und** IFC2x3 vorhanden dar; sie nennt **nicht**, dass `IfcWallStandardCase` in IFC4 **deprecated** ist (vorhanden, aber als veraltet markiert; empfohlen wird `IfcWall` + `IfcMaterialLayerSetUsage`). Ein IFC4-Export, der bevorzugt eine deprecated-Entität schreibt, ist eine benennenswerte Entscheidung, keine neutrale Tatsache.
- **verifizierbar:** ja (buildingSMART IFC4-Lexical-Doku: `IfcWallStandardCase` „DEPRECATED … removed in a future major release")

### MEDIUM-2 — Re-Eval-Trigger #1 koppelt an eine ADR-0004-„Binary-Cache-Vorbedingung", die als benannter Zustand nirgends definiert/beobachtbar ist
- **kategorie:** MEDIUM
- **quelle:** Maintainability / Modul 6 (beobachtbarer Re-Eval-Trigger)
- **pfad:** `0013-ifc-bibliothek.md` Re-Evaluierungs-Trigger #1 (Z. 208–212); Contra D (Z. 167–171)
- **befund:** Der primäre Re-Eval-Trigger („voller IFC-Reichtum nötig **und** `ADR-0004`-Binary-Cache-Vorbedingung erfüllt → Option A adoptieren") hängt an „Binary-Cache verfügbar". `ADR-0004` führt das nur als eigenen Re-Eval-Trigger („Binary-Cache für Qt6/OCC verfügbar → Option B neu bewerten"), ohne ein **beobachtbares** Kriterium oder Artefakt, an dem „verfügbar" festzustellen wäre. Der zusammengesetzte Trigger ist damit nur so beobachtbar wie sein vagestes Konjunkt; „voller IFC-Reichtum nötig" ist zudem ein Bedarfs-Urteil, kein Signal. (Der zweite Trigger — „IfcOpenShell-Dev-Paket per `apt-get install -s` belegt" — und der dritte — „Subset-Parser wird zur Wartungs-Senke" — sind dagegen konkreter bzw. operationalisierbar.)
- **verifizierbar:** ja (`ADR-0004` Re-Eval-Trigger-Liste enthält kein definiertes „Binary-Cache verfügbar"-Kriterium)

### LOW-1 — Provenance-Analogie zur JSON-Ser/De verweist auf den Persistenz-Adapter, nicht den IO-Adapter
- **kategorie:** LOW
- **quelle:** Faktenlage (Repo-Layout)
- **pfad:** `0013-ifc-bibliothek.md` Entscheidung #1 (Z. 73–74): „Muster: die vorhandene, totale, getestete JSON-Ser/De der `roofs`/`slabs`-Persistenz, slice-014c/015c"
- **befund:** Die als Vorbild zitierte JSON-Ser/De (`footprint_json`/`polygon_json`) lebt in `src/adapters/persistence/` (SQLite-Repository), während der IFC-Subset-Parser laut #2 in `src/adapters/io/` liegt. Das Muster „totaler, getesteter Repo-eigener Ser/De-Code" existiert real und ist tragfähig zitiert, liegt aber in einem **anderen** Adapter als dem, den die ADR damit begründet — die Analogie ist korrekt, der Ort-Verweis suggeriert eine IO-Adapter-Präzedenz, die es so (noch) nicht gibt.
- **verifizierbar:** ja (`grep footprint_json/polygon_json` trifft nur `src/adapters/persistence/`)

### LOW-2 — Option A führt „Geometrie über OCC" als flachen Pro, obwohl OCC in IfcOpenShell optional ist
- **kategorie:** LOW
- **quelle:** Faktenlage (IfcOpenShell)
- **pfad:** `0013-ifc-bibliothek.md` Option A Pro (Z. 126): „Geometrie über **OCC** (zu `ADR-0002` passend)"
- **befund:** IfcOpenShell kann OCC für BRep/Mesh nutzen, hat aber einen alternativen CGAL-Geometrie-Backend; das Parsen funktioniert ohne OCC. „Geometrie über OCC" als Eigenschaft der Bibliothek ist daher überzeichnet (es ist ein Backend von mehreren). Da A ohnehin verworfen wird, ist die Konsequenz gering; die Tatsachenbehauptung ist dennoch zu stark.
- **verifizierbar:** ja (IfcOpenShell-Doku/Repo: OCC- **und** CGAL-Geometrie-Backend)

### LOW-3 — Spatial-Struktur als Kette dargestellt; IFC-Aggregations-Mechanik (`IfcRelAggregates`, optionales `IfcSite`) ungenannt
- **kategorie:** LOW
- **quelle:** Faktenlage (IFC-Schema)
- **pfad:** `0013-ifc-bibliothek.md` Entscheidung #4 (Z. 105–107): „`IfcProject` → `IfcSite` → `IfcBuilding` → `IfcBuildingStorey`"
- **befund:** Die räumliche Kette ist als Containment-Pfeil notiert. Im IFC-Schema ist das eine **Komposition** über `IfcRelAggregates` (nicht `IfcRelContainedInSpatialStructure`, das physische Elemente wie Wände an ein Geschoss bindet), und `IfcSite` ist optional. Für eine Backend-/Subset-Entscheidung ist die Pfeil-Notation tolerabel; präzise Mechanik gehört aber in die Spec-Schärfung und sollte nicht als entschiedene Struktur missverstanden werden.
- **verifizierbar:** ja (buildingSMART/IFC-Spatial-Tree)

### INFO-1 — „kein OCC-versions-gekoppelter Source-Build" gilt für Option A; web-ifc (B) ist von dieser Kopplung frei
- **kategorie:** INFO
- **quelle:** Faktenlage
- **pfad:** `0013-ifc-bibliothek.md` Entscheidung #1 (Z. 78–80), Option B Pro (Z. 139–142)
- **befund:** Das Hauptargument „kein OCC-versions-gekoppelter Source-Build" trifft Option A (IfcOpenShell). Option B (web-ifc) ist OCC-frei und als Source vendierbar — die ADR sagt das in Option B selbst korrekt, der `ADR-0004`-Konflikt ist also primär ein A-Argument, kein pauschaler „jede Bibliothek"-Einwand. Kein Widerspruch, nur eine Präzisierung der Argumentlast.
- **verifizierbar:** ja

### INFO-2 — ACC-004/IO-007 (PDF) im Bezug nicht geführt; bewusst korrekt, aber Scope-Abgrenzung nur einseitig sichtbar
- **kategorie:** INFO
- **quelle:** ADR-Vollständigkeit
- **pfad:** `0013-ifc-bibliothek.md` Bezug (Z. 9), Scope (Z. 48–64)
- **befund:** Der Bezug nennt `ACC-003` (IFC-Export), nicht `ACC-004` (PDF) — korrekt, da PDF explizit in ein Schwester-ADR ausgegliedert ist. Die Auftrags-Erwähnung von `ACC-004` ist damit bewusst **nicht** Gegenstand; die Scope-Liste benennt PDF/PNG-Ausschluss sauber. Kein Mangel, nur Bestätigung der bewussten Abgrenzung.
- **verifizierbar:** ja

---

## Negativbefunde (geprüft, ohne Befund)

- **Faktenlage IFC/ISO:** „IFC = ISO 16739 (IFC4 = ISO 16739-1)" und „IFC-SPF-Klartext folgt ISO 10303-21" — beide gegen Primärquellen (ISO, buildingSMART) **bestätigt**, korrekt zitiert. Geprüft, ohne Befund.
- **Faktenlage OCC:** „Open-Source-OCCT hat **keinen** nativen IFC-Leser/-Schreiber (nicht in TKDESTEP/TKDEIGES/TKSTL; DE-Framework ab 7.7 bringt keinen offenen IFC-Connector)" — bestätigt (IFC nur in kommerziellen Open-Cascade-Produkten). „OCC liefert STEP/IGES/STL/glTF/OBJ nativ" — bestätigt. Die Disqualifikation von Option C für IFC ist faktisch korrekt. Geprüft, ohne Befund.
- **Faktenlage Bibliotheks-Lizenzen:** IfcOpenShell LGPL-3.0, liest **und** schreibt, IFC2x3+IFC4 — bestätigt; web-ifc MPL-2.0, eigenständige C++-Engine ohne OCC-Zwang, lese-/tessellier-orientiert — bestätigt. Die ADR markiert IfcOpenShells apt-Verfügbarkeit korrekt als **zu verifizieren** (`apt-get install -s`, Muster spike-001) statt zu behaupten — Modul-13-Ehrlichkeit korrekt angewandt. Geprüft, ohne Befund (Detail-Nuancen → LOW-2).
- **Scope-Disziplin (`MR-008`):** Die Lastenheft-AK-Schärfung (`LH-FA-IO-001`/002) ist explizit als **lösungsfreier Schärfungs-Slice nach Accept** ausgelagert (Scope Z. 62–64; Konsequenzen Z. 190–193); STEP/STL/DXF/PDF/PNG sind in Schwester-ADRs ausgeschlossen (Z. 51–56); Port-Signaturen bleiben Slice-Hoheit (Z. 57–61). Kein gebündeltes Leitplanken-ADR (welle-3-Lektion respektiert). Geprüft, ohne Befund.
- **Architektur-Konsistenz (Port-Namen):** `ExchangeModelPort` (driving), `ModelImporterPort`/`ModelExporterPort` (driven), `ExchangeService` — alle vier exakt so in `architecture.md` §1.1/§1.2 deklariert (Z. 81/89/90 + §1.3 Tree). Geprüft, ohne Befund.
- **Architektur-Konsistenz (Schichtung):** „IFC-Code ausschließlich in `src/adapters/io/`, Kern IFC-frei" deckt sich mit `architecture.md` §2-Tabelle (IO-Adapter darf `model`/`ports/driven`, nicht andere Adapter/GUI) und AGENTS §2.1. Geprüft, ohne Befund.
- **Arch-check-Folgepflicht (Sensor-Ehrlichkeit):** Die neue io/-Regel ist korrekt als **Folgepflicht/Konvention, noch nicht Gate** gerahmt (Konsequenzen Z. 182–185; Fitness-Function-Zeile „Folgepflicht"), analog `ADR-0002`-Regel-C-Präzedenz; `tools/arch-check.sh` führt heute real A/C/D/E, **keine** io/-Regel — die ADR überverspricht keine Sensor-Abdeckung (Modul 13). Geprüft, ohne Befund.
- **Atomarer Import / Fehlermodell:** „vollständiges In-Memory-Modell, erst nach fehlerfreiem Parsen an den Service; jeder Fehler → `E-IO-003` (`event=import_rejected`), kein Teil-Import" deckt sich mit `LH-FA-IO-001`-Negative (lastenheft.md Z. 704–705) und `architecture.md` §5 (Z. 232) sowie spezifikation.md §4 (`E-IO-003`). Geprüft, ohne Befund.
- **MADR-Format/Vollständigkeit:** Alle Benchmark-Abschnitte aus `ADR-0012` vorhanden — Status/Datum/Autor/Bezug, Kontext, Entscheidung, Verglichene Alternativen, Konsequenzen, Fitness Function, Re-Evaluierungs-Trigger, Geschichte. Bezug-Anker (`#lh-fa-io-002`, `#lh-fa-io-005`, `#7-abnahmekriterien`, `#3-projektziele`, `#4-fehler-codes…`) lösen in den Zielen auf. Geprüft, ohne Befund.
- **Hard-Rule-Konformität (AGENTS §2):** Kein Gate-Loosening ohne ADR (§2.6) — die ADR **fügt** eine Regel hinzu, lockert keine; `ADR-0004` wird nicht verletzt, sondern als Constraint respektiert (§2.9 Tool-Allowlist: kein vcpkg/Conan); `ADR-0001`/0002/0004/0005 bleiben unverändert gültig (Z. 194–196), keine Immutability-Verletzung (§2.5). Geprüft, ohne Befund.
- **Entscheidungs-Defensibilität (`ADR-0002`-Anti-Pattern „eigene Engine"):** Option D ist **kein** `ADR-0002`-Option-A-Verstoß — der Subset-Parser baut **keine** Geometrie-/BREP-Engine, sondern einen Format-Textcodec (SPF-Subset) auf bestehende analytische Domänen-Geometrie; die schwere Geometrie bleibt bei OCC. Das Langzeit-`OBJ-005`-Risiko ist als Contra D **benannt** und auf Re-Eval gelegt (siehe MEDIUM-2 zur Trigger-Schärfe). Geprüft, ohne Befund (Schärfe-Vorbehalt → MEDIUM-2).

---

## Kategorie-Summary

| Kategorie | Anzahl |
|---|---|
| HIGH | 0 |
| MEDIUM | 2 |
| LOW | 3 |
| INFO | 2 |

---

## Verdikt

**Accept-ready: ja, mit Vorbehalt.** Es gibt **keine HIGH-Findings** — keine Faktenfehler im Kern der Entscheidung, kein interner Widerspruch, kein Verstoß gegen einen Accepted-ADR, Hard Rule oder die Spec, keine überversprochene Sensor-Abdeckung. Die Entscheidung (Option D) ist aus dem genannten Kontext (`ADR-0004`-Pinning, schmales `LH-FA-IO-001`-AK, hexagonale Schichtung) **defensibel** und sauber gegen das „eigene-Engine"-Anti-Pattern abgegrenzt.

Vor Accept sollten die zwei MEDIUM-Findings **geklärt** werden — nicht weil sie die Entscheidung umstoßen, sondern weil sie ihre Genauigkeit und Re-Eval-Beobachtbarkeit betreffen: (MEDIUM-1) die IFC4-Deprecation von `IfcWallStandardCase` ist im Entscheidungstext zu vergegenwärtigen; (MEDIUM-2) der primäre Re-Eval-Trigger hängt an einem in `ADR-0004` nicht beobachtbar definierten „Binary-Cache verfügbar"-Zustand. Die drei LOW (Provenance-Ort, OCC-optional, Aggregations-Mechanik) sind nice-to-fix und blockieren nicht.
