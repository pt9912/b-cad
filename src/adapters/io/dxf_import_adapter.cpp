// DXF-Import-Mapping (ADR-0015 Option D). Liest das welle-4-Subset (spez. §1
// LH-FA-IO-003.a) über den generischen `DxfReader` und bildet es **anzahl-treu**,
// **atomar** und **total** auf ein `model::Building` ab.
//
// Subset-Grenzen (benannte Lücken, spez. §1 / Closure):
//   - DXF trägt keine Höhe/Dicke -> Default-Werte (`kDefault*`), Muster der
//     IFC-Import-Geschoss-Höhe (slice-019b).
//   - Ein b-cad-Geschoss je distinkter DXF-`LAYER` (Gruppencode 8 der LINE), in
//     Erst-Erscheinungs-Reihenfolge. Layer ohne Wand (leeres Geschoss) tauchen
//     nicht auf -> gehen im Roundtrip verloren (benannte Lücke).
//   - Nur `LINE` + `LWPOLYLINE`; weitere Entitäten (DIMENSION/HATCH/BLOCK/…)
//     werden übersprungen (Subset-Skip). `Wall.type` -> Default `Innen`,
//     `material_id` bleibt `nullopt` (keine DXF-Entsprechung).

#include "adapters/io/dxf_import_adapter.h"

#include <algorithm>
#include <cctype>
#include <cstddef>
#include <fstream>
#include <map>
#include <optional>
#include <sstream>
#include <stdexcept>
#include <string>
#include <vector>

#include "adapters/io/dxf_reader.h"
#include "hexagon/model/constants.h"
#include "hexagon/model/point2d.h"
#include "hexagon/model/storey.h"
#include "hexagon/model/wall.h"
#include "hexagon/model/wall_type.h"

namespace fs = std::filesystem;
namespace model = bcad::hexagon::model;

namespace bcad::adapters::io {
namespace {

// DXF-Gruppencodes des 2D-Linien-Subsets.
constexpr int kLayer = 8;
constexpr int kStartX = 10;
constexpr int kStartY = 20;
constexpr int kEndX = 11;
constexpr int kEndY = 21;

// Gebündelte Achse — vermeidet zwei benachbarte `Point2D`-Parameter
// (bugprone-easily-swappable-parameters), Muster IFC-Import.
struct Axis {
    model::Point2D start;
    model::Point2D end;
};

bool isBlank(const std::string& text) {
    return std::all_of(text.begin(), text.end(), [](char c) {
        return std::isspace(static_cast<unsigned char>(c)) != 0;
    });
}

std::optional<Axis> lineAxis(const DxfEntity& line) {
    const auto x1 = line.num(kStartX);
    const auto y1 = line.num(kStartY);
    const auto x2 = line.num(kEndX);
    const auto y2 = line.num(kEndY);
    if (!x1 || !y1 || !x2 || !y2) {
        return std::nullopt;
    }
    return Axis{model::Point2D{*x1, *y1}, model::Point2D{*x2, *y2}};
}

// Ein b-cad-Geschoss je distinkter DXF-LAYER, in Erst-Erscheinungs-Reihenfolge.
class StoreyTable {
public:
    model::StoreyId storeyFor(const std::string& layer) {
        const auto it = by_layer_.find(layer);
        if (it != by_layer_.end()) {
            return it->second;
        }
        const model::StoreyId sid{static_cast<int>(storeys_.size()) + 1};
        storeys_.push_back(model::Storey{sid, model::kDefaultStoreyHeightMm});
        by_layer_.emplace(layer, sid);
        return sid;
    }
    std::vector<model::Storey> take() { return std::move(storeys_); }

private:
    std::vector<model::Storey> storeys_;
    std::map<std::string, model::StoreyId> by_layer_;
};

model::Wall makeWall(int id, model::StoreyId storey, const Axis& axis) {
    model::Wall wall;
    wall.id = model::WallId{id};
    wall.storey_id = storey;
    wall.start = axis.start;
    wall.end = axis.end;
    wall.thickness_mm = model::kDefaultWallThicknessMm;  // DXF trägt keine Dicke
    wall.height_mm = model::kDefaultStoreyHeightMm;      // DXF trägt keine Höhe
    wall.type = model::WallType::Innen;
    return wall;
}

model::Building mapToBuilding(const DxfReader& reader) {
    model::Building building;
    StoreyTable storeys;
    int next_wall = 1;
    for (const DxfEntity* entity : reader.sectionEntities("ENTITIES")) {
        const std::string layer = entity->str(kLayer).value_or("0");
        if (entity->type == "LINE") {
            if (const auto axis = lineAxis(*entity)) {
                building.walls.push_back(
                    makeWall(next_wall++, storeys.storeyFor(layer), *axis));
            }
        } else if (entity->type == "LWPOLYLINE") {
            const auto pts = entity->points();
            for (std::size_t i = 0; i + 1 < pts.size(); ++i) {
                const Axis axis{
                    model::Point2D{pts[i].first, pts[i].second},
                    model::Point2D{pts[i + 1].first, pts[i + 1].second}};
                building.walls.push_back(
                    makeWall(next_wall++, storeys.storeyFor(layer), axis));
            }
        }
        // Andere Entitäten (DIMENSION/HATCH/BLOCK/ARC/CIRCLE/…) = Subset-Skip.
    }
    building.storeys = storeys.take();
    return building;
}

}  // namespace

model::Building DxfImportAdapter::read(const fs::path& path) const {
    std::ifstream in(path, std::ios::binary);
    if (!in) {
        throw std::runtime_error("E-IO-003: DXF-Datei nicht lesbar ('" +
                                 path.string() + "'); event=import_rejected");
    }
    std::ostringstream buffer;
    buffer << in.rdbuf();
    const std::string content = buffer.str();

    // Totalität (spez. §1): leere/whitespace-only Datei -> leeres Modell.
    if (isBlank(content)) {
        return model::Building{};
    }
    try {
        const DxfReader reader = DxfReader::parse(content);
        return mapToBuilding(reader);
    } catch (const std::exception& e) {
        // Nicht wohlgeformter DXF-Strom -> E-IO-003, kein Teil-Import
        // (LH-FA-IO-003 Negative, ADR-0015 #4).
        throw std::runtime_error("E-IO-003: DXF-Import abgelehnt ('" +
                                 path.string() + "'): " + e.what() +
                                 "; event=import_rejected");
    }
}

}  // namespace bcad::adapters::io
