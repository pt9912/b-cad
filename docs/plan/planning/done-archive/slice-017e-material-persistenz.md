---
id: slice-017e
titel: Material-Persistenz — materials-Tabelle + material_id-Round-Trip (SqliteProjectRepository)
status: done
welle: welle-3-auswertung
lastenheft_refs: [[LH-FA-MAT-001](../../../../spec/lastenheft.md#lh-fa-mat-001--materialien-verwalten), [LH-FA-MAT-003](../../../../spec/lastenheft.md#lh-fa-mat-003--materialzuweisung), [LH-FA-BLD-002](../../../../spec/lastenheft.md#lh-fa-bld-002--projekt-speichern)]
adr_refs: [[ADR-0001](../../adr/0001-hexagonale-architektur.md), [ADR-0003](../../adr/0003-persistenz-sqlite.md), [ADR-0006](../../adr/0006-relationales-schema-design.md)]
---

# Slice 017e: Material-Persistenz — materials + material_id-Round-Trip

**Status:** done (2026-06-16, `make schema-check` + `make gates` grün, 139 Tests).
Unabhängig (höhere Review-Latte, Persistenz): **[MR-006](../../../../harness/conventions.md#mr-006--unabhängiges-plan-review-vor-implementierungs-start)-Plan-Review** (0 HIGH)
+ **Code-Review** (0 HIGH, 1 LOW gehärtet
— [Report](../../../reviews/2026-06-16-slice-017e-code-review.md)).
**[MR-009](../../../../harness/conventions.md#mr-009--geometrielastiges-code-review-vor-welle-closure) n/a** (keine Geometrie). Closure-Notiz §8.

**Welle:** welle-3-auswertung (fünfter Slice; Material-Strang, Persistenz-Hälfte —
macht 017d durabel; Gegenstück 013c/014c/015c/016c).

**Bezug:** [LH-FA-MAT-001](../../../../spec/lastenheft.md#lh-fa-mat-001--materialien-verwalten) (Materialien) / MAT-003 (Zuweisung) durabel; [LH-FA-BLD-002](../../../../spec/lastenheft.md#lh-fa-bld-002--projekt-speichern)
(Projekt speichern, atomar) ist der Mechanismus. [ADR-0003](../../adr/0003-persistenz-sqlite.md) (SQLite-Persistenz,
atomar Temp+Rename), [ADR-0006](../../adr/0006-relationales-schema-design.md) (`materials`-Schema + `material_id`-FKs **liegen
vor** — der Schema-Vertrag ist entschieden, hier nur die Round-Trip-Mechanik),
[ADR-0001](../../adr/0001-hexagonale-architektur.md) (Adapter erfüllt `ProjectRepositoryPort`, Regel D: `sqlite3*` nur im
Persistenz-Adapter).

**Autor:** Dietmar Burkard (AI-Harness-Lauf). **Datum:** 2026-06-16.

**Schnitt-Herkunft:** Persistenz-Hälfte des Material-Strangs (Nachfolge 017d).
**Schemafrei:** `materials`-Tabelle + `material_id`-Spalten (walls/roofs/slabs) +
`on_delete: restrict` sind **bereits** in `schema.sql` **und** `data-model.yaml`
([ADR-0006](../../adr/0006-relationales-schema-design.md)-Schema-Voraussicht) → **kein Schema-Wechsel**, `make schema-check`
bleibt grün. 017e füllt die welle-2-`NULL`-Felder.

**Bewusst NICHT Teil (benannte Lücken / Folge):**

- **`density`-Spalte** — im Schema (`materials.density`), **nicht** im Domänen-
  `Material` (spez. §2.1). Folgt dem etablierten Muster „Nicht-Domänen-Feld wird
  nicht persistiert" via **Spalten-Auslassung** (F2: wie roofs.`height_mm`/
  `material_id` — nicht in der INSERT-Liste ⇒ `NULL`): **nicht round-getrippt**,
  beim Laden ignoriert — benannte Lücke (löst die 017d-LOW-1); round-trippt,
  sobald die Domäne `density` trägt.
- **`wall_types`-Persistenz** + `wall_type_id`-Spalte (Typ-Vorlage-Material) —
  bleibt `NULL` (welle-2-Scope; Wall-Type-Entität ist die 017d-Fallback-Folge).
- **`stairs`/`openings`** tragen kein `material_id` (Schema) — n/a.

---

## 1. Ziel

Material wird **durabel**: das `SqliteProjectRepository` round-trippt die
**`materials`-Tabelle** (Material-Bibliothek) und die **`material_id`-Spalten**
an Wand/Dach/Decke (die Zuweisung) — in derselben **atomaren Transaktion**
(Temp+Rename, [ADR-0003](../../adr/0003-persistenz-sqlite.md)) wie der Rest des Modells. **Kein Schema-Wechsel** (Spalten
liegen vor). **ID-Erhalt:** die Domänen-`MaterialId` wird wie alle Bauteil-Ids
explizit gebunden (`bind_int(static_cast<int>(id))`) → `wall.material_id` zeigt
nach dem Laden auf dasselbe Material.

**Korrektheits-Kern (Code-Review-Primärziel):** **NULL-vs-0.0**. Ein Material
**ohne** U-Wert/Kosten muss als **`std::nullopt`** zurückkommen, **nicht** als
`0.0` — `sqlite3_column_double` liefert für `NULL` stillschweigend `0.0`. Daher
beim Laden **jede** optionale Spalte über `sqlite3_column_type(...) == SQLITE_NULL`
prüfen, bevor der Wert gelesen wird (gleiches für `material_id`: NULL → `nullopt`).
Stille Verfälschung „kein Wert → 0" ist der schärfste Fehler dieses Slice.

## 2. Definition of Done

- [x] **`materials`-Tabelle round-trippen (`insertMaterials`/`loadMaterials`,
      `sqlite_project_repository.cpp`):** INSERT/SELECT über
      `id,name,category,color_hex,texture_path,u_value,cost_per_m2,cost_per_m3`
      (+ `project_id` Literal 1). **ID-Erhalt** (`bind_int(static_cast<int>(id))`).
      **Optionale Felder:** leeres `std::optional` → `sqlite3_bind_null`; beim
      Laden **`sqlite3_column_type == SQLITE_NULL` → `std::nullopt`**, sonst der
      Wert (Korrektheits-Kern §1). **`density` nicht in der INSERT-Spaltenliste**
      (⇒ `NULL` via Auslassung, Muster wie roofs.`height_mm`; kein Domänenfeld,
      benannte Lücke), beim Laden nicht gelesen. **Insert-Reihenfolge:**
      `materials` **vor** walls/roofs/slabs (FK `material_id → materials.id`,
      `PRAGMA foreign_keys=ON`).
- [x] **`material_id`-Spalten round-trippen (walls/roofs/slabs):**
      `insert{Walls,Roofs,Slabs}` ergänzen `material_id` in INSERT (gesetztes
      `optional<MaterialId>` → `bind_int`; leer → `bind_null`); `load{…}` lesen
      `material_id` (`SQLITE_NULL` → `std::nullopt`, sonst
      `static_cast<MaterialId>`). `stairs`/`openings` unverändert (kein Material).
      `wall_type_id` bleibt `NULL` (unverändert).
- [x] **Atomarität/Totalität ([ADR-0003](../../adr/0003-persistenz-sqlite.md), unverändert):** Materialien werden in der
      **bestehenden** BEGIN/COMMIT-Transaktion vor walls geschrieben; ein Fehler
      lässt den Vorstand intakt (E-IO, Temp verworfen). **Kein** neuer Parse-Pfad
      (Kennwerte sind `REAL`/`TEXT`, kein JSON) — die enum-/JSON-Totalitäts-Risiken
      der anderen Bauteile entfallen; der Totalitäts-Fokus ist die NULL-Behandlung.
- [x] **Round-Trip-AK-Tests (`test_sqlite_project_repository.cpp`):**
      **(a)** Material mit **vollen** Kennwerten (name/category/u_value/cost) +
      Zuweisung an Wand → save/load → Material-Liste identisch (id/name/category/
      Kennwerte), `wall.material_id` erhalten; das Material wird **manuell gegen
      `loaded.materials` aufgelöst** (F1: kein `effectiveMaterial` auf dem bloßen
      `Building` — das ist eine Service-Methode) und ist identisch.
      **(b) NULL-Korrektheit (Code-Review-Sentinel):** Material **ohne**
      U-Wert/Kosten → nach Reload `u_value`/`cost_*` == `std::nullopt`
      (**nicht** `0.0`). **(c)** Bauteil **ohne** Material (`material_id` leer) →
      nach Reload `nullopt` (nicht `MaterialId{0}`). **(d)** Dach + Decke mit
      Material → `material_id` erhalten. **(e) leere** Material-Liste round-trippt
      (kein Material). **Regression:** bestehende Round-Trip-/Negativ-Tests
      textlich unverändert grün.
- [x] **`make schema-check` grün (kein Drift, kein Schema-Edit)** — Beleg, dass
      017e rein adapter-seitig ist; **`make gates`** grün; **unabhängiges
      Code-Review** (Projektinhaber-Latte: Parsing/Schema-Drift/stille
      Verfälschung) ohne offene HIGH; Closure-Notiz mit Lerneintrag; CHANGELOG
      ([MR-004](../../../../harness/conventions.md#mr-004--top-level-changelogmd-keep-a-changelog)). **Spec:** spez. §1 Material-Block um „durabel/Round-Trip;
      `density` nicht round-getrippt (Lücke)" + §8-Historie. **Lastenheft
      unberührt** (kein [MR-010](../../../../harness/conventions.md#mr-010--lastenheft-header-version--oberste-9-historie-zeile)).

## 3. Plan (vor Code)

| Datei / Komponente | Änderungs-Art | Begründung |
|---|---|---|
| `src/adapters/persistence/sqlite_project_repository.cpp` | ändern | `insertMaterials`/`loadMaterials`; `material_id` in insert/load{Walls,Roofs,Slabs}; Insert-Reihenfolge materials vor walls |
| `src/adapters/persistence/sqlite_project_repository.h` | ggf. ändern | private Helfer-Deklarationen (falls Header-geführt) |
| `tests/adapters/test_sqlite_project_repository.cpp` | ändern | Material-Round-Trip-AK (inkl. NULL-Korrektheit) |
| `spec/spezifikation.md` | ändern | §1 Material-Block: durabel + `density`-Lücke |
| `spec/spezifikation-historie.md` | ändern | §8-Historie-Zeile |
| `CHANGELOG.md` | ändern | Slice-Eintrag ([MR-004](../../../../harness/conventions.md#mr-004--top-level-changelogmd-keep-a-changelog)) |
| `docs/reviews/2026-06-16-slice-017e-plan.md` | neu | [MR-006](../../../../harness/conventions.md#mr-006--unabhängiges-plan-review-vor-implementierungs-start)-Report |
| `docs/reviews/2026-06-16-slice-017e-code-review.md` | neu | Code-Review-Report (höhere Latte) |

**Kein Schema-Edit** (`schema.sql`/`data-model.yaml` unverändert — Spalten liegen vor).

## 4. Trigger

- slice-017d done ✓ (`model::Material` + `optional<material_id>` an Wand/Dach/Decke).
- [ADR-0006](../../adr/0006-relationales-schema-design.md)-Schema-Voraussicht ✓ (`materials` + `material_id`-FKs + `on_delete: restrict`).
- **[MR-006](../../../../harness/conventions.md#mr-006--unabhängiges-plan-review-vor-implementierungs-start)-Plan-Review vor Start** (HIGHs blockieren) — **gelaufen ✓**,
  0 HIGH (3 LOW/INFO Wording-Präzisierungen F1/F2/F3 eingearbeitet) → startbar.
- Projektinhaber 2026-06-16: höhere Review-Latte → Code-Review angesetzt.

## 5. Closure-Trigger

- DoD vollständig, `make schema-check` + `make gates` grün, **Code-Review ohne
  offene HIGH**, Closure-Notiz → danach **EVL-004/006** (Listen) + `wall_type`-
  Fallback; dann Welle-3-Verifikation + `done/welle-3-results.md`.

## 6. Risiken und offene Punkte

- **NULL-vs-0.0 (HIGH-Klasse, Code-Review-Primärziel):** `sqlite3_column_double`
  gibt für `NULL` `0.0` — ohne `column_type == SQLITE_NULL`-Prüfung würde „kein
  U-Wert" still zu `0.0`. **Stille Verfälschung.** AK (b) pinnt es; das
  Code-Review prüft jede optionale Spalte.
- **FK-Schreib-Reihenfolge:** `materials` muss **vor** walls/roofs/slabs
  insertet werden (`material_id`-FK, `foreign_keys=ON`), sonst Constraint-Fehler
  beim Save. Test belegt einen Save mit zugewiesenem Material.
- **ID-Erhalt:** Domänen-`MaterialId` explizit gebunden (Muster aller Bauteil-Ids)
  → `wall.material_id` zeigt nach Reload aufs richtige Material. Test belegt.
- **`density`-Lücke (löst 017d-LOW-1):** Schema-Spalte ohne Domänenfeld → `NULL`,
  nicht round-getrippt. Benannt (spez. §1 + Closure); round-trippt mit künftigem
  Domänen-`density`. **Kein** Daten*verlust* an einem Domänenwert (die Domäne hat
  keinen).
- **Dangling `material_id` nach DB-Korruption (F3):** **`load()` ist total** —
  es füllt die Vektoren, **validiert `material_id` nicht** gegen `materials`. Ein
  extern verbogenes `material_id` (Material fehlt) lädt graceful (Feld gesetzt);
  die **Auflösung zu `nullopt`** ist eine **Service-Eigenschaft**
  (`resolveOwnMaterial`), nicht des Lade-Pfads. `foreign_keys=ON` greift nur beim
  **Schreiben**. Kein Wurf, kein E-IO-Fall; **kein DoD-Beleg** des Dangling-Falls
  (nur der Leer-Fall AK (c) ist repo-beobachtbar).
- **Kein neuer Parse-Pfad:** Kennwerte `REAL`/`TEXT` (kein JSON/Enum) → die
  `std::stod`-Suffix-/Enum-Mapper-Risiken der anderen Bauteile entfallen — die
  Latte liegt auf NULL-Korrektheit + ID-/FK-Erhalt, nicht auf Tokenizing.
- **schema-check:** muss grün **ohne** Schema-Edit bleiben — Beleg, dass 017e rein
  adapter-seitig ist (kein Drift). Im Closure benennen.

## 7. Sub-Area-Modus-Begründung

### Sub-Area: Persistenz-Adapter (`src/adapters/persistence/`)

- **Modus:** GF; Konventionen-Dichte hoch (SQLite-Schema-/Round-Trip-Konvention,
  Atomarität, ID-Erhalt, E-IO-Totalität, Regel D); Phase-Reife 4 (Round-Trip-Muster
  über fünf Bauteil-Familien etabliert); **Risiko mittel** — Persistenz trägt die
  stille-Verfälschungs-Klasse (NULL-vs-0.0) → **Code-Review-Latte** (Projektinhaber).

### Sub-Area: Test-Infrastruktur (`tests/`)

- **Modus:** GF; Dichte hoch (Round-Trip-AK + NULL-Korrektheits-Sentinel +
  White-Box-Korruption); Risiko niedrig.

### Sub-Area: Spec-Schreibung (`spec/`)

- **Modus:** GF; Dichte niedrig (§1-Durabilitäts-/`density`-Lücken-Notiz); Risiko
  niedrig (Doku).

## 8. Closure-Notiz

**Ergebnis:** `make schema-check` + `make gates` grün (**139 Tests**, +2 Material-
Round-Trip-AK; **kein Schema-Drift**). Material ist **durabel**: das
`SqliteProjectRepository` round-trippt die `materials`-Bibliothek
(`insertMaterials`/`loadMaterials`) und die `material_id`-Zuweisung an
Wand/Dach/Decke (insert/load{Walls,Roofs,Slabs}) in der bestehenden atomaren
Speicher-Transaktion (materials **vor** den Bauteilen — FK). **NULL-sicher** über
drei zentrale `*Optional*`-Helfer (`column_type == SQLITE_NULL → nullopt`) — eine
Quelle der Wahrheit, kein „NULL → 0". ID-Erhalt (`material_id` zeigt nach Reload
aufs richtige Material). **Kein Schema-Wechsel** (Spalten + FKs lagen vor);
`density` ohne Domänenfeld → nicht round-getrippt (löst 017d-LOW-1).

**DoD:** alle Haken erfüllt — Round-Trip materials + material_id, NULL-Korrektheit,
Leer-/Unzugewiesen-Pfad, ID-/FK-Erhalt; spez. §1 Durabilität + density-Lücke;
`make schema-check`/`make gates` grün; CHANGELOG.

**Reviews (höhere Latte, beide unabhängig):** **[MR-006](../../../../harness/conventions.md#mr-006--unabhängiges-plan-review-vor-implementierungs-start)-Plan-Review** 0 HIGH
(3 LOW/INFO Wording F1/F2/F3). **Code-Review** (Projektinhaber-Latte:
Parsing/Schema-Drift/stille Verfälschung) **0 HIGH** — alle Bind-Positionen/
SELECT-Indizes exakt, NULL-vs-0.0 vollständig neutralisiert, kein Drift; **1 LOW
gehärtet** (`columnOptionalText` OOM-`nullptr` → `nullopt`), 2 INFO.

**Lerneintrag (modul-05) — ehrlich:** Das Code-Review fand **0 HIGH**. Die höhere
Latte wirkte hier als **Feedforward** (nicht Feedback): das Wissen „Persistenz =
NULL-vs-0.0-/Drift-Klasse" formte die Implementierung **vorab** — drei zentrale
`*Optional*`-Helfer als **eine** Quelle der NULL-Wahrheit (statt 8 Einzelstellen)
+ ein NULL-Korrektheits-AK, der die stille Verfälschung „kein U-Wert → 0,0" pinnt.
**Das ist NICHT ein zweites „Code-Review fängt Persistenz-HIGH"-Vorkommen** (der
015c-Zähler bleibt bei 1×) — eher der Gegenbeweis, dass die Klasse mit
vorgezogener Disziplin **gar nicht erst** entsteht. Steering-Notiz: der
Promotion-Kandidat „Code-Review bei Persistenz" reift über **gefundene** HIGHs,
nicht über saubere Läufe; ein sauberes Code-Review bei hoher Latte ist trotzdem
wertvoll (Verifikations-Vertrauen + Feedforward-Härtung der Helfer).

**Restrisiko / Nachfolge:**
- **`density`-Round-Trip** + **`wall_types`-/Typ-Vorlage-Material** — sobald die
  Domäne sie trägt (benannte Lücken).
- **EVL-004/006** (Material-/Kostenliste über `effectiveMaterial` — jetzt durabel).
- **`wall_type`-Fallback** (Wall-Type-Entität); dann Welle-3-Verifikation +
  `done/welle-3-results.md` → M3.
