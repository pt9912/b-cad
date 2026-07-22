---
id: slice-032b
titel: DRW-Impl — Kern-Werttypen (Layer/GuideLine) + EditDrawingPort + Service + guide_lines-Persistenz
status: open
welle: welle-5-erweiterung
lastenheft_refs: [[LH-FA-DRW-005](../../../../spec/lastenheft.md#lh-fa-drw-005), [LH-FA-DRW-006](../../../../spec/lastenheft.md#lh-fa-drw-006), [LH-FA-BLD-002](../../../../spec/lastenheft.md#lh-fa-bld-002--projekt-speichern)]
adr_refs: [[ADR-0001](../../adr/0001-hexagonale-architektur.md), [ADR-0003](../../adr/0003-persistenz-sqlite.md), [ADR-0006](../../adr/0006-relationales-schema-design.md), [ADR-0008](../../adr/0008-aenderungs-benachrichtigung.md), [ADR-0018](../../adr/0018-drw-2d-zeichen-daten.md)]
---

# Slice 032b: DRW-Impl — Layer/GuideLine + EditDrawingPort + Service + guide_lines-Persistenz

**Status:** open (Plan **startbar** nach Review-Einarbeitung — noch **nicht** in-progress; Impl-Start
auf Projektinhaber-Wort). **[MR-006](../../../../harness/conventions.md#mr-006--unabhängiges-plan-review-vor-implementierungs-start)-Plan-Review
2026-07-22 (Reviewer ≠ Autor, read-only): 0 HIGH / 2 MED / 2 LOW / 2 INFO → startbar**
([Report](../../../reviews/2026-07-22-slice-032b-plan.md)). M1 (Design „Port getrennt, Service geteilt")
+ M2 (Export-AK erst 032c) als tragfähig bestätigt; L1 (Wortlaut §2.2) + L2 (`make schema-regen`-Target)
+ INFO-1 (Zusatz-Sonden) eingearbeitet.
**[MR-009](../../../../harness/conventions.md#mr-009--geometrielastiges-code-review-vor-welle-closure) n/a**
(Hilfslinie = 2 Punkte, **keine** neue Solid-Geometrie —
[ADR-0018](../../adr/0018-drw-2d-zeichen-daten.md) §Konsequenzen (c)). Persistenz-Latte:
zusätzliches unabhängiges **Code-Review vor Welle-Closure** (stille-Verfälschungs-/Drift-Klasse,
Muster [slice-017e](../done-archive/slice-017e-material-persistenz.md)).

**Welle:** welle-5-erweiterung (DRW-Strang, **Impl-Hälfte** des Fundaments; Nachfolge
[slice-032a](../done/slice-032a-drw-fundament-ak-spec.md) [AK/Spec]; Muster
[slice-017d](../done-archive/slice-017d-material-domaene-verwaltung-zuweisung.md)/[017e](../done-archive/slice-017e-material-persistenz.md)
[Domäne+Port+Service, dann Persistenz]).

**Bezug:** [LH-FA-DRW-005](../../../../spec/lastenheft.md#lh-fa-drw-005) (Hilfslinien) /
[LH-FA-DRW-006](../../../../spec/lastenheft.md#lh-fa-drw-006) (Layer/Ebenen) — durch
[slice-032a](../done/slice-032a-drw-fundament-ak-spec.md) auf AK-Niveau, das Mapping liegt in
[`spezifikation.md` §1 `LH-FA-DRW-005.a`](../../../../spec/spezifikation.md). [LH-FA-BLD-002](../../../../spec/lastenheft.md#lh-fa-bld-002--projekt-speichern)
(atomar speichern) ist der Persistenz-Mechanismus. **[ADR-0018](../../adr/0018-drw-2d-zeichen-daten.md)
ist die treibende Fundament-Entscheidung** (Datenheimat = pure Werttypen auf `Building`;
Zugriffs-Naht = **neuer** Driving-Port [Muster `EvaluatePort`], **nicht** `EditStructurePort`;
Layer-Zuordnung = **direkter typisierter Bezug**, **nicht** `entity_layers`; Löschschutz =
`restrict`-Muster; additive `guide_lines`-Tabelle + `layers`-Aktivierung; **kein** neuer `op`).
[ADR-0001](../../adr/0001-hexagonale-architektur.md) (Kern framework-frei, Regel A/D),
[ADR-0003](../../adr/0003-persistenz-sqlite.md) (atomar Temp+Rename),
[ADR-0006](../../adr/0006-relationales-schema-design.md) (Schema, `restrict`, #1-konforme typisierte
Beziehung), [ADR-0008](../../adr/0008-aenderungs-benachrichtigung.md) (kein neuer `op`, Material-Präzedenz).

**Autor:** Dietmar Burkard (AI-Harness-Lauf). **Datum:** 2026-07-22.

**Schnitt-Herkunft:** Impl-Hälfte des DRW-Fundaments — die **Daten-/Persistenz-Stufe**
([ADR-0018](../../adr/0018-drw-2d-zeichen-daten.md) §Konsequenzen (b), erster Teil: „Kern+Port+Service+Persistenz").
Beobachtbar über **Persistenz-Round-Trip** (Hilfslinie/Ebene überleben Speichern/Laden
unverändert) + die drei **harten Ablehnungen** (entartete Hilfslinie / leerer Ebenen-Name /
Löschen referenzierter Ebene → [`E-VAL-001`](../../../../spec/spezifikation.md#4-fehler-codes-und-logging-felder)-Rejection,
Modell unverändert).

**Bewusst NICHT Teil (benannte Grenzen / Folge):**

- **Export-Sichtbarkeit** (`plan_geometry`/DXF/PDF/PNG-Zeichnung der Hilfslinie + Layer-
  Sichtbarkeits-Filter + [`io-smoke`](../../../../harness/README.md#sensors-feedback-gates)) ist
  **slice-032c**. Damit ist die AK-Teilklausel **„when der 2D-Grundriss exportiert wird, then
  erscheint sie im Artefakt"** ([LH-FA-DRW-005](../../../../spec/lastenheft.md#lh-fa-drw-005) Happy)
  und die **[LH-FA-DRW-006](../../../../spec/lastenheft.md#lh-fa-drw-006)-Negative** (unsichtbare
  Ebene → fehlt im Artefakt) in 032b **noch nicht** beobachtbar — 032b liefert die
  **Round-Trip-Hälfte** der Happy-AK und die drei Ablehnungen; die Export-Hälfte folgt in 032c
  ([ADR-0018](../../adr/0018-drw-2d-zeichen-daten.md) §Konsequenzen (b) „danach Export-Sichtbarkeit").
  **Ehrlich benannt** (Muster [slice-032a](../done/slice-032a-drw-fundament-ak-spec.md)
  Teilumfang-Klausel): kein stiller Vollumfang.
- **`entity_layers` / Bauteil-Layer / interaktiver 2D-Canvas / eigener DXF-Layer-Name** —
  benannte Re-Eval-Trigger ([ADR-0018](../../adr/0018-drw-2d-zeichen-daten.md) §Re-Evaluierungs-Trigger).
  `entity_layers` und `documents` bleiben **forward-deklariert** (unverdrahtet).
- **`locked`-Semantik (Bearbeitungs-Sperre):** die Ebene trägt `locked` (Schema + Modell,
  round-getrippt), aber es gibt in dieser Stufe **keinen** Edit-Pfad, den eine Sperre schützen
  könnte (kein Canvas) — `locked` ist Daten-durabel, **nicht** verhaltenswirksam (benannte Lücke).
- **Composition-Root-/GUI-Verdrahtung** des `EditDrawingPort` (Menü/Toolbar) — nicht nötig für
  die Round-Trip-AK (der Service wird im Test direkt getrieben); der Plugin-Host erhält den Port
  ohnehin „gratis" über die bestehenden Driving-Kanten
  ([ADR-0018](../../adr/0018-drw-2d-zeichen-daten.md) §Entscheidung 2). GUI-Bindung folgt mit dem
  UI-Strang.

---

## 1. Ziel

Der DRW-Kern wird **real und durabel**: `Layer` und `GuideLine` als **pure Werttypen im
framework-freien Kern** auf `Building` ([ADR-0001](../../adr/0001-hexagonale-architektur.md);
Muster `Material`-Aggregat); ein **neuer Driving-Port `EditDrawingPort`** (Anlegen/Umbenennen/
Sichtbarschalten/Löschen von Ebenen, Anlegen/Löschen von Hilfslinien) plus die Implementierung
im **bestehenden Anwendungs-Service** (`StructureEditService` — er hält bereits die **eine**
`Building`-Instanz und implementiert vier Ports; siehe §Design-Entscheidung); die **`guide_lines`-
Tabelle** wird dem Schema hinzugefügt und die **`layers`-Tabelle aktiviert**, beide vom
`SqliteProjectRepository` in derselben **atomaren** Transaktion round-getrippt
([ADR-0003](../../adr/0003-persistenz-sqlite.md)).

**Design-Entscheidung (zentral, [MR-006](../../../../harness/conventions.md#mr-006--unabhängiges-plan-review-vor-implementierungs-start)-Prüfstein) — `EditDrawingPort` wird als *eigener Port*
(Interface) geschnitten, aber vom *bestehenden* `StructureEditService` *implementiert*.**
[ADR-0018](../../adr/0018-drw-2d-zeichen-daten.md) verlangt eine **eigene Port-Naht** (Use-Case-
Familie sauber getrennt, Muster `EvaluatePort`) — das ist die **Interface**-Trennung. Die
**Implementierung** teilt sich denselben Service, weil `Layer`/`GuideLine` auf **derselben**
`Building`-Instanz leben wie Wände/Materialien: ein separater `DrawingEditService` mit **eigenem**
`Building building_;` erzeugte **zwei** Modell-Kopien (kein geteilter Zustands-Seam existiert
heute), ein separater Service mit `Building&`-Referenz erfände einen neuen Sharing-Mechanismus,
den es sonst nirgends gibt. Präzedenz: `StructureEditService` implementiert `EvaluatePort` **als
eigenen Port** ebenfalls im selben Objekt. **Ergebnis:** neue Port-Datei (Interface-Trennung
erfüllt), Implementierung als 5. Basisklasse von `StructureEditService` — **keine** neue Service-
`.cpp`, **kein** CMake-Zuwachs im Kern.

**Korrektheits-Kern (Code-Review-Primärziel, Persistenz-Latte):** **Round-Trip-Treue + FK-
Reihenfolge**. `layers` muss **vor** `guide_lines` geschrieben werden (`guide_lines.layer_id →
layers.id`), `guide_lines` **nach** `storeys` (`storey_id → storeys.id`); ID-Erhalt explizit
(`bind_int(static_cast<int>(id))`) → `guide_line.layer_id`/`.storey_id` zeigen nach dem Laden auf
dieselbe Ebene/dasselbe Geschoss. Boolean `visible`/`locked` als `INTEGER 0/1` unverfälscht
zurück. Koordinaten `*_mm` (decimal 12,3) exakt.

## 2. Definition of Done

- [ ] **Kern-Werttypen** (`src/hexagon/model/{layer,guide_line}.h`; header-only, pure
      Werte, `namespace bcad::hexagon::model`, kein I/O — Regel A):
      `enum class LayerId : int {}` + `struct Layer { LayerId id; std::string name; bool
      visible{true}; bool locked{false}; std::optional<std::string> color_hex; };`;
      `enum class GuideLineId : int {}` + `struct GuideLine { GuideLineId id; model::StoreyId
      storey_id; model::LayerId layer_id; model::Segment segment; };` (`Segment` = zwei
      `Point2D` in mm, wiederverwendet). **`Building`** (`building.h`) erhält
      `std::vector<Layer> layers;` + `std::vector<GuideLine> guide_lines;`. **Kein** CMake-Edit
      (header-only; Muster `material.h`).
- [ ] **Driving-Port `src/hexagon/ports/driving/{edit_drawing_port}.h`** (neu, `namespace
      bcad::hexagon::ports::driving`, rein abstrakt) — **eigener Port** ([ADR-0018](../../adr/0018-drw-2d-zeichen-daten.md)
      §Entscheidung 2), Command-Idiom wie `EditStructurePort` (Anlegen → `std::optional<Id>`
      [nullopt = abgelehnt], Mutation/Löschen → `bool` [false = unbekannt/abgelehnt]):
      `addLayer(const model::Layer&) → optional<LayerId>` (nullopt = **leerer** oder
      **projekt-doppelter** Name); `renameLayer(LayerId, const std::string&) → bool` (false =
      unbekannt / leer / Namens-Kollision); `setLayerVisible(LayerId, bool) → bool`;
      `removeLayer(LayerId) → bool` (false = **referenziert** [restrict] / unbekannt);
      `addGuideLine(const model::GuideLine&) → optional<GuideLineId>` (nullopt = **entartet**
      [Anfang = Ende] / unbekannte Ebene / unbekanntes Geschoss); `removeGuideLine(GuideLineId)
      → bool`. **Kein** neuer Fehler-Code — die Ablehnungen sind die
      [`E-VAL-001`](../../../../spec/spezifikation.md#4-fehler-codes-und-logging-felder)-**Rejection-
      Lesart** (Modell unverändert), **keine** Klemmung. **Kein** neuer `op`
      ([ADR-0008](../../adr/0008-aenderungs-benachrichtigung.md), Material-Präzedenz).
- [ ] **Service** (`structure_edit_service.{h,cpp}`): `StructureEditService` implementiert
      `EditDrawingPort` **zusätzlich** (5. Basisklasse); `int next_layer_id_{1};` +
      `int next_guide_line_id_{1};`. **Validierung:** `addLayer` lehnt leeren Namen ab
      (`isBlankName`, wiederverwendet) **und** projekt-doppelten Namen (App-seitig, konsistent mit
      `uq_layers_project_name` — verhindert Save-Zeit-Constraint-Wurf); `addGuideLine` lehnt
      entartete Linie ab (Anfang = Ende, exakter Koordinaten-Vergleich mm) sowie unbekannte
      `layer_id`/`storey_id`; `removeLayer` lehnt ab, solange eine `GuideLine` die Ebene
      referenziert (`layerReferenced(building_, id)`-Helfer, Muster `materialReferenced` —
      **`restrict`**). ID-Erhalt/Vergabe wie alle Bauteil-Ids.
- [ ] **Schema** — **`guide_lines`-Tabelle** in `spec/data-model.yaml` (id; `project_id`
      cascade; `storey_id` cascade; **`layer_id` `on_delete: restrict`** [DB-Löschschutz]; vier
      `*_mm` decimal 12,3 required) + **`schema.sql` via `make schema-regen` regeneriert** (neues
      Target, L2 — §3); `layers`-Tabelle **unverändert** (liegt vor, wird nur real verdrahtet).
      **`make schema-check` grün** (Byte-Gleichheit committete `schema.sql` == d-migrate(`data-model.yaml`)).
- [ ] **Persistenz** (`sqlite_project_repository.cpp`): `insertLayers`/`loadLayers`,
      `insertGuideLines`/`loadGuideLines` (Muster `insertMaterials`/`loadMaterials`); NULL-sichere
      Optional-Helfer für `color_hex` (`bindOptionalText`/`columnOptionalText`, wiederverwendet);
      `visible`/`locked` als int 0/1. **Insert-Reihenfolge:** `insertLayers` **vor**
      `insertGuideLines`, `insertGuideLines` **nach** `insertStoreys` (FKs, `foreign_keys=ON`).
      Beide in die bestehende **BEGIN/COMMIT**-Transaktion + `save`/`load`-Aufruflisten
      eingehängt; Fehler lässt den Vorstand intakt (Temp verworfen, E-IO).
      `entity_layers`/`documents` bleiben unverdrahtet (benannte Lücke).
- [ ] **AK-Tests Kern** (`tests/hexagon/test_drawing.cpp`, neu; Suite-Namen mit LH-id;
      port-double-Geometrie, OCC-frei):
      **`Drawing_LH_FA_DRW_006, EbeneAnlegenUmbenennenSichtbarkeit`** (Happy: anlegen mit
      Name/Farbe, umbenennen, sichtbar/unsichtbar → Zustand geändert);
      **`Drawing_LH_FA_DRW_006, LeererNameAbgelehnt`** (Boundary: `addLayer("   ")` → `nullopt`,
      Modell unverändert); **`Drawing_LH_FA_DRW_006, ReferenzierteEbeneLoeschenAbgelehnt`**
      (Negative: `removeLayer` einer von einer Hilfslinie belegten Ebene → `false`, Modell
      unverändert — `restrict`); **`Drawing_LH_FA_DRW_005, HilfslinieAnlegen`** (Happy in-model:
      auf sichtbarer Ebene, Endpunkte/Ebene gesetzt); **`Drawing_LH_FA_DRW_005,
      EntarteteHilfslinieAbgelehnt`** (Boundary: Anfang = Ende → `nullopt`, Modell unverändert).
      **Zusatz-Sonden für die neuen Validierungs-Branches (Review-INFO-1):** Doppel-Ebenen-Name
      abgelehnt (`uq_layers_project_name`-App-Guard), `addGuideLine` mit unbekannter Ebene/unbekanntem
      Geschoss → `nullopt`, `removeGuideLine` (Happy + unbekannte Id) — übt die Logik + hält
      `coverage-gate` ≥ 70 % komfortabel. Neues File in `tests/CMakeLists.txt` (`bcad_tests`) eingehängt.
- [ ] **AK-Tests Persistenz** (`tests/adapters/test_sqlite_project_repository.cpp`, ergänzt):
      **`SqliteProjectRepository_LH_FA_DRW, RoundTripEbeneUndHilfslinie`** — Projekt mit Ebene
      (sichtbar, Farbe) + Hilfslinie auf einem Geschoss speichern/laden → Ebene (Name/`visible`/
      `locked`/Farbe) **unverändert**, Hilfslinie (Anfangs-/Endpunkt exakt, `storey_id`/`layer_id`
      erhalten) **unverändert**; **leere** `layers`/`guide_lines` round-trippen. **Regression:**
      bestehende Round-Trip-/Atomaritäts-Tests textlich unverändert grün.
- [ ] **Spec-Nachzug (durabel):** `spezifikation.md` **§1** [`LH-FA-DRW-005.a`](../../../../spec/spezifikation.md)
      um „durabel / Round-Trip aktiv; `locked` daten-durabel, nicht verhaltenswirksam (Lücke)"
      ergänzt; **§2.2** (Review-L1: die Futur-DRW-Prosa auf Präsens/abgeschlossen umschreiben — heute
      trägt **`layers`** das „forward-deklariert", und `guide_lines` ist bereits in Futur beschrieben):
      **`layers`** forward-deklariert → **real in Betrieb**, `guide_lines` von „ergänzt (Futur)" auf
      **aktiv verdrahtet**, **slice-nummer-frei** ([MR-014](../../../../harness/conventions.md#mr-014--referenz-richtungs-verschärfung-adr-nennt-keine-slice-d-check-v0371));
      **`spezifikation-historie.md`** Provenance-Zeile + `**Letzte Änderung:**`-Header. **KEIN**
      ADR-/Slice-Token im `spezifikation.md`-**Körper**
      ([MR-011](../../../../harness/conventions.md#mr-011--referenz-integritäts-gate-matrix-ids-spans-hostpaths)/[MR-014](../../../../harness/conventions.md#mr-014--referenz-richtungs-verschärfung-adr-nennt-keine-slice-d-check-v0371);
      vor dem Gate per `grep -E 'ADR-|slice-[0-9]{3}'` selbst fangen). **Lastenheft unberührt**
      (kein [MR-010](../../../../harness/conventions.md#mr-010--lastenheft-header-version--oberste-9-historie-zeile) —
      032a hat die AK bereits gesetzt).
- [ ] **[ADR-Index](../../adr/README.md)-Folgepflicht:** die [ADR-0018](../../adr/0018-drw-2d-zeichen-daten.md)-
      **Impl-Zeile** (Kern+Port+Service+Persistenz) → erfüllt; die **Export-Zeile** bleibt **offen**
      (032c). **CHANGELOG** ([MR-004](../../../../harness/conventions.md#mr-004--top-level-changelogmd-keep-a-changelog))
      Slice-Eintrag.
- [ ] **Gates:** engster Sensor zuerst (`make schema-check`, dann `make test`), danach
      **`make gates` grün** (docs-check inkl. `planning`/`ids`/`matrix`; a-check/arch-check
      unverändert — **keine** neue Regel [ADR-0018](../../adr/0018-drw-2d-zeichen-daten.md)
      §Entscheidung 6; lint 0; coverage ≥ 70 %; build Target-Trennung). **Unabhängiges
      Code-Review vor Welle-Closure** (Persistenz-Latte) ohne offene HIGH. Closure-Notiz.
- [ ] **Lifecycle:** beim Start `git mv open/ → in-progress/` **+ Ruhe-Sentinel** `Keine offenen
      Slices` aus dem Roadmap-`## Aktuelle Welle`-Block **entfernen** — **im selben Commit**
      ([MR-017](../../../../harness/conventions.md#mr-017--planning-lifecycle-gate-d-check-modul-planning),
      [AGENTS §2.8](../../../../AGENTS.md)); bei Closure `git mv → done/` (+ Sentinel wieder setzen,
      falls letzter in-progress-Slice).

## 3. Plan (vor Code)

| Datei / Komponente | Änderungs-Art | Begründung |
|---|---|---|
| `src/hexagon/model/{layer}.h` | neu | `LayerId` + `Layer`-Werttyp (Muster `material.h`) |
| `src/hexagon/model/{guide_line}.h` | neu | `GuideLineId` + `GuideLine`-Werttyp (Segment + storey/layer ref) |
| `src/hexagon/model/building.h` | ändern | `std::vector<Layer> layers;` + `std::vector<GuideLine> guide_lines;` |
| `src/hexagon/ports/driving/{edit_drawing_port}.h` | neu | eigener Driving-Port ([ADR-0018](../../adr/0018-drw-2d-zeichen-daten.md) §2) |
| `src/hexagon/services/structure_edit_service.h` | ändern | 5. Basisklasse `EditDrawingPort` + Methoden-Decl + `next_*_id_` |
| `src/hexagon/services/structure_edit_service.cpp` | ändern | Layer/GuideLine-Logik (Validierung, `restrict`, kein `op`) |
| `spec/data-model.yaml` | ändern | **`guide_lines`-Tabelle** (layer_id `restrict`) |
| `Makefile` | ändern | **neues `make schema-regen`-Target** (L2 — spiegelt `schema-check`, stdout→`schema.sql`) |
| `src/adapters/persistence/schema.sql` | ändern (generiert) | via `make schema-regen` nach `guide_lines` |
| `src/adapters/persistence/sqlite_project_repository.cpp` | ändern | `insert/loadLayers` + `insert/loadGuideLines` + Aufruf-Reihenfolge |
| `tests/hexagon/test_drawing.cpp` | neu | Kern-AK (Layer/GuideLine + 3 Ablehnungen) |
| `tests/CMakeLists.txt` | ändern | `test_drawing.cpp` in `bcad_tests` |
| `tests/adapters/test_sqlite_project_repository.cpp` | ändern | Round-Trip-AK Ebene + Hilfslinie |
| `spec/spezifikation.md` | ändern | §1 Durabilität + §2.2 `guide_lines` aktiv (ADR-/slice-frei) |
| `spec/spezifikation-historie.md` | ändern | Provenance-Zeile + Header |
| [ADR-Index](../../adr/README.md) | ändern (Closure) | [ADR-0018](../../adr/0018-drw-2d-zeichen-daten.md) Impl-Zeile erfüllt (Export offen) |
| `CHANGELOG.md` | ändern | Slice-Eintrag ([MR-004](../../../../harness/conventions.md#mr-004--top-level-changelogmd-keep-a-changelog)) |
| `docs/reviews/2026-07-22-slice-032b-plan.md` | neu (erledigt) | [MR-006](../../../../harness/conventions.md#mr-006--unabhängiges-plan-review-vor-implementierungs-start)-Report (0 HIGH → startbar) |
| `docs/reviews/{2026-07-2x-slice-032b-code-review}.md` | neu | Code-Review-Report (Persistenz-Latte) |

**`schema.sql`-Regeneration via neuem `make schema-regen`-Target (L2-Entscheidung):** heute existiert
**kein** Regen-Target — nur `schema-check` (diffend). 032b legt **`make schema-regen`** an: dasselbe
digest-gepinnte d-migrate-Image (`DMIGRATE` aus dem `Makefile`) wie `schema-check`, schreibend statt
diffend (`… schema generate --source=/work/data-model.yaml --target=sqlite --deterministic
--report=/dev/null > src/adapters/persistence/schema.sql`); anschließend **`make schema-check`** beweist
die Byte-Gleichheit. Damit bleibt die Regen **innerhalb der [§2.9](../../../../AGENTS.md)-„nur über
make"-Disziplin** (statt eines rohen Einmal-`docker run`; Review-L2). Kleine öffentliche
Vertragsergänzung; **nicht** in `make gates` (Muster `schema-check`, d-migrate aus dem Gate-Pfad).

## 4. Trigger

- **[slice-032a](../done/slice-032a-drw-fundament-ak-spec.md) done ✓** (AK auf Niveau, Mapping in
  `spezifikation.md` §1/§2.2/§4/§6, Subset-Grenze autorisiert).
- **[ADR-0018](../../adr/0018-drw-2d-zeichen-daten.md) Accepted ✓** (Fundament entschieden).
- **[MR-006](../../../../harness/conventions.md#mr-006--unabhängiges-plan-review-vor-implementierungs-start)-Plan-Review
  (Reviewer ≠ Autor) — gelaufen ✓ 2026-07-22, 0 HIGH / 2 MED / 2 LOW / 2 INFO → startbar**
  ([Report](../../../reviews/2026-07-22-slice-032b-plan.md); L1/L2/INFO-1 eingearbeitet). **HIGHs
  blockieren den Start — keine offen.** Impl-Start (git mv → in-progress/ + Sentinel) auf
  Projektinhaber-Wort.

## 5. Closure-Trigger

- DoD vollständig, `make schema-check` + `make gates` grün, **Code-Review ohne offene HIGH**,
  Closure-Notiz → **slice-032c** (DXF/PDF/PNG-Export-Sichtbarkeit + `io-smoke`; schließt die
  Export-Hälfte der [LH-FA-DRW-005](../../../../spec/lastenheft.md#lh-fa-drw-005)-Happy- und die
  [LH-FA-DRW-006](../../../../spec/lastenheft.md#lh-fa-drw-006)-Negative-AK) wird startbar.

## 6. Risiken und offene Punkte

- **FK-Schreib-Reihenfolge (HIGH-Klasse):** `layers` vor `guide_lines`, `guide_lines` nach
  `storeys` — sonst Constraint-Fehler beim Save (`foreign_keys=ON`). AK-Round-Trip mit belegter
  Ebene belegt die Reihenfolge; das Code-Review prüft die `save`/`load`-Aufruflisten.
- **Schema-Drift (HIGH-Klasse):** `guide_lines` ist der **erste** additive Tabellen-Zuwachs seit
  Längerem — `schema.sql` **muss** aus `data-model.yaml` regeneriert werden, sonst `schema-check`
  rot. Regeneration dokumentiert (§3); `schema-check` ist der fail-closed-Beleg.
- **Round-Trip-Treue (HIGH-Klasse, Persistenz-Latte):** ID-Erhalt (`layer_id`/`storey_id` zeigen
  nach Reload aufs richtige Ziel), Boolean `visible`/`locked` unverfälscht, Koordinaten exakt
  (decimal 12,3). AK pinnt Feld-Gleichheit; Code-Review prüft Bind-Positionen/SELECT-Indizes.
- **Design-Entscheidung „Port getrennt, Service geteilt" (MED — [MR-006](../../../../harness/conventions.md#mr-006--unabhängiges-plan-review-vor-implementierungs-start)-Prüfstein):** `EditDrawingPort`
  als eigenes Interface (ADR-treu), implementiert von `StructureEditService` (eine `Building`-
  Instanz). Alternative (separater Service) verworfen: zwei Modell-Kopien / erfundener Sharing-Seam.
  Begründung in §1; das Review wägt, ob die ADR-„eigener Service"-Formulierung damit erfüllt ist
  (Interface ja; Objekt geteilt wie bei `EvaluatePort`).
- **Doppelter Layer-Name (MED):** `uq_layers_project_name` würde sonst erst zur **Save-Zeit**
  werfen. Der Service lehnt App-seitig ab (nullopt/false) — konsistente Rejection-Lesart, **nicht**
  in der AK gefordert, aber für Modell-Konsistenz nötig. Benannt; Test deckt den Happy-Fall,
  optional ein Doppel-Name-Test.
- **Beobachtbarkeits-Grenze (MED — 032a-Erbe):** die **Export**-Teilklauseln der AK
  ([LH-FA-DRW-005](../../../../spec/lastenheft.md#lh-fa-drw-005) „erscheint im Artefakt",
  [LH-FA-DRW-006](../../../../spec/lastenheft.md#lh-fa-drw-006)-Negative) sind in 032b **nicht**
  beobachtbar — 032b liefert Round-Trip + Ablehnungen, 032c den Export. **Ehrlich benannt** (kein
  über-versprochener Vollumfang; Präzedenz 032a-Teilumfang-Klausel, welle-4 export-only). **Kein
  HIGH-Kandidat**, aber der Reviewer prüft, ob die 032b-DoD ohne Export „genug Nutzerweg" trägt
  (Round-Trip-Durabilität ist die etablierte canvas-lose Beobachtungsklasse, Muster 017e Material).
- **Keine neue Gate-Regel ([ADR-0018](../../adr/0018-drw-2d-zeichen-daten.md) §6):** DRW ist
  kern-resident, header-/dependency-frei — Regel A + B + Schicht-Kanten genügen; `a-check`/
  `arch-check` unverändert. Selbst-Check: `make a-check` bleibt grün ohne `.a-check.yml`-Edit.
- **Spec-Straten ADR-/slice-frei ([MR-011](../../../../harness/conventions.md#mr-011--referenz-integritäts-gate-matrix-ids-spans-hostpaths)/[MR-014](../../../../harness/conventions.md#mr-014--referenz-richtungs-verschärfung-adr-nennt-keine-slice-d-check-v0371)):**
  §1/§2.2-Nachzug ohne ADR-/Slice-Token im Körper (Provenance nur `spezifikation-historie.md`) —
  vor dem Gate selbst greppen (Lerneintrag 032a-LOW-3).
- **`op`-Neutralität ([ADR-0008](../../adr/0008-aenderungs-benachrichtigung.md)):** DRW-Mutationen
  melden **keinen** `op` (Material-Präzedenz) — `ModelChangedPort` unberührt.

## 7. Sub-Area-Modus-Begründung

### Sub-Area: Domänen-Modell + Ports + Services (Hexagon-Kern) `src/hexagon/`

- **Modus:** GF; **Dichte:** hoch (Pure-Werttyp-Regel, eigener Driving-Port [Muster
  `EvaluatePort`], Rejection-Idiom `optional`/`bool`, kein `op`, Regel A). **Phase-Reife:** 4
  (Port+Service-Muster über fünf Bauteil-Familien + Material etabliert). **Risiko:** niedrig
  (reine Werte, keine Geometrie — [MR-009](../../../../harness/conventions.md#mr-009--geometrielastiges-code-review-vor-welle-closure) n/a).

### Sub-Area: Persistenz-Adapter `src/adapters/persistence/`

- **Modus:** GF; **Dichte:** hoch (Round-Trip-/Atomaritäts-/ID-Erhalt-/FK-Reihenfolge-Konvention,
  additiver Schema-Zuwachs, Regel D). **Phase-Reife:** 4. **Risiko:** mittel — Persistenz trägt
  die stille-Verfälschungs-/Drift-Klasse + den **ersten Schema-Tabellen-Zuwachs** → **Code-Review-
  Latte** (Muster [017e](../done-archive/slice-017e-material-persistenz.md)).

### Sub-Area: Test-Infrastruktur `tests/`

- **Modus:** GF; **Dichte:** hoch (AK mit LH-id-Suite, Round-Trip- + Ablehnungs-Sonden, Feld-
  Gleichheit). **Risiko:** niedrig.

### Sub-Area: Spec-Schreibung `spec/`

- **Modus:** GF; **Dichte:** mittel (§1-Durabilität + §2.2-Aktiv-Prosa, ADR-/slice-frei,
  `data-model.yaml`-Tabelle). **Risiko:** niedrig (Doku + additives Schema).

## 8. Closure-Notiz

*(bei Closure ausgefüllt: gelieferte Dateien, Test-Zahl, `make schema-check`/`make gates`-Ergebnis,
Review-Ergebnisse [[MR-006](../../../../harness/conventions.md#mr-006--unabhängiges-plan-review-vor-implementierungs-start) + Code-Review], Lerneintrag, Rest-Risiken/Folge 032c.)*
