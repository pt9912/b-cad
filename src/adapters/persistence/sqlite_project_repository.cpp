// Driven Adapter (Persistenz, ADR-0003): ProjectRepositoryPort über SQLite.
// Atomar (Temp + fsync + Rename); SQLite-Typen bleiben hier gekapselt
// (arch-check Regel D). Schema-DDL aus spec/data-model.yaml generiert
// (d-migrate, ADR-0006), eingebettet als kSchemaSql.
//
// Fehlercodes (slice-008b, Spec-Familie E-IO-*): gemappt nach SQLite-
// Result-Code bzw. errno — Öffnen/Anlegen/Rechte → E-IO-001, Schreiben/
// Medium-voll/IO → E-IO-002. `sqlite3_open` ist *lazy*; der „kann nicht
// öffnen"-Fehler taucht erst beim ersten Zugriff (exec) auf, trägt dann
// aber SQLITE_CANTOPEN — daher mappen wir am Result-Code, nicht an der Phase.

#include "adapters/persistence/sqlite_project_repository.h"

#include <fcntl.h>
#include <unistd.h>

#include <cerrno>
#include <filesystem>
#include <optional>
#include <stdexcept>
#include <string>
#include <system_error>

#include <sqlite3.h>

#include "adapters/persistence/schema_sql.h"  // kSchemaSql (generiert)

namespace fs = std::filesystem;

namespace bcad::adapters::persistence {
namespace {

// Domänen-fremde NOT-NULL-Spalten: deterministischer Sentinel statt
// Wall-Clock (welle-1 kennt keine Zeitstempel-Semantik; hält den Round-Trip
// byte-stabil). Echte Zeitstempel sind eine spätere Domänen-Erweiterung.
constexpr const char* kSentinelTs = "1970-01-01T00:00:00Z";

// Interner Fehlertyp mit SQLite-Result-Code. Verlässt den Adapter NIE —
// `save`/`load` übersetzen ihn in eine neutrale `std::runtime_error` mit
// Spec-Fehlercode (ADR-0001).
struct SqliteError {
    int rc{};
    std::string context;
};

// SQLite-Result-Code → Spec-Fehlercode. Öffnen/Anlegen/Rechte → E-IO-001,
// sonst (FULL/IOERR/…) → E-IO-002 (Schreibfehler/Medium voll).
std::string ioCodeFor(int rc) {
    switch (rc & 0xFF) {
        case SQLITE_CANTOPEN:
        case SQLITE_PERM:
        case SQLITE_READONLY:
        case SQLITE_AUTH:
            return "E-IO-001";
        default:
            return "E-IO-002";
    }
}

// errno (aus fs::rename) → Spec-Fehlercode.
std::string ioCodeForErrno(int err) {
    switch (err) {
        case EACCES:
        case EPERM:
        case EROFS:
            return "E-IO-001";
        default:  // ENOSPC, EIO, … → Schreib-/Finalisierungs-Fehler
            return "E-IO-002";
    }
}

// RAII für sqlite3*.
class Db {
public:
    explicit Db(const fs::path& path) {
        const int rc = sqlite3_open(path.string().c_str(), &db_);
        if (rc != SQLITE_OK) {
            const std::string msg = db_ != nullptr ? sqlite3_errmsg(db_)
                                                    : "open failed";
            sqlite3_close(db_);
            throw SqliteError{rc, "open '" + path.string() + "': " + msg};
        }
    }
    ~Db() { sqlite3_close(db_); }
    Db(const Db&) = delete;
    Db& operator=(const Db&) = delete;

    sqlite3* handle() const { return db_; }

    void exec(const char* sql) {
        char* err = nullptr;
        const int rc = sqlite3_exec(db_, sql, nullptr, nullptr, &err);
        if (rc != SQLITE_OK) {
            const std::string msg = err != nullptr ? err : "exec failed";
            sqlite3_free(err);
            throw SqliteError{rc, "exec: " + msg};
        }
    }

private:
    sqlite3* db_{nullptr};
};

// RAII für sqlite3_stmt*; gebunden/gelesen wird über `get()` mit den
// rohen sqlite3_bind_*/column_*-Funktionen (keine eigenen (int,int)-
// Wrapper → kein bugprone-easily-swappable-parameters).
class Stmt {
public:
    Stmt(Db& db, const char* sql) {
        const int rc =
            sqlite3_prepare_v2(db.handle(), sql, -1, &stmt_, nullptr);
        if (rc != SQLITE_OK) {
            throw SqliteError{rc, std::string("prepare: ") +
                                      sqlite3_errmsg(db.handle())};
        }
    }
    ~Stmt() { sqlite3_finalize(stmt_); }
    Stmt(const Stmt&) = delete;
    Stmt& operator=(const Stmt&) = delete;

    sqlite3_stmt* get() const { return stmt_; }

    // true = Zeile verfügbar (SELECT), false = fertig (INSERT/Ende).
    bool step() {
        const int rc = sqlite3_step(stmt_);
        if (rc == SQLITE_ROW) {
            return true;
        }
        if (rc == SQLITE_DONE) {
            return false;
        }
        throw SqliteError{rc, "step"};
    }
    void reset() {
        sqlite3_reset(stmt_);
        sqlite3_clear_bindings(stmt_);
    }

private:
    sqlite3_stmt* stmt_{nullptr};
};

const char* classToText(hexagon::model::WallType type) {
    switch (type) {
        case hexagon::model::WallType::Innen:
            return "innen";
        case hexagon::model::WallType::Aussen:
            return "aussen";
        case hexagon::model::WallType::Trag:
            return "trag";
    }
    throw std::runtime_error("E-IO: unbekannter WallType");
}

hexagon::model::WallType textToClass(const std::string& text) {
    if (text == "innen") {
        return hexagon::model::WallType::Innen;
    }
    if (text == "aussen") {
        return hexagon::model::WallType::Aussen;
    }
    if (text == "trag") {
        return hexagon::model::WallType::Trag;
    }
    throw std::runtime_error("E-IO: unbekannte classification '" + text + "'");
}

void insertProject(Db& db) {
    Stmt stmt(db,
              "INSERT INTO projects (id,name,file_version,unit,created_at,"
              "updated_at) VALUES (1,'b-cad',1,'mm',?,?);");
    sqlite3_bind_text(stmt.get(), 1, kSentinelTs, -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt.get(), 2, kSentinelTs, -1, SQLITE_STATIC);
    stmt.step();
}

void insertStoreys(Db& db, const hexagon::model::Building& building) {
    Stmt stmt(db,
              "INSERT INTO storeys (id,project_id,name,level_index,"
              "elevation_mm,height_mm,created_at,updated_at) "
              "VALUES (?,1,?,?,0,?,?,?);");
    int level = 0;
    for (const auto& storey : building.storeys) {
        const std::string name = "Geschoss " + std::to_string(level);
        sqlite3_bind_int(stmt.get(), 1, static_cast<int>(storey.id));
        sqlite3_bind_text(stmt.get(), 2, name.c_str(), -1, SQLITE_TRANSIENT);
        sqlite3_bind_int(stmt.get(), 3, level);
        sqlite3_bind_double(stmt.get(), 4, storey.height_mm);
        sqlite3_bind_text(stmt.get(), 5, kSentinelTs, -1, SQLITE_STATIC);
        sqlite3_bind_text(stmt.get(), 6, kSentinelTs, -1, SQLITE_STATIC);
        stmt.step();
        stmt.reset();
        ++level;
    }
}

void insertWalls(Db& db, const hexagon::model::Building& building) {
    Stmt stmt(db,
              "INSERT INTO walls (id,project_id,storey_id,classification,"
              "start_x_mm,start_y_mm,end_x_mm,end_y_mm,thickness_mm,height_mm,"
              "created_at,updated_at) VALUES (?,1,?,?,?,?,?,?,?,?,?,?);");
    for (const auto& wall : building.walls) {
        sqlite3_bind_int(stmt.get(), 1, static_cast<int>(wall.id));
        sqlite3_bind_int(stmt.get(), 2, static_cast<int>(wall.storey_id));
        sqlite3_bind_text(stmt.get(), 3, classToText(wall.type), -1,
                          SQLITE_STATIC);
        sqlite3_bind_double(stmt.get(), 4, wall.start.x_mm);
        sqlite3_bind_double(stmt.get(), 5, wall.start.y_mm);
        sqlite3_bind_double(stmt.get(), 6, wall.end.x_mm);
        sqlite3_bind_double(stmt.get(), 7, wall.end.y_mm);
        sqlite3_bind_double(stmt.get(), 8, wall.thickness_mm);
        sqlite3_bind_double(stmt.get(), 9, wall.height_mm);
        sqlite3_bind_text(stmt.get(), 10, kSentinelTs, -1, SQLITE_STATIC);
        sqlite3_bind_text(stmt.get(), 11, kSentinelTs, -1, SQLITE_STATIC);
        stmt.step();
        stmt.reset();
    }
}

void loadStoreys(Db& db, hexagon::model::Building& building) {
    Stmt stmt(db, "SELECT id,height_mm FROM storeys ORDER BY level_index;");
    while (stmt.step()) {
        hexagon::model::Storey storey;
        storey.id = static_cast<hexagon::model::StoreyId>(
            sqlite3_column_int(stmt.get(), 0));
        storey.height_mm = sqlite3_column_double(stmt.get(), 1);
        building.storeys.push_back(storey);
    }
}

void loadWalls(Db& db, hexagon::model::Building& building) {
    Stmt stmt(db,
              "SELECT id,storey_id,start_x_mm,start_y_mm,end_x_mm,end_y_mm,"
              "thickness_mm,height_mm,classification FROM walls ORDER BY id;");
    while (stmt.step()) {
        hexagon::model::Wall wall;
        wall.id =
            static_cast<hexagon::model::WallId>(sqlite3_column_int(stmt.get(), 0));
        wall.storey_id = static_cast<hexagon::model::StoreyId>(
            sqlite3_column_int(stmt.get(), 1));
        wall.start = {sqlite3_column_double(stmt.get(), 2),
                      sqlite3_column_double(stmt.get(), 3)};
        wall.end = {sqlite3_column_double(stmt.get(), 4),
                    sqlite3_column_double(stmt.get(), 5)};
        wall.thickness_mm = sqlite3_column_double(stmt.get(), 6);
        wall.height_mm = sqlite3_column_double(stmt.get(), 7);
        const auto* cls = sqlite3_column_text(stmt.get(), 8);
        wall.type = textToClass(cls != nullptr
                                    ? reinterpret_cast<const char*>(cls)
                                    : "");
        building.walls.push_back(wall);
    }
}

// Stale Temp-Artefakte entfernen (Recovery nach Absturz): das Temp-DB plus
// ein evtl. zurückgebliebenes -journal/-wal eines getöteten `save`. Sonst
// sähe ein neues sqlite3_open ein „hot journal" zu einer frisch angelegten
// Temp-DB (Wiederverwendung desselben Pfads) → Rollback-Konflikt.
void removeTempArtifacts(const fs::path& tmp) {
    std::error_code ec;
    fs::remove(tmp, ec);
    fs::remove(fs::path(tmp.string() + "-journal"), ec);
    fs::remove(fs::path(tmp.string() + "-wal"), ec);
}

// Best-effort-Durability: Datei bzw. Verzeichnis auf Platte zwingen.
void fsyncPath(const fs::path& path, bool is_dir) {
    const int flags = O_RDONLY | (is_dir ? O_DIRECTORY : 0);
    const int fdesc = ::open(path.string().c_str(), flags);
    if (fdesc < 0) {
        return;
    }
    ::fsync(fdesc);
    ::close(fdesc);
}

}  // namespace

void SqliteProjectRepository::save(const hexagon::model::Building& building,
                                   const fs::path& path) const {
    const fs::path tmp = path.string() + ".tmp";
    removeTempArtifacts(tmp);

    {
        std::optional<Db> db;
        try {
            db.emplace(tmp);
            db->exec(kSchemaSql);
            db->exec("PRAGMA foreign_keys=ON;");
            db->exec("BEGIN;");
            insertProject(*db);
            insertStoreys(*db, building);
            insertWalls(*db, building);
            db->exec("COMMIT;");
            db.reset();  // schließen vor fsync/rename
        } catch (const SqliteError& e) {
            db.reset();
            removeTempArtifacts(tmp);  // vorherige Zieldatei bleibt intakt
            throw std::runtime_error(ioCodeFor(e.rc) +
                                     ": Speichern fehlgeschlagen ('" +
                                     tmp.string() + "'): " + e.context);
        } catch (...) {
            // Nicht-SQLite-Wurf (z. B. std::bad_alloc): Aufräum-Garantie wie
            // slice-008a wahren — Orphan-Temp entfernen, Fehler unverändert
            // weiterreichen (kein E-IO-Wrap, da kein klassifizierter IO-Fehler).
            db.reset();
            removeTempArtifacts(tmp);
            throw;
        }
    }

    fsyncPath(tmp, /*is_dir=*/false);
    std::error_code ec;
    fs::rename(tmp, path, ec);
    if (ec) {
        removeTempArtifacts(tmp);
        throw std::runtime_error(ioCodeForErrno(ec.value()) + ": rename '" +
                                 tmp.string() + "' -> '" + path.string() +
                                 "': " + ec.message());
    }
    const fs::path dir = path.parent_path();
    fsyncPath(dir.empty() ? fs::path(".") : dir, /*is_dir=*/true);
}

hexagon::model::Building SqliteProjectRepository::load(
    const fs::path& path) const {
    if (!fs::exists(path)) {
        throw std::runtime_error("E-IO: Projektdatei nicht gefunden: " +
                                 path.string());
    }
    try {
        hexagon::model::Building building;
        Db db(path);
        loadStoreys(db, building);
        loadWalls(db, building);
        return building;
    } catch (const SqliteError& e) {
        throw std::runtime_error("E-IO: Projektdatei nicht lesbar ('" +
                                 path.string() + "'): " + e.context);
    }
}

}  // namespace bcad::adapters::persistence
