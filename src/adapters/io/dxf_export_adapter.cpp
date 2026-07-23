// DXF-Export-Mapping (ADR-0015 Option D). Bildet das welle-4-Subset (spez. §1
// LH-FA-IO-003.a) auf ASCII-DXF (R12/AC1009) ab und serialisiert über den
// hand-gerollten `DxfWriter`. **Atomar** (Temp + fsync + Rename, Muster
// Persistenz/`ifc_export_adapter`): vollständige Datei oder Wurf, kein
// Teil-Export.
//
// 2D-Grundriss: HEADER ($ACADVER R12), TABLES mit einer LAYER je Geschoss,
// ENTITIES mit je einer LINE pro gerader Wand. Jede LINE trägt ihr
// Geschoss-LAYER als Gruppencode 8 (die Roundtrip-Achse der Geschoss-Anzahl);
// z-Koordinaten 0. Leeres Modell -> gültige, (annähernd) leere DXF.

#include "adapters/io/dxf_export_adapter.h"

#include <fcntl.h>
#include <unistd.h>

#include <cerrno>
#include <cstddef>
#include <cstring>
#include <string>
#include <system_error>

#include <unordered_set>

#include "adapters/io/dxf_writer.h"
#include "adapters/io/plan_geometry.h"  // visibleLayerIds (geteilter Filter)
#include "hexagon/model/building.h"
#include "hexagon/model/guide_line.h"
#include "hexagon/model/layer.h"
#include "hexagon/model/storey.h"
#include "hexagon/model/wall.h"

namespace fs = std::filesystem;
namespace model = bcad::hexagon::model;

namespace bcad::adapters::io {
namespace {

std::string layerName(model::StoreyId id) {
    return "STOREY_" + std::to_string(static_cast<int>(id));
}

void writeHeader(DxfWriter& writer) {
    writer.pair(0, "SECTION");
    writer.pair(2, "HEADER");
    writer.pair(9, "$ACADVER");
    writer.pair(1, "AC1009");  // R12 — maximal lesbar
    writer.pair(0, "ENDSEC");
}

// Eine LAYER-Tabelle mit je einem Eintrag pro Geschoss (offizielle Layer).
void writeLayerTable(DxfWriter& writer, const model::Building& building) {
    writer.pair(0, "SECTION");
    writer.pair(2, "TABLES");
    writer.pair(0, "TABLE");
    writer.pair(2, "LAYER");
    for (const model::Storey& storey : building.storeys) {
        writer.pair(0, "LAYER");
        writer.pair(2, layerName(storey.id));
        writer.pair(70, "0");          // keine Flags
        writer.pair(62, "7");          // Farbe (weiß)
        writer.pair(6, "CONTINUOUS");  // Linientyp
    }
    writer.pair(0, "ENDTAB");
    writer.pair(0, "ENDSEC");
}

// Je gerade Wand eine LINE (Achse, z=0) auf ihrem Geschoss-LAYER (Gruppencode 8).
void writeEntities(DxfWriter& writer, const model::Building& building) {
    writer.pair(0, "SECTION");
    writer.pair(2, "ENTITIES");
    for (const model::Wall& wall : building.walls) {
        writer.pair(0, "LINE");
        writer.pair(8, layerName(wall.storey_id));  // Geschoss-LAYER (MED-3)
        writer.pair(10, wall.start.x_mm);
        writer.pair(20, wall.start.y_mm);
        writer.pair(30, 0.0);
        writer.pair(11, wall.end.x_mm);
        writer.pair(21, wall.end.y_mm);
        writer.pair(31, 0.0);
    }
    // Hilfslinien auf SICHTBARER Ebene (LH-FA-DRW-005, ADR-0018): je eine LINE
    // auf ihrem Geschoss-LAYER (STOREY_n bleibt unverändert — der Benutzer-Layer
    // `visible` ist reiner Export-Filter, KEIN neuer DXF-Layer, ADR-0018 §3).
    // Unsichtbare Ebene → keine LINE (DRW-005-Negative). Derselbe
    // visibleLayerIds-Filter wie PDF/PNG (plan_geometry) — kein Format-Drift.
    const std::unordered_set<int> visible = visibleLayerIds(building);
    for (const model::GuideLine& guide : building.guide_lines) {
        if (!visible.contains(static_cast<int>(guide.layer_id))) {
            continue;
        }
        writer.pair(0, "LINE");
        writer.pair(8, layerName(guide.storey_id));
        writer.pair(10, guide.segment.start.x_mm);
        writer.pair(20, guide.segment.start.y_mm);
        writer.pair(30, 0.0);
        writer.pair(11, guide.segment.end.x_mm);
        writer.pair(21, guide.segment.end.y_mm);
        writer.pair(31, 0.0);
    }
    writer.pair(0, "ENDSEC");
}

std::string buildDxf(const model::Building& building) {
    DxfWriter writer;
    writeHeader(writer);
    writeLayerTable(writer, building);
    writeEntities(writer, building);
    writer.pair(0, "EOF");
    return writer.build();
}

// --- Atomares Schreiben (Temp + fsync + Rename) ---------------------------
// errno -> Spec-Fehlercode (Muster ifc_export_adapter): Zielpfad nicht
// beschreibbar (inkl. EISDIR/ENOTDIR/ENOENT der open-Phase) -> E-IO-001, sonst
// (Medium voll / IO) -> E-IO-002.
std::string ioCodeForErrno(int err) {
    switch (err) {
        case EACCES:
        case EPERM:
        case EROFS:
        case EISDIR:
        case ENOTDIR:
        case ENOENT:
            return "E-IO-001";
        default:
            return "E-IO-002";
    }
}

[[noreturn]] void throwIo(int err, const std::string& what, const fs::path& at) {
    const std::string code = ioCodeForErrno(err);
    const std::string event =
        code == "E-IO-001" ? "io_no_permission" : "persist_error";
    throw std::runtime_error(code + ": DXF-Export " + what + " ('" +
                             at.string() + "'): " + std::strerror(err) +
                             "; event=" + event);
}

void atomicWrite(const fs::path& path, const std::string& content) {
    const fs::path tmp(path.string() + ".tmp");
    const int fd = ::open(tmp.c_str(), O_WRONLY | O_CREAT | O_TRUNC, 0644);
    if (fd < 0) {
        throwIo(errno, "Zielpfad nicht beschreibbar", tmp);
    }
    const char* data = content.data();
    std::size_t remaining = content.size();
    while (remaining > 0) {
        const ssize_t written = ::write(fd, data, remaining);
        if (written < 0) {
            const int err = errno;
            ::close(fd);
            std::error_code rm;
            fs::remove(tmp, rm);
            throwIo(err, "Schreibfehler", tmp);
        }
        data += written;
        remaining -= static_cast<std::size_t>(written);
    }
    ::fsync(fd);
    ::close(fd);
    std::error_code ec;
    fs::rename(tmp, path, ec);  // atomarer Ersatz; bei Fehler Zielpfad intakt
    if (ec) {
        std::error_code rm;
        fs::remove(tmp, rm);
        throwIo(ec.value(), "Rename auf Zielpfad", path);
    }
}

}  // namespace

void DxfExportAdapter::write(const model::Building& building,
                             const model::DerivedGeometry& /*derived*/,
                             const fs::path& path) const {
    const std::string dxf = buildDxf(building);  // vollständig im Speicher …
    atomicWrite(path, dxf);                       // … dann atomar schreiben
}

}  // namespace bcad::adapters::io
