---
id: slice-015c
titel: Decken/Fundament-Persistenz — slabs-Round-Trip (polygon_json mit Cutouts)
status: done
welle: welle-2-bauteile
lastenheft_refs: [LH-FA-SLB-001, LH-FA-SLB-003, LH-FA-FND-001, LH-FA-BLD-002, LH-FA-BLD-003]
adr_refs: [ADR-0001, ADR-0003, ADR-0006, ADR-0011]
---

# Slice 015c: Decken/Fundament-Persistenz — slabs-Round-Trip

**Status:** done (2026-06-14). MR-006-Plan-Review gelaufen
([Report](../../../reviews/2026-06-14-slice-015c-plan.md) — **keine HIGH**;
Plan-MED-1 Parser-Edge-Cases + Ein-Ring-AK und Plan-MED-2 Leeres-Projekt-
Regression eingearbeitet). **Code-Review danach** (Modul 11, unabhängig;
[Report](../../../reviews/2026-06-14-slice-015c-code-review.md)) — **keine
HIGH**, 3 MEDIUM eingearbeitet (CR-MED-1 `stod`-Totalität, CR-MED-2 Negativ-
Parse-Test, CR-MED-3 Bodenplatte-Mapper-Zweig). DoD vollständig, `make gates`
grün (105 Tests, Coverage 91,9 %). Closure-Notiz §8.

**Welle:** welle-2-bauteile (neunter Slice).

**Bezug:** ADR-0011-Folgepflicht (#6) „Persistenz" für das Platten-Modul
(slice-015b ließ Decken/Fundament nur im Speicher), LH-FA-SLB-001 (Decke),
**LH-FA-SLB-003 (Ausschnitte)**, LH-FA-FND-001 (Fundament),
LH-FA-BLD-002/003 (speichern/laden, atomar). ADR-0003 (SQLite-Adapter,
atomar), ADR-0006 (`slabs`-Schema, **liegt vor**: `slab_type`,
`thickness_mm`, `polygon_json`, `material_id`), ADR-0001 (Regel D, SQLite
nur im Adapter).

**Autor:** Dietmar Burkard. **Datum:** 2026-06-14.

**Schnitt-Herkunft:** Persistenz-Hälfte des Platten-Strangs (Muster
013b/013c, 014b/014c), in 015b bewusst herausgelöst. Vor dem ersten
lebenden Save-Use-Case (heute kein Save-Pfad in `main` verdrahtet → kein
§2.2-Datenverlust-Fenster).

---

## 1. Ziel

`Building.slabs` (Decken/Fundament/Bodenplatten aus slice-015b) überleben
Speichern/Laden: der `SqliteProjectRepository` schreibt Platten in die
bestehende `slabs`-Tabelle (ADR-0006) in derselben atomaren Transaktion
wie Wände/Öffnungen/Dächer und liest sie feldgleich zurück — **inklusive
der Aussparungen (LH-FA-SLB-003)**, weil `cutouts` ein echtes
Domänenfeld ist.

## 2. Definition of Done

- [x] **`save` schreibt Platten** in `slabs` (explizite `id`,
      `project_id` Literal `1`, `storey_id`, `slab_type` text
      decke/fundament/bodenplatte, `thickness_mm`, `polygon_json`).
      `material_id` bleibt `NULL` (welle-2-Scope: kein Material —
      **nicht** im Domänenmodell). Innerhalb der bestehenden
      `BEGIN`/`COMMIT`-Transaktion, **nach `insertStoreys`** (FK
      `slabs.storey_id → storeys.id`). `base_z` wird **nicht** gespeichert
      (abgeleitet aus `type`/Geschoss, `slab.h`/`slab_geometry`) → keine
      Spalte, nichts zu persistieren.
- [x] **`polygon_json`-Format entschieden + dokumentiert:** verschachtelte
      Ring-Arrays `[[fx0,fy0,fx1,fy1,…],[c1x0,c1y0,…],…]` — **Element 0 =
      Grundriss-Ring (footprint), Elemente 1..n = Ausschnitt-Ringe
      (cutouts)**, jeder Ring eine flache `x,y`-Folge mit `%.17g`
      (double-Round-Trip, DBL_DECIMAL_DIG). Generalisiert das 014c-Flach-
      Array (die in der 014c-Closure angekündigte Erweiterung „Ring statt
      5-Tupel"). Kein JSON-Präzedenz-Bruch: **ein fokussierter
      Serialisierer + Parser** im Persistenz-Adapter (variabel lang →
      `std::string`-Builder statt der 256-Byte-`snprintf`-Konstante von
      014c); kapselt, verlässt den Adapter nicht; liest nur eigene Writes.
- [x] **Cutouts round-trippen (nicht `NULL` — Abgrenzung zu 014c):**
      `cutouts` ist ein **Domänen-Modell-Feld** (`Slab::cutouts`, 015b
      `addSlabCutout`/`cutoutInsideSlab`) und ein sichtbares Feature —
      eine Platte, die ihren Ausschnitt beim Speichern verlöre, wäre
      **stiller Verlust eines realen Domänen-Features** (§2.2-Empfindlich-
      keit). Gegensatz roofs: `height_mm` war *abgeleitet*, `material_id`
      *nicht* im Domänenmodell → dort legitim `NULL`. Hier sind
      footprint **und** cutouts domänen-getragen → beide round-trippen.
- [x] **`load` rekonstruiert `Building.slabs`**: `SELECT` aus `slabs`
      (geordnet nach `id`); `slab_type` → `SlabType`; `polygon_json` →
      `footprint` (Ring 0) + `cutouts` (Ring 1..n). Feldgleich für die
      welle-2-Domänenfelder (`Slab`: id, storey_id, type, thickness,
      footprint-Punkte, cutouts).
- [x] **`slab_type`-Mapper total:** `slabTypeToText`/`textToSlabType`
      (Muster `roofTypeToText`) — das Schema trägt **keine**
      `CHECK`-Constraint auf `slab_type` (anders als `roofs`), daher
      erzwingt der Mapper die gültige Menge {decke,fundament,bodenplatte}
      und wirft bei Unbekanntem neutral (`E-IO`, kein Lib-Typ leckt).
- [x] **Round-Trip-AK-Test** (`LH-FA-SLB`/`LH-FA-FND` im Namen): Building
      mit ≥ 1 **Decke** (Polygon-footprint ≥ 4 Punkte, **mindestens ein
      nicht-glatter `double`**, z. B. `1234.56789012345`, mit
      `EXPECT_DOUBLE_EQ` → belegt `%.17g` diskriminierend; MED-1-Präzedenz
      014c) **mit ≥ 1 Ausschnitt** **und** ≥ 1 **Fundament** (anderer Typ,
      ohne Ausschnitt) → `save` → `load` → `slabs` feldgleich (Anzahl, id,
      storey_id, type, thickness, footprint-Punkte feldgleich, cutouts-
      Anzahl + Punkte feldgleich). Bestehende BLD-002/003-, DOR/WIN- und
      ROF-Round-Trip-Tests bleiben textlich grün (Regression).
- [x] **Kein Schema-Wechsel** (ADR-0006-Tabelle liegt vor) → **kein**
      `make schema-check`-Drift. `make gates` grün; arch-check Regel D
      (kein `sqlite3*` außerhalb `adapters/persistence/`); Closure-Notiz
      mit Lerneintrag; CHANGELOG (MR-004); **neue ADR-0011-(#6)-Index-Zeile
      „Decken/Fundament (LH-FA-SLB-*/FND-*) → erfüllt durch slice-015a/b/c"
      ergänzen** (analog der Dach-Zeile von 014c — es existiert noch
      **keine** SLB/FND-Zeile im Folgepflichten-Index).

## 3. Plan (vor Code)

| Datei / Komponente | Änderungs-Art | Begründung |
|---|---|---|
| `src/adapters/persistence/sqlite_project_repository.cpp` | ändern | `slabTypeToText`/`textToSlabType`, `polygonJsonForSlab`/`parseSlabPolygonJson` (nested rings), `insertSlabs`/`loadSlabs`; Aufruf in `save` nach `insertRoofs`, in `load` nach `loadRoofs` |
| `tests/adapters/test_sqlite_project_repository.cpp` | ändern | Round-Trip-AK mit Decke (footprint + Ausschnitt, nicht-glatter double) + Fundament; using-Decls (Slab/SlabId/SlabType/Footprint/Point2D) |
| `docs/plan/adr/README.md` | ändern | ADR-0011-(#6)-Folgepflicht „Decken/Fundament" → erfüllt (slice-015a/b/c) |
| `CHANGELOG.md` | ändern | Slice-Eintrag (MR-004) |
| `docs/reviews/2026-06-14-slice-015c-plan.md` | neu | MR-006-Report |

## 4. Trigger

- slice-015b done ✓ (`Building.slabs` + Domänen-Typ `Slab` mit footprint +
  cutouts).
- MR-006-Plan-Review vor Implementierungs-Start.

## 5. Closure-Trigger

- DoD vollständig, `make gates` grün, Closure-Notiz → die Platten-Familie
  (015a/b/c) ist abgeschlossen; welle-2 setzt mit **STR (Treppen)** entlang
  der ADR-0011-Leitplanke fort, danach Welle-2-Closure.

## 6. Risiken und offene Punkte

- **Verschachtelte `polygon_json` (zentrale Entscheidung):** das
  014c-Format war ein festes 5-Zahlen-Array (fixe `snprintf`-Konstante);
  Platten haben variabel viele Punkte und 0..n Ausspar-Ringe → der
  Serialisierer baut einen `std::string` dynamisch, der Parser scannt
  **balancierte `[...]`-Sub-Arrays** (eigenes, deterministisches Format,
  liest nur eigene Writes). Robustheit per Round-Trip-AK belegt, nicht per
  allgemeinem JSON-Parser; defensiver Parse → Format-Fehler ⇒ neutraler
  `E-IO`-Wurf (Adapter total nach außen, Regel D gewahrt).
- **Ring-Validität:** jeder Ring hat **gerade** Zahl an Werten (x,y-Paare);
  ungerade Anzahl / leerer Top-Level / nicht-parsbare Zahl → `E-IO`. Der
  footprint ist Pflicht (Ring 0 muss existieren); cutouts optional (0..n).
- **Fremdschlüssel-Reihenfolge:** `slabs.storey_id → storeys.id` +
  `ON DELETE CASCADE`; Platten **nach** `insertStoreys` einfügen.
- **`%.17g`-Präzision:** nötig für exakten double-Round-Trip im Text
  (`polygon_json` ist Text → Präzision explizit; nicht-glatter double im AK
  diskriminierend, MED-1-Präzedenz 014c).
- **`slab_type` ohne `CHECK`:** das Schema lässt jeden Text zu — der
  Mapper erzwingt die gültige Menge und wirft bei Unbekanntem (sonst
  schlüge erst der `textToSlabType`-`load` fehl). Gültige Menge
  decke/fundament/bodenplatte, kleinschreibung (Muster roof_type).
- **Welle-2-Feld-Scope:** `material_id` bleibt `NULL` (nicht im
  Domänenmodell); sein Round-Trip kommt mit Material (welle-3). `base_z`
  wird bewusst **nicht** gespeichert (abgeleitet) — kein Round-Trip-Verlust,
  da der Kern es bei Bedarf neu ableitet (`slab_geometry`).
- **Explizite `id` in AUTOINCREMENT-PK:** wie `walls`/`roofs` (SQLite
  übernimmt gebundene Werte).
- **Keine `Building`-Identität über `==`:** der AK vergleicht Felder
  einzeln (Muster bestehender Round-Trip-Tests).

## 7. Sub-Area-Modus-Begründung

### Sub-Area: Persistenz-Adapter (`src/adapters/persistence/`)

- **Modus:** GF; Dichte hoch (SQLite nur im Adapter/Regel D, atomar,
  ADR-0006-Schema, E-IO-Codes, JSON-Ser/De gekapselt); Risiko niedrig–
  mittel (erste **verschachtelte** JSON-Ser/De im Repo — durch
  Round-Trip-AK gedeckt).

### Sub-Area: Test-Infrastruktur (`tests/`)

- **Modus:** GF; Dichte hoch (Round-Trip-AK mit `LH-`-ID, Feld-Vergleich
  inkl. Ring-Punkte); Risiko niedrig.

## 8. Closure-Notiz

**Closure-Kriterien (beobachtbar):**

- **`save`/`load` round-trippen Platten** über die `slabs`-Tabelle
  (`sqlite_project_repository.cpp` `insertSlabs`/`loadSlabs` +
  `slabTypeToText`/`textToSlabType` + `polygonToJson`/`parseSlabPolygonJson`).
  Platten in derselben atomaren `BEGIN`/`COMMIT`-Transaktion **nach**
  `insertStoreys` (FK `storey_id`); `slab_type`/`thickness_mm` als Spalten,
  Grundriss **und Aussparungen** als `polygon_json`; `material_id` bleibt `NULL`,
  `base_z` wird nicht gespeichert (abgeleitet).
- **Verschachtelte JSON-Ser/De (erste im Repo):** `polygon_json` =
  `[[footprint-Ring],[cutout-Ring]…]`, `%.17g`; generalisiert das 014c-Flach-
  Array zu Ringen (die in der 014c-Closure angekündigte Erweiterung „Ring statt
  5-Tupel"). `std::string`-Builder (variabel lang) + balancierter `[...]`-Scan
  auf Tiefe 1; Parser total (Format-Fehler/unbalanciert/ungerade Wertzahl →
  neutraler `E-IO`-Wurf, kein Lib-Typ leckt — Regel D gewahrt).
- **Cutouts round-trippen feldgleich (Abgrenzung zu 014c):** `cutouts` ist ein
  Domänen-Modell-Feld (`Slab::cutouts`) → es wird persistiert, nicht `NULL`
  gesetzt (anders als die *abgeleitete* roofs-`height_mm` / das *nicht-
  domänen* `material_id`). Kein stiller Verlust eines realen Features (§2.2).
- **`slab_type`-Mapper total:** das Schema trägt **keine** CHECK-Constraint auf
  `slab_type` (anders als `roofs`) → der Mapper {decke,fundament,bodenplatte}
  erzwingt die gültige Menge und wirft bei Unbekanntem (`E-IO`).
- **Round-Trip-AK** `SqliteProjectRepository_LH_FA_SLB_FND.RoundTripErhaeltPlatten`:
  **Decke** (Polygon-footprint, nicht-glatter `double` 1234.56789012345 prüft
  `%.17g` diskriminierend) **mit Ausschnitt** (LH-FA-SLB-003) + **Fundament**
  ohne Ausschnitt (**Ein-Ring-Fall**, `cutouts.empty()`, Plan-MED-1) +
  **Bodenplatte** (dritter Mapper-Zweig, CR-MED-3) → feldgleich (id, storey_id,
  type, thickness, footprint-Punkte, cutout-Punkte). `LeeresProjektRoundTrip` um
  `slabs.empty()` erweitert (Plan-MED-2). **Negativ-Parse-AK**
  `MalformedSpaltenWerfenNeutral` (CR-MED-2): white-box-korrumpierte
  `polygon_json`/`slab_type` (ungerade Wertzahl, unbalanciert, Müll-Suffix,
  unbekannter Typ) → `load` wirft neutral `E-IO`. Bestehende BLD-002/003-,
  DOR/WIN- und ROF-Round-Trip-Tests **textlich unverändert grün** (Regression).
- **Kein Schema-Wechsel** (ADR-0006 trug `slabs`); `make schema-check` nicht
  ausgelöst. **`make gates` grün** (2026-06-14): docs-check 0, gate-consistency,
  arch-check **A–E** (Regel D: `sqlite3*` nur im Persistenz-Adapter), lint 0 +
  suppression-gate, **Tests 105/105** (zuvor 103 → +Round-Trip-Platten →
  +Negativ-Parse), Coverage **91,9 %** (Parser-Wurf-Pfade durch den
  Negativ-Test gedeckt).
- **ADR-0011 (#6)-Index:** neue Folgepflicht-Zeile „Decken/Fundament
  (LH-FA-SLB-*/FND-*) → erfüllt durch slice-015a/b/c" ergänzt (analog der
  Dach-Zeile von 014c).

**Lerneintrag:**

- **Erste verschachtelte JSON-Ser/De — das 014c-Format wuchs wie angekündigt:**
  die 014c-Closure sagte „beim Polygon-Grundriss-Vollumfang wird das Format zum
  Ring erweitert" — genau das trat ein. Das feste 5-Zahlen-Array (fixe
  `snprintf`-Konstante) wich einem `std::string`-Builder + Tiefe-1-Klammer-Scan.
  Schlüssel: der Parser bleibt **total nach außen** (E-IO statt Lib-Typ-Leck),
  obwohl er strukturierter ist — die Regel-D-Kapselung skaliert mit der
  Format-Komplexität.
- **„Domänen-getragen ⇒ round-trippt" als scharfe Trennlinie:** der 013c/014c-
  Grundsatz (Nicht-Domänen-Felder → NULL/Default) hat eine *positive* Kehrseite,
  die hier zum ersten Mal griff: `cutouts` **muss** persistieren, weil die
  Domäne es trägt — sonst wäre es stiller Feature-Verlust. Die Schema-Spalte
  `polygon_json` musste dafür ein zweites Konzept (Ausspar-Ringe) aufnehmen,
  ohne neue Spalte/Tabelle.
- **Schema-Voraussicht (5. Vorkommen):** `slabs` lag vollständig vor — rein
  mechanische Adapter-Erweiterung, kein DDL-Touch.
- **Code-Review zahlt auch bei „mechanischer" Persistenz (nicht nur Geometrie):**
  die Praxis „Slices vor Welle-Closure unabhängig code-reviewen" (3× nach
  013b/014b/015b geometrielastig) fand hier am **Persistenz**-Slice eine echte
  Totalitäts-Lücke trotz grüner Gates — `std::stod` verschluckte Müll-Suffixe
  still (`"1.5x"`→`1.5`), womit der im Code/CHANGELOG behauptete „total/E-IO"-
  Vertrag verletzt war. Lehre: nicht nur Geometrie-Slices reviewen; jeder Slice
  mit **eigener Parsing-/Format-Logik** trägt die gleiche Lücken-Klasse. Fix:
  `stod(&consumed)` + Vollständig-Verbrauch-Check + Negativ-Parse-AK.

**Restrisiko / Nachfolge:** Platten-Familie (015a/b/c) abgeschlossen.
Persistenz weiterhin ohne lebenden Save-Use-Case (bereit, sobald
Speichern/Laden in `main` verdrahtet wird — kein §2.2-Datenverlust-Fenster).
Offen (welle-3+): `material_id`-Round-Trip mit Material, nicht-rechteckige
Ausschnitte, Auto-Ableitung des Grundrisses aus dem Gebäudeumriss.
welle-2-Fortsetzung: **STR (Treppen)** entlang der ADR-0011-Leitplanke, dann
Welle-2-Closure (unabhängige Verifikation + `done/welle-2-results.md` inkl.
Carveout-Audit).
