# ADR-0014: STEP-/STL-Export-Backend (OCC-DataExchange hinter `ModelExporterPort`, geometrie-resident)

**Status:** Accepted

**Datum:** 2026-06-17

**Autor:** Dietmar Burkard (welle-4-austausch, STEP/STL-Export-Backend-ADR, ausgearbeitet im AI-Harness-Lauf)

**Bezug:** [LH-FA-IO-005](../../../spec/lastenheft.md#lh-fa-io-005) (STEP-Export), [LH-FA-IO-006](../../../spec/lastenheft.md#lh-fa-io-006) (STL-Export), [OBJ-005](../../../spec/lastenheft.md#3-projektziele) (offene Austauschformate), ADR-0001 (Schichtung — OCC bleibt im Geometrie-Adapter, Regel C), ADR-0002 (OCC = Geometrie-Kern; STEP-/Format-Export bewusst ausgegliedert), ADR-0004 (Toolchain-/Dependency-Pinning), ADR-0005 (Drittanbieter-Lizenz-Attribution), ADR-0013 (Schwester-ADR IFC; `ModelExporterPort`-Naht, mit slice-019c real eingeführt)

---

## Kontext

welle-4-austausch macht b-cad **offen austauschbar** (Meilenstein M4,
[OBJ-005](../../../spec/lastenheft.md#3-projektziele)). Der **IFC**-Strang ist mit ADR-0013 + slice-019b/c entschieden und
geliefert (Import + Export). Die nächsten Export-Formate der Welle sind **STEP**
([LH-FA-IO-005](../../../spec/lastenheft.md#lh-fa-io-005)) und **STL** ([LH-FA-IO-006](../../../spec/lastenheft.md#lh-fa-io-006)).

ADR-0002 hat den **STEP-/Format-Export bewusst ausgegliedert**: »gehört hinter
`ModelExporterPort` in `src/adapters/io/` … und damit in eine eigene
**IO/Export-ADR**. Ob ein Exporter OCC nutzt, ist dort zu entscheiden (inkl. der
Adapter-Grenzen-Frage Geometrie↔IO).« **Diese ADR löst genau das ein.**

Anders als IFC sind STEP/STL **OCC-nativ**: der bereits gewählte Geometrie-Kern
**OpenCascade (ADR-0002)** liefert sie über seine **DataExchange-Module**
(STEP über `STEPControl_Writer`, STL über `StlAPI`/`RWStl`) — kein externer
Bibliotheks-Bedarf. Der Grund, warum ADR-0013 zu einem selbst getragenen Codec
(Option D) zwang (OCC kann **kein** IFC), gilt hier **nicht**: für STEP/STL ist
OCC die richtige Antwort.

Drei Lösungsfragen, die der Spec-Text nicht entscheidet:

1. **Backend:** OCC-DataExchange nativ, ein selbst getragener Writer (wie IFC
   Option D) oder eine externe Bibliothek?
2. **Schicht (die ADR-0002-Ausgliederungsfrage):** OCC-Header dürfen **nur** in
   `src/adapters/geometry/` (`arch-check` **Regel C**); der `ModelExporterPort`
   ist Kern-/Driven-Port, und der **IFC**-Exporter (`IfcExportAdapter`,
   slice-019c) lebt in `src/adapters/io/`. **Wo lebt der STEP/STL-Exporter,
   wenn er OCC braucht — ohne Regel C (OCC außerhalb geometry/) oder Regel B
   (Adapter→Adapter) zu brechen?**
3. **Repräsentation/Umfang:** Was wird exportiert — B-Rep-Solids (STEP),
   tesselliertes Netz (STL) — und über welchen Bauteil-Subset?

**Nicht offen** (bewusst außerhalb dieser ADR — Scope-Verengung, Präzedenz
ADR-0002/ADR-0013):

- **DXF-Import/-Export** ([LH-FA-IO-003](../../../spec/lastenheft.md#lh-fa-io-003)/004) — weder OCC- noch IFC-nativ; eigenes ADR.
- **PDF-/PNG-Export** ([LH-FA-IO-007](../../../spec/lastenheft.md#lh-fa-io-007)/008) — Render-/Plot-Pfad, eigenes ADR.
- **Lastenheft-AK-Schärfung** ([LH-FA-IO-005](../../../spec/lastenheft.md#lh-fa-io-005)/006) bleibt **lösungsfrei**
  ([MR-008](../../../harness/conventions.md#mr-008--lastenheft-schärfung-bleibt-lösungsfrei)) und ist ein **eigener Schärfungs-Slice** nach Accept (Präzedenz
  slice-019a für IFC) — diese ADR ist die Lösungsschicht, nicht der
  Anforderungstext. **Exakte Port-Dispatch-Mechanik** (`ExchangeFormat`-
  Erweiterung, Multi-Exporter-Verdrahtung) legt der Impl-Slice fest, nicht diese
  ADR (ADR-0001-Kern-Hoheit, Präzedenz ADR-0013).

## Entscheidung

1. **Backend — OCC-DataExchange, nativ (Option A). Keine neue Abhängigkeit
   (ADR-0004-konform).** STEP wird über `STEPControl_Writer`, STL über
   `StlAPI_Writer`/`RWStl` geschrieben. Die nötigen Toolkits (z. B. `TKDESTEP`
   + `TKDE`/`TKXSBase` für STEP, `TKDESTL`/`TKRWMesh` für STL) sind Teil der
   **bereits installierten OCC-Distribution** (ADR-0002) — sie werden nur
   **zusätzlich gelinkt**, **kein** vcpkg/Conan, **kein** Source-Build, **kein**
   Binary-Cache-Bedarf. Damit entfällt der ADR-0013-Contra-D-Konflikt
   vollständig; den eigenen Writer (Option D der IFC-Frage) brauchen wir hier
   **nicht** — STEP ist ein großes Schema (AP203/214/242), das OCC ausgereift
   liefert; eine Re-Implementierung wäre unverhältnismäßig.

2. **Schicht — der STEP/STL-Exporter lebt in `src/adapters/geometry/`** (dort,
   und nur dort, ist OCC erlaubt, `arch-check` **Regel C**) und **implementiert
   den bereits deklarierten `ModelExporterPort`** (Kern/Driven, `architecture.md`
   §1.2; Header deklariert slice-019b, erste reale Implementierung `IfcExportAdapter`
   slice-019c). Der **Composition Root** verdrahtet je
   `ExchangeFormat` die passende `ModelExporterPort`-**Implementierung**:
   **IFC → io-resident** (`IfcExportAdapter`, slice-019c), **STEP/STL →
   geometrie-resident** (neuer Adapter). Der format-neutrale `ExchangeService`
   (Kern) dispatcht; die Geometrie-Erzeugung (`Building` → OCC-`TopoDS_Shape`)
   nutzt die im Geometrie-Adapter **bereits vorhandene** OCC-Solid-/Mesh-Mechanik
   (Extrusion/Boolean/Tessellation, ADR-0002/0011).

   **Damit ist die ADR-0002-Naht „Geometrie↔IO" entschieden:** **`ModelExporterPort`
   ist die Naht.** ADR-0002 **vermutete** `src/adapters/io/` als Ort, ließ die
   Grenzfrage aber **bewusst offen** (»inkl. der Adapter-Grenzen-Frage Geometrie↔IO«)
   — diese ADR **entscheidet** sie zugunsten `geometry/` (Regel C), **ohne** ADR-0002
   zu widersprechen (dort war es eine offene Frage, keine Festlegung). Ein Port darf
   Implementierungen in **verschiedenen** Adaptern haben; das `ExchangeFormat` wählt
   die Implementierung. So sieht der IO-Adapter
   **kein** OCC (Regel C gewahrt) und **kein** Adapter importiert einen anderen
   (Regel B gewahrt) — der Kern kennt nur den Port. Verworfen wird damit, den
   STEP/STL-Exporter in `io/` zu legen (bräuchte OCC-Header in `io/` → Regel-C-
   Bruch) **oder** `io/` in `geometry/` rufen zu lassen (Regel-B-Bruch).

3. **Repräsentation & Umfang (Backend-Ebene; AK lösungsfrei im Slice).** **STEP**
   schreibt **B-Rep-Solids** (die extrudierten/boolesch geschnittenen Bauteil-
   Solids, wie der Geometrie-Adapter sie baut); **STL** schreibt das
   **tessellierte Dreiecksnetz** (binär als Backend-Default; ob ASCII zusätzlich
   angeboten wird, ist AK → Schärfungs-Slice). Bauteil-
   Subset = die **3D-fähigen** Bauteile (Wände inkl. Öffnungs-Cutouts, Dächer,
   Decken/Fundament, Treppen). Die **exakten AK** (welche Bauteile in welle-4,
   ein-Solid-vs-`Compound`, Einheiten/Toleranz, STEP-Schema AP214 vs. AP242,
   STL-Auflösung, Totalität bei degeneriertem/leerem Modell) gehören präzise in
   die **Spec-Schärfung** ([LH-FA-IO-005](../../../spec/lastenheft.md#lh-fa-io-005)/006, lösungsfrei → Impl-Slice), nicht in
   diese Backend-Entscheidung.

4. **Atomarer Export & Fehler-Codes.** Der Export schreibt **atomar** (Temp +
   Rename, Muster Persistenz [ADR-0003](0003-persistenz-sqlite.md) / IFC-Export slice-019c); nicht
   beschreibbarer Zielpfad → [`E-IO-001`](../../../spec/spezifikation.md#4-fehler-codes-und-logging-felder) (`event=io_no_permission`),
   **kein** Teil-Export, Zielpfad unverändert. Ein OCC-Schreib-/Konvertierungs-
   fehler (degeneriertes/nicht-manifolds Shape) → **neutrale**
   `std::runtime_error`; **kein OCC-Ausnahmetyp verlässt den Adapter** (ADR-0001,
   Muster `OccGeometryAdapter`/[`E-GEO-002`](../../../spec/spezifikation.md#4-fehler-codes-und-logging-felder)). Die genaue Totalität (leeres
   Modell → leere-aber-gültige Datei vs. benannter Fehler) entscheidet der
   Schärfungs-Slice.

5. **Lizenz/Attribution (ADR-0005).** OCC ist **bereits** Abhängigkeit (ADR-0002)
   — STEP/STL fügt **keine** neue Drittanbieter-Lizenz hinzu (nur weitere
   Toolkits derselben OCC-Distribution). **Kein** neuer Manifest-Eintrag (anders
   als ein Bibliotheks-Zukauf es brächte).

## Verglichene Alternativen

### Option A — OCC-DataExchange nativ (gewählt)

- **Pro:** **Null** neue Abhängigkeit → ADR-0004-konform; **ausgereift** (OCCT
  DataExchange ist der CAD-Standard-Pfad); **B-Rep-Treue** für STEP (echte
  Solids, nicht nur Mesh) + Mesh für STL; STEP AP203/214/242 wählbar; STL
  binär/ASCII; **schichttreu** über die `ModelExporterPort`-Naht, gedeckt von
  **bestehender** Regel C; keine Lizenz-Neulast.
- **Contra:** bindet den Export an OCC — aber OCC ist gesetzt (ADR-0002), und
  ein OCC-Wechsel beträfe ohnehin den ganzen Geometrie-Adapter; zusätzliche
  Toolkits erhöhen die Link-Fläche/Image-Größe geringfügig.

### Option B — eigener STEP/STL-Writer (analog IFC Option D)

- **Pro:** keine zusätzlichen OCC-Toolkits; volle Format-Kontrolle (für STL —
  ein simples Format — machbar).
- **Contra:** **STEP** (ISO 10303 AP214/242) ist ein **großes, komplexes
  B-Rep-Schema** — ein eigener STEP-Writer ist **unverhältnismäßig** (anders als
  der schmale IFC-SPF-Subset, der zu Option D führte, weil OCC kein IFC kann).
  OCC liefert STEP geschenkt; es selbst zu bauen ist Verschwendung und
  Wartungs-Senke. **Verworfen.**

### Option C — externe Konvertier-Bibliothek (z. B. assimp)

- **Pro:** deckt viele Formate auf einmal.
- **Contra:** **neue Schwer-Abhängigkeit** (ADR-0004-Konflikt: vcpkg/Conan/
  Source-Build), **Lizenz-Neulast** (ADR-0005), **redundant** zu OCC (das
  STEP/STL bereits kann). **Verworfen.**

### Schicht-Alternative — Exporter in `src/adapters/io/` (verworfen)

- Bräuchte **OCC-Header in `io/`** → **Regel-C-Bruch**; oder `io/` ruft
  `geometry/` → **Regel-B-Bruch**. Beides unvereinbar mit ADR-0001. Daher
  **geometrie-resident** (Entscheidung #2).

## Konsequenzen

- **Positiv:** STEP/STL **ohne** Toolchain-Eingriff (OCC-nativ), **ohne** neue
  Lizenz-Pflicht; schichttreu (ADR-0001, `ModelExporterPort`-Naht, Regel B/C
  gewahrt); **B-Rep-Treue** für STEP. Der Format-neutrale `ExchangeService`
  (slice-019c) trägt einen weiteren Exporter, ohne Kern-Änderung an der
  Port-Semantik.
- **Negativ / Folgepflicht (Slice):** der Impl-Slice **belegt zuerst beobachtbar**
  (Muster spike-001 / ADR-0013-Contra-A), dass die DataExchange-Toolkits im
  gepinnten 26.04-Snapshot **ohne** neuen Paketmanager auflösen — per
  `apt-get install -s libocct-data-exchange-dev` (oder Äquivalent) bzw. CMake-
  `find`-Probe; lösen sie **nicht** auf, ist das ein **neuer apt-Eintrag =
  ADR-0004-Berührung** → eigener Beschluss/Re-Eval, **kein** stiller Zukauf. Dann:
  zusätzliche OCC-Toolkits in `src/adapters/CMakeLists.txt` linken (`TKDESTEP`
  etc.); **neuer geometrie-residenter `ModelExporterPort`-Adapter** (`Building` → OCC-Shapes → STEP/STL,
  atomar); `ExchangeFormat` um `Step`/`Stl` erweitern + `ExchangeService`-
  Dispatch (Multi-Exporter) + Composition-Root-Verdrahtung; AK-Tests mit
  [`LH-FA-IO-005`](../../../spec/lastenheft.md#lh-fa-io-005)/`006` im Namen + **Adapter-Pfad-Integrationstest** (Datei →
  Re-Read/Prüfung; [`E-IO-001`](../../../spec/spezifikation.md#4-fehler-codes-und-logging-felder) durch den echten Adapter).
- **Keine neue `arch-check`-Regel nötig:** der Exporter ist **OCC-Code in
  `geometry/`** — genau dort, wo **Regel C** ihn bereits erlaubt **und**
  einschließt. Sauberer Kontrast zu ADR-0013 (Option D: kein externer Header →
  „Regel F" gegenstandslos); hier trägt die **bestehende Regel C** die Isolation.
- **Negativ / Risiko (benannt):** B-Rep-Export degenerierter/nicht-manifolds
  Solids kann OCC-seitig scheitern → als neutraler Fehler benannt; Totalität im
  Schärfungs-Slice. STEP-Interoperabilität mit Fremd-CAD hängt am gewählten
  AP-Schema (Schärfungs-Slice).
- **Negativ / Folgepflicht (Spec-Nachzug):** `spec/spezifikation.md` §7 führt die
  STEP-/STL-Backends **als offen** (welle-4) und §6 erwähnt OCC-STEP/STL nur
  beiläufig — mit Accept wird das **stale**. Im Schärfungs-Slice nachziehen
  (§7-STEP/STL-Offene-Punkte streichen, §6 auf den entschiedenen Stand: OCC-
  DataExchange nativ), Präzedenz ADR-0013 → slice-019a. Wird **mit Accept** als
  Zeile in **ADR-Index §ADR-Folgepflichten** gebucht (analog ADR-0013).
- **ADR-0001/0002/0004/0005 bleiben unverändert gültig** — diese ADR baut auf
  ihnen auf und löst die ADR-0002-Ausgliederung (STEP-/Format-Export) ein.

## Fitness Function

| Tooling | Regel | Make-Target |
|---|---|---|
| Schichtung (heute real) | OCC-`.hxx` **nur** in `src/adapters/geometry/` — deckt den STEP/STL-Exporter ab (Regel C); kein Adapter importiert einen anderen (Regel B); Kern bindet kein OCC | `make arch-check` (ADR-0001/0002, real) |
| Keine neue Dependency (Impl-Slice) | `make build` linkt nur **OCC-Toolkits derselben Distribution** — kein neuer `find_package`-Zugang außer dem bestehenden `OpenCASCADE` | `make build` |
| Export-AK (Schärfungs-/Impl-Slice) | STEP/STL-Datei entsteht; **Re-Read-Orakel** (OCC `STEPControl_Reader` liest die geschriebene STEP wieder; STL trägt erwartete Dreiecks-/Shape-Anzahl) — Datei→Domain/Shape, analytisch | `make test` |
| Integration über den echten Pfad (Schärfungs-/Impl-Slice) | **Mindestens ein** Test übt `ExchangeService` → `ModelExporterPort`-**Adapter** (Datei) + Negative (nicht beschreibbarer Pfad → [`E-IO-001`](../../../spec/spezifikation.md#4-fehler-codes-und-logging-felder) durch den echten Adapter, kein Teil-Export) — welle-3-Lehre slice-015b | `make test` |

## Re-Evaluierungs-Trigger

- **STEP-Schema-Reichtum** über das Bauteil-Solid hinaus nötig (PMI, Assemblies,
  AP242-Semantik, Material/Farbe via XDE) → Re-Eval (OCC `TKDEXCAF`/XDE kann
  mehr; Erweiterung im Schärfungs-Slice oder Folge-ADR).
- **Export braucht Geometrie, die der Geometrie-Adapter nicht baut** (z. B.
  nicht-prismatische Bauteile) → folgt der Bauteil-Geometrie-Erweiterung, nicht
  dieser ADR.
- **IGES/glTF/OBJ** (ebenfalls OCC-nativ) bei Bedarf → **gleiche Naht**
  (`ModelExporterPort`, geometrie-resident), kleiner Folge-Slice/ADR-Zusatz.
- **DXF** ([LH-FA-IO-003](../../../spec/lastenheft.md#lh-fa-io-003)/004) bleibt ein **eigenes** Backend-ADR (weder OCC-
  noch IFC-nativ) — diese ADR prägt es nicht vor.

## Geschichte

| Datum | Ereignis | Verweis |
|---|---|---|
| 2026-06-17 | Proposed (welle-4-austausch; STEP/STL-Export-Backend — OCC-DataExchange nativ Option A, geometrie-residente `ModelExporterPort`-Naht; löst die ADR-0002-Ausgliederung „STEP-/Format-Export → eigene IO/Export-ADR" ein) | welle-4 / STEP-STL-Export-ADR |
| 2026-06-17 | **Unabhängiges Text-Review** (Reviewer ≠ Autor, vor Accept): **0 HIGH**, 2 MED + 3 LOW + 1 INFO eingearbeitet — MED-1 ADR-0002-`io/`-Vermutung explizit als offene Frage geframed (kein Widerspruch); MED-2 Toolkit-Verfügbarkeit als beobachtbaren Impl-Slice-Beleg verankert (`apt-get install -s`, spike-001-Muster; sonst ADR-0004-Berührung); LOW Port-Provenance (deklariert 019b / Impl 019c), STL-ASCII als AK entschärft, §7-Nachzug firmiert. **Accept ausstehend (Projektinhaber)** | [`docs/reviews/2026-06-17-adr-0014-text-review.md`](../../reviews/2026-06-17-adr-0014-text-review.md) |
| 2026-06-17 | **Accepted** (Projektinhaber) — Text-Review **0 HIGH**, alle MED/LOW eingearbeitet. STEP/STL-Export-Backend = OCC-DataExchange nativ (keine neue Dependency), geometrie-residente `ModelExporterPort`-Naht (Regel C). Damit ist die ADR-0002-Ausgliederung „STEP-/Format-Export → eigene IO/Export-ADR" eingelöst; Folgepflichten (Toolkit-Beleg, Impl-Adapter, `ExchangeFormat`-Dispatch, AK-Schärfung [LH-FA-IO-005](../../../spec/lastenheft.md#lh-fa-io-005)/006, Spec-§6/§7-Nachzug) im ADR-Index | welle-4-Buchung |
