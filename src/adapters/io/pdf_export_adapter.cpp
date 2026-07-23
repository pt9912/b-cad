// PDF-Export-Mapping (ADR-0016 Option D, slice-025b). Bildet das welle-4-Subset
// (gerade Wand-Achsen je Geschoss, spez. §1 LH-FA-IO-007.a) auf einen
// maßstäblichen 2D-Grundriss ab: je Geschoss eine A4-Seite (Rahmen + Achsen bei
// festem Maßstab 1:100 + "M 1:100"-Label), serialisiert über den PdfWriter,
// atomar geschrieben (E-IO-001). Export-only.

#include "adapters/io/pdf_export_adapter.h"

#include <string>

#include "adapters/io/io_atomic_write.h"
#include "adapters/io/pdf_writer.h"
#include "hexagon/model/building.h"
#include "hexagon/model/plan_view.h"

namespace fs = std::filesystem;
namespace model = bcad::hexagon::model;

namespace bcad::adapters::io {

// ADR-0019/ADR-0020: die 2D-Projektions-Werttypen liegen im `model/`-Kern; der
// PDF-Adapter serialisiert die kern-gelieferte `PlanView` aus dem Bündel.
using model::PlanSegment;
using model::PlanView;
using model::StoreyPlan;

namespace {

// A4-Seite + Layout-Konstanten (PDF-Punkte). Fester Maßstab 1:100 (dokumentiert
// im "M 1:100"-Label + hier). mm → pt: pt = mm · (72/25.4) / 100.
constexpr double kPageWidthPt = 595.0;
constexpr double kPageHeightPt = 842.0;
constexpr double kMarginPt = 40.0;   // Ursprung der Zeichnung (Modell-min)
constexpr double kFramePt = 20.0;    // Rahmen-Abstand zum Seitenrand
constexpr double kLineWidthPt = 0.5;
constexpr double kLabelFontPt = 8.0;
constexpr double kScaleDenominator = 100.0;  // 1:100
constexpr double kPointsPerMm = 72.0 / 25.4;
constexpr double kMmToPt = kPointsPerMm / kScaleDenominator;

// Maßstabs-Label aus der Konstante (bleibt mit dem Maßstab synchron).
std::string scaleLabel() {
    return "M 1:" + std::to_string(static_cast<int>(kScaleDenominator));
}

// Modell-mm → Seiten-Punkt (Ursprung = Modell-Bounding-Box-Minimum + Rand).
double toPageX(double x_mm, const PlanView& view) {
    return kMarginPt + ((x_mm - view.min_x_mm) * kMmToPt);
}
double toPageY(double y_mm, const PlanView& view) {
    return kMarginPt + ((y_mm - view.min_y_mm) * kMmToPt);
}

// Eine Seite: Rahmen + (maßstäbliche) Wand-Achsen des Geschosses + Label.
void drawPage(PdfWriter& writer, const StoreyPlan& plan, const PlanView& view) {
    writer.beginPage();
    writer.setLineWidth(kLineWidthPt);
    writer.rect(kFramePt, kFramePt, kPageWidthPt - (2.0 * kFramePt),
                kPageHeightPt - (2.0 * kFramePt));
    for (const PlanSegment& seg : plan.segments) {
        writer.line(toPageX(seg.x1_mm, view), toPageY(seg.y1_mm, view),
                    toPageX(seg.x2_mm, view), toPageY(seg.y2_mm, view));
    }
    writer.text(kFramePt + kLabelFontPt, kFramePt + kLabelFontPt, kLabelFontPt,
                scaleLabel());
    writer.endPage();
}

}  // namespace

void PdfExportAdapter::write(const model::Building& /*building*/,
                             const model::DerivedGeometry& derived,
                             const fs::path& path) const {
    // ADR-0020: der Kern liefert die 2D-Projektion im Bündel; der Adapter
    // serialisiert nur (kein Eigen-`projectPlan` mehr).
    const PlanView& view = derived.plan;
    PdfWriter writer(PdfPageSize{kPageWidthPt, kPageHeightPt});

    if (view.storeys.empty()) {
        // Totalität: leeres Modell → eine gültige, (annähernd) leere Seite.
        drawPage(writer, StoreyPlan{}, view);
    } else {
        for (const StoreyPlan& plan : view.storeys) {  // eine Seite je Geschoss
            drawPage(writer, plan, view);
        }
    }

    writeFileAtomically(path, writer.build(), "PDF");  // atomar, E-IO-001
}

}  // namespace bcad::adapters::io
