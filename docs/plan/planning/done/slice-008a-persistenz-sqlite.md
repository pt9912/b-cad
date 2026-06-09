---
id: slice-008a
titel: Persistenz — ProjectRepositoryPort + SQLite-Adapter (save/load, atomar)
status: done
welle: welle-1-mvp
lastenheft_refs: [LH-FA-BLD-002, LH-FA-BLD-003, ACC-005]
adr_refs: [ADR-0001, ADR-0003, ADR-0006]
---

# Slice 008a: Persistenz — Port + SQLite-Adapter (save/load, atomar)

**Status:** done · **Welle:** welle-1-mvp

**Bezug:** ACC-005, LH-FA-BLD-002 (speichern, atomar), LH-FA-BLD-003
(laden); ADR-0003 (SQLite hinter `ProjectRepositoryPort`, Temp+Rename),
ADR-0006 (Schema aus `data-model.yaml` via d-migrate), ADR-0001 (Kern führt).

**Autor:** Dietmar Burkard · **Datum:** 2026-06-09.

---

## 1. Ziel

Ein welle-1-Projekt (`Building` = Geschosse + Wände) **atomar speichern**
und **identisch wieder laden** (ACC-005). SQLite hinter einem neuen Driven
Port `ProjectRepositoryPort`; die DDL wird **aus `spec/data-model.yaml`
generiert** (d-migrate, ADR-0006) und committet (generate-and-commit +
Drift-Check). **Carveout:** Crash-Recovery-Test (`kill -9`, LH-QA-005) und
`E-IO-002` (Medium voll) → **slice-008b**.

## 2. Definition of Done

- [x] **Port** `src/hexagon/ports/driven/project_repository_port.h`:
      `save(const Building&, path)` / `load(path) -> Building`. Keine
      SQLite-Typen in der Signatur (ADR-0001); Fehler als neutrale
      `std::runtime_error` (wie OCC-Adapter E-GEO-002), kein sqlite-Leak.
- [x] **Adapter** `src/adapters/persistence/sqlite_project_repository.{h,cpp}`:
      DB aus eingebettetem `schema.sql` erzeugen; `save` schreibt
      Project+Storeys+Walls in **einer Transaktion** in eine **Temp-Datei**
      und **rename**t atomar über das Ziel (ADR-0003); `load` liest zurück.
      Header **ohne** `sqlite3.h`.
- [x] **Schema** `src/adapters/persistence/schema.sql`: aus `data-model.yaml`
      via `d-migrate schema generate --target sqlite --deterministic` (Image
      **@sha256-Digest** gepinnt, s. Drift-Gate) generiert + committet; in den
      Adapter eingebettet (CMake `configure_file` → String-Resource, kein
      Laufzeit-Dateipfad).
- [x] **Drift-Gate** `make schema-check` (NICHT in `make gates`):
      regeneriert via
      `ghcr.io/pt9912/d-migrate:0.9.7@sha256:69afc2147754c23b2d34c6a5ad8fbaae3787a5c061efd32f45d1c953bbc52fd9`
      (**@sha256-Digest** gepinnt — ADR-0004-Prinzip auf die externe
      Tool-Dependency angewandt; ein Floating-Tag erzeugte sonst nicht
      reproduzierbare DDL) und difft gegen die committete `schema.sql`;
      Abweichung = Fehler. d-migrate bleibt aus dem hermetischen
      Gate-Build-Pfad, **muss aber in der CI-Befehlsliste stehen** (sonst
      feuert das Drift-Gate nie automatisch; `harness/README`: Lauf-Wahrheit
      pro Commit gehört in CI).
- [x] **arch-check Regel D**: `sqlite3.h`/`<sqlite3...>` **nur** in
      `src/adapters/persistence/` (füllt ADR-0003-Fitness; analog Regel C).
      `harness/README` + `AGENTS.md` Sensor-Zeile aktualisiert.
- [x] **Round-Trip-Test** `tests/adapters/test_sqlite_project_repository.cpp`:
      Building (mehrere Geschosse + Wände, alle `WallType`) → `save` → `load`
      → feldgleich (BLD-002 Happy / BLD-003). Leeres Projekt; Ids erhalten.
- [x] `make gates` grün; ADR-0003 → **Accepted** (Crash-Recovery-Fitness als
      Folgepflicht 008b im ADR-Index, analog ADR-0002 Regel C).

## 3. Plan (vor Code)

| Datei | Art | Begründung |
|---|---|---|
| `src/hexagon/ports/driven/project_repository_port.h` | neu | Driven Port (Kern-Vertrag), framework-frei |
| `src/adapters/persistence/sqlite_project_repository.{h,cpp}` | neu | SQLite-Adapter (Transaktion + Temp+Rename) |
| `src/adapters/persistence/schema.sql` | neu | generierte SQLite-DDL (committet) |
| `src/adapters/persistence/CMakeLists.txt` bzw. `src/adapters/CMakeLists.txt` | Änderung | Adapter bauen, `sqlite3` linken, `schema.sql` einbetten |
| `tools/arch-check.sh` | Änderung | Regel D (sqlite nur in persistence/) |
| `Makefile` | Änderung | `make schema-check` (Drift-Gate, docker run d-migrate) |
| `tests/adapters/CMakeLists.txt` + Test | Änderung/neu | Round-Trip |
| `docs/plan/adr/0003-persistenz-sqlite.md` / `adr/README.md` | Änderung | Accepted + Folgepflicht-Eintrag |
| `harness/README.md`, `AGENTS.md` | Änderung | Sensor-Zeile arch-check Regel D |

## 4. Domänen↔Schema-Mapping (welle-1)

- **Ein implizites `projects`-Row** je Datei (`name="b-cad"`, `file_version=1`,
  `unit="mm"`, Zeitstempel). Das Domänen-`Building` hat (noch) keine
  Projekt-Metadaten — bewusst minimal.
- **storeys:** `id` = `StoreyId` (explizit, Id-Erhalt), `project_id`,
  `level_index` = Vektor-Index, `elevation_mm` = 0 (welle-1; kumulativ
  später), `height_mm`.
- **walls:** `id` = `WallId` (explizit), `classification` =
  `WallType`→`{innen,aussen,trag}`, `start/end` →
  `start_x/y_mm`,`end_x/y_mm`, `thickness_mm`, `height_mm`, `base_z_mm`=0,
  `wall_type_id`/`material_id` = NULL.
- **Domänen-fremde NOT-NULL-Spalten deterministisch synthetisiert** (sonst
  `NOT NULL`-Verletzung beim INSERT — diese Spalten haben kein Domänen-
  Pendant): `projects.name="b-cad"`; `storeys.name="Geschoss <level_index>"`;
  `created_at`/`updated_at` auf projects/storeys/walls = fester Sentinel
  `"1970-01-01T00:00:00Z"` (welle-1 kennt keine Zeitstempel-Semantik).
  **Kein `now()`/Wall-Clock** — sonst bricht der deterministische
  (byte-stabile) Anspruch und der Round-Trip-Vergleich. `walls.name` ist
  nullable → NULL.
- **Round-Trip-Identität** gilt für die *Domänen-Felder*; die Spalten, die
  das Schema mehr hat (elevation/level_index/…), werden deterministisch
  synthetisiert und beim Laden für das Domänen-Modell nicht benötigt.
- **Reihenfolge-Annahme:** `load` rekonstruiert `storeys` per
  `ORDER BY level_index` und `walls` per `ORDER BY id`. Der index-weise
  Vergleich ist nur *reihenfolge-erhaltend*, wenn die In-Memory-Vektoren
  schon nach Einfüge-/Id-Reihenfolge geordnet sind — für welle-1 gegeben
  (monoton steigende Ids, append-only, kein Reorder). **Kein
  reihenfolge-erhaltender Vertrag für beliebigen Input** (relevant erst,
  wenn Geschoss-/Wand-Umsortierung dazukommt).
- **`REAL`/W200:** unkritisch — der Kern nutzt `double`, SQLite-`REAL` ist
  ein double → für unsere Werte verlustfrei.

## 5. Trigger / Closure-Trigger

- Trigger: slice-007 done (validiertes Schema) ✓, ADR-0003 vorhanden ✓.
- Closure: DoD vollständig, `make gates` grün, `make schema-check` grün,
  Round-Trip grün, Review, Closure-Notiz.

## 6. Risiken / offene Punkte

- **Atomarität-Tiefe:** 008a liefert Temp+Rename (POSIX-atomar auf gleichem
  FS) + `fsync` vor Rename; der **Nachweis** unter `kill -9` (LH-QA-005) ist
  008b. 008a behauptet LH-QA-005 **nicht** als erfüllt.
- **`E-IO-002` (Medium voll):** Negativpfad in 008b; 008a meldet generischen
  neutralen Fehler bei Schreibfehler.
- **Id-Erhalt vs. AUTOINCREMENT:** Domänen-Ids werden explizit eingefügt
  (INTEGER-PK erlaubt das); kein Vertrauen auf DB-Vergabe.
- **Sizing:** Adapter + Schema-Einbettung + Drift-Gate + Regel D sind dicht;
  falls zu groß, Drift-Gate (`make schema-check`) als kleiner Folge-Schritt.

## 7. Closure-Notiz

**Closure-Kriterien (beobachtbar):**
- `make gates` grün: test **28/28** inkl. Round-Trip (BLD-002 Happy/BLD-003),
  arch-check **Regel D**, coverage 89,8 %. Der `record-gates`-Nachweis
  matcht den Arbeitsbaum (Round-Trip ist belegt, nicht behauptet).
- `make schema-check` grün (digest-gepinnt): committete `schema.sql` ==
  `d-migrate(data-model.yaml)`.

**Lerneintrag:**
- **generate-and-commit + Drift-Gate:** d-migrate (`@sha256`-gepinnt, externe
  Tool-Dep nach ADR-0004-Prinzip) generiert die SQLite-DDL; `schema.sql`
  committet + via `configure_file` eingebettet (`CMAKE_CONFIGURE_DEPENDS`
  erzwingt Reconfigure bei Drift). `make schema-check` bewusst **außerhalb**
  `make gates` (d-migrate aus dem hermetischen Pfad), dafür in der
  CI-Sensor-Liste — sonst zahnlos.
- **Atomik:** Insert in Transaktion → `fsync` → `rename` → Dir-`fsync`
  (LH-FA-BLD-002 Boundary). Vorherige Zieldatei bleibt bei Fehler unangetastet.
- **Kapselung:** SQLite-Typen vollständig in der `.cpp`; arch-check **Regel D**
  setzt das maschinell durch (ADR-0003-Fitness erfüllt).
- **Determinismus:** NOT-NULL-Synthese mit Sentinel statt `now()`; binden über
  die rohen `sqlite3_bind_*` (kein eigener `(int,int)`-Wrapper →
  `bugprone-easily-swappable-parameters` vermieden).

**Restrisiko / Nachfolge:**
- **slice-008b:** Crash-Recovery-Test (`kill -9`, LH-QA-005) + `E-IO-002`
  (Medium voll). ADR-0003-Folgepflicht im ADR-Index vermerkt.
- **ADR-0006-Re-Eval:** `cost_per_m2/m3` (`decimal(12,2)`→`REAL`) verliert
  exakte Dezimalstellen — irrelevant in welle-1, Re-Eval-Punkt sobald
  EVL/Kosten landen.

## 8. Sub-Area-Modus-Begründung

- **Sub-Area:** Persistenz-Adapter (Driven). **Modus:** GF — neuer Port +
  Adapter, kein Altbestand.
- **Konventionen-Dichte:** hoch (ADR-0001/0003/0006, arch-check, Docker-Gates).
- **Phase-Reife:** Phase 4 (lauffähiger Adapter + Round-Trip).
- **Evidenz-/Diskrepanz-Risiko:** mittel — Domänen↔Schema-Mapping + atomares
  Schreiben müssen real getestet sein (Round-Trip), nicht behauptet.
