---
id: slice-008b
titel: Persistenz-Härtung — Crash-Recovery (kill -9) + E-IO-002
status: done
welle: welle-1-mvp
lastenheft_refs: [LH-QA-005, LH-FA-BLD-002]
adr_refs: [ADR-0001, ADR-0003]
---

# Slice 008b: Persistenz-Härtung — Crash-Recovery + E-IO-002

**Status:** done · **Welle:** welle-1-mvp

**Bezug:** LH-QA-005 (Crash-Recovery), LH-FA-BLD-002 Boundary/Negative
(`E-IO-002`, voriger Stand unverändert); ADR-0003 (atomar, Temp+Rename) —
schließt die in slice-008a vermerkte Folgepflicht. `architecture.md` §
Fehlerpfade (`E-IO-002` → `event=persist_error`).

**Autor:** Dietmar Burkard · **Datum:** 2026-06-09.

---

## 1. Ziel

Die in slice-008a strukturell vorhandene Atomik (Temp → `fsync` → `rename`)
**nachweisen**, nicht nur behaupten: ein `kill -9` während `save` darf den
letzten konsistenten Stand nie zerstören (LH-QA-005). Plus der spezifizierte
Negativpfad `E-IO-002` (Schreibfehler/Medium voll → Fehlercode, vorheriger
Dateistand byte-unverändert).

## 2. Definition of Done

- [x] **Fehlercode-Mapping im Adapter** (die Spec unterscheidet, §1.x
      `spezifikation.md`): Open-/Anlege-/Rechte-Fehler (`SQLITE_CANTOPEN`,
      `EACCES`/`EROFS`/`EISDIR`) → **`E-IO-001`** (kein Schreibrecht/kann
      nicht anlegen); Schreib-/Voll-/IO-Fehler (`SQLITE_FULL`/`SQLITE_IOERR`,
      `ENOSPC`) → **`E-IO-002`** (Medium voll/Schreibfehler). Bei **jedem**
      Fehler bleibt die vorherige Zieldatei **byte-unverändert** (Temp
      verworfen, kein Rename). Die beiden Codes **kollabieren nicht** in einen.
- [x] **Crash-Recovery-Test** (`tests/adapters/...`): echtes `fork()` +
      `SIGKILL` auf einen Kind-Prozess, der `save` in einer Schleife
      ausführt; über **N Trials** mit variierendem Kill-Zeitpunkt. Nach
      jedem Kill lädt die Zieldatei **vollständig** (Zustand A *oder* B,
      nie korrupt/halb) → LH-QA-005 erfüllt. (Kind nutzt `_exit`, keine
      GTest-Assertions; Assertions im Eltern-Prozess.)
- [x] **`E-IO-001`-Test** (deterministisch): Temp-Pfad mit einem nicht-leeren
      Verzeichnis besetzen → `save` scheitert beim **Öffnen/Anlegen** der
      Temp-DB → wirft **`E-IO-001`**; zuvor gespeichertes A unverändert. Der
      Test räumt das Verzeichnis selbst per `fs::remove_all` auf (`save`s
      `fs::remove(tmp)` entfernt nur leere).
- [x] **`E-IO-002`-Test** (deterministisch, echter Schreibfehler): `SIGXFSZ`
      ignorieren + `setrlimit(RLIMIT_FSIZE, klein)` um den `save`-Aufruf →
      der Temp-DB-**Write** überschreitet das Limit → `SQLITE_FULL`/`IOERR`
      → wirft **`E-IO-002`**; A unverändert. Testet den realen
      Produktions-`save` **ohne Test-Seam** (`RLIMIT_FSIZE` greift auch als
      root; Limit wird nach dem Aufruf wiederhergestellt).
- [x] `make gates` grün; **ADR-0003-Folgepflicht** „Crash-Recovery" im
      ADR-Index → **erfüllt** (slice-008b).

## 3. Plan (vor Code)

| Datei | Art | Begründung |
|---|---|---|
| `src/adapters/persistence/sqlite_project_repository.cpp` | Änderung | `save`-Fehlerpfad **nach Fehlerart**: `E-IO-001` (Open/Anlage) bzw. `E-IO-002` (Write/Voll/Rename-`ENOSPC`) |
| `tests/adapters/test_sqlite_crash_recovery.cpp` | neu | `fork`+`kill -9`-Recovery + `E-IO-001`/`E-IO-002` |
| `tests/CMakeLists.txt` | Änderung | Test registrieren |
| `docs/plan/adr/README.md` | Änderung | ADR-0003-Folgepflicht „erfüllt" |

## 4. Mechanik (Begründung)

- **Warum echtes `fork`+`SIGKILL`** statt eines bloßen Struktur-Arguments:
  Die ADR-0003-Fitness verlangt `kill -9` *zwischen Schreibphasen*. Eine
  reine „rename ist atomar"-Behauptung ist Inferenz; der Test ist Evidenz.
  Eine nicht-atomare Implementierung würde über N Trials eine korrupte/
  halb-geschriebene Zieldatei hinterlassen → `load` wirft/liefert Unsinn →
  Test rot. Der Test kann also nicht falsch-grün sein.
- **Vorbedingung A:** vor dem Fork wird ein vollständiger Stand A
  gespeichert, damit die Zieldatei **immer** existiert (Kind speichert
  wiederholt B). Ziel ist nach jedem Kill A *oder* B — `rename(2)` ist
  POSIX-atomar (Datei zeigt nie auf einen halben Inode).
- **Orphan-`*.tmp`:** ein getötetes Kind kann eine korrupte Temp-DB
  hinterlassen; sie betrifft die Zieldatei nicht und wird beim nächsten
  `save` (`fs::remove(tmp)` am Anfang) bereinigt — das *ist* der
  Recovery-Schritt.
- **Code-Trennung `E-IO-001`/`E-IO-002`:** `save` fängt **phasenweise** —
  Fehler beim *Öffnen/Anlegen* der Temp-DB → `E-IO-001`, Fehler beim
  *Schreiben/Commit* → `E-IO-002`; der Rename mappt per `errno`
  (`ENOSPC`/`EIO` → `E-IO-002`, `EACCES`/`EROFS` → `E-IO-001`). So bleiben
  die zwei Spec-Codes getrennt statt im generischen Wrap zu kollabieren.

## 5. Trigger / Closure

- Trigger: slice-008a done (Adapter + Atomik vorhanden) ✓.
- Closure: DoD vollständig, `make gates` grün, beide Tests grün, Review,
  Closure-Notiz.

## 6. Risiken / offene Punkte

- **Test-Laufzeit/Flake:** N Trials mit `usleep`-Jitter; bewusst so
  gewählt, dass der Test deterministisch grün ist (jeder Ausgang A/B ist
  gültig) — er fängt Nicht-Atomik statistisch, ist aber nicht selbst flaky.
- **`fsync`-Durability vs. Atomik:** 008b prüft **Atomik** (kein halb
  geschriebener Stand). Echte Power-Loss-Durability (Block-Layer) ist nicht
  Teil des Test-Budgets — `fsync` ist best-effort gesetzt.
- **Provokations-Mechanik:** `E-IO-001` via besetztem Temp-Verzeichnis
  (Open-Fehler), `E-IO-002` via `RLIMIT_FSIZE` (echter Write-Fehler) — beide
  deterministisch, beide am realen `save`.
- **E-IO-001-Scope:** dieser Slice deckt den **Persist-Pfad** ab (`save`
  open/create → `E-IO-001`, inkl. Test); der breitere
  BLD-001-„Projekt-anlegen"-UX-Flow (Lastenheft) bleibt Service/UI, später.
- **`event=persist_error` deferred:** die Spec koppelt `E-IO-002` an dieses
  Log-Feld (`architecture.md` §Fehlerpfade), aber es gibt **noch keine
  Logging-/OTel-Senke** im Adapter (`TracingPort` = spätere Welle) → hier
  bewusst nur der Fehlercode; das Event-Feld ist ein offener Punkt.

## 7. Closure-Notiz

**Closure-Kriterien (beobachtbar):**
- `make gates` grün: test **32/32** (Crash-Recovery 60 Trials, `E-IO-001`,
  `E-IO-002`, Rename-`errno`-Pfad), suppression-gate ok, coverage 93,1 %.
  Der `record-gates`-Nachweis matcht den Arbeitsbaum.

**§4↔Code-Angleichung (Review-Befund #3):** §4 beschreibt das Fangen
*phasenweise*; die Implementierung mappt bewusst am **SQLite-Result-Code**
(`rc & 0xFF`: `CANTOPEN`/`PERM`/`READONLY`/`AUTH` → `E-IO-001`, sonst →
`E-IO-002`; Rename per `errno`). Grund: `sqlite3_open` ist **lazy** — der
„kann nicht öffnen"-Fehler taucht erst beim ersten `exec` auf, trägt dann
aber `SQLITE_CANTOPEN`. Result-Code-Mapping ist damit präziser als die
Phasen-Heuristik; der Adapter-Header-Kommentar dokumentiert das. **Maßgeblich
ist der Code; §4 ist der überholte Plan-Stand.**

**Lerneintrag:**
- **Evidenz statt Inferenz:** echtes `fork()`+`SIGKILL` über 60 Trials
  beweist die Atomik (Ziel immer A *oder* B); der Test kann nicht falsch-grün
  sein. `db.reset()` schließt die DB **vor** `fsync`/`rename`.
- **Zwei Spec-Codes getrennt** am Result-Code/`errno`; deterministisch
  getestet (besetztes Verzeichnis → `E-IO-001`; `RLIMIT_FSIZE` → `E-IO-002`;
  Ziel-Verzeichnis → Rename-`errno`-Pfad).
- **Aufräum-Garantie:** zusätzlicher `catch (...)` im `save` entfernt das
  Orphan-Temp auch bei Nicht-SQLite-Würfen (`bad_alloc`) — wie 008a.
- **`removeTempArtifacts`** bereinigt `-journal`/`-wal` mit (Hot-Journal-
  Konflikt bei Pfad-Wiederverwendung nach Kill).
- **Test-Isolation:** `FileSizeLimit`-RAII stellt `RLIMIT_FSIZE` + `SIGXFSZ`-
  Disposition exception-safe wieder her.

**Restrisiko / Nachfolge:** `event=persist_error`-Logging (deferred bis
`TracingPort`/spätere Welle); breiterer BLD-001-„Projekt-anlegen"-UX-Flow
(Service/UI). Damit ist die welle-1-Persistenz (008a+008b) abgeschlossen.

## 8. Sub-Area-Modus-Begründung

- **Sub-Area:** Persistenz-Adapter (Härtung). **Modus:** GF (Erweiterung von 008a).
- **Konventionen-Dichte:** hoch (ADR-0003, Fehlercode-Familie, Docker-Gates).
- **Phase-Reife:** Phase 4 (Nachweis statt Behauptung).
- **Evidenz-/Diskrepanz-Risiko:** mittel — der Crash-Test muss real killen,
  nicht simulieren.
