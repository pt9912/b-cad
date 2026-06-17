// IFC-Export-Mapping (ADR-0013 Option D). Bildet das welle-4-Subset
// (spez. §1 LH-FA-IO-001.a) auf IFC4-Entitäten ab und serialisiert über den
// hand-gerollten IfcSpfWriter. **Atomar** (Temp + fsync + Rename, Muster
// Persistenz): vollständige Datei oder Wurf, kein Teil-Export.
//
// Roundtrip-Disziplin: die Attribut-Positionen + der byte-exakte
// RepresentationIdentifier 'Axis'/'Body' entsprechen genau dem, was der
// 019b-Importer liest (sonst fände der Re-Import die Achse nicht). Bottom-up
// gebaut (referenzierte Entität zuerst) → keine Vorwärts-Referenzen.
//
// Subset-Grenzen (benannte Lücken, Spiegel 019b): nur Geschosse + gerade Wände;
// Türen/Fenster/Dach/Decken/Treppen/Material-Bibliothek werden NICHT
// geschrieben. Wand-Body trägt nur die Extrusions-Höhe (Profil/Placement nicht
// ausgeführt — der Import liest nur die Tiefe). `Wall.type`/`material_id`-
// Override haben keine IFC-Subset-Entsprechung.

#include "adapters/io/ifc_export_adapter.h"

#include <fcntl.h>
#include <unistd.h>

#include <cerrno>
#include <cstddef>
#include <cstring>
#include <map>
#include <stdexcept>
#include <string>
#include <system_error>
#include <vector>

#include "adapters/io/ifc_spf_writer.h"
#include "hexagon/model/building.h"
#include "hexagon/model/storey.h"
#include "hexagon/model/wall.h"

namespace fs = std::filesystem;
namespace model = bcad::hexagon::model;

namespace bcad::adapters::io {
namespace {

// --- Domänen → IFC4-Mapping (über den SPF-Writer) -------------------------

// Deterministischer Platzhalter-GlobalId (22 Zeichen, IFC-base64-Alphabet).
// Der 019b-Importer liest GlobalId nie; relevant ist nur ein wohlgeformter,
// deterministischer String (byte-stabiler Roundtrip).
std::string ifcGuid(int seq) {
    static const char* const kAlphabet =
        "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ"
        "abcdefghijklmnopqrstuvwxyz_$";
    std::string guid(22, '0');
    int value = seq;
    int pos = 21;
    while (value > 0 && pos >= 0) {
        guid[static_cast<std::size_t>(pos)] = kAlphabet[value % 64];
        value /= 64;
        --pos;
    }
    return guid;
}

// Schreib-Kontext: Writer + laufender GlobalId-Zähler.
struct ExportContext {
    IfcSpfWriter& writer;
    int guid_seq{0};
    std::string nextGuid() { return ifcGuid(++guid_seq); }
};

struct SpatialRoot {
    int project_id{0};
    int building_id{0};
};

struct StoreyExport {
    std::vector<int> ifc_ids;
    std::map<model::StoreyId, int> by_storey;
};

int writeUnits(ExportContext& ctx) {
    // mm: IfcSIUnit(.LENGTHUNIT., .MILLI., .METRE.) + IfcUnitAssignment.
    const int unit = ctx.writer.add("IFCSIUNIT(*,.LENGTHUNIT.,.MILLI.,.METRE.)");
    return ctx.writer.add("IFCUNITASSIGNMENT(" + spfRefList({unit}) + ")");
}

// Ein IfcBuildingStorey je b-cad-Geschoss; Elevation = kumulierte Höhe (Spiegel
// der 019b-Import-Ableitung Höhe = Elevation-Differenz). Index 9 = Elevation.
StoreyExport writeStoreys(ExportContext& ctx, const model::Building& building) {
    StoreyExport out;
    double elevation = 0.0;
    for (const model::Storey& storey : building.storeys) {
        const int id = ctx.writer.add(
            "IFCBUILDINGSTOREY(" + spfString(ctx.nextGuid()) + ",$," +
            spfString("Storey") + ",$,$,$,$,$,.ELEMENT.," + spfReal(elevation) +
            ")");
        out.ifc_ids.push_back(id);
        out.by_storey[storey.id] = id;
        elevation += storey.height_mm;
    }
    return out;
}

// Eine gerade Wand: Achs-Polyline ('Axis') + Extrusions-Body ('Body') +
// Material-Layer (Dicke) + Geschoss-Containment. RepresentationIdentifier
// byte-exakt (MED-1). Wall.Representation = Index 6 (IFC4 9-Attribut-Form).
void writeWall(ExportContext& ctx, const model::Wall& wall, int storey_ifc_id) {
    IfcSpfWriter& w = ctx.writer;
    const int p1 = w.add("IFCCARTESIANPOINT((" + spfReal(wall.start.x_mm) + "," +
                         spfReal(wall.start.y_mm) + ",0.))");
    const int p2 = w.add("IFCCARTESIANPOINT((" + spfReal(wall.end.x_mm) + "," +
                         spfReal(wall.end.y_mm) + ",0.))");
    const int poly = w.add("IFCPOLYLINE(" + spfRefList({p1, p2}) + ")");
    const int axis = w.add("IFCSHAPEREPRESENTATION($," + spfString("Axis") +
                           "," + spfString("Curve2D") + "," + spfRefList({poly}) +
                           ")");
    const int solid =
        w.add("IFCEXTRUDEDAREASOLID($,$,$," + spfReal(wall.height_mm) + ")");
    const int body = w.add("IFCSHAPEREPRESENTATION($," + spfString("Body") + "," +
                           spfString("SweptSolid") + "," + spfRefList({solid}) +
                           ")");
    const int pds =
        w.add("IFCPRODUCTDEFINITIONSHAPE($,$," + spfRefList({axis, body}) + ")");
    const int wall_id = w.add("IFCWALL(" + spfString(ctx.nextGuid()) + ",$," +
                              spfString("Wall") + ",$,$,$," + spfRef(pds) +
                              ",$,$)");
    const int layer =
        w.add("IFCMATERIALLAYER($," + spfReal(wall.thickness_mm) + ",$)");
    const int layerset = w.add("IFCMATERIALLAYERSET(" + spfRefList({layer}) +
                               "," + spfString("Standard") + ")");
    const int usage = w.add("IFCMATERIALLAYERSETUSAGE(" + spfRef(layerset) +
                            ",.AXIS2.,.POSITIVE.,0.)");
    w.add("IFCRELASSOCIATESMATERIAL(" + spfString(ctx.nextGuid()) + ",$,$,$," +
          spfRefList({wall_id}) + "," + spfRef(usage) + ")");
    w.add("IFCRELCONTAINEDINSPATIALSTRUCTURE(" + spfString(ctx.nextGuid()) +
          ",$,$,$," + spfRefList({wall_id}) + "," + spfRef(storey_ifc_id) + ")");
}

// Räumliche Komposition (IfcRelAggregates): Projekt → Gebäude → Geschosse.
void writeAggregation(ExportContext& ctx, const SpatialRoot& root,
                      const std::vector<int>& storey_ids) {
    ctx.writer.add("IFCRELAGGREGATES(" + spfString(ctx.nextGuid()) + ",$,$,$," +
                   spfRef(root.project_id) + "," + spfRefList({root.building_id}) +
                   ")");
    if (!storey_ids.empty()) {
        ctx.writer.add("IFCRELAGGREGATES(" + spfString(ctx.nextGuid()) +
                       ",$,$,$," + spfRef(root.building_id) + "," +
                       spfRefList(storey_ids) + ")");
    }
}

std::string buildIfc(const model::Building& building) {
    IfcSpfWriter writer;
    ExportContext ctx{writer};
    const int units = writeUnits(ctx);
    const int project = ctx.writer.add("IFCPROJECT(" + spfString(ctx.nextGuid()) +
                                       ",$," + spfString("b-cad") +
                                       ",$,$,$,$,$," + spfRef(units) + ")");
    const int building_id =
        ctx.writer.add("IFCBUILDING(" + spfString(ctx.nextGuid()) + ",$," +
                       spfString("Building") + ",$,$,$,$,$,$,$,$,$)");
    const StoreyExport storeys = writeStoreys(ctx, building);
    for (const model::Wall& wall : building.walls) {
        const auto it = storeys.by_storey.find(wall.storey_id);
        if (it != storeys.by_storey.end()) {
            writeWall(ctx, wall, it->second);
        }
        // Orphan-Wand (storey_id ohne Geschoss): übersprungen — tritt bei einem
        // vom Editor erzeugten Building nicht auf (defensiv).
    }
    writeAggregation(ctx, SpatialRoot{project, building_id}, storeys.ifc_ids);
    return writer.build();
}

// --- Atomares Schreiben (Temp + fsync + Rename) ---------------------------

// errno → Spec-Fehlercode: Zielpfad nicht beschreibbar → E-IO-001, sonst
// (Medium voll / IO) → E-IO-002. **Bewusste Asymmetrie zur Persistenz**
// (`sqlite_project_repository.cpp` mappt nur EACCES/EPERM/EROFS → E-IO-001):
// der Export scheitert in der **open-Phase** (EISDIR/ENOTDIR/ENOENT, z. B. Pfad
// ist ein Verzeichnis / fehlender Elternpfad) — das ist „Zielpfad nicht
// beschreibbar" (E-IO-001), nicht Medium-voll. NICHT auf den Persistenz-Satz
// „angleichen" (bräche den EISDIR-getriebenen E-IO-001-Negative-Test).
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
    throw std::runtime_error(code + ": IFC-Export " + what + " ('" +
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

void IfcExportAdapter::write(const model::Building& building,
                             const fs::path& path) const {
    const std::string ifc = buildIfc(building);  // vollständig im Speicher …
    atomicWrite(path, ifc);                       // … dann atomar schreiben
}

}  // namespace bcad::adapters::io
