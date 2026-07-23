// 2D-Grundriss-Projektion (ADR-0016, slice-025b; nach ADR-0019/ADR-0020 in den
// Kern gehoben). Rein/format-agnostisch: je Geschoss die Wand-Achsen + sichtbare
// Hilfslinien als Segmente + gemeinsame Bounding-Box. Reiner Umzug aus dem
// früheren io-`plan_geometry` (Logik unverändert).

#include "hexagon/services/geometry/plan_projection.h"

#include <algorithm>
#include <unordered_set>
#include <utility>

#include "hexagon/model/guide_line.h"
#include "hexagon/model/layer_visibility.h"
#include "hexagon/model/storey.h"
#include "hexagon/model/wall.h"

namespace bcad::hexagon::services {
namespace {

// Nimmt ein Segment in die gemeinsame Bounding-Box auf (erste Geometrie
// initialisiert die Box). Eine Quelle — Wände UND sichtbare Hilfslinien
// erweitern dieselbe BBox, sonst würde eine Hilfslinie außerhalb der Wände
// in PDF/PNG abgeschnitten.
void accumulate(model::PlanView& view, const model::PlanSegment& seg) {
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

model::PlanView projectPlan(const model::Building& building) {
    model::PlanView view;
    view.storeys.reserve(building.storeys.size());
    const std::unordered_set<int> visible = model::visibleLayerIds(building);

    for (const model::Storey& storey : building.storeys) {
        model::StoreyPlan plan;
        plan.storey_id = static_cast<int>(storey.id);
        for (const model::Wall& wall : building.walls) {
            if (wall.storey_id != storey.id) {
                continue;
            }
            const model::PlanSegment seg{wall.start.x_mm, wall.start.y_mm,
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
            const model::PlanSegment seg{
                guide.segment.start.x_mm, guide.segment.start.y_mm,
                guide.segment.end.x_mm, guide.segment.end.y_mm};
            accumulate(view, seg);
            plan.segments.push_back(seg);
        }
        view.storeys.push_back(std::move(plan));
    }
    return view;
}

}  // namespace bcad::hexagon::services
