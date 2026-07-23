// PNG-Export-Mapping (ADR-0016 Option D, slice-025c). Projiziert den Grundriss
// (Reuse plan_geometry), rechnet die Fit-to-Canvas-Transformation (geguardet gegen
// degenerierte Bounding-Box), zeichnet je Geschoss die Wand-Achsen in der
// Geschoss-Farbe in eine feste Leinwand, encodiert via png_writer und schreibt
// atomar (E-IO-001, Reuse io_atomic_write). Export-only.

#include "adapters/io/png_export_adapter.h"

#include <array>
#include <cmath>
#include <cstddef>
#include <string>

#include "adapters/io/io_atomic_write.h"
#include "adapters/io/plan_geometry.h"
#include "adapters/io/png_writer.h"
#include "hexagon/model/building.h"

namespace fs = std::filesystem;
namespace model = bcad::hexagon::model;

namespace bcad::adapters::io {
namespace {

constexpr int kCanvasWidthPx = 800;
constexpr int kCanvasHeightPx = 600;
constexpr double kMarginPx = 30.0;
constexpr Rgb kBackground{255, 255, 255};

// Geschoss-Palette (zyklisch nach Geschoss-Index) — hält die „Wand-Achsen je
// Geschoss" im kombinierten Bild unterscheidbar.
constexpr std::array<Rgb, 4> kPalette = {{{0, 0, 0}, {0, 0, 200}, {200, 0, 0},
                                          {0, 150, 0}}};

// Fit-to-Canvas-Skala (Modell-mm → Pixel), **geguardet**: Breite ODER Höhe 0
// (einzelne vertikale/horizontale/Punkt-Wand, leeres Modell) → kein Div-durch-0.
double fitScale(const PlanView& view) {
    const double avail_w = kCanvasWidthPx - (2.0 * kMarginPx);
    const double avail_h = kCanvasHeightPx - (2.0 * kMarginPx);
    const double bbox_w = view.max_x_mm - view.min_x_mm;
    const double bbox_h = view.max_y_mm - view.min_y_mm;
    if (bbox_w > 0.0 && bbox_h > 0.0) {
        const double sx = avail_w / bbox_w;
        const double sy = avail_h / bbox_h;
        return sx < sy ? sx : sy;
    }
    if (bbox_w > 0.0) {
        return avail_w / bbox_w;
    }
    if (bbox_h > 0.0) {
        return avail_h / bbox_h;
    }
    return 1.0;  // leer / Punkt
}

}  // namespace

void PngExportAdapter::write(const model::Building& building,
                             const model::DerivedGeometry& /*derived*/,
                             const fs::path& path) const {
    const PlanView view = projectPlan(building);
    const double scale = fitScale(view);
    const double origin_x = (kCanvasWidthPx - ((view.max_x_mm - view.min_x_mm) * scale)) / 2.0;
    const double origin_y = (kCanvasHeightPx - ((view.max_y_mm - view.min_y_mm) * scale)) / 2.0;

    // Modell-mm → Pixel (y-Achse gespiegelt: Modell +Y erscheint oben).
    const auto to_px = [&](double x_mm) {
        return static_cast<int>(std::lround(origin_x + ((x_mm - view.min_x_mm) * scale)));
    };
    const auto to_py = [&](double y_mm) {
        return static_cast<int>(std::lround(
            kCanvasHeightPx - (origin_y + ((y_mm - view.min_y_mm) * scale))));
    };

    Bitmap bitmap({kCanvasWidthPx, kCanvasHeightPx}, kBackground);
    for (std::size_t i = 0; i < view.storeys.size(); ++i) {
        const Rgb color = kPalette.at(i % kPalette.size());
        for (const PlanSegment& seg : view.storeys[i].segments) {
            bitmap.drawLine({{to_px(seg.x1_mm), to_py(seg.y1_mm)},
                             {to_px(seg.x2_mm), to_py(seg.y2_mm)}},
                            color);
        }
    }

    writeFileAtomically(path, encodePng(bitmap), "PNG");  // atomar, E-IO-001
}

}  // namespace bcad::adapters::io
