// PNG-Export-Mapping (ADR-0016 Option D, slice-025c). Nutzt die kern-gelieferte
// 2D-`PlanView` aus dem `DerivedGeometry`-Bündel (seit slice-042b, ADR-0019/0020),
// rechnet die Fit-to-Canvas-Transformation (geguardet gegen
// degenerierte Bounding-Box), zeichnet je Geschoss die Wand-Achsen in der
// Geschoss-Farbe in eine feste Leinwand, encodiert via png_writer und schreibt
// atomar (E-IO-001, Reuse io_atomic_write). Export-only.

#include "adapters/io/png_export_adapter.h"

#include <array>
#include <cmath>
#include <cstddef>
#include <string>
#include <utility>
#include <vector>

#include "adapters/io/io_atomic_write.h"
#include "adapters/io/png_writer.h"
#include "hexagon/model/building.h"
#include "hexagon/model/plan_view.h"

namespace fs = std::filesystem;
namespace model = bcad::hexagon::model;

namespace bcad::adapters::io {

// ADR-0019/ADR-0020: die 2D-Projektions-Werttypen liegen im `model/`-Kern; der
// PNG-Adapter serialisiert die kern-gelieferte `PlanView` aus dem Bündel.
using model::PlanSegment;
using model::PlanView;

namespace {

constexpr int kCanvasWidthPx = 800;
constexpr int kCanvasHeightPx = 600;
constexpr double kMarginPx = 30.0;
constexpr Rgb kBackground{255, 255, 255};

// Geschoss-Palette (zyklisch nach Geschoss-Index) — hält die „Wand-Achsen je
// Geschoss" im kombinierten Bild unterscheidbar.
constexpr std::array<Rgb, 4> kPalette = {{{0, 0, 0}, {0, 0, 200}, {200, 0, 0},
                                          {0, 150, 0}}};

// Sichtbarer Provenance-Titelblock (slice-046b): schwarze Fußzeile im **weißen
// Unterrand** — die Geometrie liegt innerhalb `kMarginPx`, das untere Band ist frei
// (kein Überlappen mit Wand-Tinte → die Ink-Sonde ist streng).
constexpr int kFooterX = 6;
constexpr int kFooterY = kCanvasHeightPx - 15;
constexpr Rgb kFooterColor{0, 0, 0};

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

void PngExportAdapter::write(const model::Building& /*building*/,
                             const model::DerivedGeometry& derived,
                             const fs::path& path,
                             const model::ExportProvenance& provenance) const {
    // ADR-0020: der Kern liefert die 2D-Projektion im Bündel; der Adapter
    // serialisiert nur (kein Eigen-`projectPlan` mehr).
    const PlanView& view = derived.plan;
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

    // slice-046b: sichtbare Provenance-Fußzeile im weißen Unterrand (leer →
    // keine Zeile, Sentinel-Fall → PNG byte-gleich zu vorher). Geteilte Zeichenkette
    // mit dem PDF-Footer (`ExportProvenance::footerLine`, kein Drift).
    const std::string footer = provenance.footerLine();
    if (!footer.empty()) {
        bitmap.drawText({kFooterX, kFooterY}, footer, kFooterColor);
    }

    // Injizierte tEXt-Metadaten (nur nicht-leere) — neben den statischen
    // Software/Title; der Adapter komponiert, der Writer bleibt domänen-frei.
    std::vector<std::pair<std::string, std::string>> texts;
    if (!provenance.date.empty()) {
        texts.emplace_back("Date", provenance.date);
    }
    if (!provenance.source.empty()) {
        texts.emplace_back("Source", provenance.source);
    }
    if (!provenance.version.empty()) {
        texts.emplace_back("Version", provenance.version);
    }

    writeFileAtomically(path, encodePng(bitmap, texts), "PNG");  // atomar, E-IO-001
}

}  // namespace bcad::adapters::io
