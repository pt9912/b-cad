// Geteilte 2D-Plan-Projektion (slice-025b, ADR-0016). Rein/format-agnostisch:
// je Geschoss die Wand-Achsen als Segmente + gemeinsame Bounding-Box. Muster
// dxf_export_adapter (dieselbe Datenquelle: storeys + walls).

#include "adapters/io/plan_geometry.h"

#include <algorithm>
#include <unordered_set>

#include "hexagon/model/building.h"
#include "hexagon/model/guide_line.h"
#include "hexagon/model/layer.h"
#include "hexagon/model/storey.h"
#include "hexagon/model/wall.h"

namespace model = bcad::hexagon::model;

namespace bcad::adapters::io {
namespace {

// Nimmt ein Segment in die gemeinsame Bounding-Box auf (erste Geometrie
// initialisiert die Box). Eine Quelle — Wände UND sichtbare Hilfslinien
// erweitern dieselbe BBox, sonst würde eine Hilfslinie außerhalb der Wände
// in PDF/PNG abgeschnitten.
void accumulate(PlanView& view, const PlanSegment& seg) {
    if (!view.has_geometry) {
        view.min_x_mm = view.max_x_mm = seg.x1_mm;
        view.min_y_mm = view.max_y_mm = seg.y1_mm;
        view.has_geometry = true;
    }
    view.min_x_mm = std::min({view.min_x_mm, seg.x1_mm, seg.x2_mm});
    view.max_x_mm = std::max({view.max_x_mm, seg.x1_mm, seg.x2_mm});
    view.min_y_mm = std::min({view.min_y_mm, seg.y1_mm, seg.y2_mm});
    view.max_y_mm = std::max({view.max_y_mm, seg.y1_mm, seg.y2_mm});
}

}  // namespace

std::unordered_set<int> visibleLayerIds(const model::Building& building) {
    std::unordered_set<int> visible;
    for (const model::Layer& layer : building.layers) {
        if (layer.visible) {
            visible.insert(static_cast<int>(layer.id));
        }
    }
    return visible;
}

PlanView projectPlan(const model::Building& building) {
    PlanView view;
    view.storeys.reserve(building.storeys.size());
    const std::unordered_set<int> visible = visibleLayerIds(building);

    for (const model::Storey& storey : building.storeys) {
        StoreyPlan plan;
        plan.storey_id = static_cast<int>(storey.id);
        for (const model::Wall& wall : building.walls) {
            if (wall.storey_id != storey.id) {
                continue;
            }
            const PlanSegment seg{wall.start.x_mm, wall.start.y_mm,
                                  wall.end.x_mm, wall.end.y_mm};
            accumulate(view, seg);
            plan.segments.push_back(seg);
        }
        // Hilfslinien auf SICHTBARER Ebene (LH-FA-DRW-005, ADR-0018): als
        // schlichtes 2D-Segment in denselben Grundriss + dieselbe BBox.
        // Unsichtbare Ebene → übersprungen (Export-Filter, DRW-005-Negative).
        for (const model::GuideLine& guide : building.guide_lines) {
            if (guide.storey_id != storey.id) {
                continue;
            }
            if (!visible.contains(static_cast<int>(guide.layer_id))) {
                continue;
            }
            const PlanSegment seg{
                guide.segment.start.x_mm, guide.segment.start.y_mm,
                guide.segment.end.x_mm, guide.segment.end.y_mm};
            accumulate(view, seg);
            plan.segments.push_back(seg);
        }
        view.storeys.push_back(std::move(plan));
    }
    return view;
}

}  // namespace bcad::adapters::io
