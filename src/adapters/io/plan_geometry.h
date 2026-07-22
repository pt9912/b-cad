#pragma once

#include <unordered_set>
#include <vector>

#include "hexagon/model/building.h"

namespace bcad::adapters::io {

// Geteilte, format-agnostische 2D-Plan-Projektion (slice-025b, ADR-0016;
// io-resident, kein OCC/Qt). Reduziert ein `Building` auf die **2D-Grundriss-
// Sicht**: je Geschoss die geraden **Wand-Achsen** als Segmente (Modell-mm) +
// eine gemeinsame Bounding-Box über alle Segmente. Datenquelle wie der
// DXF-Export (`building.storeys` + `building.walls`). `model::Wall` ist ein
// Einzel-Segment → „gerade Wand-Achsen" = **alle** Wände (kein Filter).
//
// Von PDF (025b) und PNG (025c) gemeinsam genutzt: der jeweilige Writer bildet
// Modell-mm über seinen Maßstab in Seiten-/Pixel-Einheiten ab. Die gemeinsame
// Bounding-Box hält alle Geschoss-Seiten im selben Koordinatenrahmen.

// Ein Achs-Segment in Modell-Millimetern.
struct PlanSegment {
    double x1_mm{};
    double y1_mm{};
    double x2_mm{};
    double y2_mm{};
};

// Die Achsen eines Geschosses (Geschoss-Reihenfolge des Modells).
struct StoreyPlan {
    int storey_id{};
    std::vector<PlanSegment> segments;
};

// Der projizierte Grundriss: je Geschoss ein `StoreyPlan` (in Modell-
// Reihenfolge; ein Geschoss ohne Wände trägt eine leere Segment-Liste) + die
// gemeinsame Bounding-Box. `has_geometry == false` ⇔ kein Segment (leeres Modell)
// → die Box ist dann degeneriert (0,0,0,0) und der Aufrufer zeichnet eine leere
// Seite (Totalität).
struct PlanView {
    std::vector<StoreyPlan> storeys;
    double min_x_mm{};
    double min_y_mm{};
    double max_x_mm{};
    double max_y_mm{};
    bool has_geometry{false};
};

// Roh-Ids der SICHTBAREN Ebenen (`layer.visible`, LH-FA-DRW-006). Geteilte,
// format-agnostische Quelle des Export-Sichtbarkeits-Filters (ADR-0018 §3:
// unsichtbare Ebene → ihre Hilfslinien werden nicht gezeichnet) — von `projectPlan`
// (PDF/PNG) UND dem DXF-Export genutzt, damit alle Formate identisch filtern.
std::unordered_set<int> visibleLayerIds(const hexagon::model::Building& building);

// Projiziert `building` auf die 2D-Grundriss-Sicht (deterministisch aus der
// Modell-Reihenfolge). Rein — keine I/O, kein Format. Enthält je Geschoss die
// Wand-Achsen + die Hilfslinien auf sichtbarer Ebene (LH-FA-DRW-005, ADR-0018)
// als schlichte 2D-Segmente; die gemeinsame BBox schließt beide ein.
PlanView projectPlan(const hexagon::model::Building& building);

}  // namespace bcad::adapters::io
