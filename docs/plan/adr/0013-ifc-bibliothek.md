# ADR-0013: IFC-Bibliothek und -Schema-Version (Import/Export hinter dem IO-Port)

**Status:** Accepted

**Datum:** 2026-06-16

**Autor:** Dietmar Burkard (welle-4-austausch, IFC-Bibliotheks-ADR, ausgearbeitet im AI-Harness-Lauf)

**Bezug:** [LH-FA-IO-001](../../../spec/lastenheft.md#lh-fa-io-001--ifc-import) (IFC-Import), [LH-FA-IO-002](../../../spec/lastenheft.md#lh-fa-io-002) (IFC-Export), [ACC-003](../../../spec/lastenheft.md#7-abnahmekriterien), [OBJ-005](../../../spec/lastenheft.md#3-projektziele) (offene Austauschformate), ADR-0001 (Schichtung — Format-Bibliothek ist adapter-lokal), ADR-0002 (OCC = Backend für STEP/STL, **nicht** IFC), ADR-0004 (Toolchain-/Dependency-Pinning — apt-Snapshot, kein vcpkg/Conan ohne Binary-Cache), ADR-0005 (Drittanbieter-Lizenz-Attribution)

---

## Kontext

welle-4-austausch macht b-cad **offen austauschbar** (Meilenstein M4,
[OBJ-005](../../../spec/lastenheft.md#3-projektziele)): Import/Export der Formate IFC/DXF/STEP/STL + PDF/PNG.
Das **IFC-Backend** ist der Roadmap-Closure-Trigger der Welle und die offene
ADR-Frage aus dem ADR-Index (»IFC-Bibliothek und -Schema-Version,
[LH-FA-IO-001](../../../spec/lastenheft.md#lh-fa-io-001--ifc-import)/002«). Diese ADR entscheidet **nur** das IFC-Backend; sie
prägt die übrigen Formate **nicht** vor (siehe Scope).

IFC (ISO 16739, Industry Foundation Classes) ist — anders als die
übrigen Zielformate — **kein** Format, das der bereits gewählte Geometrie-Kern
mitbringt: **OpenCascade (ADR-0002)** liefert STEP/IGES/STL/glTF/OBJ über seine
DataExchange-Module nativ, **aber keinen IFC-Leser/-Schreiber** in der
Open-Source-Distribution. IFC braucht also eine **eigene** Lösung. Das ist der
Grund, warum ADR-0002 den Format-Export bewusst **ausgegliedert** hat
(»STEP-/Format-Export → eigene IO/Export-ADR«) und warum die IFC-Frage hier
getrennt fällt.

Vor der Implementierung sind drei Lösungsfragen offen, die der Spec-Text nicht
entscheidet:

1. **Bibliothek/Backend:** Womit werden IFC-Dateien gelesen/geschrieben? Eine
   vollständige IFC-Toolkit-Abhängigkeit (IfcOpenShell), eine vendierbare
   Parser-Engine (web-ifc), das vorhandene OCC (das IFC **nicht** kann) oder ein
   selbst getragener Subset-Leser/-Schreiber im Adapter?
2. **Toolchain-Verträglichkeit:** Jede neue Abhängigkeit muss durch die
   **gepinnte, paketmanager-freie** Build-Toolchain (ADR-0004: apt-Snapshot,
   **kein** vcpkg/Conan, **kein** Binary-Cache — Source-Builds bewusst
   zurückgestellt). Eine Bibliothek, die nur per Source-Build (OCC-Versions-
   gekoppelt) oder per fremdem Paketmanager kommt, kollidiert hart mit ADR-0004.
3. **Schicht & Schema-Umfang:** Wo lebt das Backend (ADR-0001), und **wie viel
   IFC** deckt welle-4 ab — voller Schema-Reichtum oder ein benannter Subset
   passend zum schmalen Akzeptanzkriterium ([LH-FA-IO-001](../../../spec/lastenheft.md#lh-fa-io-001--ifc-import): Geschoss-/
   Wand-Anzahl stimmt, Nicht-IFC → [`E-IO-003`](../../../spec/spezifikation.md#4-fehler-codes-und-logging-felder), **kein** Teil-Import)?

**Nicht offen** (bewusst außerhalb dieser ADR — Scope-Verengung, Präzedenz
ADR-0002/slice-009-Split):

- **STEP-/STL-Export** ([LH-FA-IO-005](../../../spec/lastenheft.md#lh-fa-io-005)/006) — OCC-nativ; gehört in das aus
  ADR-0002 ausgegliederte **Export-Backend-ADR** (`ModelExporterPort`-Naht
  Geometrie↔IO), nicht hierher.
- **DXF-Import/-Export** ([LH-FA-IO-003](../../../spec/lastenheft.md#lh-fa-io-003)/004) — weder OCC- noch IFC-Sache;
  eigene Bibliothek/eigenes ADR.
- **PDF-/PNG-Export** ([LH-FA-IO-007](../../../spec/lastenheft.md#lh-fa-io-007)/008) — Render-/Plot-Pfad, eigenes ADR.
- **Exakte Port-Signaturen.** `ExchangeModelPort` (driving),
  `ModelImporterPort`/`ModelExporterPort` (driven) und der `ExchangeService`
  sind in `architecture.md` §1.1/§4 bereits **deklariert**; ihre konkreten
  Methoden-Signaturen legt der Implementierungs-Slice fest (ADR-0001-Kern-
  Hoheit), nicht diese ADR.
- **Lastenheft-AK-Schärfung** ([LH-FA-IO-001](../../../spec/lastenheft.md#lh-fa-io-001--ifc-import)/002) bleibt **lösungsfrei**
  ([MR-008](../../../harness/conventions.md#mr-008--lastenheft-schärfung-bleibt-lösungsfrei)) und ist ein **eigener Schärfungs-Slice** nach Accept — diese ADR ist
  die Lösungsschicht, nicht der Anforderungstext.

## Entscheidung

1. **IFC-Backend welle-4 — ein selbst getragener, vendierter IFC-SPF-Subset-
   Leser/-Schreiber im IO-Adapter (Option D).** Der IFC-Adapter unter
   `src/adapters/io/` liest und schreibt **IFC im STEP-Physical-File-Encoding
   (ISO 10303-21, `.ifc`-Klartext)** für einen **benannten Entitäts-
   Subset** (siehe #4). **Keine** neue externe Schwer-Abhängigkeit — der Parser
   ist Repo-eigener C++-Code derselben Klasse wie der **hand-gerollte, totale,
   getestete Format-Codec des Persistenz-Adapters** (`footprint_json`/
   `polygon_json` in `src/adapters/persistence/`, slice-014c/015c) — **gleiches
   Repo-Muster** (totaler, analytisch getesteter Format-Codec), **anderer
   Adapter** (`io/` statt `persistence/`). Begründung:
   das deckt das **schmale Akzeptanzkriterium** ([LH-FA-IO-001](../../../spec/lastenheft.md#lh-fa-io-001--ifc-import): Geschosse +
   Wände, Anzahl stimmt) **proportional** ab, ist **vollständig mit
   analytischen Orakeln testbar** (Roundtrip ohne echten Adapter, wie die
   bestehende Format-Logik), und — entscheidend — **kollidiert nicht mit
   ADR-0004**: kein vcpkg/Conan, kein OCC-versions-gekoppelter Source-Build,
   kein Binary-Cache-Bedarf.

2. **Schicht — Backend ist adapter-lokal (ADR-0001).** Der IFC-Code lebt
   **ausschließlich** in `src/adapters/io/` hinter den **bereits deklarierten**
   `ModelImporterPort`/`ModelExporterPort` (driven), angestoßen über
   `ExchangeModelPort` (driving) + `ExchangeService` (`architecture.md` §1.1/§4).
   Der Kern (`src/hexagon/`) kennt **kein** IFC, kein SPF, keine Format-Typen —
   er bekommt/liefert **Domain-Bauteile** (pure Werttypen). **Folgepflicht:**
   eine neue `arch-check`-Regel (Format-/IFC-Symbole nur in `src/adapters/io/`),
   analog **Regel C** (OCC), **Regel D** (SQLite), **Regel E** (Qt) — siehe
   Fitness Function.

3. **Atomarer Import — ganz oder gar nicht.** Der Import baut zuerst ein
   **vollständiges In-Memory-Domain-Modell** und übergibt es erst nach
   **fehlerfreiem** Parsen an den Service; jeder Parse-/Format-/Schema-Fehler →
   [`E-IO-003`](../../../spec/spezifikation.md#4-fehler-codes-und-logging-felder) (`event=import_rejected`), **kein** Teil-Import
   ([LH-FA-IO-001](../../../spec/lastenheft.md#lh-fa-io-001--ifc-import)-Negative, `architecture.md` §5). Eine Datei, deren
   Inhalt **außerhalb** des unterstützten Subsets liegt (unbekannte Entität in
   einer **tragenden** Rolle, fehlende Pflicht-Referenz), wird **abgelehnt**,
   nicht still teil-importiert — Ehrlichkeits-Vorrang vor Pseudo-Toleranz.

4. **Schema-Version & Entitäts-Subset (welle-4, bewusst benannt).** Ziel-Schema
   ist **IFC4** (ISO 16739-1) für den Export; der Import akzeptiert IFC4 **und**
   IFC2x3, soweit die Subset-Entitäten schema-kompatibel sind. **Wand-Repräsentation:**
   der Export schreibt **`IfcWall` mit `IfcMaterialLayerSetUsage`** — **nicht**
   `IfcWallStandardCase`, das in **IFC4 deprecated** ist (in IFC2x3/IFC4 noch
   vorhanden, aber nicht mehr empfohlen); der Import akzeptiert **beide** (`IfcWall`
   **und** `IfcWallStandardCase`, Rückwärts-Kompatibilität). **Subset welle-4:**
   die räumliche Struktur `IfcProject` → `IfcSite` (optional) → `IfcBuilding` →
   `IfcBuildingStorey` (Spatial-Komposition über `IfcRelAggregates`, Bauteil-
   Zuordnung über `IfcRelContainedInSpatialStructure`) und **gerade,
   achsen-getragene Wände** (Achs-Repräsentation + Dicke aus dem
   `IfcMaterialLayerSetUsage`/`IfcMaterialLayerSet` + Höhe). Die **exakten
   Beziehungs-/Platzierungs-Mechaniken** (Relationship-Entitäten, optionale
   `IfcSite`, Einheiten/`IfcUnitAssignment`) gehören präzise in die
   **Spec-Schärfung** ([LH-FA-IO-001](../../../spec/lastenheft.md#lh-fa-io-001--ifc-import)/002, lösungsfrei → Impl-Slice), nicht in
   diese Backend-Entscheidung. **Benannte Lücken (Re-Eval-Trigger, Muster
   welle-2/-3):** Türen/Fenster (`IfcDoor`/`IfcWindow`), Dach (`IfcRoof`),
   Decken/Fundament (`IfcSlab`), Treppen (`IfcStair`), beliebige BREP-/Swept-
   Solid-Geometrie nicht-prismatischer Wände, Property-Sets/Materialien über den
   Layer hinaus, Georeferenzierung.

5. **Lizenz/Attribution (ADR-0005).** Der Subset-Parser ist **Repo-eigener
   Code** → keine neue Drittanbieter-Lizenz im IFC-Pfad (anders als eine
   gewählte Bibliothek es brächte). Wird über den Re-Evaluierungs-Trigger (§ unten) später
   eine externe IFC-Bibliothek adoptiert, ist deren Lizenz (IfcOpenShell:
   LGPL-3.0; web-ifc: MPL-2.0) in das ADR-0005-Attributions-Manifest
   aufzunehmen — das ist Teil des Adoptions-ADR, nicht dieser Entscheidung.

## Verglichene Alternativen

### Option A — IfcOpenShell (vollständiges IFC-Toolkit)

- **Pro:** De-facto-Standard im offenen IFC-Ökosystem; IFC2x3 **und** IFC4;
  robustes Real-Datei-Parsen; Geometrie **wahlweise über OCC** (CGAL-Backend
  alternativ, OCC ist optional — die OCC-Variante passte zu ADR-0002);
  liest **und** schreibt; deckt weit mehr als den Subset ab.
- **Contra:** **Toolchain-Bruch (ADR-0004).** Das C++-Dev-Paket ist **nicht**
  verlässlich als apt-Paket im 26.04-Snapshot verfügbar (zu **verifizieren**
  per `apt-get install -s` im Toolchain-Image, Muster spike-001 —
  **nicht behauptet**); der Rückfall ist ein **Source-Build**, OCC-versions-
  gekoppelt und schwer — genau die in ADR-0004 (Option B) **zurückgestellte**
  Klasse, solange kein Binary-Cache steht. Große Abhängigkeitsfläche;
  LGPL-3.0-Relink-/Attributions-Pflicht. **Über-dimensioniert** für das
  welle-4-Subset-AK.

### Option B — web-ifc (ThatOpen, vendierbare Parser-Engine)

- **Pro:** Eigenständige C++17-IFC-Engine **ohne** OCC-Zwang; als **Source
  vendierbar** (kein apt/Paketmanager-Bedarf, ADR-0004-verträglicher als A);
  MPL-2.0 (datei-granulares Copyleft, Vendoring + Attribution sauber); robustes
  Real-Datei-**Lesen**.
- **Contra:** Auf **Lesen/Tessellieren** (Web/WASM) ausgelegt — **Schreiben/
  Export** (IO-002/[ACC-003](../../../spec/lastenheft.md#7-abnahmekriterien)) ist schwächer, der SPF-Writer wäre ohnehin
  selbst zu bauen; großer Fremd-Code-Import (CMake-Integration, Pflege,
  ADR-0005-Manifest) für einen **Subset**; ein eigenes Geometrie-/Entitäts-
  Modell, das auf die Domäne zu mappen ist. Schwergewicht ggü. dem AK.

### Option C — OpenCascade DataExchange (vorhanden, ADR-0002)

- **Pro:** **Null** neue Abhängigkeit; liefert STEP/IGES/STL/glTF/OBJ nativ.
- **Contra:** **Kann kein IFC.** Die offene OCCT-Distribution enthält **keinen**
  IFC-Leser/-Schreiber (IFC ist nicht unter TKDESTEP/TKDEIGES/TKSTL; das
  DE-Framework ab 7.7 bringt **keinen** offenen IFC-Connector). Damit ist C die
  **richtige** Antwort für das **Schwester**-ADR (STEP/STL-Export-Backend) —
  aber **disqualifiziert** für die IFC-Frage.

### Option D — vendierter IFC-SPF-Subset-Leser/-Schreiber im IO-Adapter (gewählt)

- **Pro:** **Keine** neue Schwer-Abhängigkeit → **ADR-0004-konform** (kein
  vcpkg/Conan, kein Source-Build, kein Binary-Cache); **proportional** zum
  schmalen AK (Geschosse + Standard-Wände); **vollständig analytisch testbar**
  (Roundtrip-Orakel ohne echten Adapter, wie die bestehende JSON-Format-Logik);
  **symmetrisch** Lesen **und** Schreiben in **einer** Code-Basis; volle
  Kontrolle über Fehler-Totalität ([`E-IO-003`](../../../spec/spezifikation.md#4-fehler-codes-und-logging-felder), kein Teil-Import);
  adapter-lokal (ADR-0001), keine Lizenz-Neulast (ADR-0005).
- **Contra:** Selbst getragenes IFC ist für **beliebige Real-Dateien fragil**
  (großes Schema; Subset lehnt manches ab, das ein echtes Toolkit importierte)
  — ein **Langzeit-Risiko** für [OBJ-005](../../../spec/lastenheft.md#3-projektziele) (offene Interoperabilität), das
  **bewusst** als Re-Evaluierungs-Trigger (§ unten) auf eine echte Bibliothek (A, ersatzweise
  B) gelegt wird, **sobald** die ADR-0004-Binary-Cache-Vorbedingung steht.
  Die Versuchung, den Subset-Parser unkontrolliert zu erweitern, ist real
  (Wartungs-Senke) — die benannte Subset-Grenze (#4) hält sie sichtbar.

## Konsequenzen

- **Positiv:** welle-4-IFC startet **ohne** Toolchain-Eingriff, ohne neue
  Lizenz-Pflicht und **ohne** OCC-Kopplung des IO-Adapters; der IFC-Pfad ist
  schichttreu (ADR-0001), atomar ([`E-IO-003`](../../../spec/spezifikation.md#4-fehler-codes-und-logging-felder)) und analytisch testbar.
  Der schmale, **ehrlich benannte** Subset folgt der welle-2/-3-Disziplin
  (liefern, was das AK verlangt; Lücken benennen).
- **Negativ / Folgepflicht (Slice):** neue **`arch-check`-Regel** (Format-/
  IFC-Symbole nur in `src/adapters/io/`) — bis sie steht, gilt die Isolation als
  Konvention, **nicht** als Gate (Muster ADR-0002-Regel-C-Folgepflicht, keine
  Sensor-Abdeckung überversprechen). `ExchangeService` + `ModelImporterPort`/
  `ModelExporterPort` implementieren; AK-Tests mit [`LH-FA-IO-001`](../../../spec/lastenheft.md#lh-fa-io-001--ifc-import)/`002` im Namen.
- **Negativ / Risiko (benannt):** Real-Datei-Robustheit bleibt hinter einem
  echten Toolkit zurück (Contra D) — explizit als Re-Eval-Trigger gelegt, nicht
  verschwiegen.
- **Lastenheft-Schärfung getrennt:** [LH-FA-IO-001](../../../spec/lastenheft.md#lh-fa-io-001--ifc-import)/002-AK
  (Happy/Boundary/Negative, lösungsfrei) + Spec-Mechanik (Subset-Mapping,
  [`E-IO-003`](../../../spec/spezifikation.md#4-fehler-codes-und-logging-felder)-Auslösung) entstehen im **Schärfungs-Slice** nach Accept
  ([MR-008](../../../harness/conventions.md#mr-008--lastenheft-schärfung-bleibt-lösungsfrei)), nicht in dieser ADR.
- **Negativ / Folgepflicht (Spec-Nachzug):** `spec/spezifikation.md` §6
  (Vertragstabelle, »IFC — Schema-Version offen (ADR-Folge)«) und §7 Offene
  Punkte (»IFC-Schema-Version und -Bibliothek«) werden mit dem **Accept** dieser
  ADR **stale** — die Schema-Version ist hier entschieden (IFC4-Export /
  IFC2x3+4-Import-Subset). Nachzug im Schärfungs-Slice (§6 mechanik-aktualisierend
  auf den entschiedenen Stand, §7-Offene-Punkte-Zeile streichen); wird als
  Folgepflicht in den **ADR-Index** (`README.md` §ADR-Folgepflichten) aufgenommen,
  sobald accepted — damit die höherrangige Spec nicht still stale bleibt.
- **ADR-0001/0002/0004/0005 bleiben unverändert gültig** — diese ADR baut auf
  ihnen auf und ändert sie nicht; ADR-0002 wird nur in seinem **eigenen** Scope
  bestätigt (OCC = STEP/STL, nicht IFC).

## Fitness Function

| Tooling | Regel | Make-Target |
|---|---|---|
| Codec-Orakel (Schärfungs-/Impl-Slice) | IFC-SPF-Codec Happy/Boundary/Negative gegen **analytische** Orakel (Geschoss-/Wand-Anzahl, Roundtrip Export→Import erhält Geschosse+Wände) — Codec-Ebene, schnell, ohne Adapter | `make test` |
| **Integration über den echten Pfad** (Schärfungs-/Impl-Slice) | **Mindestens ein** Test übt `ExchangeService` → `ModelImporterPort`/`ModelExporterPort`-**Adapter** aus (Datei → Domain → Datei): Happy (Anzahl == Quelle) **und** Negative (Nicht-IFC → [`E-IO-003`](../../../spec/spezifikation.md#4-fehler-codes-und-logging-felder) **durch den echten Adapter**, kein Teil-Import) — sonst kann die Verdrahtung hinter grünem Codec-Test fehlen (welle-3-Lehre slice-015b: Integrationspfad ungeübt trotz grüner Gates) | `make test` |
| Schichtung (heute real) | Kern `src/hexagon/` bindet keine Format-/Adapter-Typen ein; kein Adapter importiert einen anderen | `make arch-check` (ADR-0001, real) |
| Schichtung (Folgepflicht) | Format-/IFC-Symbole **nur** in `src/adapters/io/` (neue Regel, analog Regel C/D/E) | `make arch-check` (Folge-Slice) |

## Re-Evaluierungs-Trigger

- **Voller IFC-Reichtum nötig** (beliebige Geometrie, Tür/Fenster/Dach/Decke/
  Treppe, Property-Sets, robuste Real-Datei-Interoperabilität für [OBJ-005](../../../spec/lastenheft.md#3-projektziele))
  → **Option A (IfcOpenShell)** adoptieren (ersatzweise B, web-ifc). Damit der
  Trigger **beobachtbar** ist (Modul 6), zählt als „nötig" ein **konkretes
  Beleg-Artefakt**: ein Real-IFC, das am AK scheitert (Subset lehnt eine
  tragende Entität ab), **oder** eine Lastenheft-AK-Schärfung, die eine der
  benannten Lücken (#4) verbindlich fordert. Die **Toolchain-Vorbedingung** für
  A ist ihrerseits beobachtbar zu belegen (nächster Trigger), nicht als vages
  „Binary-Cache verfügbar" zu lesen. Adoption → **Supersedes-ADR**, kein stiller
  Tausch.
- **IfcOpenShell wird ADR-0004-konform installierbar** — beobachtbar genau dann,
  wenn **`apt-get install -s libifcopenshell-dev` im gepinnten Snapshot auflöst**
  (Muster spike-001) **oder** ein gepinnter Binary-Cache/Registry für
  Qt/OCC/IFC steht (ADR-0004-Re-Eval-Artefakt) → der Source-Build-Einwand gegen
  A entfiele; A neu bewerten.
- **Subset-Parser wird zur Wartungs-Senke** (wiederholtes Erweitern für
  Real-Dateien) → das ist das Signal, dass D seine Proportionalität verloren
  hat → A/B vorziehen.
- **DXF/STEP/STL/PDF/PNG** brauchen je ein **eigenes** Backend-ADR (diese ADR
  prägt sie nicht vor).

## Geschichte

| Datum | Ereignis | Verweis |
|---|---|---|
| 2026-06-16 | Proposed (welle-4-austausch erster Schritt; IFC-Backend-Wahl, Subset + Re-Eval auf echte Bibliothek) | welle-4 / IFC-Bibliotheks-ADR |
| 2026-06-16 | **Unabhängiges Text-Review** (Reviewer ≠ Autor, vor Accept): **0 HIGH**, 2 MED + 3 LOW eingearbeitet — MED-1 `IfcWallStandardCase` in IFC4 deprecated → Export schreibt `IfcWall`+`IfcMaterialLayerSetUsage`, Import akzeptiert beide; MED-2 Re-Eval-Trigger #1 beobachtbar gemacht (Beleg-Artefakt statt vager Binary-Cache-Konjunktion); LOW-1 Format-Codec-Präzedenz korrekt im Persistenz-Adapter verortet; LOW-2 IfcOpenShell-OCC-Backend als optional präzisiert; LOW-3 Spatial-Beziehungs-Mechanik in die Spec-Schärfung delegiert. **Accept ausstehend (Projektinhaber)** | [`docs/reviews/2026-06-16-adr-0013-text-review.md`](../../reviews/2026-06-16-adr-0013-text-review.md) |
| 2026-06-16 | **Projektinhaber-Review (2. Runde, vor Accept):** 3 MED + 1 LOW eingearbeitet — MED Index-Eintrag (Proposed) im ADR-Index ergänzt + Offene-Themen-Zeile nachgezogen; MED Spec-Stale-Nachzug (§6 Vertragstabelle / §7 Offene Punkte) als explizite Folgepflicht; MED Fitness Function um **Adapter-Pfad-Integrationstest** (`ExchangeService`/Importer-Exporter) erweitert (Codec-Orakel allein unzureichend); LOW `(#6)`-Verweis auf §Re-Evaluierungs-Trigger korrigiert. **Accept ausstehend (Projektinhaber)** | Projektinhaber-Findings |
| 2026-06-16 | **Accepted** (Projektinhaber) — beide Review-Runden **0 HIGH**, alle MED/LOW eingearbeitet. Damit ist der welle-4-austausch-Closure-Trigger »ADR zu IFC-Bibliothek accepted« erfüllt → Welle **gestartet** (Roadmap §Aktuelle Welle); Folgepflichten im Index | welle-4-Buchung |
