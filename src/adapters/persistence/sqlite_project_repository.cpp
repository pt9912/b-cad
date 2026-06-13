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

#include <array>
#include <cerrno>
#include <cstdio>
#include <filesystem>
#include <optional>
#include <sstream>
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

const char* openingTypeToText(hexagon::model::OpeningKind kind) {
    switch (kind) {
        case hexagon::model::OpeningKind::Door:
            return "door";
        case hexagon::model::OpeningKind::Window:
            return "window";
    }
    throw std::runtime_error("E-IO: unbekannter OpeningKind");
}

hexagon::model::OpeningKind textToOpeningKind(const std::string& text) {
    if (text == "door") {
        return hexagon::model::OpeningKind::Door;
    }
    if (text == "window") {
        return hexagon::model::OpeningKind::Window;
    }
    throw std::runtime_error("E-IO: unbekannter opening_type '" + text + "'");
}

const char* swingToText(hexagon::model::SwingDirection swing) {
    switch (swing) {
        case hexagon::model::SwingDirection::Left:
            return "left";
        case hexagon::model::SwingDirection::Right:
            return "right";
    }
    throw std::runtime_error("E-IO: unbekannte SwingDirection");
}

hexagon::model::SwingDirection textToSwing(const std::string& text) {
    if (text == "left") {
        return hexagon::model::SwingDirection::Left;
    }
    if (text == "right") {
        return hexagon::model::SwingDirection::Right;
    }
    throw std::runtime_error("E-IO: unbekannte swing_direction '" + text + "'");
}

const char* roofTypeToText(hexagon::model::RoofType type) {
    switch (type) {
        case hexagon::model::RoofType::Sattel:
            return "sattel";
        case hexagon::model::RoofType::Walm:
            return "walm";
        case hexagon::model::RoofType::Pult:
            return "pult";
    }
    throw std::runtime_error("E-IO: unbekannter RoofType");
}

hexagon::model::RoofType textToRoofType(const std::string& text) {
    if (text == "sattel") {
        return hexagon::model::RoofType::Sattel;
    }
    if (text == "walm") {
        return hexagon::model::RoofType::Walm;
    }
    if (text == "pult") {
        return hexagon::model::RoofType::Pult;
    }
    throw std::runtime_error("E-IO: unbekannter roof_type '" + text + "'");
}

// Rechteckiger Dach-Grundriss als JSON-Array
// `[origin_x, origin_y, width, depth, base_z]` (ADR-0006 footprint_json).
// `%.17g` → exakter double-Round-Trip (DBL_DECIMAL_DIG). Eigenes,
// deterministisches Format; nur dieser Adapter schreibt/liest es.
std::string footprintToJson(const hexagon::model::Roof& roof) {
    std::array<char, 256> buf{};
    std::snprintf(buf.data(), buf.size(), "[%.17g,%.17g,%.17g,%.17g,%.17g]",
                  roof.origin.x_mm, roof.origin.y_mm, roof.width_mm,
                  roof.depth_mm, roof.base_z_mm);
    return buf.data();
}

// Parst das eigene 5-Zahlen-Array zurück; wirft bei Format-Fehler neutral
// (`E-IO`, kein Lib-Typ verlässt den Adapter — total nach außen).
void parseFootprintJson(const std::string& json, hexagon::model::Roof& roof) {
    const std::size_t lhs = json.find('[');
    const std::size_t rhs = json.rfind(']');
    if (lhs == std::string::npos || rhs == std::string::npos || rhs < lhs) {
        throw std::runtime_error("E-IO: footprint_json invalide: '" + json + "'");
    }
    std::stringstream inner(json.substr(lhs + 1, rhs - lhs - 1));
    std::array<double, 5> values{};
    std::string token;
    std::size_t count = 0;
    try {
        while (std::getline(inner, token, ',')) {
            if (count >= values.size()) {
                throw std::runtime_error("E-IO: footprint_json zu viele Werte");
            }
            values[count++] = std::stod(token);
        }
    } catch (const std::logic_error& e) {  // stod: invalid_argument/out_of_range
        throw std::runtime_error(std::string("E-IO: footprint_json Zahl ungültig: ") +
                                 e.what());
    }
    if (count != values.size()) {
        throw std::runtime_error("E-IO: footprint_json erwartet 5 Werte");
    }
    roof.origin = {values[0], values[1]};
    roof.width_mm = values[2];
    roof.depth_mm = values[3];
    roof.base_z_mm = values[4];
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

// Öffnungen (Türen/Fenster, ADR-0011/0006): Basistabelle `openings` +
// 1:1-Spezialisierung `doors`/`windows` (CTI). NACH `insertWalls` (FK
// `openings.wall_id → walls.id`), in derselben Transaktion. Nicht-
// Domänen-Spalten (`name`, `swing_angle_deg`, `is_external`,
// `frame_material`, …) bleiben auf Default/`NULL` — welle-2-Scope (M4).
void insertOpenings(Db& db, const hexagon::model::Building& building) {
    Stmt openingStmt(
        db,
        "INSERT INTO openings (id,project_id,wall_id,opening_type,offset_mm,"
        "width_mm,height_mm,sill_height_mm,created_at,updated_at) "
        "VALUES (?,1,?,?,?,?,?,?,?,?);");
    Stmt doorStmt(db,
                  "INSERT INTO doors (opening_id,swing_direction) VALUES (?,?);");
    Stmt windowStmt(db, "INSERT INTO windows (opening_id) VALUES (?);");
    for (const auto& opening : building.openings) {
        sqlite3_bind_int(openingStmt.get(), 1, static_cast<int>(opening.id));
        sqlite3_bind_int(openingStmt.get(), 2,
                         static_cast<int>(opening.wall_id));
        sqlite3_bind_text(openingStmt.get(), 3, openingTypeToText(opening.kind),
                          -1, SQLITE_STATIC);
        sqlite3_bind_double(openingStmt.get(), 4, opening.offset_mm);
        sqlite3_bind_double(openingStmt.get(), 5, opening.width_mm);
        sqlite3_bind_double(openingStmt.get(), 6, opening.height_mm);
        sqlite3_bind_double(openingStmt.get(), 7, opening.sill_height_mm);
        sqlite3_bind_text(openingStmt.get(), 8, kSentinelTs, -1, SQLITE_STATIC);
        sqlite3_bind_text(openingStmt.get(), 9, kSentinelTs, -1, SQLITE_STATIC);
        openingStmt.step();
        openingStmt.reset();

        if (opening.kind == hexagon::model::OpeningKind::Door) {
            sqlite3_bind_int(doorStmt.get(), 1, static_cast<int>(opening.id));
            sqlite3_bind_text(doorStmt.get(), 2, swingToText(opening.swing), -1,
                              SQLITE_STATIC);
            doorStmt.step();
            doorStmt.reset();
        } else {
            sqlite3_bind_int(windowStmt.get(), 1, static_cast<int>(opening.id));
            windowStmt.step();
            windowStmt.reset();
        }
    }
}

// Dächer (LH-FA-ROF-*, ADR-0011/0006): NACH `insertStoreys` (FK
// `roofs.storey_id → storeys.id`). `height_mm` (abgeleitete Firsthöhe)
// und `material_id` bleiben `NULL` — welle-2-Scope.
void insertRoofs(Db& db, const hexagon::model::Building& building) {
    Stmt stmt(db,
              "INSERT INTO roofs (id,project_id,storey_id,roof_type,pitch_deg,"
              "overhang_mm,footprint_json) VALUES (?,1,?,?,?,?,?);");
    for (const auto& roof : building.roofs) {
        const std::string footprint = footprintToJson(roof);
        sqlite3_bind_int(stmt.get(), 1, static_cast<int>(roof.id));
        sqlite3_bind_int(stmt.get(), 2, static_cast<int>(roof.storey_id));
        sqlite3_bind_text(stmt.get(), 3, roofTypeToText(roof.type), -1,
                          SQLITE_STATIC);
        sqlite3_bind_double(stmt.get(), 4, roof.pitch_deg);
        sqlite3_bind_double(stmt.get(), 5, roof.overhang_mm);
        sqlite3_bind_text(stmt.get(), 6, footprint.c_str(), -1,
                          SQLITE_TRANSIENT);
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

void loadOpenings(Db& db, hexagon::model::Building& building) {
    Stmt stmt(db,
              "SELECT o.id,o.wall_id,o.opening_type,o.offset_mm,o.width_mm,"
              "o.height_mm,o.sill_height_mm,d.swing_direction FROM openings o "
              "LEFT JOIN doors d ON d.opening_id=o.id ORDER BY o.id;");
    while (stmt.step()) {
        hexagon::model::Opening opening;
        opening.id = static_cast<hexagon::model::OpeningId>(
            sqlite3_column_int(stmt.get(), 0));
        opening.wall_id = static_cast<hexagon::model::WallId>(
            sqlite3_column_int(stmt.get(), 1));
        const auto* type = sqlite3_column_text(stmt.get(), 2);
        opening.kind = textToOpeningKind(
            type != nullptr ? reinterpret_cast<const char*>(type) : "");
        opening.offset_mm = sqlite3_column_double(stmt.get(), 3);
        opening.width_mm = sqlite3_column_double(stmt.get(), 4);
        opening.height_mm = sqlite3_column_double(stmt.get(), 5);
        opening.sill_height_mm = sqlite3_column_double(stmt.get(), 6);
        if (opening.kind == hexagon::model::OpeningKind::Door) {
            const auto* swing = sqlite3_column_text(stmt.get(), 7);
            opening.swing = textToSwing(
                swing != nullptr ? reinterpret_cast<const char*>(swing) : "left");
        }
        building.openings.push_back(opening);
    }
}

void loadRoofs(Db& db, hexagon::model::Building& building) {
    Stmt stmt(db,
              "SELECT id,storey_id,roof_type,pitch_deg,overhang_mm,"
              "footprint_json FROM roofs ORDER BY id;");
    while (stmt.step()) {
        hexagon::model::Roof roof;
        roof.id =
            static_cast<hexagon::model::RoofId>(sqlite3_column_int(stmt.get(), 0));
        roof.storey_id = static_cast<hexagon::model::StoreyId>(
            sqlite3_column_int(stmt.get(), 1));
        const auto* type = sqlite3_column_text(stmt.get(), 2);
        roof.type = textToRoofType(
            type != nullptr ? reinterpret_cast<const char*>(type) : "");
        roof.pitch_deg = sqlite3_column_double(stmt.get(), 3);
        roof.overhang_mm = sqlite3_column_double(stmt.get(), 4);
        const auto* footprint = sqlite3_column_text(stmt.get(), 5);
        parseFootprintJson(
            footprint != nullptr ? reinterpret_cast<const char*>(footprint) : "",
            roof);
        building.roofs.push_back(roof);
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
            insertOpenings(*db, building);  // nach Wänden (FK wall_id)
            insertRoofs(*db, building);      // nach Geschossen (FK storey_id)
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
        loadOpenings(db, building);
        loadRoofs(db, building);
        return building;
    } catch (const SqliteError& e) {
        throw std::runtime_error("E-IO: Projektdatei nicht lesbar ('" +
                                 path.string() + "'): " + e.context);
    }
}

}  // namespace bcad::adapters::persistence
