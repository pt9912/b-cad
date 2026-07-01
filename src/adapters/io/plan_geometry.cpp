// Geteilte 2D-Plan-Projektion (slice-025b, ADR-0016). Rein/format-agnostisch:
// je Geschoss die Wand-Achsen als Segmente + gemeinsame Bounding-Box. Muster
// dxf_export_adapter (dieselbe Datenquelle: storeys + walls).

#include "adapters/io/plan_geometry.h"

#include <algorithm>

#include "hexagon/model/building.h"
#include "hexagon/model/storey.h"
#include "hexagon/model/wall.h"

namespace model = bcad::hexagon::model;

namespace bcad::adapters::io {

PlanView projectPlan(const model::Building& building) {
    PlanView view;
    view.storeys.reserve(building.storeys.size());

    for (const model::Storey& storey : building.storeys) {
        StoreyPlan plan;
        plan.storey_id = static_cast<int>(storey.id);
        for (const model::Wall& wall : building.walls) {
            if (wall.storey_id != storey.id) {
                continue;
            }
            const PlanSegment seg{wall.start.x_mm, wall.start.y_mm,
                                  wall.end.x_mm, wall.end.y_mm};
            if (!view.has_geometry) {
                view.min_x_mm = view.max_x_mm = seg.x1_mm;
                view.min_y_mm = view.max_y_mm = seg.y1_mm;
                view.has_geometry = true;
            }
            view.min_x_mm = std::min({view.min_x_mm, seg.x1_mm, seg.x2_mm});
            view.max_x_mm = std::max({view.max_x_mm, seg.x1_mm, seg.x2_mm});
            view.min_y_mm = std::min({view.min_y_mm, seg.y1_mm, seg.y2_mm});
            view.max_y_mm = std::max({view.max_y_mm, seg.y1_mm, seg.y2_mm});
            plan.segments.push_back(seg);
        }
        view.storeys.push_back(std::move(plan));
    }
    return view;
}

}  // namespace bcad::adapters::io
