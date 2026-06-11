#pragma once

// Gemeinsames, deterministisches Geometrie-Double für Kern-Tests
// (ADR-0001 §Testbarkeit) — vorher dreifach dupliziert (Review L4).
// Volumen einer extrudierten Wand analytisch: Länge · Stärke · Höhe.
// Der echte OCC-Adapter (slice-003b) wird gegen denselben analytischen
// Wert (innerhalb der Toleranz) geprüft.

#include <cmath>

#include "hexagon/model/solid.h"
#include "hexagon/model/wall.h"
#include "hexagon/ports/driven/geometry_kernel_port.h"

namespace bcad::testing {

inline double analyticVolume(const hexagon::model::Wall& w) {
    const double dx = w.end.x_mm - w.start.x_mm;
    const double dy = w.end.y_mm - w.start.y_mm;
    const double length = std::sqrt((dx * dx) + (dy * dy));
    return length * w.thickness_mm * w.height_mm;
}

class AnalyticGeometry final : public hexagon::ports::driven::GeometryKernelPort {
public:
    hexagon::model::Solid extrudeWall(const hexagon::model::Wall& w) const override {
        return hexagon::model::Solid{analyticVolume(w)};
    }
};

}  // namespace bcad::testing
