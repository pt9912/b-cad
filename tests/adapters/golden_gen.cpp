// golden_gen (slice-044a): dedizierter Generator der Export-Golden-Dateien.
// Baut das geteilte `goldenModel()` und exportiert es über den ECHTEN
// `ExchangeService` je Format in das per argv[1] übergebene Ausgabe-Verzeichnis
// (model.{ifc,dxf,step,stl,pdf,png}).
//
// Läuft bewusst NICHT über das Produktions-Binary (das baut
// `buildAcc001KernDemo`, nicht `goldenModel()` → Golden-Modell-Drift,
// MR-006-044-MED-2). `make golden-regen` mountet `tests/adapters/golden` nach
// /out und ruft dieses Tool; `make golden-check` regeneriert nach /tmp und
// difft gegen die committeten Golden.

#include <exception>
#include <filesystem>

#include <cstdio>

#include "golden_model.h"

#include "adapters/geometry/occ_geometry_adapter.h"
#include "adapters/geometry/step_export_adapter.h"
#include "adapters/geometry/stl_export_adapter.h"
#include "adapters/io/dxf_export_adapter.h"
#include "adapters/io/ifc_export_adapter.h"
#include "adapters/io/pdf_export_adapter.h"
#include "adapters/io/png_export_adapter.h"
#include "hexagon/ports/driving/exchange_model_port.h"
#include "hexagon/services/exchange_service.h"

int main(int argc, char** argv) {
    if (argc < 2) {
        std::fprintf(stderr, "usage: golden_gen <out-dir>\n");
        return 2;
    }
    namespace fs = std::filesystem;
    namespace geo = bcad::adapters::geometry;
    namespace io = bcad::adapters::io;
    using bcad::hexagon::ports::driving::ExchangeFormat;
    using bcad::hexagon::services::ExchangeService;

    const fs::path out_dir(argv[1]);

    // Dieselben Adapter wie der Composition Root: STL braucht den OCC-Kern
    // injiziert, die übrigen fünf sind default-konstruiert.
    const geo::OccGeometryAdapter geometry;
    const geo::StlExportAdapter stl(geometry);
    const geo::StepExportAdapter step;
    const io::IfcExportAdapter ifc;
    const io::DxfExportAdapter dxf;
    const io::PdfExportAdapter pdf;
    const io::PngExportAdapter png;

    const ExchangeService service(
        {},  // Import ungenutzt
        {{ExchangeFormat::Ifc, &ifc},
         {ExchangeFormat::Dxf, &dxf},
         {ExchangeFormat::Step, &step},
         {ExchangeFormat::Stl, &stl},
         {ExchangeFormat::Pdf, &pdf},
         {ExchangeFormat::Png, &png}});

    struct Target {
        ExchangeFormat format;
        const char* name;
    };
    const Target targets[] = {
        {ExchangeFormat::Ifc, "model.ifc"},   {ExchangeFormat::Dxf, "model.dxf"},
        {ExchangeFormat::Step, "model.step"}, {ExchangeFormat::Stl, "model.stl"},
        {ExchangeFormat::Pdf, "model.pdf"},   {ExchangeFormat::Png, "model.png"},
    };

    const bcad::hexagon::model::Building model = bcad::golden::goldenModel();
    const bcad::hexagon::model::ExportProvenance provenance =
        bcad::golden::goldenProvenance();  // slice-046: feste Herkunft im Golden
    for (const Target& target : targets) {
        try {
            service.exportModel(model, out_dir / target.name, target.format, provenance);
            std::fprintf(stderr, "golden_gen: wrote %s\n", target.name);
        } catch (const std::exception& e) {
            std::fprintf(stderr, "golden_gen: FAILED %s: %s\n", target.name,
                         e.what());
            return 1;
        }
    }
    return 0;
}
