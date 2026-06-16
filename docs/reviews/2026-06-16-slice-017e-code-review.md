# Code-Review — slice-017e (Material-Persistenz)

**Review-Art:** Code-Review (fertiger Diff, modul-10) — **höhere Latte**
(Projektinhaber 2026-06-16: Persistenz trägt Parsing/Schema-Drift/stille
Datenverfälschung, Steering-#4-Klasse aus slice-015c).
**Datum:** 2026-06-16
**Reviewer:** unabhängiger Agent (ohne Autoren-Kontext)
**Gegenstand:** `git diff HEAD` — `sqlite_project_repository.cpp` (insertMaterials/
loadMaterials + `material_id` an walls/roofs/slabs + NULL-sichere `*Optional*`-Helfer),
`test_sqlite_project_repository.cpp` (Material-Round-Trip-AK); Quer-Check `schema.sql`,
Modell-Header. `make test` grün (139 Tests).
**Verdict:** **0 HIGH — closure-ready** (1 LOW gehärtet, 2 INFO).

## Findings & Dispositionen

| # | Kat | Quelle | Pfad | Befund | Disposition |
|---|---|---|---|---|---|
| L1 | LOW | Code-Review | `sqlite_project_repository.cpp` `columnOptionalText` | Bei SQLite-internem OOM/Encoding-`nullptr` (Spalte **nicht** NULL-typisiert) gab der Helfer „present-empty" statt `nullopt` — theoretisch (nur unter Allokationsfehler), kein Round-Trip-Effekt. | **Gehärtet:** `text == nullptr → std::nullopt` (eindeutig „kein Wert"). |
| I1 | INFO | Code-Review | `materials.density` | `density` aus INSERT/SELECT ausgelassen (⇒ NULL); kein Domänenfeld → korrekt, kein Drop. | keine Aktion (benannte Lücke) |
| I2 | INFO | Code-Review | `material.h` / insertMaterials | Default-`MaterialId{0}` würde rowid 0 binden (SQLite speichert 0 literal). Vom Service nie erzeugt (positive Ids), nicht exerziert. | keine Aktion (Awareness) |

## Negativbefunde (geprüft, ohne Befund)

- **NULL-vs-0.0 (Kern-Risiko):** **jede** optionale Spalte (materials `u_value`/`cost_per_m2`/`cost_per_m3`/`color_hex`/`texture_path`; walls/roofs/slabs `material_id`) wird ausschließlich über `columnOptionalDouble`/`columnOptionalText`/`columnOptionalMaterial` gelesen, je mit `sqlite3_column_type == SQLITE_NULL → nullopt`. **Kein** nackter `column_double`/`_int` auf einer optionalen Spalte; jeder nackte Read trifft eine NOT-NULL-Spalte.
- **Bind-Positionen:** alle gegen `?`-Platzhalter + Spaltenliste verifiziert — insertMaterials 1–8 (project_id Literal); insertWalls 1–12 (material_id=10, created_at=11, updated_at=12 — Shift korrekt); insertRoofs 1–7 (material_id=7); insertSlabs 1–6 (material_id=6). Exakt.
- **SELECT-Lade-Indizes:** loadMaterials 0–7; loadWalls material_id @9; loadRoofs @6; loadSlabs @5. Alle passend.
- **Insert-Reihenfolge / FK:** `insertMaterials` vor walls/roofs/slabs in `save()`; `PRAGMA foreign_keys=ON` gesetzt. Save mit zugewiesenem Material erfüllt die FK.
- **ID-Erhalt:** `static_cast<int>(id)` ↔ `static_cast<MaterialId>(column_int)`; SQLite ehrt explizite PK-Werte; `loaded.walls[0].material_id` löst per `find_if` auf dasselbe Material (Test nicht-vakuös: prüft `!= end()` + Name).
- **Schema-Drift:** `git diff HEAD` berührt **kein** `schema.sql`, **kein** `data-model.yaml`. Spalten lagen vor. Kein Drift.
- **Totalität:** leere Bibliothek, all-NULL-Material, NULL-`material_id` laden ohne Wurf/UB. `load()` erzwingt keine FK; ein Dangling-FK kann beim Save gar nicht entstehen. Keine Type-Confusion.
- **Test-Qualität:** der NULL-Korrektheits-Test ist diskriminierend (`EXPECT_FALSE(...has_value())` würde unter einem nackten `column_double` **fehlschlagen**); `ORDER BY id` macht die Index-Assertions deterministisch.
- **Ressourcen/Lebensdauer:** `bindOptionalText` mit `SQLITE_TRANSIENT` (SQLite kopiert; kein Dangle vom `optional<string>`-Temporary).

## Closure-Freigabe

Keine HIGH. L1 gehärtet, I1/I2 ohne Aktion. **slice-017e ist closure-ready.**
