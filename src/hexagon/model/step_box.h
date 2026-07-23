#pragma once

namespace bcad::hexagon::model {

// Achsen-paralleler Stufen-Quader (mm) — Extents einer soliden Stufe **ohne**
// Geländer. Pure Geometrie-Daten (kein OCC/Framework, header-only wie die
// übrigen `model/`-Werttypen). Kern-privat abgeleitet vom
// `services/geometry`-Treppen-Helfer (`stairStepBoxes`); über das
// `DerivedGeometry`-Bündel an den STEP-Export gereicht, damit der Adapter die
// Ableitung **nicht** selbst zieht (ADR-0020).
struct StepBox {
    double x0_mm{};
    double x1_mm{};
    double y0_mm{};
    double y1_mm{};
    double z0_mm{};
    double z1_mm{};
};

}  // namespace bcad::hexagon::model
