# ADR-0015: DXF-Import/-Export-Backend (selbst getragener DXF-Subset-Codec im IO-Adapter, Option D; 2D-Grundriss)

**Status:** Accepted

**Datum:** 2026-06-17

**Autor:** Dietmar Burkard (welle-4-austausch, DXF-Backend-ADR, ausgearbeitet im AI-Harness-Lauf)

**Bezug:** [LH-FA-IO-003](../../../spec/lastenheft.md#lh-fa-io-003) (DXF-Import), [LH-FA-IO-004](../../../spec/lastenheft.md#lh-fa-io-004) (DXF-Export), [OBJ-005](../../../spec/lastenheft.md#3-projektziele) (offene Austauschformate), ADR-0001 (Schichtung — Format-Codec im IO-Adapter), ADR-0002 (OCC = Geometrie-Kern — trägt **keinen** DXF-Translator, eigene Feststellung s. §Alternativen/Option O), ADR-0004 (Toolchain-/Dependency-Pinning), ADR-0005 (Drittanbieter-Lizenz-Attribution), [ADR-0013](0013-ifc-bibliothek.md) (Schwester-ADR IFC; **Option D** = selbst getragener Subset-Codec io-resident, `ModelImporterPort`/`ModelExporterPort`, „Regel F gegenstandslos" — direkte Präzedenz), [ADR-0014](0014-step-stl-export-backend.md) (Schwester-ADR STEP/STL; benennt DXF dort als „weder OCC- noch IFC-nativ → eigenes ADR")

---

## Kontext

welle-4-austausch macht b-cad **offen austauschbar** (Meilenstein M4,
[OBJ-005](../../../spec/lastenheft.md#3-projektziele)). **IFC** ([ADR-0013](0013-ifc-bibliothek.md) + slice-019a/b/c) und **STEP/STL**
([ADR-0014](0014-step-stl-export-backend.md) + slice-020a/b) sind entschieden und geliefert. Verbleibend an
Austauschformaten dieser Welle (neben dem Render-Pfad PDF/PNG): **DXF** —
[LH-FA-IO-003](../../../spec/lastenheft.md#lh-fa-io-003) (Import) und [LH-FA-IO-004](../../../spec/lastenheft.md#lh-fa-io-004) (Export). [ADR-0014](0014-step-stl-export-backend.md) hat DXF
ausdrücklich ausgegliedert: »DXF-Import/-Export — **weder OCC- noch IFC-nativ;
eigenes ADR**«. **Diese ADR löst genau das ein.**

**DXF ist 2D.** Das *Drawing Exchange Format* (AutoCAD) ist ein **ASCII-getaggtes
Gruppencode-Format** (Paare aus Gruppencode + Wert; Sektionen `HEADER`/`TABLES`/
`BLOCKS`/`ENTITIES`/`OBJECTS`). Anders als IFC (3D-Bauteilmodell) und STEP/STL
(3D-Solid/Mesh) bildet DXF die **2D-Grundriss-Sicht** eines Modells ab — das ist
die fachliche Eigenheit, die diese ADR prägt.

Wie bei IFC kann der gewählte Geometrie-Kern **OpenCascade (ADR-0002) kein DXF**:
die offene OCCT-Edition trägt **keinen** DXF-Translator (DXF/DWG sind ODA-/
kommerziell). Der Grund, der STEP/STL zu OCC-nativ machte ([ADR-0014](0014-step-stl-export-backend.md)), gilt für
DXF **nicht** — DXF ist hier der **zweite IFC-artige Fall**.

Drei Lösungsfragen, die der Spec-Text nicht entscheidet:

1. **Backend:** selbst getragener DXF-Codec (wie IFC **Option D**), eine externe
   DXF-Bibliothek (libdxfrw/dxflib), oder OCC (kann nicht)?
2. **Schicht:** io-resident (reiner Text, kein OCC — wie IFC) oder geometrie-
   resident (so brauchten es STEP/STL **nur** wegen OCC)?
3. **Repräsentation/2D-Semantik:** Was ist DXF für ein **3D**-Gebäudemodell —
   Grundriss je Geschoss (Wand-Achsen als `LINE`/`LWPOLYLINE`), welcher Subset?
   Und beim **Import**: 2D-Linien → Wand-Achsen mit welcher Höhe/Dicke (die DXF
   nicht trägt)?

**Nicht offen** (bewusst außerhalb dieser ADR — Scope-Verengung, Präzedenz
ADR-0013/0014):

- **PDF-/PNG-Export** ([LH-FA-IO-007](../../../spec/lastenheft.md#lh-fa-io-007)/008) — Render-/Plot-Pfad, eigenes ADR.
- **Lastenheft-AK-Schärfung** ([LH-FA-IO-003](../../../spec/lastenheft.md#lh-fa-io-003)/004) bleibt **lösungsfrei**
  ([MR-008](../../../harness/conventions.md#mr-008--lastenheft-schärfung-bleibt-lösungsfrei)) und ist ein **eigener Schärfungs-Slice** nach Accept (Präzedenz
  slice-019a/020a) — diese ADR ist die Lösungsschicht, nicht der Anforderungstext.
- **Exakte Port-Dispatch-Mechanik** (`ExchangeFormat::Dxf`, Importer-/Exporter-
  Verdrahtung im `ExchangeService`) legt der Impl-Slice fest, nicht diese ADR
  (ADR-0001-Kern-Hoheit, Präzedenz [ADR-0013](0013-ifc-bibliothek.md)/[ADR-0014](0014-step-stl-export-backend.md)).

## Entscheidung

1. **Backend — selbst getragener DXF-Subset-Codec (Option D). Keine neue
   Abhängigkeit (ADR-0004-konform).** Wie [ADR-0013](0013-ifc-bibliothek.md) für IFC: DXF-ASCII ist ein
   dokumentiertes, schmales Gruppencode-Format; ein **2D-Subset** (`ENTITIES`
   `LINE` + `LWPOLYLINE`, minimaler `HEADER`, `LAYER`-Tabelle) ist hand-rollbar als
   **symmetrischer Reader + Writer** (Muster IFC-SPF-Codec, `adapters/io/`). **Keine**
   externe DXF-Lib (libdxfrw/dxflib = neue Dependency → ADR-0004-/0005-Konflikt),
   **kein** OCC (kann kein DXF). Re-Eval auf eine echte DXF-Bibliothek, wenn voller
   DXF-Reichtum (BLOCKS, SPLINE, HATCH, DIMENSION, DWG) nötig **und** ADR-0004-konform
   installierbar.

2. **Schicht — io-resident (`src/adapters/io/`), implementiert `ModelImporterPort`/
   `ModelExporterPort`** (Kern/Driven, bereits deklariert; reale Implementierungen
   IFC slice-019b/c). DXF ist **reiner ASCII-Text, kein OCC, kein Geometrie-Kern** —
   **wie IFC** ([ADR-0013](0013-ifc-bibliothek.md), io-resident), **nicht** wie STEP/STL ([ADR-0014](0014-step-stl-export-backend.md),
   geometrie-resident **nur** weil sie OCC brauchten). Der **Composition Root**
   verdrahtet je `ExchangeFormat::Dxf` den DXF-Adapter; der format-neutrale
   `ExchangeService` (Kern) dispatcht. **Asymmetrie der beiden Richtungen
   (Impl-Slice-Arbeit):** die **Export**-Seite ist bereits ein Registry
   (`ExporterMap`, slice-020b) → der DXF-Exporter ist ein weiterer Map-Eintrag, ohne
   Kern-Änderung. Die **Import**-Seite dispatcht heute **einen einzigen**, fest
   verdrahteten (IFC-)`ModelImporterPort` per `switch`; DXF-Import braucht daher eine
   **Kern-Erweiterung des Import-Dispatch** (Importer-Registry analog `ExporterMap`
   oder zweite Referenz) — die **exakte Form legt der Impl-Slice fest**
   (ADR-0001-Kern-Hoheit), nicht diese ADR. Der DXF-Codec selbst ist durch `arch-check`
   **Regel A+B** isoliert (kein externer Header → **„Regel F gegenstandslos"**, exakt
   wie [ADR-0013](0013-ifc-bibliothek.md)/Option D).

3. **Repräsentation & 2D-Semantik (Backend-Ebene; AK lösungsfrei im Slice).** DXF
   bildet die **2D-Grundriss-Sicht** ab:
   - **Export ([LH-FA-IO-004](../../../spec/lastenheft.md#lh-fa-io-004)):** je Geschoss die **Achsen gerader Wände** als
     2D-`LINE`/`LWPOLYLINE` in der `ENTITIES`-Sektion, **nach Geschoss auf DXF-
     `LAYER`** getrennt.
   - **Import ([LH-FA-IO-003](../../../spec/lastenheft.md#lh-fa-io-003)):** 2D-`LINE`/`LWPOLYLINE` → **gerade Wand-Achsen**.
     Da DXF **keine Höhe/Dicke** trägt, bekommen importierte Wände **Default-Höhe/
     -Dicke** (benannte Lücke — analog zur IFC-Import-Geschoss-Höhe, die slice-019b
     defaultete).
   - **Subset** = **gerade Wände als 2D-Achsen + Geschoss-Layer**. **Benannte Lücke:**
     Räume, Bemaßung (`DIMENSION`), Schraffur (`HATCH`), Blöcke (`BLOCK`/`INSERT`),
     Text (`TEXT`/`MTEXT`), Bögen/Kreise (`ARC`/`CIRCLE`), 3D-Entitäten, beliebige
     Geometrie — beim Import **übersprungen**, beim Export **nicht geschrieben**.
   - Die **exakten AK** (Achse vs. Footprint-Polygon, Layer-Schema/-Benennung,
     Einheiten/`$INSUNITS`, welche `ENTITIES`, Import-Defaults für Höhe/Dicke,
     Roundtrip-Treue, Totalität) gehören in die **Spec-Schärfung** ([LH-FA-IO-003](../../../spec/lastenheft.md#lh-fa-io-003)/
     004, lösungsfrei → Impl-Slice), nicht in diese Backend-Entscheidung.

4. **Atomarer Export, atomarer Import & Fehler-Codes.** Export schreibt **atomar**
   (Temp + Rename, Muster IFC/STEP/STL slice-019c/020b); nicht beschreibbarer
   Zielpfad → [`E-IO-001`](../../../spec/spezifikation.md#4-fehler-codes-und-logging-felder) (`event=io_no_permission`), **kein** Teil-Export. **Import**
   ist **atomar** (vollständiges In-Memory-Modell oder Wurf): kaputte/nicht-DXF-Datei
   → [`E-IO-003`](../../../spec/spezifikation.md#4-fehler-codes-und-logging-felder) (`event=import_rejected`), **kein** Teil-Import (Muster IFC-Import
   slice-019b). Die genaue Totalität (leere/strukturlose DXF → leeres Modell, kein
   Wurf) entscheidet der Schärfungs-Slice.

5. **Lizenz/Attribution (ADR-0005).** Ein selbst getragener Codec fügt **keine** neue
   Drittanbieter-Lizenz hinzu — **kein** neuer Manifest-Eintrag (exakt wie
   [ADR-0013](0013-ifc-bibliothek.md)/Option D; anders als ein Bibliotheks-Zukauf es brächte).

## Verglichene Alternativen

### Option D — selbst getragener DXF-Subset-Codec (gewählt)

- **Pro:** **Null** neue Abhängigkeit (ADR-0004-konform); **io-resident schichttreu**
  wie IFC (kein OCC nötig, reiner Text); volle Kontrolle über den schmalen
  2D-Grundriss-Subset; **keine** Lizenz-Neulast (ADR-0005); **symmetrisch** Lesen +
  Schreiben (Muster IFC-SPF, gut testbar per Roundtrip); durch **Regel A+B** isoliert.
- **Contra:** deckt nur einen **Subset** (gerade Wände als 2D-Linien) — voller
  DXF-Reichtum (Blöcke, Schraffur, Bögen) bleibt aus; als **benannte Lücke +
  Re-Eval-Trigger** geführt (kein stiller Vollumfang).

### Option L — externe DXF-Bibliothek (z. B. libdxfrw, dxflib)

- **Pro:** deckt mehr DXF-Entitäten/-Versionen (Blöcke, Bögen, DWG bei manchen).
- **Contra:** **neue Schwer-Abhängigkeit** (ADR-0004-Konflikt: vcpkg/Conan/Source-
  Build, kein Snapshot-apt), **Lizenz-Neulast** (ADR-0005). Für den schmalen
  2D-Grundriss-Subset **unverhältnismäßig** (gleiche Abwägung wie IFC Option A/B in
  [ADR-0013](0013-ifc-bibliothek.md)). **Verworfen** — Re-Eval bei echtem DXF-Reichtums-Bedarf.

### Option O — OpenCascade

- OCC (offene Edition) trägt **keinen** DXF-Translator → **disqualifiziert** (wie IFC,
  [ADR-0013](0013-ifc-bibliothek.md) §Alternativen). DXF ist **nicht** OCC-nativ (Kontrast STEP/STL, [ADR-0014](0014-step-stl-export-backend.md)).

### Schicht-Alternative — geometrie-resident (verworfen)

- Unnötig: DXF braucht **kein** OCC (reiner ASCII-Text). **io-resident** (wie IFC) ist
  schichttreu **und** einfacher; geometrie-resident wäre nur bei OCC-Bedarf richtig
  (so bei STEP/STL, [ADR-0014](0014-step-stl-export-backend.md)). Daher **io-resident** (Entscheidung #2).

## Konsequenzen

- **Positiv:** DXF **ohne** Toolchain-Eingriff (self-rolled, ADR-0004), **ohne** neue
  Lizenz-Pflicht (ADR-0005); schichttreu (io-resident wie IFC, Regel A+B); ein
  **symmetrischer Reader+Writer** (Roundtrip-prüfbar, Muster IFC-SPF). Der format-
  neutrale `ExchangeService` (slice-019c/020b, Exporter-Registry) trägt DXF als
  weiteres Format **ohne** Kern-Semantik-Änderung; die Importer-Seite ergänzt DXF
  analog zum IFC-Importer.
- **Negativ / Folgepflicht (Slice):** **AK-Schärfung** ([LH-FA-IO-003](../../../spec/lastenheft.md#lh-fa-io-003)/004 von Outline
  auf AK, lösungsfrei, [MR-008](../../../harness/conventions.md#mr-008--lastenheft-schärfung-bleibt-lösungsfrei)) + `spezifikation.md` §1-Mapping + §6/§7-Nachzug
  (Präzedenz slice-019a/020a). Dann **Impl:** DXF-Subset-Codec (Reader+Writer) im
  IO-Adapter + `ModelImporterPort`/`ModelExporterPort` + `ExchangeFormat::Dxf`-
  Dispatch + Composition-Root-Verdrahtung; **AK-/Roundtrip-Tests** ([`LH-FA-IO-003`](../../../spec/lastenheft.md#lh-fa-io-003)/
  `004` im Namen) + **Adapter-Pfad-Integrationstest** (Datei → Domain → Datei;
  [`E-IO-001`](../../../spec/spezifikation.md#4-fehler-codes-und-logging-felder)/[`E-IO-003`](../../../spec/spezifikation.md#4-fehler-codes-und-logging-felder) durch den echten Adapter).
- **Keine neue `arch-check`-Regel nötig** (Option D): der DXF-Codec hat **keinen**
  externen Header → **Regel A+B** isolieren ihn (`arch-check` grün inkl. `io/`), wie
  bei [ADR-0013](0013-ifc-bibliothek.md) („Regel F gegenstandslos"). Erst eine später adoptierte DXF-
  **Bibliothek** bräuchte ein Header-Gate analog C/D/E.
- **Negativ / Risiko (benannt):** der **2D-Grundriss verliert die 3D-Information**
  (Höhe/Dicke/Geschoss-Stapelung); der DXF-Import kann sie **nicht** rekonstruieren →
  **Default-Höhe/-Dicke** (benannte Lücke). DXF-Interoperabilität mit Fremd-CAD hängt
  am Subset (gerade `LINE`/`LWPOLYLINE`); reichhaltige DXF-Dateien werden nur teilweise
  importiert (Subset-Skip, **kein** stiller Vollumfang). DXF-Versionen (R12/2000/…)
  differieren — der Schärfungs-Slice fixiert die Ziel-Version.
- **Negativ / Folgepflicht (Spec-Nachzug):** `spec/spezifikation.md` §7 führt die
  **DXF**-Backends **als offen** (in der Sammelklausel „DXF-/PDF-/PNG-Backends bleiben
  offen") und §6 hat **keine** DXF-Zeile — mit Accept wird das **stale**. Im
  Schärfungs-Slice nachziehen (§7 DXF chirurgisch aus der Klausel entfernen — PDF/PNG
  bleiben offen; §6 neue DXF-Vertragszeile); zusätzlich die **§4-[`E-IO-001`](../../../spec/spezifikation.md#4-fehler-codes-und-logging-felder)-
  Bedingung** um „DXF-Export" erweitern (sie zählt heute nur „IFC-Export" auf — STEP/STL
  trägt dieselbe latente Lücke, parallel mitziehen). Präzedenz slice-019a/020a. Wird **mit
  Accept** als Zeile in **ADR-Index §ADR-Folgepflichten** gebucht (analog ADR-0013/0014).
- **ADR-0001/0002/0004/0005/0013/0014 bleiben unverändert gültig** — diese ADR baut auf
  ihnen auf (Option-D-Muster von [ADR-0013](0013-ifc-bibliothek.md), Naht-Muster von [ADR-0014](0014-step-stl-export-backend.md)).

## Fitness Function

| Tooling | Regel | Make-Target |
|---|---|---|
| Schichtung (heute real) | Der DXF-Codec lebt in `src/adapters/io/`, hat **keinen** externen/OCC-Header — **Regel A** (Kern framework-frei) + **Regel B** (kein Adapter→Adapter) isolieren ihn; **Regel C** (OCC nur geometry/) bleibt unberührt | `make arch-check` (ADR-0001, real) |
| Keine neue Dependency (Impl-Slice) | `make build` zieht **keinen** neuen `find_package`/apt-Eintrag für DXF (reines C++/STL, wie der IFC-SPF-Codec) | `make build` |
| Export-/Import-AK (Schärfungs-/Impl-Slice) | DXF-Datei entsteht; **Roundtrip-Orakel** (Export → DXF-Import erhält die **Wand-Achsen-Anzahl** je Geschoss); Negative: nicht-DXF/kaputt → [`E-IO-003`](../../../spec/spezifikation.md#4-fehler-codes-und-logging-felder), kein Teil-Import | `make test` |
| Integration über den echten Pfad (Schärfungs-/Impl-Slice) | **Mindestens ein** Test übt `ExchangeService` → `ModelImporterPort`/`ModelExporterPort`-**DXF-Adapter** (Datei) + Negative (nicht beschreibbarer Pfad → [`E-IO-001`](../../../spec/spezifikation.md#4-fehler-codes-und-logging-felder) durch den echten Adapter) — welle-3-Lehre slice-015b | `make test` |

## Re-Evaluierungs-Trigger

- **Voller DXF-Reichtum** (Blöcke/`INSERT`, `HATCH`, `DIMENSION`, `SPLINE`, `ARC`/
  `CIRCLE`, Layer-Stile) über den 2D-Linien-Subset hinaus. **Beobachtbar** (Bar wie
  [ADR-0013](0013-ifc-bibliothek.md)-Re-Eval, Beleg-Artefakt statt vagem „nötig"): eine **reale DXF-Datei,
  die am AK scheitert**, weil eine tragende Entität nicht unterstützt ist, **oder**
  eine **AK-Schärfung, die eine der benannten Lücken verbindlich fordert**. Adoptiert
  man dann eine DXF-**Bibliothek**, ist ihre ADR-0004-Konformität **selbst beobachtbar
  zu belegen** (löst im gepinnten Snapshot **ohne** neuen Paketmanager auf —
  `apt-get install -s …`/Probe, spike-001-/[ADR-0014](0014-step-stl-export-backend.md)-Muster); sonst neuer apt-Eintrag =
  **ADR-0004-Berührung** (eigener Beschluss). Alternativ Codec-Ausbau im Slice.
- **DWG** (binär, nicht ASCII-DXF) → eigener Beschluss (ODA-/Lizenz-Frage).
- **3D-DXF** (`3DFACE`, `POLYLINE`-Mesh, Z-Koordinaten) statt 2D-Grundriss → folgt der
  Bedarfs-Klärung, nicht dieser ADR.
- **Bemaßter/beschrifteter Plan** (Maße, Raumstempel) → überschneidet sich mit dem
  PDF-/Plot-Pfad ([LH-FA-IO-007](../../../spec/lastenheft.md#lh-fa-io-007), eigenes ADR) — gemeinsam betrachten.

## Geschichte

| Datum | Ereignis | Verweis |
|---|---|---|
| 2026-06-17 | Proposed (welle-4-austausch; DXF-Backend — selbst getragener DXF-Subset-Codec **Option D** io-resident, 2D-Grundriss; löst die [ADR-0014](0014-step-stl-export-backend.md)-Ausgliederung „DXF = eigenes ADR" ein) | welle-4 / DXF-Backend-ADR |
| 2026-06-17 | **Unabhängiges Text-Review** (Reviewer ≠ Autor, vor Accept): **0 HIGH**, 2 MED + 4 LOW + 2 INFO eingearbeitet — **MED-1** (Import-Dispatch ist **kein** Registry: ein einzelner fest verdrahteter IFC-`ModelImporterPort` per `switch` → DXF-Import = Kern-Erweiterung des Import-Dispatch, Form legt der Impl-Slice fest); **MED-2** (Re-Eval-Trigger beobachtbar: reale-DXF-scheitert-am-AK / AK-Schärfung als Beleg-Artefakt + DXF-Lib-Adoption ADR-0004-beobachtbar belegen, Muster ADR-0013-MED-2/spike-001); **LOW-1** (»kann kein DXF« als eigene Feststellung, nicht ADR-0002 zugeschrieben); **LOW-2** (§4-[`E-IO-001`](../../../spec/spezifikation.md#4-fehler-codes-und-logging-felder)-Bedingung um DXF-Export erweitern, STEP/STL-Parität). Option-D-Entscheidung, „Regel F gegenstandslos"-Wiederverwendung (arch-check grün über header-freien io/-Codec belegt) und §6/§7-Chirurgie bestätigt. **Accept ausstehend (Projektinhaber)** | [`docs/reviews/2026-06-17-adr-0015-text-review.md`](../../reviews/2026-06-17-adr-0015-text-review.md) |
| 2026-06-17 | **Accepted** (Projektinhaber) — Text-Review **0 HIGH**, alle MED/LOW eingearbeitet. DXF-Backend = **selbst getragener DXF-Subset-Codec (Option D)** io-resident, 2D-Grundriss (gerade Wand-Achsen je Geschoss-`LAYER`), Import → Default-Höhe/-Dicke (benannte Lücke). Löst die [ADR-0014](0014-step-stl-export-backend.md)-Ausgliederung „DXF = eigenes ADR" ein; Folgepflichten (AK-Schärfung [LH-FA-IO-003](../../../spec/lastenheft.md#lh-fa-io-003)/004 + Spec-§1/§6/§7/§4-Nachzug, Impl inkl. Import-Dispatch-Kern-Erweiterung) im ADR-Index | welle-4-Buchung |
