---
id: slice-044a
titel: Golden files — Export-Golden (6) + Infrastruktur + STEP-Header-Fix (die 044a-Naht)
status: done
welle: welle-5-erweiterung
lastenheft_refs: [[LH-FA-IO-001](../../../../spec/lastenheft.md#lh-fa-io-001--ifc-import), [LH-FA-IO-006](../../../../spec/lastenheft.md#lh-fa-io-006)]
adr_refs: [[ADR-0013](../../adr/0013-ifc-bibliothek.md), [ADR-0014](../../adr/0014-step-stl-export-backend.md), [ADR-0015](../../adr/0015-dxf-backend.md), [ADR-0016](../../adr/0016-pdf-png-backend.md), [ADR-0006](../../adr/0006-relationales-schema-design.md), [ADR-0004](../../adr/0004-toolchain-dependency-pinning.md)]
---

# Slice 044: Golden files für alle Austausch-Formate

**Status:** done (2026-07-24 — implementiert + [MR-009](../../../../harness/conventions.md#mr-009--geometrielastiges-code-review-vor-welle-closure) 0 HIGH; `make gates` grün). Der **[MR-006](../../../../harness/conventions.md#mr-006--unabhängiges-plan-review-vor-implementierungs-start)-Plan-Review 2026-07-23** ([Report](../../../reviews/2026-07-23-slice-044-plan.md)) ging mit **0 HIGH / 2 MED / 3 LOW / 2 INFO** durch (Split empfohlen; MED-1 [IFC-[`E-IO-003`](../../../../spec/spezifikation.md#4-fehler-codes-und-logging-felder)], MED-2 [dedizierter Generator + geteilte `goldenModel()`-TU + writable-Mount], LOW-1/2/3, INFO eingearbeitet).

> **Split ausgeführt ([MR-020](../../../../harness/conventions.md#mr-020--adr-folgepflicht-sichtbarkeit-closure-disziplin)(3)):** Dieser Plan ist die **044a-Naht** — **Export-Golden (alle 6) + Infrastruktur + STEP-Header-Fix**, voll in-Repo/deterministisch. Die **Import-Golden-fremd-Naht (IFC/DXF)** ist als eigener Plan **[`slice-044b`](../open/slice-044b-golden-import-fremd.md)** (in `open/`) ausgegliedert — sie hängt an externer Datei-Beschaffung + Kuratierung je MED-1 und nutzt die 044a-Infrastruktur. Die DoD-Abschnitte unten sind entsprechend markiert: **„Import-Golden fremd" gehört zu 044b** (hier nicht umgesetzt).

**Welle:** welle-5-erweiterung. **QA-/Test-Infrastruktur-Slice** (Quergewerk-nah, Muster slice-022 io-smoke) — mit
**einer** produktionscode-Änderung (STEP-Header-Fixierung, s. DoD). **Vorgänger:** slice-043 (done); baut auf den
sechs Austausch-Adaptern (welle-4) + der `schema-regen`/`schema-check`-Blaupause auf.

**Autor:** Dietmar Burkard (AI-Harness-Lauf). **Datum:** 2026-07-23.

**Bezug:** die vier Backend-ADRs [ADR-0013](../../adr/0013-ifc-bibliothek.md) (IFC),
[ADR-0014](../../adr/0014-step-stl-export-backend.md) (STEP/STL), [ADR-0015](../../adr/0015-dxf-backend.md) (DXF),
[ADR-0016](../../adr/0016-pdf-png-backend.md) (PDF/PNG); [ADR-0006](../../adr/0006-relationales-schema-design.md)
(die `schema-regen`/`schema-check`-Präzedenz für „generiertes committetes Artefakt + Drift-Gate");
[ADR-0004](../../adr/0004-toolchain-dependency-pinning.md) (gepinnte Toolchain — der OCC-Versions-Anker für
STEP/STL). Keine Accepted-ADR wird geändert ([MR-016](../../../../harness/conventions.md)).

---

## 0. Sizing / Split ([MR-006](../../../../harness/conventions.md#mr-006--unabhängiges-plan-review-vor-implementierungs-start): Split **dringend empfohlen**)

Der Projektinhaber wählte **„Beides (voll)"** — Export-Golden (6) **+** Import-Golden-fremd (IFC/DXF). Das
Plan-Review empfiehlt den Split **nachdrücklich**: die **Export-Golden + Infrastruktur + STEP-Fix** (die
**044a-Naht**) sind vollständig **in-Repo, deterministisch, selbst-erzeugt → jetzt startbar**; die
**Import-Golden-fremd** (die **044b-Naht**) hängen an **externer Datei-Beschaffung + Inhalts-Vetting +
Provenance** und brauchen die **IFC-Fixture-Kuratierung je MED-1** (jede Wand mit 'Axis'-Polyline + Containment,
sonst [`E-IO-003`](../../../../spec/spezifikation.md#4-fehler-codes-und-logging-felder)). **Empfohlener Weg:** **044a jetzt** implementieren (dieser Plan, DoD-Abschnitte „Infrastruktur" +
„STEP-Header-Fixierung" + „Export-Golden"); die DoD-Abschnitt **„Import-Golden fremd"** wird als **slice-044b**
(eigener Plan, nach 044a; nutzt die 044a-Infrastruktur) umgesetzt. Der Schnitt A|B fällt sauber an dieser
DoD-Überschrift. **„Beides" bleibt erfüllt — über zwei sequenzielle Slices statt einem.**

## 1. Ziel

Ergänze zu den vorhandenen **Decode-/Struktur-Orakeln** (die Semantik prüfen, aber Byte-Reihenfolge/Formatierung
tolerieren) ein **Byte-genaues Golden-file-Netz**, das **Encoder-Drift bei unveränderter Semantik** fängt (die
Klasse, die die Struktur-Orakel durchlassen):
- **Export-Golden (alle 6):** eine committete Referenz-Ausgabe eines **festen** Test-Modells je Format; ein Test
  re-exportiert über den echten `ExchangeService` und **vergleicht byte-genau** gegen die Referenz.
- **Import-Golden fremd (nur IFC + DXF — die einzigen Import-Formate):** committete, **fremd-erzeugte** Dateien
  (anderer Codec/Tool) als Eingabe; ein Test importiert und prüft das Ergebnis-Modell → **Fremd-Codec-Konformanz**
  (belegt, dass b-cad *anderer Tools* Bytes liest, nicht nur seinen eigenen Roundtrip).

**Nicht-Ziel:** STEP/STL/PDF/PNG haben **keinen Import** in b-cad → **kein** Fremd-Import-Golden für sie (nur
Export-Golden). Cross-Tool-Viewer-Validierung (unseren Export in FreeCAD öffnen) ist **nicht** Teil (kein
Golden-file, bräuchte Fremd-Parser im Container).

## 2. Determinismus-Grundlage (aus der Code-Recon — der Kern-Prüfstein)

| Format | Klasse | Deterministisch? | Nicht-Determinismus-Quelle | Golden-Strategie |
|---|---|---|---|---|
| **IFC** | Text (SPF) | ✅ ja | keine (`ifc_spf_writer` Sentinel-TS `1970-…`, IDs ab 1, seq-GUIDs) | **byte-exakt** |
| **DXF** | Text | ✅ ja | keine (nur `$ACADVER`, keine Handles/Datum) | **byte-exakt** |
| **PDF** | Text/binär | ✅ ja | keine (kein `/CreationDate`/`/ID`/`/Producer`) | **byte-exakt** |
| **PNG** | binär | ✅ ja | keine (kein `tIME`-Chunk) | **byte-exakt** |
| **STEP** | Text (SPF) | ❌ **nein** | OCC `STEPControl_Writer` brennt Wall-Clock + OCC-Version in `FILE_NAME` | **Adapter-Header fixieren** → byte-exakt |
| **STL** | binär | ⚠️ ja*, OCC-resident | Header fest; ABER float32/Dreiecksordnung aus OCC-Tessellation (`BRepMesh`, Deflection 1.0) sind **OCC-versions-/plattform-abhängig** | byte-exakt **nur im gepinnten Docker-Image** ([ADR-0004](../../adr/0004-toolchain-dependency-pinning.md)); OCC-Upgrade färbt `golden-check` bewusst rot |

## 3. Definition of Done

### Infrastruktur (gemeinsam)
- [ ] **`tests/adapters/golden/`-Baum** + **`BCAD_TEST_GOLDEN_DIR`-Compile-Definition** (`tests/CMakeLists.txt`,
      Muster `BCAD_TEST_PLUGIN_DIR` — aber auf **`${CMAKE_CURRENT_SOURCE_DIR}/adapters/golden`**, die **Quell**-
      Golden, nicht das Build-Dir). Das Docker-Build-Image trägt den Quellbaum → zur Testlaufzeit lesbar.
- [ ] **`.gitattributes`** (neu im Repo) für die **binären** Golden: `tests/adapters/golden/**/*.stl binary`,
      `*.png binary` (+ `*.pdf binary`, da PDF als Ganzes binär) — verhindert CRLF-Mangling + markiert Nicht-Diff.
      Text-Golden (IFC/DXF/STEP) bleiben normal (diff-bar in der Review).
- [ ] **`make golden-regen` + `make golden-check`** (Muster [ADR-0006](../../adr/0006-relationales-schema-design.md)
      `schema-regen`/`schema-check`; **beide NICHT in `make gates`**, aber als **reale Make-Targets** in die
      CI-Befehlsliste [`harness/README.md`] — sonst bricht `gate-consistency`). `golden-regen` schreibt alle
      Export-Golden neu über einen **dedizierten kleinen Generator** (`tests/tools/golden_gen` o. ä.), **NICHT über
      das Produktions-Binary** ([MR-006](../../../../harness/conventions.md#mr-006--unabhängiges-plan-review-vor-implementierungs-start)-MED-2:
      das Binary baut `buildAcc001KernDemo`, nicht `goldenModel()` → sonst Golden-Modell-Drift); der Generator baut
      **exakt dasselbe `goldenModel()`** wie der Test (geteilte TU, LOW-3) + ruft dieselben Export-Adapter. Schreiben
      über einen **writable Mount** (Muster `acc-002-beleg` `-v …:/out`, nicht `schema-regen`s stdout-Redirect — der
      taugt nicht für 6 Dateien inkl. binärer STL/PNG). `golden-check` erzeugt sie erneut + `diff` gegen die
      committeten → „ok" nur bei Byte-Gleichheit.

### STEP-Header-Fixierung (Produktionscode — Projektinhaber-Wahl)
- [ ] **`src/adapters/geometry/step_export_adapter.cpp`:** den `FILE_NAME`-Header vor `Write` auf **feste
      Sentinel-Werte** setzen (OCC `APIHeaderSection_MakeHeader` bzw. `writer.Model()->ChangeHeader()` — Name/
      Zeitstempel `1970-01-01T00:00:00`/leerer Autor/feste Preprocessor-Zeichenkette), **analog dem
      `ifc_spf_writer`-Sentinel-Muster**. Ergebnis: STEP wird **byte-deterministisch** (verbessert auch den echten
      Export = reproduzierbar). Die bestehenden STEP-Struktur-Orakel (`CLOSED_SHELL`-Zählung) bleiben **grün**
      (nur der Header ändert sich, nicht die Geometrie). **Kein** ADR-Bruch (Reproduzierbarkeits-Verbesserung,
      keine Gate-Lockerung). **Selbst-Reproduzierbarkeit vor dem STEP-Golden-Commit prüfen**
      ([MR-006](../../../../harness/conventions.md#mr-006--unabhängiges-plan-review-vor-implementierungs-start)-LOW-2):
      `golden-regen` **zweimal** laufen + `diff` — die STEP-Topologie (OCC-Entity-Ordering) muss run-zu-run im selben
      Image byte-stabil sein, sonst ist das STEP-Byte-Golden illusorisch (dann tolerantes Orakel).

### Export-Golden (alle 6) — **die 044a-Naht**
- [ ] **Festes Test-Modell `goldenModel()` in EINER geteilten TU** (`tests/adapters/{golden_model}.{h,cpp}` —
      [MR-006](../../../../harness/conventions.md#mr-006--unabhängiges-plan-review-vor-implementierungs-start)-LOW-3:
      **sowohl** der Golden-Test **als auch** der Regen-Generator linken **dieselbe** TU, kein Copy-Drift; die
      heutigen Export-Tests tragen je ein **eigenes** lokales `sampleBuilding()`): Wände + Decke + Dach + Treppe +
      eine sichtbare Hilfslinie, sodass **jedes** Format nicht-triviale Geometrie trägt. **Nicht** `buildAcc001KernDemo`
      (main-resident, änderungsanfällig).
- [ ] **Golden-Dateien committet** `tests/adapters/golden/model.{ifc,dxf,step,stl,pdf,png}` — die byte-genaue
      Ausgabe von `goldenModel()` je Format, erzeugt über `make golden-regen`.
- [ ] **Golden-Vergleichs-Test** `tests/adapters/{test_golden_export}.cpp`: je Format re-exportiert über den
      **echten `ExchangeService`** und `EXPECT` die Ausgabe **byte-identisch** zur committeten Golden-Datei
      (`BCAD_TEST_GOLDEN_DIR`). **Komplementär** zu den Struktur-Orakeln (die bleiben — Golden ersetzt sie nicht,
      es fängt Byte-Drift bei unveränderter Struktur).
- [ ] **STL-Caveat dokumentiert** (im Test + Golden-`README`): das STL-Golden ist **OCC-versions-gebunden**
      ([ADR-0004](../../adr/0004-toolchain-dependency-pinning.md)); ein OCC-Upgrade bricht `golden-check` **bewusst**
      (Signal, dass die Tessellation sich änderte) → dann `golden-regen` + Diff-Review. Review entscheidet, ob STL
      byte-golden bleibt **oder** auf ein toleranteres Orakel (Dreieckszahl + Extent, schon vorhanden) zurückfällt.

### Import-Golden fremd (nur IFC + DXF) — **die 044b-Naht**
- [ ] **Fremd-Dateien beschafft + gevettet + committet** unter `tests/adapters/golden/foreign/`:
      - **IFC:** aus [buildingSMART/Sample-Test-Files](https://github.com/buildingSMART/Sample-Test-Files)
        (**CC-BY-4.0** — committfähig mit Attribution). **Achtung ([MR-006](../../../../harness/conventions.md#mr-006--unabhängiges-plan-review-vor-implementierungs-start)-MED-1):
        b-cads IFC-Import ist enger als „hat Extrusions-Wände" und wirft.** Pflicht **je Wand**: eine
        `IFCSHAPEREPRESENTATION` mit `RepresentationIdentifier='Axis'` + `IFCPOLYLINE` **UND** eine
        `IFCRELCONTAINEDINSPATIALSTRUCTURE`-Verortung; `IfcExtrudedAreaSolid` liefert nur die Höhe (Default-Fallback).
        Fehlt einer Wand die 'Axis'-Polyline **oder** die Containment → die **ganze Datei** wird per [`E-IO-003`](../../../../spec/spezifikation.md#4-fehler-codes-und-logging-felder)
        abgelehnt (**kein** graziöses Skip). → Die Fixture muss so **kuratiert** sein, dass **jede** `IfcWall`
        beides trägt; eine reale `Body`/`SweptSolid`-only-Sample-Datei genügt **nicht**. Notfalls eine minimale,
        konforme IFC4 gezielt erzeugen (z. B. IfcOpenShell/FreeCAD-Export) statt blind eine Sample-Datei zu greifen.
      - **DXF:** aus einem der vier **MIT**-Repos ([mozman/ezdxf](https://github.com/mozman/ezdxf),
        [gdsestimating/dxf-parser](https://github.com/gdsestimating/dxf-parser),
        [haplokuon/netDxf](https://github.com/haplokuon/netDxf), [ixmilia/dxf](https://github.com/ixmilia/dxf));
        eine **2D**-Datei mit **`LINE`-Entities auf Layern** (XY-Ebene) in einer Version, die b-cads R12-Importer
        liest. **Achtung 2D vs. 3D:** DXF trägt auch 3D (`3DFACE`/`3DSOLID`/3D-`POLYLINE`), aber b-cads DXF ist
        **2D-only** ([ADR-0015](../../adr/0015-dxf-backend.md)) — eine 3D-Demo-Fixture (z. B. `3dface.dxf`)
        importierte **0 Entities**. Also gezielt eine **2D-`LINE`-Grundriss**-Datei wählen.
      **Vetting-Pflicht (Rest-Risiko #1):** jede Kandidaten-Datei **durch b-cads Importer jagen** und nur behalten,
      wenn sie **importierbaren Inhalt** trägt (≥1 Wand/Linie). Trägt keine (fremde Geometrie-Repräsentation, die
      b-cads Subset überspringt) → die Fixture belegt nur „robust überspringen, kein Wurf" (schwächer, ehrlich zu
      benennen; ggf. eine Datei mit passendem Inhalt suchen/erzeugen).
- [ ] **Provenance-/Attributions-Manifest** `tests/adapters/golden/foreign/PROVENANCE.md`: je Fremd-Datei
      **Quelle-URL, Lizenz (CC-BY-4.0/MIT), Attributions-Text, `sha256`** + der repo-weite CC-BY-Attributions-
      Vermerk (Namensnennung buildingSMART). **[MR-011](../../../../harness/conventions.md#mr-011--referenz-integritäts-gate-matrix-ids-spans-hostpaths)/[MR-014](../../../../harness/conventions.md)**:
      token-frei (keine `slice-*`/`ADR-*` unverlinkt).
- [ ] **Import-Golden-Test** `tests/adapters/{test_golden_import}.cpp`: importiert je (kuratierte, s. o.) Fremd-Datei
      über den echten `ExchangeService` und `EXPECT` das Ergebnis-Modell (Zähl-/Wert-Orakel: Geschosse/Wände/Segmente
      > 0 + bekannte Werte; **kein Wurf** — die Fixture ist so gewählt, dass b-cad sie **annimmt**, nicht [`E-IO-003`](../../../../spec/spezifikation.md#4-fehler-codes-und-logging-felder)
      wirft [MED-1]; fremde **überspringbare** Entitäten [z. B. DXF-3D/Text] werden graziös übersprungen). **Kein**
      `find(...)/erase`-String-Mutations-Muster (bricht bei echten Fremd-Dateien — bekannte Substrings nicht garantiert).
      **DXF-Quelle:** [ezdxf](https://github.com/mozman/ezdxf)/[ixmilia](https://github.com/ixmilia/dxf) (solide MIT)
      bevorzugen; [netDxf](https://github.com/haplokuon/netDxf) meiden ([MR-006](../../../../harness/conventions.md#mr-006--unabhängiges-plan-review-vor-implementierungs-start)-INFO-2:
      Lizenz-Historie teils LGPL) — MIT-Alternativen reichen.

### Doku
- [ ] **CHANGELOG** ([MR-004](../../../../harness/conventions.md#mr-004--top-level-changelogmd-keep-a-changelog)) +
      `tests/adapters/golden/README.md` (Zweck, `golden-regen`/`golden-check`-Workflow, STL-OCC-Caveat, Provenance-
      Verweis); **CI-Befehlsliste** um `golden-check` ergänzt (wo `schema-check`/`io-smoke` stehen). **Kein**
      ADR-Index-Eintrag (keine ADR-Folgepflicht — QA-Infrastruktur). Closure-Notiz.

## 4. Plan (vor Code)

| Datei / Komponente | Änderungs-Art | Begründung |
|---|---|---|
| `tests/adapters/golden/model.{ifc,dxf,step,stl,pdf,png}` | neu (generiert) | Export-Golden je Format (via `golden-regen`) |
| `tests/adapters/golden/foreign/*.{ifc,dxf}` + `PROVENANCE.md` | neu | Fremd-Import-Golden + Herkunft/Lizenz |
| `tests/adapters/golden/README.md` | neu | Workflow + STL-Caveat |
| `tests/adapters/{test_golden_export}.cpp`, `{test_golden_import}.cpp` | neu | Byte-Vergleich (Export) + Fremd-Import-Konformanz |
| `tests/CMakeLists.txt` | ändern | `BCAD_TEST_GOLDEN_DIR`-Compile-Def + beide Tests registrieren |
| `.gitattributes` | neu | Binär-Golden (`*.stl`/`*.png`/`*.pdf`) als binary markieren |
| `Makefile` | ändern | `golden-regen` + `golden-check` (Muster `schema-regen`/`schema-check`) + CI-Liste |
| `src/adapters/geometry/step_export_adapter.cpp` | ändern | `FILE_NAME`-Header fixieren (byte-deterministisch, IFC-Sentinel-Muster) |
| `CHANGELOG.md` | ändern | Doku-Nachzug |

**Bewusst NICHT Teil:** Fremd-Import-Golden für STEP/STL/PDF/PNG (kein Import); Cross-Tool-Viewer-Validierung;
ein `make gates`-Member (Golden-Check ist CI/manuell wie `schema-check`/`io-smoke` — GUI-Binary + OCC-Residenz).

## 5. Trigger

- Die sechs Austausch-Adapter (welle-4) + die `schema-regen`/`schema-check`-Präzedenz stehen; die Determinismus-
  Recon ist gemacht (Encoder-für-Encoder belegt); die Fremd-Datei-Lizenzen sind geprüft (IFC CC-BY-4.0, DXF 4× MIT).
- **[MR-006](../../../../harness/conventions.md#mr-006--unabhängiges-plan-review-vor-implementierungs-start)-Plan-Review vor Start** (Reviewer ≠ Autor), HIGH blockiert.
  Byte-Golden-Encoder → **Code-Review vor Welle-Closure** ([MR-009](../../../../harness/conventions.md#mr-009--geometrielastiges-code-review-vor-welle-closure)) empfohlen (STEP-Header-Fix + Determinismus).

## 6. Closure-Trigger

- DoD vollständig, `make gates` grün (die neuen Golden-Tests laufen **in** `test`/`gates` — sie lesen die
  committeten Golden, brauchen `golden-regen` nicht; nur `golden-check` [Regen-Drift] ist CI-only), `make golden-check`
  grün, `make io-smoke`/`schema-check` weiter grün, Byte-Golden reproduzierbar im gepinnten Image. Closure-Notiz.

## 7. Risiken und offene Punkte

- **Rest-Risiko #1 — Fremd-Datei-Inhalt (der Wert-Kern).** b-cads Importer sind **enge Subsets**; eine Fremd-Datei
  ohne `IfcExtrudedAreaSolid`-Wände (IFC) bzw. **2D-`LINE`-auf-Layern** (DXF — b-cad ist 2D-only, überspringt 3D
  wie `3DFACE`) liefert **0 importierte Bauteile** → die Fixture belegt nur Skip-Robustheit (schwach). **Mitigation (DoD):** jede Kandidatin **vor** dem Commit durch den
  Importer jagen; nur inhaltstragende behalten; sonst weitersuchen/gezielt wählen (mehrere Repos = Redundanz).
- **Rest-Risiko #2 — STL/STEP-Geometrie ist OCC-versions-resident** ([ADR-0004](../../adr/0004-toolchain-dependency-pinning.md)).
  Byte-Golden koppelt STL (und STEP-Topologie) an die OCC-Version des Build-Images; ein Upgrade bricht `golden-check`.
  **Bewusst** (Signal), aber Review entscheidet: STL byte-golden **oder** tolerantes Orakel (Dreieckszahl+Extent).
  Das **STEP-Golden** wird durch die Header-Fixierung deterministisch, bleibt aber topologie-OCC-abhängig.
- **Rest-Risiko #3 — Golden-Regen-Determinismus.** Wenn `golden-regen` das Binary nutzt (Composition-Root), muss
  das Test-Modell **fix** sein (kein `buildAcc001KernDemo`-Drift). **Mitigation:** dedizierter `goldenModel()`-Helfer
  + der Generator baut ihn identisch wie der Test → Regen und Test speisen aus **einer** Quelle.
- **Rest-Risiko #4 — Lizenz/Attribution.** CC-BY-4.0 (IFC) **verlangt** Namensnennung; das `PROVENANCE.md` + ein
  repo-weiter NOTICE-Vermerk müssen korrekt sein (sonst Lizenzverstoß beim Weiterverteilen). MIT (DXF) verlangt den
  Lizenz-/Copyright-Vermerk mitzuführen. **Review prüft die Attribution.**
- **Rest-Risiko #5 — Byte-Golden vs. Struktur-Orakel-Redundanz.** Golden darf die vorhandenen Decode-Orakel
  **nicht ersetzen** (sie fangen verschiedene Fehlerklassen: Golden=Byte-Drift, Decode=Semantik). Beide bleiben.
- **Repo-Gewicht:** ~6 kleine Export-Golden + 2 Fremd-Dateien; binär (STL/PNG) via `.gitattributes`. Vertretbar.
  **Scope:** groß (daher §0-Split-Option) — überwiegend Test/Doku + ein kleiner OCC-Header-Fix.

## 8. Sub-Area-Modus-Begründung

### Sub-Area: Test/Infra `tests/` + `Makefile` + `.gitattributes`
- **Modus:** GF; **Dichte:** hoch (2 Test-Suiten + Golden-Baum + Regen/Check-Targets + Compile-Def). **Risiko:**
  mittel — Determinismus + Fremd-Inhalt-Vetting + Attribution die Prüfsteine.

### Sub-Area: Produktionscode `src/adapters/geometry/step_export_adapter.cpp`
- **Modus:** GF; **Dichte:** niedrig (nur Header-Fixierung). **Risiko:** niedrig — Struktur-Orakel als Netz;
  **Code-Review** ([MR-009](../../../../harness/conventions.md#mr-009--geometrielastiges-code-review-vor-welle-closure)) empfohlen.

## 9. Closure-Notiz (044a — 2026-07-24)

**Umgesetzt (die 044a-Naht):**
- **Infrastruktur:** `BCAD_TEST_GOLDEN_DIR`-Compile-Def (auf den **Quell**baum `tests/adapters/golden`), neues
  `.gitattributes` (`*.stl`/`*.png`/`*.pdf` unter `tests/adapters/golden/**` als `binary`), `make golden-regen` +
  `make golden-check` (Muster `schema-regen`/`schema-check`; **NICHT** in `make gates`, in der CI-Befehlsliste).
- **STEP-Header-Fix (Produktionscode):** `step_export_adapter.cpp` fixiert den ISO-10303-21-HEADER
  (`FILE_NAME`/`FILE_DESCRIPTION`) via OCC `APIHeaderSection_MakeHeader` + `Apply` auf Sentinel-Werte
  (Epoch-Zeitstempel `1970-01-01T00:00:00`, `b-cad` statt OCC-Versions-String) — analog `ifc_spf_writer`. Ergebnis
  byte-verifiziert: `FILE_NAME('','1970-01-01T00:00:00',(''),(''),'b-cad','b-cad','')`, keine Wall-Clock/OCC-Version.
  Die STEP-Struktur-Orakel (`CLOSED_SHELL`) bleiben grün (nur Header geändert). Selbst-Reproduzierbarkeit
  (LOW-2): `make golden-check` regeneriert run-zu-run byte-identisch (grün).
- **Export-Golden (alle 6):** geteilte `goldenModel()`-TU (`tests/adapters/golden_model.{h,cpp}`; Wände + Decke +
  Sattel-Dach + Treppe + sichtbare Ebene/Hilfslinie) — von **beiden** gelinkt: dem dedizierten Generator
  `golden_gen` (`golden_gen.cpp`, **nicht** das Produktions-Binary, MED-2) und dem Byte-Vergleichs-Test
  `test_golden_export.cpp` (6 `GoldenExport.*`-Tests, byte-genau gegen die committeten Golden, **komplementär** zu
  den Struktur-Orakeln). Golden committet: `tests/adapters/golden/model.{ifc,dxf,step,stl,pdf,png}`.
- **STL-Caveat** dokumentiert (`golden/README.md` + Test-Kommentar): STL/STEP-Topologie OCC-versions-gebunden
  ([ADR-0004](../../adr/0004-toolchain-dependency-pinning.md)); ein OCC-Upgrade bricht `golden-check`/`GoldenExport.Stl*`
  **bewusst** → dann Regen + Diff-Review.

**Gates:** `make build`/`make test` grün (**262 Tests**, +6 `GoldenExport.*`); `make golden-check` grün
(committet == regen byte-genau); `make docs-check` grün. Voller `make gates` + `io-smoke`/`schema-check`: s. Report.

**Split-Entscheidung:** 044 → **044a** (dieser Plan, done-Kandidat) + **[`slice-044b`](../open/slice-044b-golden-import-fremd.md)**
(Import-Golden-fremd, `open/`, [MR-020](../../../../harness/conventions.md#mr-020--adr-folgepflicht-sichtbarkeit-closure-disziplin)(3)-Skelett). **„Beides" bleibt erfüllt — zwei sequenzielle Slices.**

**Review:** [MR-006](../../../../harness/conventions.md#mr-006--unabhängiges-plan-review-vor-implementierungs-start) 0 HIGH (Plan);
[MR-009](../../../../harness/conventions.md#mr-009--geometrielastiges-code-review-vor-welle-closure)-Code-Review **0 HIGH**
(unabhängig, adversarial): STEP-Header-Fix ist **header-only** (DATA/`CLOSED_SHELL` unberührt), OCC-API korrekt, kein
Crash bei leerem Modell; Determinismus empirisch bestätigt (`golden-check` byte-exakt); Oracle-Soundness (kein
empty-vs-empty, temp-Kollision, Modell-Identität). Nicht-blockierende Findings (`.gitattributes`-EOL-Härtung + IFC-
Kommentar) eingearbeitet; Service-Wiring-Dedup als LOW bewusst zurückgestellt (Divergenz gate-gefangen).

**Lerneintrag:** OCC-`STEPControl_Writer` brennt Wall-Clock **und** OCC-Version in den HEADER
(`preprocessor_version`/`originating_system`) → beides über `APIHeaderSection_MakeHeader`+`Apply` neutralisieren, nicht
nur den Zeitstempel. Byte-Golden in `tests/`-`.md` tripsen die `ids`-Linkpflicht **nicht** (empirisch, docs-check grün).
Der Generator darf **nicht** das Produktions-Binary sein (`buildAcc001KernDemo` ≠ `goldenModel()`).
