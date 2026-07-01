# ADR-0016: PDF-/PNG-Export-Backend (selbst getragener Vektor-PDF-/Raster-PNG-Writer im IO-Adapter, Option D; 2D-Maßstabsplan)

**Status:** Accepted

**Datum:** 2026-07-01

**Autor:** Dietmar Burkard (welle-4-austausch, PDF/PNG-Backend-ADR, ausgearbeitet im AI-Harness-Lauf)

**Bezug:** [LH-FA-IO-007](../../../spec/lastenheft.md#lh-fa-io-007) (PDF-Export, maßstäblicher Plan), [LH-FA-IO-008](../../../spec/lastenheft.md#lh-fa-io-008) (PNG-Export), [ACC-004](../../../spec/lastenheft.md#7-abnahmekriterien) (maßstäblicher PDF-Plan erfolgreich), [OBJ-005](../../../spec/lastenheft.md#3-projektziele) (offene Austauschformate — **kontextueller M4-Rahmen**: es zählt IFC/DXF/STEP/STL auf, PDF/PNG sind Ausgabe-Formate → **direkte Bindung** [LH-FA-IO-007](../../../spec/lastenheft.md#lh-fa-io-007)/008 + [ACC-004](../../../spec/lastenheft.md#7-abnahmekriterien)), ADR-0001 (Schichtung — Format-Writer im IO-Adapter), ADR-0002 (OCC = Geometrie-Kern — trägt **keinen** PDF-/PNG-Export, eigene Feststellung s. §Alternativen/Option O), ADR-0004 (Toolchain-/Dependency-Pinning), ADR-0005 (Drittanbieter-Lizenz-Attribution), [ADR-0009](0009-gui-framework-qt6.md) (Qt = Driving-/UI-Schicht, **Regel E** — warum **nicht** genutzt, s. §Alternativen/Option Q), [ADR-0010](0010-headless-gl-xvfb.md) (Headless-GL — warum **nicht** nötig: kein Render-/GL-Pfad), [ADR-0013](0013-ifc-bibliothek.md) (Schwester-ADR IFC; **Option D** = selbst getragener Subset-Codec io-resident — Muster für den analytisch testbaren Format-Writer), [ADR-0014](0014-step-stl-export-backend.md) (Schwester-ADR STEP/STL; **Gegenfolie**: geometrie-resident war dort richtig, **weil** OCC-DataExchange nativ gebraucht wurde — gilt hier **nicht**), [ADR-0015](0015-dxf-backend.md) (Schwester-ADR DXF; **direkte Präzedenz**: 2D-Grundriss-Sicht, io-resident, Option D — dieselbe Datenquelle Wand-Achsen je Geschoss)

---

## Kontext

welle-4-austausch macht b-cad **offen austauschbar** (Meilenstein M4,
[OBJ-005](../../../spec/lastenheft.md#3-projektziele)). **IFC** ([ADR-0013](0013-ifc-bibliothek.md) + slice-019a/b/c), **STEP/STL**
([ADR-0014](0014-step-stl-export-backend.md) + slice-020a/b, 024a/b) und **DXF** ([ADR-0015](0015-dxf-backend.md) + slice-021a/b) sind
entschieden und geliefert. Verbleibend an Austauschformaten dieser Welle: der
**Render-/Plot-Pfad PDF/PNG** — [LH-FA-IO-007](../../../spec/lastenheft.md#lh-fa-io-007) (PDF, **maßstäblicher Plan**,
[ACC-004](../../../spec/lastenheft.md#7-abnahmekriterien)) und [LH-FA-IO-008](../../../spec/lastenheft.md#lh-fa-io-008) (PNG). [ADR-0013](0013-ifc-bibliothek.md)/[0014](0014-step-stl-export-backend.md)/[0015](0015-dxf-backend.md) haben
PDF/PNG je ausdrücklich ausgegliedert: »PDF-/PNG-Export — **Render-/Plot-Pfad,
eigenes ADR**«. **Diese ADR löst genau das ein.** [ACC-004](../../../spec/lastenheft.md#7-abnahmekriterien) (PDF) ist
**milestone-bindend** (M4 = [ACC-003](../../../spec/lastenheft.md#7-abnahmekriterien) + [ACC-004](../../../spec/lastenheft.md#7-abnahmekriterien)); [LH-FA-IO-008](../../../spec/lastenheft.md#lh-fa-io-008) (PNG)
trägt **kein** ACC.

**PDF/PNG sind Ausgabe-/Render-Formate, kein Modell-Austausch.** Anders als IFC,
DXF, STEP und STL (semantisches bzw. geometrisches Modell, **round-trip-fähig**)
sind PDF und PNG **export-only Darstellungen**: PDF ist ein maßstäblicher
2D-Plan (Vektor), PNG ein Rasterbild. Es gibt **keinen Import** — aus einem PDF
oder PNG liest man **kein** Gebäudemodell zurück. Diese Asymmetrie prägt die
ADR: PDF/PNG sind **nur** in der Exporter-Registry (`ExporterMap`), **nie** in
der Importer-Registry; es gibt **keinen** PDF/PNG-Import-Adapter. Der
Export-Fehlercode [`E-IO-001`](../../../spec/spezifikation.md#4-fehler-codes-und-logging-felder) ist der relevante; ein Import-*Request* für
PDF/PNG wird vom `ExchangeService` als **export-only-Lookup-Miss** mit
[`E-IO-003`](../../../spec/spezifikation.md#4-fehler-codes-und-logging-felder) (`event=import_rejected`) abgewiesen — **identisch** zu STEP/STL
(slice-021b-MED-1) —, es entsteht **kein** PDF/PNG-Parse-Pfad (Kontrast IFC/DXF,
die echte Import-Adapter tragen).

**PDF ist 2D-Vektor, PNG ist 2D-Raster — beide dokumentierte, schmale Formate.**
PDF ist ein **Objekt-/Stream-Format** (Header `%PDF-1.x`, nummerierte Objekte,
`xref`-Tabelle, `trailer`, Seitenbaum, ein **Content-Stream** aus
Grafik-Operatoren — `m` moveto, `l` lineto, `S` stroke). PNG ist ein
**Chunk-Format** (8-Byte-Signatur, `IHDR`/`IDAT`/`IEND`, `IDAT` = zlib-gewrappter
DEFLATE-Strom, **CRC-32** je Chunk). Beide bilden **dieselbe 2D-Grundriss-Sicht**
ab wie DXF ([ADR-0015](0015-dxf-backend.md)) — die **Achsen gerader Wände je Geschoss** —, nur
**maßstäblich gerahmt** (PDF) bzw. in ein Pixelraster **gezeichnet** (PNG). Das
ist die fachliche Eigenheit, die diese ADR prägt.

Wie bei IFC/DXF trägt der gewählte Geometrie-Kern **OpenCascade (ADR-0002)
keinen** PDF-/PNG-Export (OCC ist Solid-Modellierung + DataExchange für
STEP/STL/IGES, **kein** Plot-/Raster-Backend). Der Grund, der STEP/STL zu
OCC-nativ machte ([ADR-0014](0014-step-stl-export-backend.md)), gilt für PDF/PNG **nicht**. **Qt (ADR-0009)**
besitzt zwar `QPdfWriter`/`QPainter` (Vektor-PDF) und `QImage` (Raster-PNG), ist
aber die **Driving-/UI-Schicht** — die eigene Abwägung dazu steht unter
§Alternativen/Option Q.

Drei Lösungsfragen, die der Spec-Text nicht entscheidet:

1. **Backend:** selbst getragener PDF-/PNG-Writer (wie IFC/DXF **Option D**), Qt
   nativ (`QPdfWriter`/`QImage`), oder eine externe Render-/PDF-/Raster-
   Bibliothek (libharu, podofo, cairo, libpng)?
2. **Schicht:** io-resident (reine Byte-Serialisierung, kein OCC, kein Qt — wie
   IFC/DXF) oder ui/render-resident (so bräuchte es Qt bzw. den GL-Pfad)?
3. **Repräsentation/2D-Semantik:** Was ist ein **maßstäblicher Plan** ([ACC-004](../../../spec/lastenheft.md#7-abnahmekriterien))
   für ein **3D**-Gebäudemodell — Wand-Achsen je Geschoss (wie DXF) in welchem
   Maßstab/Seitenformat? Und was rastert PNG — den **2D-Plan** oder einen
   **3D-Screenshot**, in welcher Auflösung?

**Nicht offen** (bewusst außerhalb dieser ADR — Scope-Verengung, Präzedenz
[ADR-0013](0013-ifc-bibliothek.md)/[0014](0014-step-stl-export-backend.md)/[0015](0015-dxf-backend.md)):

- **Lastenheft-AK-Schärfung** ([LH-FA-IO-007](../../../spec/lastenheft.md#lh-fa-io-007)/008) bleibt **lösungsfrei**
  ([MR-008](../../../harness/conventions.md#mr-008--lastenheft-schärfung-bleibt-lösungsfrei)) und ist ein **eigener Schärfungs-Slice** nach Accept (Präzedenz
  slice-019a/020a/021a) — diese ADR ist die Lösungsschicht, nicht der
  Anforderungstext.
- **Exakte Maßstab-/Seiten-/Auflösungs-AK** (fester Maßstab 1:100 vs.
  Fit-to-Page; Seitenformat A4/A3; eine Seite/Bild je Geschoss vs. kombiniert;
  Linienstärke; PNG-Auflösung/DPI; Farbraum) legt der **Schärfungs-/Impl-Slice**
  fest, nicht diese Backend-Entscheidung.
- **Exakte Port-Dispatch-Mechanik** (`ExchangeFormat::Pdf`/`Png`, Exporter-
  Verdrahtung im `ExchangeService`/Composition Root) legt der Impl-Slice fest
  (ADR-0001-Kern-Hoheit, Präzedenz [ADR-0013](0013-ifc-bibliothek.md)/[0014](0014-step-stl-export-backend.md)/[0015](0015-dxf-backend.md)).

## Entscheidung

1. **Backend — selbst getragener Vektor-PDF-Writer + Raster-PNG-Writer
   (Option D). Keine neue Abhängigkeit (ADR-0004-konform), kein Qt.** Wie
   [ADR-0013](0013-ifc-bibliothek.md) für IFC und [ADR-0015](0015-dxf-backend.md) für DXF: beide Formate sind dokumentierte,
   schmale Byte-Formate; ein **2D-Subset** ist hand-rollbar in `src/adapters/io/`
   (Muster IFC-SPF-/DXF-Codec):
   - **PDF** — ein minimaler **Vektor**-Writer (`PdfWriter`): `%PDF-1.x`-Header,
     nummerierte Objekte (Katalog, Seitenbaum, je Plan eine `Page`), ein
     **Content-Stream** aus Grafik-Operatoren (`m`/`l`/`S`) für die Planlinien +
     Rahmen, `xref`-Tabelle, `trailer`, `%%EOF`.
   - **PNG** — ein minimaler **Raster**-Writer (`PngWriter`): 8-Byte-Signatur,
     `IHDR`/`IDAT`/`IEND`; `IDAT` als **stored-DEFLATE** (unkomprimierte
     zlib-Blöcke — **kein** zlib-Link) mit **Adler-32**, **CRC-32** je Chunk. Die
     Planlinien werden in einen eigenen RGB(A)-Bitmap-Puffer **gerastert**.
   - **Keine** externe Bibliothek (libharu/podofo/cairo/libpng = neue Dependency
     → ADR-0004-/0005-Konflikt), **kein** OCC (kann kein PDF/PNG), **kein** Qt
     (Driving-/UI-Schicht, **Regel E** — würde einen *driven* Export-Port in die
     GUI-Schicht ziehen, Option Q). Re-Eval auf eine echte Render-/PDF-Lib, wenn
     reicher Plan-Inhalt (Füllung/Schraffur, Bemaßung, Text/Schrift,
     Anti-Aliasing, **komprimiertes** PNG, 3D-Screenshot) verbindliches AK wird.

2. **Schicht — io-resident (`src/adapters/io/`), implementiert
   `ModelExporterPort`** (Kern/Driven, bereits real durch IFC/DXF slice-019c/021b
   und STEP/STL slice-020b). PDF/PNG sind **reine Byte-Serialisierung aus dem
   Domänen-`Building`** — **kein** OCC, **kein** Qt, **kein** GL-Render —, **wie
   IFC/DXF** ([ADR-0013](0013-ifc-bibliothek.md)/[0015](0015-dxf-backend.md), io-resident), **nicht** wie STEP/STL ([ADR-0014](0014-step-stl-export-backend.md),
   geometrie-resident **nur** weil sie OCC-DataExchange brauchten), erst recht
   **nicht** ui-/render-resident.
   - **Keine Service-/Registry-Architektur-Änderung (Kontrast DXF-Import
     slice-021b).** Die **Export**-Seite ist bereits eine **Registry**
     (`ExporterMap`, eingeführt slice-020b) → der PDF- und der PNG-Exporter sind
     je ein weiterer **Map-Eintrag** im Composition Root (`src/main.cpp`), analog
     `--export-dxf`. Der **einzige Kern-Touch** ist die **additive Erweiterung
     des `ExchangeFormat`-Enums** um `Pdf`/`Png` in
     `src/hexagon/ports/driving/exchange_model_port.h` (Driving-Port) — **keine**
     neue Service-/Registry-Logik. Da PDF/PNG **export-only** sind, berühren sie
     die `ImporterMap` **nicht** (kein Import-Dispatch — anders als der
     DXF-Import, der slice-021b die `ImporterMap` **architektonisch** einführte).
   - Der PDF/PNG-Writer ist durch `arch-check` **Regel A** (Kern framework-frei)
     **+ Regel B** (kein Adapter→Adapter) isoliert — **kein** externer Header →
     **„Regel F gegenstandslos"** (ADR-0015-Prägung; das Option-D-Muster
     header-freier Codec stammt aus [ADR-0013](0013-ifc-bibliothek.md)). **Keine neue arch-check-Regel.**

3. **Repräsentation & 2D-Semantik (Backend-Ebene; AK lösungsfrei im Slice).**
   Dieselbe **2D-Grundriss-Sicht** wie DXF ([ADR-0015](0015-dxf-backend.md)) — die **Achsen gerader
   Wände je Geschoss** —, nur maßstäblich gerahmt bzw. gerastert. **Gemeinsame
   Datenquelle** mit dem DXF-Export: `building.storeys` (Geschoss-Trennung) +
   `building.walls` (gerade Wand-Achsen `start`/`end`):
   - **PDF ([LH-FA-IO-007](../../../spec/lastenheft.md#lh-fa-io-007), [ACC-004](../../../spec/lastenheft.md#7-abnahmekriterien)):** **maßstäblicher Plan** — je Geschoss
     die Wand-Achsen als 2D-Vektorlinien, mit **definiertem Maßstab** (Modell-mm →
     PDF-Punkte, 1 pt = 1/72 Zoll) in ein Seiten-Koordinatensystem skaliert, mit
     Rahmen.
   - **PNG ([LH-FA-IO-008](../../../spec/lastenheft.md#lh-fa-io-008)):** **Rasterbild desselben 2D-Plans** (Planlinien
     in eine Bitmap gezeichnet). **Nicht** ein 3D-Viewer-Screenshot — das wäre
     viewer-/GL-resident ([ADR-0009](0009-gui-framework-qt6.md)/[0010](0010-headless-gl-xvfb.md)) und ist eine **benannte Lücke/
     Re-Eval** (§Trigger).
   - **Subset = Achsen-Plan-Teilumfang** — gerade Wand-**Achsen** je Geschoss (wie
     der DXF-Subset, [ADR-0015](0015-dxf-backend.md)), **nicht** gefüllte Wand-**Umrisse mit
     Footprint/Dicke. Das Lastenheft fordert nur „maßstäblicher PDF-Plan"
     ([ACC-004](../../../spec/lastenheft.md#7-abnahmekriterien)) ohne Plan-Reichtum festzulegen; ob der **Achsen**-Plan
     [ACC-004](../../../spec/lastenheft.md#7-abnahmekriterien) erfüllt oder ein **Footprint-/Dicke-Plan** verlangt wird,
     bestätigt der Schärfungs-Slice ([LH-FA-IO-007](../../../spec/lastenheft.md#lh-fa-io-007)). **Wand-Footprint/-Dicke
     ist jetzt out of scope** und ein **AK-abhängiger Re-Eval-Trigger** (§Trigger).
     **Weitere benannte Lücke:** Räume, Bemaßung, Schraffur/Füllflächen,
     Text/Raumstempel, Möblierung, 3D-Ansicht, Schrift — beim Export **nicht
     geschrieben**; PNG **unkomprimiert** (stored-DEFLATE).
   - Die **exakten AK** (Maßstab fest 1:100 vs. Fit-to-Page, Seitenformat,
     eine Seite/Bild je Geschoss vs. kombiniert, Einheiten, Linienstärke,
     PNG-Auflösung/DPI, Farbraum, Totalität leeres Modell → gültige **leere**
     Seite/Bild ohne Absturz, Test-Orakel) gehören in die **Spec-Schärfung**
     ([LH-FA-IO-007](../../../spec/lastenheft.md#lh-fa-io-007)/008, lösungsfrei → Impl-Slice), **nicht** in diese
     Backend-Entscheidung.

4. **Atomarer Export & Fehler-Codes.** Export schreibt **atomar** (Temp + fsync +
   Rename, Muster IFC/STEP/STL/DXF slice-019c/020b/021b); nicht beschreibbarer
   Zielpfad → [`E-IO-001`](../../../spec/spezifikation.md#4-fehler-codes-und-logging-felder) (`event=io_no_permission`), **kein** Teil-Export, der
   Zielpfad bleibt intakt. **Binär-sicher:** PDF/PNG sind **Byte-Ströme** (nicht
   Text) — der etablierte `atomicWrite` schreibt bereits rohe Bytes (`::write`),
   byte-treu. **Kein PDF/PNG-Import-Adapter:** ein Import-*Request* wird vom
   `ExchangeService` als export-only-Lookup-Miss mit [`E-IO-003`](../../../spec/spezifikation.md#4-fehler-codes-und-logging-felder)
   (`event=import_rejected`) abgewiesen — **identisch** zu STEP/STL
   (slice-021b-MED-1) —; es gibt **keinen** PDF/PNG-Parse-Pfad (Kontrast
   IFC/DXF-Import).

5. **Lizenz/Attribution (ADR-0005).** Ein selbst getragener Writer fügt **keine**
   neue Drittanbieter-Lizenz hinzu — **kein** neuer Manifest-Eintrag (exakt wie
   [ADR-0013](0013-ifc-bibliothek.md)/[0015](0015-dxf-backend.md) Option D; anders als ein Bibliotheks-Zukauf es brächte).

## Verglichene Alternativen

### Option D — selbst getragener Vektor-PDF- + Raster-PNG-Writer (gewählt)

- **Pro:** **Null** neue Abhängigkeit (ADR-0004-konform); **io-resident
  schichttreu** wie IFC/DXF (kein OCC, kein Qt, kein GL — reine
  Byte-Serialisierung aus dem Modell); volle Kontrolle über den schmalen
  2D-Plan-Subset; **keine** Lizenz-Neulast (ADR-0005); **analytisch testbar**
  (der Content-Stream / die PNG-Chunks lassen sich **voll decodieren** — inkl. zlib-Inflate + Prüfsummen, Muster
  STEP/STL-Re-Read-Orakel, IFC-Roundtrip); durch **Regel A+B** isoliert; nutzt
  die **bestehende Exporter-Registry** ohne Service-/Registry-Architektur-Änderung (nur additive `ExchangeFormat`-Enum-Erweiterung).
- **Contra:** deckt nur einen **Subset** (gerade Wand-Achsen als 2D-Linien) —
  reicher Plan (Füllung, Bemaßung, Text, Titelblock) und **komprimiertes/
  anti-aliasing-reiches** PNG bleiben aus; PNG ist **unkomprimiert**
  (stored-DEFLATE) → größere Dateien. Als **benannte Lücke + Re-Eval-Trigger**
  geführt (kein stiller Vollumfang).

### Option Q — Qt-nativ (`QPdfWriter`/`QPainter` + `QImage`)

- **Pro:** Qt ist **bereits** Dependency ([ADR-0009](0009-gui-framework-qt6.md)) und **purpose-built** —
  robustes Vektor-PDF (`QPdfWriter`/`QPainter`), Raster-PNG mit Anti-Aliasing +
  **freier Kompression** (`QImage`), Schrift/Text; oberflächlich der [ADR-0014](0014-step-stl-export-backend.md)-
  Analog (»nutze die native Fähigkeit einer schon vorhandenen Dependency«).
- **Contra (entscheidend):** Qt ist die **Driving-/UI-Schicht** — `arch-check`
  **Regel E** begrenzt Qt-Header auf `src/adapters/ui/` + `src/main.cpp`. Ein
  **driven** Export-Port (`ModelExporterPort`) in der **UI**-Schicht ist
  schicht-schief; native Qt-Nutzung bräuchte eine **Regel-E-Lockerung** bzw.
  einen neuen Qt-gebundenen Render-Adapter (ADR-Kosten, [AGENTS.md §2.6](../../../AGENTS.md)).
  **Gegenfolie [ADR-0014](0014-step-stl-export-backend.md):** OCC-DataExchange war **geometrie-resident** (ein
  *driven* Adapter, **Regel C**) → native Nutzung war schichttreu; Qt ist
  *driving/ui* → **nicht** schichttreu für einen *driven* Exporter. PDF/PNG
  brauchen zudem **kein** GL-Render ([ADR-0010](0010-headless-gl-xvfb.md)) — der Plan kommt aus
  Modell-/Plan-Primitiven, **nicht** aus einem GUI-/Framebuffer-Screenshot.
  **Verworfen** (Projektinhaber-Entscheidung 2026-07-01: Exportpfad aus dem
  Modell-/Plan-Primitive-Stream, unnötige GUI-Kopplung vermeiden) — Re-Eval bei
  Bedarf an reicher Darstellung (Trigger).

### Option L — externe Render-/PDF-/Raster-Bibliothek (z. B. libharu, podofo, cairo, libpng)

- **Pro:** deckt reichen PDF-/Raster-Umfang (Füllung, Schrift, Bemaßung,
  komprimiertes PNG, Farbmanagement).
- **Contra:** **neue Schwer-Abhängigkeit** (ADR-0004-Konflikt: vcpkg/Conan/
  Source-Build, kein Snapshot-apt), **Lizenz-Neulast** (ADR-0005). Für den
  schmalen 2D-Plan-Subset **unverhältnismäßig** (gleiche Abwägung wie IFC
  Option A/B in [ADR-0013](0013-ifc-bibliothek.md), DXF Option L in [ADR-0015](0015-dxf-backend.md)). **Verworfen** — Re-Eval
  bei echtem Reichtums-Bedarf.

### Option O — OpenCascade

- OCC (offene Edition) trägt **keinen** PDF-/PNG-Export (Solid + STEP/STL/IGES-
  DataExchange, **kein** Plot-/Raster-Backend) → **disqualifiziert** (wie DXF,
  [ADR-0015](0015-dxf-backend.md) Option O). PDF/PNG sind **nicht** OCC-nativ (Kontrast STEP/STL,
  [ADR-0014](0014-step-stl-export-backend.md)).

### Schicht-Alternative — ui/render-resident (Qt-/GL-Framebuffer-Grab, verworfen)

- Unnötig: PDF/PNG brauchen **kein** OCC und **kein** Qt/GL (reine
  Byte-Serialisierung aus dem Modell). **io-resident** (wie IFC/DXF) ist
  schichttreu **und** einfacher; render-resident wäre nur bei einem echten
  3D-Screenshot richtig (benannte Lücke, §Trigger). Daher **io-resident**
  (Entscheidung #2).

## Konsequenzen

- **Positiv:** PDF/PNG **ohne** Toolchain-Eingriff (self-rolled, ADR-0004),
  **ohne** neue Lizenz-Pflicht (ADR-0005); schichttreu (io-resident wie IFC/DXF,
  Regel A+B); **zwei symmetrisch aufgebaute, analytisch prüfbare Writer**
  (PDF-Content-Stream / PNG-Chunks **voll decodierbar**, inkl. Prüfsummen). Der format-neutrale
  `ExchangeService` (slice-019c/020b/021b, Exporter-Registry) trägt PDF/PNG als
  weitere Formate **ohne** Service-/Registry-Architektur-Änderung (nur additive `ExchangeFormat`-Enum-Erweiterung um `Pdf`/`Png`); **keine** ImporterMap-Berührung
  (export-only). **[ACC-004](../../../spec/lastenheft.md#7-abnahmekriterien) (PDF)** wird erfüllbar → M4-Pfad.
- **Negativ / Folgepflicht (Slice):** **AK-Schärfung** ([LH-FA-IO-007](../../../spec/lastenheft.md#lh-fa-io-007)/008 von
  Outline auf AK, lösungsfrei, [MR-008](../../../harness/conventions.md#mr-008--lastenheft-schärfung-bleibt-lösungsfrei)) + `spezifikation.md` §1-Mapping +
  §6/§7-Nachzug (Präzedenz slice-019a/020a/021a). Dann **Impl:** `PdfWriter` +
  `PngWriter` im IO-Adapter + `PdfExportAdapter`/`PngExportAdapter`
  (`ModelExporterPort`) + `ExchangeFormat::Pdf`/`Png`-Dispatch (nur `ExporterMap`)
  + Composition-Root-Verdrahtung; **AK-Tests** ([`LH-FA-IO-007`](../../../spec/lastenheft.md#lh-fa-io-007)/`008` im Namen,
  inkl. **Maßstabs-Sonde**: bekannte Modell-Kante ergibt die erwartete
  Seiten-/Pixel-Länge) + **Adapter-Pfad-Integrationstest** (Modell → Datei, Datei
  **voll decodieren** [Struktur + Prüfsummen, s. Fitness Function]; [`E-IO-001`](../../../spec/spezifikation.md#4-fehler-codes-und-logging-felder) durch den echten Adapter). **[MR-009](../../../harness/conventions.md#mr-009--geometrielastiges-code-review-vor-welle-closure)** ist voraussichtlich
  **n/a** (keine neue **Solid**-/Bauteil-Geometrie im Kern — 2D-Projektion
  bestehender Wand-Achsen); der Impl-Slice trägt gleichwohl **[MR-006](../../../harness/conventions.md#mr-006--unabhängiges-plan-review-vor-implementierungs-start)** +
  unabhängiges Code-Review (Muster slice-021b) — die genaue Review-Tiefe
  entscheidet der Schärfungs-Slice.
- **Keine neue `arch-check`-Regel nötig** (Option D): der PDF/PNG-Writer hat
  **keinen** externen Header → **Regel A+B** isolieren ihn (`arch-check` grün
  inkl. `io/`), wie [ADR-0015](0015-dxf-backend.md) („Regel F gegenstandslos"; header-freier Codec =
  [ADR-0013](0013-ifc-bibliothek.md)-Muster). Erst eine später
  adoptierte Render-**Bibliothek** oder Qt-Nutzung bräuchte ein Header-Gate
  (analog C/D/E) bzw. eine Regel-E-Entscheidung.
- **Negativ / Risiko (benannt):** der **2D-Plan-Subset** trägt nur gerade
  Wand-Achsen je Geschoss (kein reicher Plan-Inhalt); **PNG ist unkomprimiert**
  (stored-DEFLATE) → große Dateien bei hoher Auflösung; **kein** 3D-Screenshot
  (viewer-resident, benannte Lücke). Fremd-Reader-Interoperabilität hängt am
  Minimal-Subset (PDF 1.x Grundoperatoren; PNG 8-bit RGB, unkomprimiert) — der
  Schärfungs-Slice fixiert die Zielversionen/-parameter.
- **Negativ / Folgepflicht (Spec-Nachzug):** `spec/spezifikation.md` §7 führt die
  **PDF-/PNG**-Backends **als offen** (Rest der Sammelklausel „PDF-/PNG-Backends
  bleiben offen") und §6 hat **keine** PDF-/PNG-Zeile — mit Accept wird das
  **stale**. Im Schärfungs-Slice nachziehen (§7 PDF/PNG chirurgisch entfernen —
  danach ist die Backend-Offen-Klausel **leer**; §6 **zwei** neue Vertragszeilen
  PDF + PNG); zusätzlich die **§4-[`E-IO-001`](../../../spec/spezifikation.md#4-fehler-codes-und-logging-felder)-Bedingung** um „PDF-/PNG-Export"
  erweitern (sie zählt heute IFC-/STEP-/STL-/DXF-Export auf). Präzedenz
  slice-019a/020a/021a. Wird **mit Accept** als Zeile in **ADR-Index
  §ADR-Folgepflichten** gebucht (analog ADR-0013/0014/0015).
- **ADR-0001/0002/0004/0005/0009/0010/0013/0014/0015 bleiben unverändert gültig** —
  diese ADR baut auf ihnen auf (Option-D-Muster von [ADR-0013](0013-ifc-bibliothek.md), 2D-Plan-Datenquelle
  von [ADR-0015](0015-dxf-backend.md), Naht-/Gegenfolie-Argument von [ADR-0014](0014-step-stl-export-backend.md), Regel-E-Grenze von
  [ADR-0009](0009-gui-framework-qt6.md)).

## Fitness Function

| Tooling | Regel | Make-Target |
|---|---|---|
| Schichtung (heute real) | Der PDF/PNG-Writer lebt in `src/adapters/io/`, hat **keinen** externen/OCC-/Qt-Header — **Regel A** (Kern framework-frei) + **Regel B** (kein Adapter→Adapter) isolieren ihn; **Regel C** (OCC nur geometry/) + **Regel E** (Qt nur ui/+main) bleiben unberührt | `make arch-check` (ADR-0001/0009, real) |
| Keine neue Dependency (Impl-Slice) | `make build` zieht **keinen** neuen `find_package`/apt-Eintrag für PDF/PNG (reines C++/STL, wie der IFC-SPF-/DXF-Codec) | `make build` |
| Export-AK (Schärfungs-/Impl-Slice) | PDF-/PNG-Datei entsteht + **vollständiges Struktur-/Decode-Orakel** — **nicht** nur oberflächliches Re-Parse, weil der Writer **selbst getragen** ist: **PDF** = `xref`-Tabelle + `trailer` + Stream-`/Length`-Integrität + Content-Stream-Operatoren (Planlinien-Zahl je Geschoss); **PNG** = **vollständiger zlib-Inflate** der `IDAT` + **Adler-32** + **je-Chunk-CRC-32** + **Scanline-Filterbytes** → Pixel-Rekonstruktion (fängt kaputte xref/Stream-Längen, defekte IDAT-DEFLATE, CRC-/Adler-Fehler, fehlende Filterbytes — die ein `IHDR`-only-Re-Parse durchrutschen ließe) + **Maßstabs-Sonde** (bekannte Kante → erwartete Seiten-/Pixel-Länge). Verschärfung optional über einen **externen Standard-Reader** als Zweit-Orakel, sofern ADR-0004-konform verfügbar. Negative: nicht beschreibbarer Pfad → [`E-IO-001`](../../../spec/spezifikation.md#4-fehler-codes-und-logging-felder), kein Teil-Export | `make test` |
| Integration über den echten Pfad (Schärfungs-/Impl-Slice) | **Mindestens ein** Test übt `ExchangeService` → `ModelExporterPort`-**PDF-/PNG-Adapter** (Datei) + Negative (nicht beschreibbarer Pfad → [`E-IO-001`](../../../spec/spezifikation.md#4-fehler-codes-und-logging-felder) durch den echten Adapter) — welle-3-Lehre slice-015b | `make test` |
| Binary-Smoke (CI-only, Impl-Slice) | `make io-smoke` deckt PDF-/PNG-Export headless (exit 0 + nicht-leere Datei) — erweitert um die zwei neuen export-only-Formate, Muster STEP/STL | `make io-smoke` |

## Re-Evaluierungs-Trigger

- **Reicher PDF-Plan** (Füllflächen/Schraffur, Bemaßung/`DIMENSION`, Text/
  Raumstempel/Titelblock, Schrift-Einbettung, mehrere Ansichten je Seite) über
  den 2D-Achsen-Subset hinaus. **Beobachtbar** (Bar wie [ADR-0013](0013-ifc-bibliothek.md)/[0015](0015-dxf-backend.md), Beleg-
  Artefakt statt vagem „nötig"): ein **realer Abnahme-/Plan-Anspruch, der am AK
  scheitert**, weil ein tragendes Plan-Element fehlt, **oder** eine **AK-Schärfung,
  die eine der benannten Lücken verbindlich fordert**. Adoptiert man dann eine
  Render-/PDF-**Bibliothek**, ist ihre ADR-0004-Konformität **selbst beobachtbar
  zu belegen** (löst im gepinnten Snapshot **ohne** neuen Paketmanager auf —
  `apt-get install -s …`/Probe, spike-001-/[ADR-0014](0014-step-stl-export-backend.md)-Muster); sonst neuer
  apt-Eintrag = **ADR-0004-Berührung** (eigener Beschluss). Alternativ
  Writer-Ausbau im Slice.
- **Anti-Aliasing / komprimiertes / fotografisches PNG** (echte DEFLATE-
  Kompression statt stored-Blöcke; Farbtiefe/Alpha; Größenproblem bei hoher
  Auflösung) → Qt-`QImage` (Option Q, Regel-E-Entscheidung) oder libpng/zlib
  (Option L, ADR-0004-Beleg).
- **3D-Viewer-Screenshot als PNG** (Framebuffer-Grab der 3D-Ansicht statt
  2D-Plan) → **viewer-/render-resident** ([ADR-0009](0009-gui-framework-qt6.md)/[0010](0010-headless-gl-xvfb.md), headless Xvfb) — anderer
  Semantik, eigener Beschluss (nicht dieser io-residente Plan-Pfad).
- **Wand-Footprint/-Dicke im Plan** (gefüllte Wand-**Umrisse mit Dicke** statt
  reiner Achslinien) — falls die [ACC-004](../../../spec/lastenheft.md#7-abnahmekriterien)-Schärfung ([LH-FA-IO-007](../../../spec/lastenheft.md#lh-fa-io-007))
  „maßstäblicher Plan" als **mehr** als den Achsen-Plan festlegt. **Beobachtbar:**
  die AK-Schärfung fordert den Footprint verbindlich, **oder** ein realer Plan
  scheitert am Achsen-Subset. Die Datenquelle (Wand-Dicke + Footprint-Berechnung)
  liegt im Kern **bereits** vor → **reiner Writer-Ausbau** im Slice, **keine**
  ADR-0004-Berührung.
- **Bemaßter/beschrifteter Plan** (Maße, Raumstempel) → überschneidet sich mit
  dem DXF-Reichtum ([ADR-0015](0015-dxf-backend.md) Re-Eval) — gemeinsam betrachten.

## Geschichte

| Datum | Ereignis | Verweis |
|---|---|---|
| 2026-07-01 | Proposed (welle-4-austausch; PDF-/PNG-Backend — selbst getragener Vektor-PDF- + Raster-PNG-Writer **Option D** io-resident, 2D-Maßstabsplan; löst die [ADR-0013](0013-ifc-bibliothek.md)/[0014](0014-step-stl-export-backend.md)/[0015](0015-dxf-backend.md)-Ausgliederung „PDF/PNG = eigenes ADR" ein). Projektinhaber-Richtung: Option D, io-resident, kein Qt/GL-Screenshot | welle-4 / PDF-PNG-Backend-ADR |
| 2026-07-01 | **Unabhängiges Text-Review** (Reviewer ≠ Autor, vor Accept): **0 HIGH**, 1 MED + 2 LOW + 4 INFO eingearbeitet — **MED-1** (Fehlercode-Formulierung an den realen export-only-`ExchangeService` angleichen: ein Import-*Request* für PDF/PNG → export-only-Lookup-Miss [`E-IO-003`](../../../spec/spezifikation.md#4-fehler-codes-und-logging-felder), **identisch** STEP/STL, statt „nicht anwendbar"); **LOW-1** („Regel F gegenstandslos" als [ADR-0015](0015-dxf-backend.md)-Prägung, header-freier Codec = [ADR-0013](0013-ifc-bibliothek.md)-Muster); **LOW-2** ([OBJ-005](../../../spec/lastenheft.md#3-projektziele) kontextueller M4-Rahmen, direkte Bindung [LH-FA-IO-007](../../../spec/lastenheft.md#lh-fa-io-007)/008 + [ACC-004](../../../spec/lastenheft.md#7-abnahmekriterien)); **INFO** (ExporterMap-Provenance slice-020b; [ADR-0010](0010-headless-gl-xvfb.md) in die „bleiben gültig"-Liste). Option-D-Kern (io-resident, kein Qt, ExporterMap-only) + §6/§7/§4-Nachzug-Ansage gegen den realen Code bestätigt. **Accept ausstehend (Projektinhaber)** | [`docs/reviews/2026-07-01-adr-0016-text-review.md`](../../reviews/2026-07-01-adr-0016-text-review.md) |
| 2026-07-01 | **Projektinhaber-Durchsicht** (zweite Runde, vor Accept): 1 MED + 2 LOW eingearbeitet — **MED** (Datei-Orakel zu schwach: bei selbst getragenen Writern verlangt die Fitness Function ein **vollständiges Struktur-/Decode-Orakel** — PDF `xref`/`trailer`/Stream-`/Length` + PNG voll-Inflate/Adler-32/je-Chunk-CRC-32/Scanline-Filterbytes —, **nicht** nur `IHDR`-/Content-Stream-Re-Parse); **LOW-1** („kein Kern-Umbau" präzisiert: die **additive `ExchangeFormat`-Enum-Erweiterung** um `Pdf`/`Png` ist der einzige Kern-Touch, keine Service-/Registry-Architektur-Änderung); **LOW-2** (PDF-Subset explizit als **Achsen-Plan-Teilumfang** benannt; Wand-Footprint/-Dicke out of scope + AK-abhängiger Re-Eval-Trigger). **Accept ausstehend (Projektinhaber)** | [`docs/reviews/2026-07-01-adr-0016-text-review.md`](../../reviews/2026-07-01-adr-0016-text-review.md) §Nachtrag |
| 2026-07-01 | **Accepted** (Projektinhaber) — Text-Review **0 HIGH** + Projektinhaber-Durchsicht (1 MED + 2 LOW), alle eingearbeitet. PDF-/PNG-Backend = **selbst getragener Vektor-PDF- + Raster-PNG-Writer (Option D)** io-resident, 2D-Achsen-Maßstabsplan (gerade Wand-Achsen je Geschoss), **export-only** via `ExporterMap`, kein Qt, keine neue Dependency. Löst die [ADR-0013](0013-ifc-bibliothek.md)/[0014](0014-step-stl-export-backend.md)/[0015](0015-dxf-backend.md)-Ausgliederung „PDF/PNG = eigenes ADR" ein; Folgepflichten (AK-Schärfung [LH-FA-IO-007](../../../spec/lastenheft.md#lh-fa-io-007)/008 + Spec-§1/§6/§7/§4-Nachzug, Impl inkl. additiver `ExchangeFormat`-Enum-Erweiterung + voll-Decode-Test-Orakel) im ADR-Index. **Letzte Format-ADR welle-4** ([ACC-004](../../../spec/lastenheft.md#7-abnahmekriterien)/M4) | welle-4-Buchung |
