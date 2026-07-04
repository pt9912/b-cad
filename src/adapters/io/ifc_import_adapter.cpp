// IFC-Import-Mapping (ADR-0013 Option D). Liest das welle-4-Entitäts-Subset
// (spez. §1 LH-FA-IO-001.a) über den generischen `SpfReader` und bildet es
// **anzahl-treu**, **atomar** und **total** auf ein `model::Building` ab.
//
// Subset-Grenzen (benannte Lücken, spez. §1 / Closure):
//   - Placement-Komposition wird NICHT aufgelöst — die Achs-Polyline wird in
//     ihren gelesenen (Geschoss-/Identity-)Koordinaten übernommen; der b-cad-
//     Exporter (slice-019c) schreibt entsprechend.
//   - `IfcRelAggregates` (Spatial-Komposition) wird nicht traversiert:
//     Geschosse werden direkt über `IfcBuildingStorey` enumeriert (anzahl-treu),
//     die Wand->Geschoss-Bindung über `IfcRelContainedInSpatialStructure`.
//   - `Wall.type` -> Default `Innen` (keine IFC-Entsprechung im Subset);
//     `material_id` bleibt `nullopt`.

#include "adapters/io/ifc_import_adapter.h"

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

#include "adapters/io/ifc_spf_reader.h"
#include "hexagon/model/constants.h"
#include "hexagon/model/point2d.h"
#include "hexagon/model/storey.h"
#include "hexagon/model/wall.h"
#include "hexagon/model/wall_type.h"

namespace fs = std::filesystem;
namespace model = bcad::hexagon::model;

namespace bcad::adapters::io {
namespace {

// IFC-Attribut-Positionen (0-basiert, IFC4 == IFC2x3 für den Subset).
constexpr std::size_t kStoreyElevationAttr = 9;
constexpr std::size_t kWallRepresentationAttr = 6;
constexpr std::size_t kProductDefRepresentationsAttr = 2;
constexpr std::size_t kShapeRepIdentifierAttr = 1;
constexpr std::size_t kShapeRepItemsAttr = 3;
constexpr std::size_t kPolylinePointsAttr = 0;
constexpr std::size_t kCartesianCoordsAttr = 0;
constexpr std::size_t kExtrudedDepthAttr = 3;
constexpr std::size_t kRelContainedRelatedAttr = 4;
constexpr std::size_t kRelContainedStructureAttr = 5;
constexpr std::size_t kRelAssocRelatedAttr = 4;
constexpr std::size_t kRelAssocMaterialAttr = 5;
constexpr std::size_t kLayerSetUsageForSetAttr = 0;
constexpr std::size_t kLayerSetLayersAttr = 0;
constexpr std::size_t kLayerThicknessAttr = 1;

struct Axis {
    model::Point2D start;
    model::Point2D end;
};

struct StoreyMapping {
    std::vector<model::Storey> storeys;
    std::map<int, model::StoreyId> by_ifc_id;  // IFC-#id -> b-cad-StoreyId
};

bool isBlank(const std::string& text) {
    return std::all_of(text.begin(), text.end(), [](char c) {
        return std::isspace(static_cast<unsigned char>(c)) != 0;
    });
}

bool listContainsRef(const SpfValue* list, int id) {
    if (list == nullptr || list->kind != SpfValue::Kind::List) {
        return false;
    }
    return std::any_of(list->items.begin(), list->items.end(),
                       [id](const SpfValue& item) { return asRef(&item) == id; });
}

// --- Geschoss-Mapping: ein b-cad-Geschoss je IfcBuildingStorey ------------

StoreyMapping mapStoreys(const SpfReader& reader) {
    struct Item {
        int ifc_id{0};
        double elevation{0.0};
    };
    std::vector<Item> items;
    for (const SpfEntity* storey : reader.byKeyword("IFCBUILDINGSTOREY")) {
        const double elevation =
            asNumber(attributeAt(*storey, kStoreyElevationAttr)).value_or(0.0);
        items.push_back({storey->id, elevation});
    }
    // Deterministisch: aufsteigend nach Elevation, dann #id (Tiebreak).
    std::sort(items.begin(), items.end(), [](const Item& a, const Item& b) {
        if (a.elevation < b.elevation) {
            return true;
        }
        if (b.elevation < a.elevation) {
            return false;
        }
        return a.ifc_id < b.ifc_id;
    });

    StoreyMapping out;
    for (std::size_t i = 0; i < items.size(); ++i) {
        const model::StoreyId sid{static_cast<int>(i) + 1};
        // Geschoss-Höhe = Differenz zur nächsthöheren Elevation; oberstes
        // Geschoss -> Default (Elevation bleibt transient, spez. §1-Zusatz).
        double height = model::kDefaultStoreyHeightMm;
        if (i + 1 < items.size()) {
            const double diff = items[i + 1].elevation - items[i].elevation;
            if (diff > model::kGeometryToleranceMm) {
                height = diff;
            }
        }
        out.storeys.push_back(model::Storey{sid, height});
        out.by_ifc_id[items[i].ifc_id] = sid;
    }
    return out;
}

// --- Wand-Mapping: Achse, Dicke, Höhe, Geschoss-Verortung -----------------

std::optional<model::Point2D> cartesianXY(const SpfReader& reader, int point_id) {
    const SpfEntity* point = reader.byId(point_id);
    if (point == nullptr || point->keyword != "IFCCARTESIANPOINT") {
        return std::nullopt;
    }
    const SpfValue* coords = attributeAt(*point, kCartesianCoordsAttr);
    if (coords == nullptr || coords->kind != SpfValue::Kind::List ||
        coords->items.size() < 2) {
        return std::nullopt;
    }
    const auto x = asNumber(coords->items.data());
    const auto y = asNumber(&coords->items[1]);
    if (!x || !y) {
        return std::nullopt;
    }
    return model::Point2D{*x, *y};
}

// Findet die ShapeRepresentation einer Wand mit gegebenem
// RepresentationIdentifier ('Axis' / 'Body').
const SpfEntity* findShapeRep(const SpfReader& reader, const SpfEntity& wall,
                              const std::string& identifier) {
    const auto rep_ref = asRef(attributeAt(wall, kWallRepresentationAttr));
    if (!rep_ref) {
        return nullptr;
    }
    const SpfEntity* shape = reader.byId(*rep_ref);
    if (shape == nullptr || shape->keyword != "IFCPRODUCTDEFINITIONSHAPE") {
        return nullptr;
    }
    const SpfValue* reps = attributeAt(*shape, kProductDefRepresentationsAttr);
    if (reps == nullptr || reps->kind != SpfValue::Kind::List) {
        return nullptr;
    }
    for (const SpfValue& item : reps->items) {
        const auto rid = asRef(&item);
        if (!rid) {
            continue;
        }
        const SpfEntity* sr = reader.byId(*rid);
        if (sr != nullptr && sr->keyword == "IFCSHAPEREPRESENTATION" &&
            asString(attributeAt(*sr, kShapeRepIdentifierAttr)) == identifier) {
            return sr;
        }
    }
    return nullptr;
}

std::optional<Axis> polylineEndpoints(const SpfReader& reader,
                                      const SpfEntity& polyline) {
    const SpfValue* points = attributeAt(polyline, kPolylinePointsAttr);
    if (points == nullptr || points->kind != SpfValue::Kind::List ||
        points->items.size() < 2) {
        return std::nullopt;
    }
    const auto first = asRef(&points->items.front());
    const auto last = asRef(&points->items.back());
    if (!first || !last) {
        return std::nullopt;
    }
    const auto start = cartesianXY(reader, *first);
    const auto end = cartesianXY(reader, *last);
    if (!start || !end) {
        return std::nullopt;
    }
    return Axis{*start, *end};
}

std::optional<Axis> resolveWallAxis(const SpfReader& reader,
                                    const SpfEntity& wall) {
    const SpfEntity* axis_rep = findShapeRep(reader, wall, "Axis");
    if (axis_rep == nullptr) {
        return std::nullopt;
    }
    const SpfValue* items = attributeAt(*axis_rep, kShapeRepItemsAttr);
    if (items == nullptr || items->kind != SpfValue::Kind::List) {
        return std::nullopt;
    }
    for (const SpfValue& item : items->items) {
        const auto ref = asRef(&item);
        if (!ref) {
            continue;
        }
        const SpfEntity* poly = reader.byId(*ref);
        if (poly != nullptr && poly->keyword == "IFCPOLYLINE") {
            if (auto axis = polylineEndpoints(reader, *poly)) {
                return axis;
            }
        }
    }
    return std::nullopt;
}

model::StoreyId resolveWallStorey(
    const SpfReader& reader, int wall_id,
    const std::map<int, model::StoreyId>& by_ifc) {
    for (const SpfEntity* rel :
         reader.byKeyword("IFCRELCONTAINEDINSPATIALSTRUCTURE")) {
        if (!listContainsRef(attributeAt(*rel, kRelContainedRelatedAttr),
                             wall_id)) {
            continue;
        }
        const auto structure = asRef(attributeAt(*rel, kRelContainedStructureAttr));
        if (!structure) {
            continue;
        }
        const auto it = by_ifc.find(*structure);
        if (it != by_ifc.end()) {
            return it->second;
        }
    }
    throw std::runtime_error("Wand #" + std::to_string(wall_id) +
                             " ohne Geschoss-Verortung "
                             "(IfcRelContainedInSpatialStructure)");
}

std::optional<double> layerSetThickness(const SpfReader& reader, int set_id) {
    const SpfEntity* set = reader.byId(set_id);
    if (set == nullptr || set->keyword != "IFCMATERIALLAYERSET") {
        return std::nullopt;
    }
    const SpfValue* layers = attributeAt(*set, kLayerSetLayersAttr);
    if (layers == nullptr || layers->kind != SpfValue::Kind::List) {
        return std::nullopt;
    }
    double total = 0.0;
    bool any = false;
    for (const SpfValue& item : layers->items) {
        const auto ref = asRef(&item);
        if (!ref) {
            continue;
        }
        const SpfEntity* layer = reader.byId(*ref);
        if (layer == nullptr || layer->keyword != "IFCMATERIALLAYER") {
            continue;
        }
        if (const auto thickness = asNumber(attributeAt(*layer, kLayerThicknessAttr))) {
            total += *thickness;
            any = true;
        }
    }
    return any ? std::optional<double>(total) : std::nullopt;
}

const SpfEntity* materialForWall(const SpfReader& reader, int wall_id) {
    for (const SpfEntity* rel : reader.byKeyword("IFCRELASSOCIATESMATERIAL")) {
        if (!listContainsRef(attributeAt(*rel, kRelAssocRelatedAttr), wall_id)) {
            continue;
        }
        if (const auto mat = asRef(attributeAt(*rel, kRelAssocMaterialAttr))) {
            return reader.byId(*mat);
        }
    }
    return nullptr;
}

double resolveWallThickness(const SpfReader& reader, int wall_id) {
    const SpfEntity* material = materialForWall(reader, wall_id);
    if (material == nullptr) {
        return model::kDefaultWallThicknessMm;
    }
    int set_id = 0;
    if (material->keyword == "IFCMATERIALLAYERSETUSAGE") {
        set_id = asRef(attributeAt(*material, kLayerSetUsageForSetAttr)).value_or(0);
    } else if (material->keyword == "IFCMATERIALLAYERSET") {
        set_id = material->id;
    }
    if (set_id != 0) {
        if (const auto thickness = layerSetThickness(reader, set_id)) {
            return *thickness;
        }
    }
    return model::kDefaultWallThicknessMm;
}

double resolveWallHeight(const SpfReader& reader, const SpfEntity& wall) {
    const SpfEntity* body = findShapeRep(reader, wall, "Body");
    if (body == nullptr) {
        return model::kDefaultStoreyHeightMm;
    }
    const SpfValue* items = attributeAt(*body, kShapeRepItemsAttr);
    if (items == nullptr || items->kind != SpfValue::Kind::List) {
        return model::kDefaultStoreyHeightMm;
    }
    for (const SpfValue& item : items->items) {
        const auto ref = asRef(&item);
        if (!ref) {
            continue;
        }
        const SpfEntity* solid = reader.byId(*ref);
        if (solid == nullptr || solid->keyword != "IFCEXTRUDEDAREASOLID") {
            continue;
        }
        const auto depth = asNumber(attributeAt(*solid, kExtrudedDepthAttr));
        if (depth && *depth > 0.0) {
            return *depth;
        }
    }
    return model::kDefaultStoreyHeightMm;
}

std::vector<model::Wall> mapWalls(
    const SpfReader& reader,
    const std::map<int, model::StoreyId>& by_ifc) {
    std::vector<const SpfEntity*> ifc_walls = reader.byKeyword("IFCWALL");
    for (const SpfEntity* wall : reader.byKeyword("IFCWALLSTANDARDCASE")) {
        ifc_walls.push_back(wall);
    }
    // Deterministisch nach #id (Quell-Reihenfolge), unabhängig vom Keyword.
    std::sort(ifc_walls.begin(), ifc_walls.end(),
              [](const SpfEntity* a, const SpfEntity* b) { return a->id < b->id; });

    std::vector<model::Wall> walls;
    int next_id = 1;
    for (const SpfEntity* source : ifc_walls) {
        const auto axis = resolveWallAxis(reader, *source);
        if (!axis) {
            throw std::runtime_error(
                "Wand #" + std::to_string(source->id) +
                " ohne Achs-Repräsentation (IfcShapeRepresentation 'Axis')");
        }
        model::Wall wall;
        wall.id = model::WallId{next_id};
        ++next_id;
        wall.storey_id = resolveWallStorey(reader, source->id, by_ifc);
        wall.start = axis->start;
        wall.end = axis->end;
        wall.thickness_mm = resolveWallThickness(reader, source->id);
        wall.height_mm = resolveWallHeight(reader, *source);
        wall.type = model::WallType::Innen;  // benannte Lücke (kein IFC-Typ)
        walls.push_back(wall);
    }
    return walls;
}

model::Building mapToBuilding(const SpfReader& reader) {
    const StoreyMapping storeys = mapStoreys(reader);
    model::Building building;
    building.storeys = storeys.storeys;
    building.walls = mapWalls(reader, storeys.by_ifc_id);
    return building;
}

}  // namespace

model::Building IfcImportAdapter::read(const fs::path& path) const {
    const std::ifstream in(path, std::ios::binary);
    if (!in) {
        throw std::runtime_error("E-IO-003: IFC-Datei nicht lesbar ('" +
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
        const SpfReader reader = SpfReader::parse(content);
        return mapToBuilding(reader);
    } catch (const std::exception& e) {
        // Parse-/Format-Fehler oder fehlende tragende Pflicht-Referenz ->
        // E-IO-003, kein Teil-Import (LH-FA-IO-001 Negative, ADR-0013 #3).
        throw std::runtime_error("E-IO-003: IFC-Import abgelehnt ('" +
                                 path.string() + "'): " + e.what() +
                                 "; event=import_rejected");
    }
}

}  // namespace bcad::adapters::io
