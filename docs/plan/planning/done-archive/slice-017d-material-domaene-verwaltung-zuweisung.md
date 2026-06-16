---
id: slice-017d
titel: Material — Domäne + Verwaltung + Zuweisung (in-memory, EditStructurePort/EvaluatePort)
status: done
welle: welle-3-auswertung
lastenheft_refs: [[LH-FA-MAT-001](../../../../spec/lastenheft.md#lh-fa-mat-001--materialien-verwalten), [LH-FA-MAT-002](../../../../spec/lastenheft.md#lh-fa-mat-002--materialbibliothek), [LH-FA-MAT-003](../../../../spec/lastenheft.md#lh-fa-mat-003--materialzuweisung), [LH-FA-MAT-005](../../../../spec/lastenheft.md#lh-fa-mat-005--u-wert), [LH-FA-MAT-006](../../../../spec/lastenheft.md#lh-fa-mat-006--kostenkennwerte)]
adr_refs: [[ADR-0001](../../adr/0001-hexagonale-architektur.md), [ADR-0006](../../adr/0006-relationales-schema-design.md), [ADR-0012](../../adr/0012-evaluations-architektur.md)]
---

# Slice 017d: Material — Domäne + Verwaltung + Zuweisung (in-memory)

**Status:** done (2026-06-16, `make gates` grün, 137 Tests). Unabhängiges
**[MR-006](../../../../harness/conventions.md#mr-006--unabhängiges-plan-review-vor-implementierungs-start)-Plan-Review** (1 HIGH removeMaterial-RESTRICT + 1 MED + 2 LOW
eingearbeitet) ([Report](../../../reviews/2026-06-16-slice-017d-plan.md)).
**[MR-009](../../../../harness/conventions.md#mr-009--geometrielastiges-code-review-vor-welle-closure) n/a** (keine Geometrie/Persistenz/Parsing). Closure-Notiz §8.

**Welle:** welle-3-auswertung (vierter Slice; Material-Strang, parallel zu EVL).

**Bezug:** [LH-FA-MAT-001](../../../../spec/lastenheft.md#lh-fa-mat-001--materialien-verwalten) (verwalten) / MAT-002 (Bibliothek) / MAT-003 (Zuweisung)
/ MAT-005 (U-Wert) / MAT-006 (Kosten). Spec-Grundlage **liegt vor** (slice-017a):
§2.1 `model::Material`-Form + FK-Zuweisungs-Autorität; §1 effektive
Auflösungsregel. [ADR-0006](../../adr/0006-relationales-schema-design.md) (`materials`-Schema + `material_id`-FKs — der
Persistenz-Vertrag ist entschieden, hier **noch nicht** umgesetzt → slice-017e).
[ADR-0012](../../adr/0012-evaluations-architektur.md) (Material = von der Auswertung **konsumierte Eingabe**). [ADR-0001](../../adr/0001-hexagonale-architektur.md)
(Material = pure Domäne; Verwaltung über Driving-Port). **Kein neuer ADR**
(slice-017a-Closure: „Material ist [ADR-0006](../../adr/0006-relationales-schema-design.md)-/Spec-Sache").

**Autor:** Dietmar Burkard (AI-Harness-Lauf). **Datum:** 2026-06-16.

**Schnitt-Herkunft:** erste Material-Implementierung (Domäne + Verwaltung +
Zuweisung). **Port-Entscheidung (Projektinhaber 2026-06-16):** projekt-eigene
Materialien sind Teil des Domänenmodells (`materials.project_id`) → Verwaltung +
Zuweisung über den **bestehenden** `EditStructurePort`/`StructureEditService`
(Material = Bauteil-Eigenschaft), read-only Material-/Auflösungsdaten über
`EvaluatePort` (Quelle für EVL-004/006). **Kein neuer Port.** `MaterialLibraryPort`
(driven, architecture.md §1.2) bleibt der **externe Katalog-/Import**-Belang —
welle-3 zurückgestellt, architecture.md reconcilet.

**Bewusst NICHT Teil (Folge-Slices / benannte Lücken):**

- **`wall_type`-Template-Fallback der Auflösung** — die Domäne trägt `Wall.type`
  als **Enum** {Innen/Aussen/Trag}, **keine** material-tragende Wall-Type-Entität;
  der §1-Fallback „sonst über den `wall_type`" braucht erst eine solche Entität.
  017d liefert den **Override** (eigenes `material_id`) + Auflösung-zu-eigenem-
  Material; der Typ-Vorlage-Fallback ist **benannte Lücke** (Folge-Slice, spez. §1
  ehrlich nachziehen).
- **Material-Persistenz** (`materials`-Tabelle + `material_id`-Spalten-Round-Trip,
  die welle-2-`NULL`-Felder) → **slice-017e**.
- **EVL-004/006** (Material-/Kostenliste-Aggregation) → Folge-Slice (nutzt die
  hier gelegte `EvaluatePort`-Auflösung).
- **MAT-004 Texturen** (darstellungs-nah, Sicht) · Material-Rendering/Farbe.

---

## 1. Ziel

Das Gebäudemodell trägt **Materialien** als pure Domänen-Eigenschaft: anlegen/
ändern/entfernen (MAT-001), auflisten (MAT-002), an Wand/Dach/Decke **zuweisen**
(MAT-003) mit Kennwerten U-Wert (MAT-005) / Kosten (MAT-006). Verwaltung +
Zuweisung sind **Bauteil-Edit-Operationen** (`EditStructurePort`); die
**effektive Material-Auflösung** je Bauteil ist eine **read-only-Ableitung**
(`EvaluatePort`, Quelle für die EVL-Listen). Alles **in-memory** (Persistenz =
017e); **keine Geometrie**.

**Konsum statt Render ([ADR-0012](../../adr/0012-evaluations-architektur.md)-Geist):** Material wird in welle-3 **per Pull**
von der Auswertung konsumiert und hat (noch) **kein gerendertes Szenen-Korrelat**
(Farbe/Textur = MAT-004, Sicht). Material-Mutationen melden daher **keinen
`ModelChangedPort`-`op`** (ein `op` ohne Beobachtungs-Bezug wäre Vokabular ohne
Korrelat — vgl. [ADR-0012](../../adr/0012-evaluations-architektur.md) #3 / [ADR-0011](../../adr/0011-bauteil-hosting-wandoeffnung.md) #4); Re-Eval, sobald Material die
Darstellung treibt.

## 2. Definition of Done

- [x] **`model::Material` (neu, `src/hexagon/model/material.h`, pure Werte,
      framework-frei):** spez. §2.1-Form
      `{ MaterialId id; std::string name; std::string category;
      std::optional<double> u_value; std::optional<double> cost_per_m2;
      std::optional<double> cost_per_m3; std::optional<std::string> color_hex;
      std::optional<std::string> texture_path; }`. Starker Id-Typ
      `enum class MaterialId : int {}` (Muster `WallId`). **`Building.materials`**
      Vektor (`src/hexagon/model/building.h`). `color_hex`/`texture_path` werden
      getragen, aber in welle-3 nicht aktiv genutzt (MAT-004 Sicht).
- [x] **Material-CRUD (`EditStructurePort` + `StructureEditService`, additiv):**
      `addMaterial(const model::Material& prototype) → std::optional<MaterialId>`
      (id vom Service vergeben; **leerer/whitespace-only Name → kein Wert**,
      MAT-001 Negative) · `updateMaterial(MaterialId, const model::Material&) →
      bool` (Name/Kategorie/Kennwerte ändern; leerer Name → `false`, Modell
      unverändert; unbekannte Id → `false`) · `removeMaterial(MaterialId) → bool`
      (unbekannt → `false`; **noch zugewiesenes Material ist NICHT löschbar →
      `false`, Modell unverändert** — `on_delete: restrict`-Treue der
      `material_id`-FKs, [ADR-0006](../../adr/0006-relationales-schema-design.md) #5: **kein stiller Verlust der Zuweisung**;
      erst löschbar, wenn kein Bauteil es referenziert — HIGH-1). **Kein `op`**
      (s. §1).
- [x] **Zuweisung (MAT-003, `EditStructurePort` + Service):** `material_id` als
      **`std::optional<MaterialId>`** an `Wall` / `Roof` / `Slab` (Override).
      `setWallMaterial(WallId, std::optional<MaterialId>) → bool` (+ `setRoof…`/
      `setSlab…`): zuweisen (gültige Material-Id), **abwählen** (`std::nullopt` →
      „kein Material", immer ok), unbekannte Material-Id **oder** unbekanntes
      Bauteil → `false`/Modell unverändert. **Kein `op`** (s. §1).
- [x] **Effektive Auflösung (read-only, `EvaluatePort` + Service):**
      `effectiveMaterial(WallId|RoofId|SlabId) → std::optional<model::Material>`
      = das eigene `material_id` zu seinem `Material` aufgelöst; **kein
      `wall_type`-Fallback** in welle-3 (benannte Lücke — Wall.type ist Enum,
      keine material-tragende Typ-Entität). Unbekanntes Bauteil / kein Material →
      `std::nullopt` (Totalität, kein Wurf). + `materials() →
      const std::vector<Material>&` (MAT-002-Liste; read-only, kein Re-Detect,
      keine Mutation).
- [x] **AK-Tests mit `LH-FA-MAT-*` im Namen** (Kern, OCC-frei über
      `AnalyticGeometry`-Double, analytische Orakel):
      **MAT-001** (add Happy → in `materials()`; **leerer/whitespace Name → kein
      Wert**; update; **remove eines noch zugewiesenen Materials → `false`,
      unverändert** (RESTRICT, [ADR-0006](../../adr/0006-relationales-schema-design.md) #5); **remove nach Abwahl → entfernt**);
      **MAT-002** (mehrere Materialien → `materials()` listet sie);
      **MAT-003** (Wand+Material → `effectiveMaterial` liefert das Material mit
      Kennwerten; **unzugewiesen → `nullopt`, kein Fehler**; unbekannte
      Material-Id → `false`/unverändert; Abwahl `nullopt`); **MAT-005/006**
      (U-Wert/Kosten am Material gesetzt → über `effectiveMaterial` abrufbar);
      **Roof/Slab-Zuweisung** (analog Wand); **read-only/total** (`effectiveMaterial`/
      `materials` mutieren nicht; zwei Abfragen identisch). **Regression:**
      bestehende Tests (Bauteile/EVL) textlich unverändert grün (additiv).
- [x] **architecture.md reconcilet (Sicht-Stratum-Folgepflicht):** §1.1
      `EditStructurePort` trägt **Material-Verwaltung/-Zuweisung**; §1.1/§1.2
      `EvaluatePort` trägt **Material-Auflösung/-Liste** (Quelle EVL-004/006);
      `MaterialLibraryPort` (§1.2 driven) auf **externen Katalog-/Import** präzisiert
      (welle-3 zurückgestellt). **ADR-/Slice-frei im Körper** ([MR-011](../../../../harness/conventions.md#mr-011--referenz-integritäts-gate-matrix-ids-spans-hostpaths)/SDP); eine
      `## Geschichte`-Zeile mit **prägender ADR** ([ADR-0006](../../adr/0006-relationales-schema-design.md)/[ADR-0012](../../adr/0012-evaluations-architektur.md)), **kein
      Slice** (MEDIUM-1, §2.7 — architecture.md trägt keine Slices). spez. §1:
      (a) **Auflösungsregel** `wall_type`-Fallback als welle-3-Lücke (Override
      geliefert) benennen; (b) **Material-Name-Pflicht** (leer/whitespace →
      abgelehnt, MAT-001-Negative) als spez.-Regel (LOW-2, [MR-008](../../../../harness/conventions.md#mr-008--lastenheft-schärfung-bleibt-lösungsfrei)-Heimat statt
      nur im Test) + §8-Historie. **Lastenheft unberührt** (MAT-AK schon auf
      Niveau, 017a; kein
      [MR-010](../../../../harness/conventions.md#mr-010--lastenheft-header-version--oberste-9-historie-zeile)).
- [x] **`make arch-check` grün** (`Material`/Service-Pfad pure Domäne, kein
      OCC/Qt/SQLite); **`make test`** grün; **`make gates`** grün; Closure-Notiz
      mit Lerneintrag (Form: benannte Spec-Lücke = `wall_type`-Fallback);
      CHANGELOG ([MR-004](../../../../harness/conventions.md#mr-004--top-level-changelogmd-keep-a-changelog)). **[MR-009](../../../../harness/conventions.md#mr-009--geometrielastiges-code-review-vor-welle-closure) n/a.**

## 3. Plan (vor Code)

| Datei / Komponente | Änderungs-Art | Begründung |
|---|---|---|
| `src/hexagon/model/material.h` | neu | `model::Material` + `MaterialId` (pure Werte, spez. §2.1) |
| `src/hexagon/model/building.h` | ändern | `std::vector<Material> materials` |
| `src/hexagon/model/{wall,roof,slab}.h` | ändern | `std::optional<MaterialId> material_id` (Override) |
| `src/hexagon/ports/driving/edit_structure_port.h` | ändern | Material-CRUD + `set{Wall,Roof,Slab}Material` (additiv) |
| `src/hexagon/ports/driving/evaluate_port.h` | ändern | `effectiveMaterial(...)` + `materials()` (read-only) |
| `src/hexagon/services/structure_edit_service.{h,cpp}` | ändern | CRUD/Zuweisung/Auflösung in-memory; Remove RESTRICT (referenziert → abgelehnt) |
| `tests/hexagon/test_material.cpp` | neu | MAT-001/002/003/005/006-AK |
| `tests/CMakeLists.txt` | ändern | neue Testdatei |
| `spec/architecture.md` | ändern | §1.1/§1.2 Material-Reconciliation + `## Geschichte` |
| `spec/spezifikation.md` | ändern | §1 Auflösungsregel: `wall_type`-Fallback welle-3-Lücke |
| `spec/spezifikation-historie.md` | ändern | §8-Historie-Zeile |
| `CHANGELOG.md` | ändern | Slice-Eintrag ([MR-004](../../../../harness/conventions.md#mr-004--top-level-changelogmd-keep-a-changelog)) |
| `docs/reviews/2026-06-16-slice-017d-plan.md` | neu | [MR-006](../../../../harness/conventions.md#mr-006--unabhängiges-plan-review-vor-implementierungs-start)-Report |

## 4. Trigger

- slice-017a done ✓ (Material-Spec-Grundlage: §2.1 + §1-Auflösungsregel).
- Projektinhaber-Port-Entscheidung 2026-06-16: bestehende Ports (Option 1).
- **[MR-006](../../../../harness/conventions.md#mr-006--unabhängiges-plan-review-vor-implementierungs-start)-Plan-Review vor Start** (HIGHs blockieren) — **gelaufen ✓**,
  1 HIGH (removeMaterial-RESTRICT) + 1 MED + 2 LOW eingearbeitet → startbar.

## 5. Closure-Trigger

- DoD vollständig, `make gates` grün, Closure-Notiz → **slice-017e**
  (Material-Persistenz-Round-Trip) + **EVL-004/006** (Listen über die
  `EvaluatePort`-Auflösung); `wall_type`-Fallback-Lücke als Folge benannt.

## 6. Risiken und offene Punkte

- **Auflösungs-Teilumfang (Override-only, benannte Lücke):** der §1-Fallback
  „sonst über den `wall_type`" entfällt in welle-3 — Wall.type ist Enum, keine
  material-tragende Typ-Entität. 017d liefert nur den Override. Ehrlich in spez.
  §1 + Closure benannt; Folge-Slice (Wall-Type-Entität + Fallback). **[MR-006](../../../../harness/conventions.md#mr-006--unabhängiges-plan-review-vor-implementierungs-start) prüft
  die Lösungsfreiheit der Benennung.**
- **Remove-RESTRICT (HIGH-1, [ADR-0006](../../adr/0006-relationales-schema-design.md) #5):** ein **noch zugewiesenes** Material
  ist **nicht löschbar** (`removeMaterial → false`, Modell unverändert) — die
  `material_id`-FKs sind `on_delete: restrict` ([ADR-0006](../../adr/0006-relationales-schema-design.md) #5: kein **stiller**
  Verlust der Zuweisung, vgl. Datenverlust-Hard-Rule AGENTS §2.2). In-memory
  gespiegelt: erst löschbar, wenn kein Bauteil es referenziert. Test pinnt beide
  Fälle (referenziert → abgelehnt; nach Abwahl → entfernt). *(Plan-Review HIGH-1:
  die ursprüngliche „Verweise still räumen"-Variante war genau der von [ADR-0006](../../adr/0006-relationales-schema-design.md)
  verbotene stille Verlust.)*
- **Domäne↔Schema-Divergenz `density` (LOW-1):** das `materials`-Schema
  (`data-model.yaml`) trägt `density`, die spez.-§2.1-Domäne **nicht** — der
  Domänentyp folgt der Spec (Rang 2). Die Divergenz wird bei der Persistenz
  (017e) sichtbar; hier nur benannt.
- **Kein `op` für Material (Design, [ADR-0012](../../adr/0012-evaluations-architektur.md)-Geist):** Material-Mutationen melden
  nichts (kein Szenen-Korrelat in welle-3, Pull-Konsum). Re-Eval bei
  Material-Rendering (Farbe/Textur MAT-004). Im Test belegt (Mutation ohne
  Listener-Meldung), in Closure benannt.
- **Port-Wachstum `EditStructurePort`:** drei `set…Material`-Overloads +
  3 CRUD-Methoden. Bewusst am bestehenden Port (Projektinhaber-Entscheidung,
  Material = Bauteil-Edit) statt neuer Port — hält die Architekturfläche klein.
- **Name-Validierung (MAT-001):** leer/whitespace-only → abgelehnt; sonst keine
  Eindeutigkeits-Pflicht in welle-3 (Schema hat keine Unique-Constraint auf
  `materials.name`; Bibliothek darf Duplikate führen).
- **`EvaluatePort`-Vorbau:** `effectiveMaterial`/`materials` sind die minimalen
  read-only-Bausteine für EVL-004/006 — nicht mehr (keine Listen-Aggregation hier;
  die ist der Folge-Slice).

## 7. Sub-Area-Modus-Begründung

### Sub-Area: Domänen-Modell + Services (`src/hexagon/`)

- **Modus:** GF; Konventionen-Dichte hoch (pure Werttypen, starker Id-Typ,
  Driving-Port-Erweiterung am Muster Bauteil-Edit, read-only-Auflösung,
  Totalität); Phase-Reife 4 (Material-Spec 017a kohärent); Risiko niedrig
  (Domänen-Eigenschaft + Aggregation, keine Geometrie, kein OCC).

### Sub-Area: Test-Infrastruktur (`tests/`)

- **Modus:** GF; Dichte hoch (AK mit `LH-FA-MAT-`-ID, analytische Orakel,
  Registrierung); Risiko niedrig.

### Sub-Area: Spec-Schreibung (`spec/`)

- **Modus:** GF; Dichte mittel (§1-Auflösungsregel-Lücke ehrlich, architecture.md-
  Reconciliation Sicht-Stratum, SDP-/[MR-011](../../../../harness/conventions.md#mr-011--referenz-integritäts-gate-matrix-ids-spans-hostpaths)-Disziplin: kein ADR/Slice im Körper);
  Risiko niedrig (Doku).

## 8. Closure-Notiz

**Ergebnis:** `make gates` grün (EXIT 0; **137 Tests**, +6 Material-AK; Coverage
92,5 %). Das Material-System ist als **pure Domänen-Eigenschaft** umgesetzt:
`model::Material` (spez. §2.1, Kennwerte U-Wert/Kosten) + `Building.materials`;
Verwaltung/Zuweisung über `EditStructurePort`
(`addMaterial`/`updateMaterial`/`removeMaterial`/`set{Wall,Roof,Slab}Material`),
read-only-Liste/Auflösung über `EvaluatePort` (`materials()`/`effectiveMaterial`)
als Quelle für EVL-004/006. **Override-Auflösung** geliefert; `wall_type`-Fallback
zurückgestellt. Material-Mutationen **op-frei** (Pull-Konsum, kein Szenen-Korrelat).
**Kein neuer Port** (Projektinhaber-Entscheidung) — `MaterialLibraryPort` (driven)
auf externen Katalog präzisiert (architecture.md reconcilet, `## Geschichte`
ADR-Provenance statt Slice).

**DoD:** alle Haken erfüllt — `model::Material`/`MaterialId` + `Building.materials`
+ `material_id`-Override an Wand/Dach/Decke, CRUD + Zuweisung + effektive Auflösung,
AK [`LH-FA-MAT-001/002/003/005/006`](../../../../spec/lastenheft.md#lh-fa-mat-001--materialien-verwalten) (anlegen/Name-Pflicht/RESTRICT/Liste/Zuweisung/
Kennwerte/Roof+Slab/kein-op-Sanity), `make arch-check` + `make gates` grün, spez. §1
+ architecture.md nachgezogen, CHANGELOG.

**[MR-006](../../../../harness/conventions.md#mr-006--unabhängiges-plan-review-vor-implementierungs-start)-Review (unabhängig):** 1 HIGH + 1 MED + 2 LOW eingearbeitet. **HIGH-1 war
der wertvolle Fang:** `removeMaterial` „räumt Verweise still" widersprach
`on_delete: restrict` ([ADR-0006](../../adr/0006-relationales-schema-design.md) #5) — der genau-so von [ADR-0006](../../adr/0006-relationales-schema-design.md) verbotene **stille
Zuweisungs-Verlust**. Korrigiert auf RESTRICT-Treue (zugewiesenes Material nicht
löschbar). **[MR-009](../../../../harness/conventions.md#mr-009--geometrielastiges-code-review-vor-welle-closure) n/a** (keine Geometrie/Persistenz/Parsing).

**Lerneintrag (modul-05, Form „benannte Spec-Lücke"):** Der `wall_type`-Template-
Fallback der Material-Auflösung (spez. §1) ist welle-3 **nicht** umgesetzt — das
Domänenmodell trägt den Wandtyp als **Enum**, keine material-tragende Wall-Type-
Entität. Geliefert ist der **Override** (eigenes `material_id`); der Fallback ist in
spez. §1 als benannte Lücke + Re-Eval verankert (Folge-Bedarf: Wall-Type-Entität,
sobald typ-vererbtes Material gebraucht wird). *(Zweiter, kleiner Eintrag: das
Plan-Review fing den RESTRICT-HIGH — die Datenverlust-Hard-Rule AGENTS §2.2 wirkt
auch **in-memory**, nicht nur in der Persistenz.)*

**Restrisiko / Nachfolge:**
- **slice-017e (Material-Persistenz):** `materials`-Tabelle + `material_id`-Spalten-
  Round-Trip (die welle-2-`NULL`-Felder) + `on_delete: restrict` schema-seitig;
  Domäne↔Schema-`density`-Divergenz (LOW-1) dort auflösen. **Höhere Review-Latte
  (Projektinhaber 2026-06-16):** Persistenz bringt Parsing/Schema-Drift/stille
  Datenverfälschung → **unabhängiges Code-Review angesetzt** (Steering #4-Klasse).
- **EVL-004/006** (Material-/Kostenliste über `effectiveMaterial`).
- **`wall_type`-Fallback** (Wall-Type-Entität) · MAT-004 Texturen/Material-Rendering.
