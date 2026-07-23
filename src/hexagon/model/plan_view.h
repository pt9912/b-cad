#pragma once

#include <vector>

namespace bcad::hexagon::model {

// 2D-Grundriss-Sicht eines `Building` (ADR-0016, slice-025b; ADR-0019/ADR-0020:
// die reine 2D-Projektion ist Kern-Rechnung, ihre Werttypen leben im `model/`-
// Kern — so darf das kern-berechnete `DerivedGeometry`-Bündel sie tragen und der
// Kern-Read-Port `PlanViewPort` sie liefern, ohne dass ein Adapter die Projektion
// selbst ableitet). Reine Werttypen (kein OCC/Qt); Längeneinheit mm.

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

// Der projizierte Grundriss: je Geschoss ein `StoreyPlan` (in Modell-Reihenfolge;
// ein Geschoss ohne Wände trägt eine leere Segment-Liste) + die gemeinsame
// Bounding-Box. `has_geometry == false` ⇔ kein Segment (leeres Modell) → die Box
// ist dann degeneriert (0,0,0,0) und der Aufrufer zeichnet eine leere Seite
// (Totalität).
struct PlanView {
    std::vector<StoreyPlan> storeys;
    double min_x_mm{};
    double min_y_mm{};
    double max_x_mm{};
    double max_y_mm{};
    bool has_geometry{false};
};

}  // namespace bcad::hexagon::model
