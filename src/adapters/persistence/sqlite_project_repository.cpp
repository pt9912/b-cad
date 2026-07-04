// Driven Adapter (Persistenz, ADR-0003): ProjectRepositoryPort über SQLite.
// Atomar (Temp + fsync + Rename); SQLite-Typen bleiben hier gekapselt
// (a-check Regel D). Schema-DDL aus spec/data-model.yaml generiert
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
#include <vector>

#include <sqlite3.h>

#include "adapters/persistence/schema_sql.h"  // kSchemaSql (generiert)
#include "hexagon/services/geometry/stair_geometry.h"  // stairRiseMm (rise write-derived)

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
    Db(Db&&) = delete;
    Db& operator=(Db&&) = delete;

    [[nodiscard]] sqlite3* handle() const { return db_; }

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
    Stmt(Stmt&&) = delete;
    Stmt& operator=(Stmt&&) = delete;

    [[nodiscard]] sqlite3_stmt* get() const { return stmt_; }

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

// Platten-Typ ↔ Text. `slabs.slab_type` trägt (anders als `roofs.roof_type`)
// **keine** CHECK-Constraint im Schema (ADR-0006) — daher erzwingt der Mapper
// hier die gültige Menge {decke,fundament,bodenplatte} und wirft bei
// Unbekanntem neutral (`E-IO`, kein Lib-Typ verlässt den Adapter).
const char* slabTypeToText(hexagon::model::SlabType type) {
    switch (type) {
        case hexagon::model::SlabType::Decke:
            return "decke";
        case hexagon::model::SlabType::Fundament:
            return "fundament";
        case hexagon::model::SlabType::Bodenplatte:
            return "bodenplatte";
    }
    throw std::runtime_error("E-IO: unbekannter SlabType");
}

hexagon::model::SlabType textToSlabType(const std::string& text) {
    if (text == "decke") {
        return hexagon::model::SlabType::Decke;
    }
    if (text == "fundament") {
        return hexagon::model::SlabType::Fundament;
    }
    if (text == "bodenplatte") {
        return hexagon::model::SlabType::Bodenplatte;
    }
    throw std::runtime_error("E-IO: unbekannter slab_type '" + text + "'");
}

// Treppen-Typ ↔ Text. `stairs.stair_type` trägt **keine** CHECK-Constraint
// (ADR-0006) — der Mapper erzwingt die gültige Menge {gerade} (welle-2-Teil-
// umfang) und wirft bei Unbekanntem neutral (`E-IO`, kein Lib-Typ leckt).
const char* stairTypeToText(hexagon::model::StairType type) {
    switch (type) {
        case hexagon::model::StairType::Gerade:
            return "gerade";
    }
    throw std::runtime_error("E-IO: unbekannter StairType");
}

hexagon::model::StairType textToStairType(const std::string& text) {
    if (text == "gerade") {
        return hexagon::model::StairType::Gerade;
    }
    throw std::runtime_error("E-IO: unbekannter stair_type '" + text + "'");
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

// Platten-Grundriss + Aussparungen als verschachtelte Ring-Arrays
// `[[fx0,fy0,…],[c1x0,c1y0,…],…]` (ADR-0006 `slabs.polygon_json`): Element 0
// ist der Grundriss-Ring (footprint), Elemente 1..n sind die Ausschnitt-Ringe
// (cutouts, LH-FA-SLB-003). `%.17g` → exakter double-Round-Trip. Generalisiert
// das 014c-Flach-Array (angekündigte Erweiterung „Ring statt 5-Tupel");
// variabel lang → `std::string`-Builder. Eigenes, deterministisches Format;
// nur dieser Adapter schreibt/liest es.
void appendRingJson(std::string& out, const std::vector<hexagon::model::Point2D>& pts) {
    std::array<char, 64> buf{};
    out += '[';
    for (std::size_t i = 0; i < pts.size(); ++i) {
        if (i != 0) {
            out += ',';
        }
        std::snprintf(buf.data(), buf.size(), "%.17g", pts[i].x_mm);
        out += buf.data();
        out += ',';
        std::snprintf(buf.data(), buf.size(), "%.17g", pts[i].y_mm);
        out += buf.data();
    }
    out += ']';
}

std::string polygonToJson(const hexagon::model::Slab& slab) {
    std::string out = "[";
    appendRingJson(out, slab.footprint.points);  // Ring 0 = Grundriss
    for (const auto& cutout : slab.cutouts) {
        out += ',';
        appendRingJson(out, cutout.points);  // Ring 1..n = Aussparungen
    }
    out += ']';
    return out;
}

// Parst einen Ring (flache `x,y`-Folge) zu Punkten; gerade Wertzahl Pflicht,
// jedes Token **vollständig** numerisch (kein `stod`-Müll-Suffix wie "1.5x",
// kein leeres Token) — sonst neutraler `E-IO`-Wurf (total gegen Fremd-Inhalt).
std::vector<hexagon::model::Point2D> parseRing(const std::string& inner) {
    std::stringstream stream(inner);
    std::vector<double> values;
    std::string token;
    try {
        while (std::getline(stream, token, ',')) {
            if (token.empty()) {
                throw std::runtime_error("E-IO: polygon_json leeres Zahl-Token");
            }
            std::size_t consumed = 0;
            const double value = std::stod(token, &consumed);
            if (consumed != token.size()) {  // Teil-Parse (Müll-Suffix) → ablehnen
                throw std::runtime_error("E-IO: polygon_json Zahl ungültig: '" +
                                         token + "'");
            }
            values.push_back(value);
        }
    } catch (const std::logic_error& e) {  // stod: invalid_argument/out_of_range
        throw std::runtime_error(std::string("E-IO: polygon_json Zahl ungültig: ") +
                                 e.what());
    }
    if (values.size() % 2 != 0) {
        throw std::runtime_error("E-IO: polygon_json Ring mit ungerader Wertzahl");
    }
    std::vector<hexagon::model::Point2D> points;
    points.reserve(values.size() / 2);
    for (std::size_t i = 0; i + 1 < values.size(); i += 2) {
        points.push_back({values[i], values[i + 1]});
    }
    return points;
}

// Parst das eigene verschachtelte Format zurück (Ring 0 → footprint, 1..n →
// cutouts) per balanciertem `[...]`-Scan auf Tiefe 1; wirft bei Format-Fehler
// neutral (`E-IO`, total nach außen — kein Lib-Typ leckt, Regel D).
void parseSlabPolygonJson(const std::string& json, hexagon::model::Slab& slab) {
    std::vector<std::vector<hexagon::model::Point2D>> rings;
    int depth = 0;
    std::size_t ring_start = std::string::npos;
    for (std::size_t i = 0; i < json.size(); ++i) {
        const char chr = json[i];
        if (chr == '[') {
            ++depth;
            if (depth == 2) {
                ring_start = i + 1;  // Inhalt beginnt nach dem inneren '['
            }
        } else if (chr == ']') {
            if (depth == 2 && ring_start != std::string::npos) {
                rings.push_back(parseRing(json.substr(ring_start, i - ring_start)));
                ring_start = std::string::npos;
            }
            --depth;
            if (depth < 0) {
                throw std::runtime_error("E-IO: polygon_json unbalanciert: '" + json +
                                         "'");
            }
        }
    }
    if (depth != 0) {
        throw std::runtime_error("E-IO: polygon_json unbalanciert: '" + json + "'");
    }
    if (rings.empty()) {
        throw std::runtime_error("E-IO: polygon_json ohne Grundriss-Ring: '" + json +
                                 "'");
    }
    slab.footprint.points = rings.front();
    slab.cutouts.clear();
    for (std::size_t i = 1; i < rings.size(); ++i) {
        slab.cutouts.push_back(hexagon::model::Footprint{rings[i]});
    }
}

void insertProject(Db& db) {
    Stmt stmt(db,
              "INSERT INTO projects (id,name,file_version,unit,created_at,"
              "updated_at) VALUES (1,'b-cad',1,'mm',?,?);");
    sqlite3_bind_text(stmt.get(), 1, kSentinelTs, -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt.get(), 2, kSentinelTs, -1, SQLITE_STATIC);
    stmt.step();
}

// --- Material-Persistenz (slice-017e): NULL-sichere Optional-Bindung/-Lesung ---
// KORREKTHEITS-KERN: `sqlite3_column_double`/`_int` liefert für `NULL` still
// `0.0`/`0` — daher beim Laden IMMER `sqlite3_column_type == SQLITE_NULL` prüfen,
// sonst würde „kein Wert" zu 0 verfälscht (stille Korruption, slice-015c-Klasse).
// Eine Quelle der Wahrheit für alle optionalen Spalten.

void bindOptionalDouble(Stmt& stmt, int pos, const std::optional<double>& v) {
    if (v.has_value()) {
        sqlite3_bind_double(stmt.get(), pos, *v);
    } else {
        sqlite3_bind_null(stmt.get(), pos);
    }
}

void bindOptionalText(Stmt& stmt, int pos, const std::optional<std::string>& v) {
    if (v.has_value()) {
        sqlite3_bind_text(stmt.get(), pos, v->c_str(), -1, SQLITE_TRANSIENT);
    } else {
        sqlite3_bind_null(stmt.get(), pos);
    }
}

void bindOptionalMaterial(Stmt& stmt, int pos,
                          const std::optional<hexagon::model::MaterialId>& m) {
    if (m.has_value()) {
        sqlite3_bind_int(stmt.get(), pos, static_cast<int>(*m));
    } else {
        sqlite3_bind_null(stmt.get(), pos);
    }
}

std::optional<double> columnOptionalDouble(Stmt& stmt, int col) {
    if (sqlite3_column_type(stmt.get(), col) == SQLITE_NULL) {
        return std::nullopt;
    }
    return sqlite3_column_double(stmt.get(), col);
}

std::optional<std::string> columnOptionalText(Stmt& stmt, int col) {
    if (sqlite3_column_type(stmt.get(), col) == SQLITE_NULL) {
        return std::nullopt;
    }
    const auto* text = sqlite3_column_text(stmt.get(), col);
    // text == nullptr nur bei SQLite-interner OOM/Encoding-Konversion (Spalte
    // ist nicht NULL-typisiert) — als nullopt behandeln statt present-empty
    // (Code-Review L1), damit „kein Wert" eindeutig bleibt.
    if (text == nullptr) {
        return std::nullopt;
    }
    return std::string(reinterpret_cast<const char*>(text));
}

std::optional<hexagon::model::MaterialId> columnOptionalMaterial(Stmt& stmt,
                                                                 int col) {
    if (sqlite3_column_type(stmt.get(), col) == SQLITE_NULL) {
        return std::nullopt;
    }
    return static_cast<hexagon::model::MaterialId>(
        sqlite3_column_int(stmt.get(), col));
}

// Materialien (LH-FA-MAT-*, ADR-0006): projekt-eigene Material-Bibliothek. VOR
// walls/roofs/slabs (FK `material_id → materials.id`, `foreign_keys=ON`).
// `density` ist KEIN Domänenfeld (spez. §2.1) → aus der INSERT-Spaltenliste
// ausgelassen (⇒ `NULL`, Muster wie roofs.`height_mm`). Kennwerte/Texte
// optional → NULL-sicher gebunden.
void insertMaterials(Db& db, const hexagon::model::Building& building) {
    Stmt stmt(db,
              "INSERT INTO materials (id,project_id,name,category,color_hex,"
              "texture_path,u_value,cost_per_m2,cost_per_m3) "
              "VALUES (?,1,?,?,?,?,?,?,?);");
    for (const auto& material : building.materials) {
        sqlite3_bind_int(stmt.get(), 1, static_cast<int>(material.id));
        sqlite3_bind_text(stmt.get(), 2, material.name.c_str(), -1,
                          SQLITE_TRANSIENT);
        sqlite3_bind_text(stmt.get(), 3, material.category.c_str(), -1,
                          SQLITE_TRANSIENT);
        bindOptionalText(stmt, 4, material.color_hex);
        bindOptionalText(stmt, 5, material.texture_path);
        bindOptionalDouble(stmt, 6, material.u_value);
        bindOptionalDouble(stmt, 7, material.cost_per_m2);
        bindOptionalDouble(stmt, 8, material.cost_per_m3);
        stmt.step();
        stmt.reset();
    }
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
              "material_id,created_at,updated_at) "
              "VALUES (?,1,?,?,?,?,?,?,?,?,?,?,?);");
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
        bindOptionalMaterial(stmt, 10, wall.material_id);
        sqlite3_bind_text(stmt.get(), 11, kSentinelTs, -1, SQLITE_STATIC);
        sqlite3_bind_text(stmt.get(), 12, kSentinelTs, -1, SQLITE_STATIC);
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
// `roofs.storey_id → storeys.id`) UND `insertMaterials` (FK `material_id →
// materials.id`). `height_mm` (abgeleitete Firsthöhe) bleibt `NULL`;
// `material_id` wird seit slice-017e round-getrippt (NULL bei keinem Material).
// `thickness_mm` (Dach-Volumenkörper, LH-FA-ROF-006) wird seit slice-023c
// round-getrippt — explizite Spalte (`bind_double`), Default 200 nur für Zeilen
// ohne expliziten Schreiber (Alt-Datei vor 023c → kDefaultRoofThicknessMm).
void insertRoofs(Db& db, const hexagon::model::Building& building) {
    Stmt stmt(db,
              "INSERT INTO roofs (id,project_id,storey_id,roof_type,pitch_deg,"
              "overhang_mm,thickness_mm,footprint_json,material_id) "
              "VALUES (?,1,?,?,?,?,?,?,?);");
    for (const auto& roof : building.roofs) {
        const std::string footprint = footprintToJson(roof);
        sqlite3_bind_int(stmt.get(), 1, static_cast<int>(roof.id));
        sqlite3_bind_int(stmt.get(), 2, static_cast<int>(roof.storey_id));
        sqlite3_bind_text(stmt.get(), 3, roofTypeToText(roof.type), -1,
                          SQLITE_STATIC);
        sqlite3_bind_double(stmt.get(), 4, roof.pitch_deg);
        sqlite3_bind_double(stmt.get(), 5, roof.overhang_mm);
        sqlite3_bind_double(stmt.get(), 6, roof.thickness_mm);
        sqlite3_bind_text(stmt.get(), 7, footprint.c_str(), -1,
                          SQLITE_TRANSIENT);
        bindOptionalMaterial(stmt, 8, roof.material_id);
        stmt.step();
        stmt.reset();
    }
}

// Platten (LH-FA-SLB-*/FND-*, ADR-0011/0006): NACH `insertStoreys` (FK
// `slabs.storey_id → storeys.id`) UND `insertMaterials` (FK `material_id →
// materials.id`). `material_id` wird seit slice-017e round-getrippt (NULL bei
// keinem Material); `base_z` wird **nicht** gespeichert (abgeleitet aus
// Typ/Geschoss, `slab_geometry`). Grundriss + Aussparungen als `polygon_json`.
void insertSlabs(Db& db, const hexagon::model::Building& building) {
    Stmt stmt(db,
              "INSERT INTO slabs (id,project_id,storey_id,slab_type,"
              "thickness_mm,polygon_json,material_id) VALUES (?,1,?,?,?,?,?);");
    for (const auto& slab : building.slabs) {
        const std::string polygon = polygonToJson(slab);
        sqlite3_bind_int(stmt.get(), 1, static_cast<int>(slab.id));
        sqlite3_bind_int(stmt.get(), 2, static_cast<int>(slab.storey_id));
        sqlite3_bind_text(stmt.get(), 3, slabTypeToText(slab.type), -1,
                          SQLITE_STATIC);
        sqlite3_bind_double(stmt.get(), 4, slab.thickness_mm);
        sqlite3_bind_text(stmt.get(), 5, polygon.c_str(), -1, SQLITE_TRANSIENT);
        bindOptionalMaterial(stmt, 6, slab.material_id);
        stmt.step();
        stmt.reset();
    }
}

// from_storey-Höhe für die abgeleitete `rise` (MED-1: **total** — fehlt das
// Geschoss in `building.storeys`, neutraler `E-IO`-Wurf statt stillem `rise=0`;
// rollt in der Transaktion zurück).
double fromStoreyHeight(const hexagon::model::Building& building,
                        hexagon::model::StoreyId id) {
    for (const auto& storey : building.storeys) {
        if (storey.id == id) {
            return storey.height_mm;
        }
    }
    throw std::runtime_error("E-IO: stairs from_storey unbekannt");
}

// Treppen (LH-FA-STR-*, ADR-0011/0006): NACH `insertStoreys` (FK
// `stairs.from/to_storey_id → storeys.id`, RESTRICT). `name` bleibt `NULL`
// (welle-2-Scope). `rise_mm` ist **abgeleitet** (Geschosshöhe/step_count, via
// Kern-Helfer `stairRiseMm`) → write-derived; beim Laden NICHT zurückgelesen
// (der Kern leitet es neu ab — Muster roofs-`height_mm`). Geländer render-only.
void insertStairs(Db& db, const hexagon::model::Building& building) {
    Stmt stmt(db,
              "INSERT INTO stairs (id,project_id,from_storey_id,to_storey_id,"
              "stair_type,start_x_mm,start_y_mm,width_mm,step_count,rise_mm,"
              "tread_mm) VALUES (?,1,?,?,?,?,?,?,?,?,?);");
    for (const auto& stair : building.stairs) {
        const double rise = hexagon::services::stairRiseMm(
            stair, fromStoreyHeight(building, stair.from_storey_id));
        sqlite3_bind_int(stmt.get(), 1, static_cast<int>(stair.id));
        sqlite3_bind_int(stmt.get(), 2, static_cast<int>(stair.from_storey_id));
        sqlite3_bind_int(stmt.get(), 3, static_cast<int>(stair.to_storey_id));
        sqlite3_bind_text(stmt.get(), 4, stairTypeToText(stair.type), -1,
                          SQLITE_STATIC);
        sqlite3_bind_double(stmt.get(), 5, stair.start.x_mm);
        sqlite3_bind_double(stmt.get(), 6, stair.start.y_mm);
        sqlite3_bind_double(stmt.get(), 7, stair.width_mm);
        sqlite3_bind_int(stmt.get(), 8, stair.step_count);
        sqlite3_bind_double(stmt.get(), 9, rise);
        sqlite3_bind_double(stmt.get(), 10, stair.tread_mm);
        stmt.step();
        stmt.reset();
    }
}

// Materialien laden (LH-FA-MAT-*): die Bibliothek. Optionale Kennwerte/Texte
// NULL-sicher (`columnOptional*` — kein „NULL → 0"); `density` nicht gelesen
// (kein Domänenfeld). FK-frei beim Laden; ID-Erhalt für die `material_id`-
// Auflösung der Bauteile.
void loadMaterials(Db& db, hexagon::model::Building& building) {
    Stmt stmt(db,
              "SELECT id,name,category,color_hex,texture_path,u_value,"
              "cost_per_m2,cost_per_m3 FROM materials ORDER BY id;");
    while (stmt.step()) {
        hexagon::model::Material material;
        material.id = static_cast<hexagon::model::MaterialId>(
            sqlite3_column_int(stmt.get(), 0));
        const auto* name = sqlite3_column_text(stmt.get(), 1);
        material.name =
            (name != nullptr) ? reinterpret_cast<const char*>(name) : "";
        const auto* category = sqlite3_column_text(stmt.get(), 2);
        material.category =
            (category != nullptr) ? reinterpret_cast<const char*>(category) : "";
        material.color_hex = columnOptionalText(stmt, 3);
        material.texture_path = columnOptionalText(stmt, 4);
        material.u_value = columnOptionalDouble(stmt, 5);
        material.cost_per_m2 = columnOptionalDouble(stmt, 6);
        material.cost_per_m3 = columnOptionalDouble(stmt, 7);
        building.materials.push_back(material);
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
              "thickness_mm,height_mm,classification,material_id "
              "FROM walls ORDER BY id;");
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
        wall.material_id = columnOptionalMaterial(stmt, 9);
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
              "SELECT id,storey_id,roof_type,pitch_deg,overhang_mm,thickness_mm,"
              "footprint_json,material_id FROM roofs ORDER BY id;");
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
        roof.thickness_mm = sqlite3_column_double(stmt.get(), 5);
        const auto* footprint = sqlite3_column_text(stmt.get(), 6);
        parseFootprintJson(
            footprint != nullptr ? reinterpret_cast<const char*>(footprint) : "",
            roof);
        roof.material_id = columnOptionalMaterial(stmt, 7);
        building.roofs.push_back(roof);
    }
}

void loadSlabs(Db& db, hexagon::model::Building& building) {
    Stmt stmt(db,
              "SELECT id,storey_id,slab_type,thickness_mm,polygon_json,"
              "material_id FROM slabs ORDER BY id;");
    while (stmt.step()) {
        hexagon::model::Slab slab;
        slab.id =
            static_cast<hexagon::model::SlabId>(sqlite3_column_int(stmt.get(), 0));
        slab.storey_id = static_cast<hexagon::model::StoreyId>(
            sqlite3_column_int(stmt.get(), 1));
        const auto* type = sqlite3_column_text(stmt.get(), 2);
        slab.type = textToSlabType(
            type != nullptr ? reinterpret_cast<const char*>(type) : "");
        slab.thickness_mm = sqlite3_column_double(stmt.get(), 3);
        const auto* polygon = sqlite3_column_text(stmt.get(), 4);
        parseSlabPolygonJson(
            polygon != nullptr ? reinterpret_cast<const char*>(polygon) : "", slab);
        slab.material_id = columnOptionalMaterial(stmt, 5);
        building.slabs.push_back(slab);
    }
}

// Treppen laden: feldgleich für die Domänenfelder; `rise_mm` (abgeleitet) und
// `name` (nicht im Modell) werden **nicht** zurückgelesen.
void loadStairs(Db& db, hexagon::model::Building& building) {
    Stmt stmt(db,
              "SELECT id,from_storey_id,to_storey_id,stair_type,start_x_mm,"
              "start_y_mm,width_mm,step_count,tread_mm FROM stairs ORDER BY id;");
    while (stmt.step()) {
        hexagon::model::Stair stair;
        stair.id = static_cast<hexagon::model::StairId>(
            sqlite3_column_int(stmt.get(), 0));
        stair.from_storey_id = static_cast<hexagon::model::StoreyId>(
            sqlite3_column_int(stmt.get(), 1));
        stair.to_storey_id = static_cast<hexagon::model::StoreyId>(
            sqlite3_column_int(stmt.get(), 2));
        const auto* type = sqlite3_column_text(stmt.get(), 3);
        stair.type = textToStairType(
            type != nullptr ? reinterpret_cast<const char*>(type) : "");
        stair.start = {sqlite3_column_double(stmt.get(), 4),
                       sqlite3_column_double(stmt.get(), 5)};
        stair.width_mm = sqlite3_column_double(stmt.get(), 6);
        stair.step_count = sqlite3_column_int(stmt.get(), 7);
        stair.tread_mm = sqlite3_column_double(stmt.get(), 8);
        building.stairs.push_back(stair);
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
            insertMaterials(*db, building);  // vor walls/roofs/slabs (FK material_id)
            insertWalls(*db, building);
            insertOpenings(*db, building);  // nach Wänden (FK wall_id)
            insertRoofs(*db, building);      // nach Geschossen (FK storey_id)
            insertSlabs(*db, building);      // nach Geschossen (FK storey_id)
            insertStairs(*db, building);     // nach Geschossen (FK from/to_storey)
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
        loadMaterials(db, building);
        loadWalls(db, building);
        loadOpenings(db, building);
        loadRoofs(db, building);
        loadSlabs(db, building);  // Lade-Reihenfolge: Stil-Konsistenz (kein FK auf load)
        loadStairs(db, building);
        return building;
    } catch (const SqliteError& e) {
        throw std::runtime_error("E-IO: Projektdatei nicht lesbar ('" +
                                 path.string() + "'): " + e.context);
    }
}

}  // namespace bcad::adapters::persistence
