// Byte-genaues Golden-file-Netz für alle sechs Austauschformate (slice-044a,
// LH-FA-IO-001..006). Ergänzt die vorhandenen Decode-/Struktur-Orakel (die
// Semantik prüfen, aber Byte-Reihenfolge/Formatierung tolerieren) um den Fang
// für **Encoder-Drift bei unveränderter Semantik**: je Format wird das geteilte
// `goldenModel()` über den ECHTEN `ExchangeService` re-exportiert und **byte-
// identisch** gegen die committete Golden-Datei (`BCAD_TEST_GOLDEN_DIR`)
// verglichen. **Komplementär** — Golden ersetzt die Struktur-Orakel nicht.
//
// Die Golden-Dateien entstehen über `make golden-regen` (dedizierter
// `golden_gen`, dieselbe `goldenModel()`-TU); `make golden-check` verifiziert
// Regen-Determinismus. STL ist OCC-tessellations-/versions-gebunden (ADR-0004):
// ein OCC-Upgrade bricht den STL-Byte-Vergleich BEWUSST (Signal → Diff-Review),
// STEP-Topologie ebenso — der STEP-HEADER dagegen ist seit slice-044a fixiert.

#include <algorithm>
#include <cstddef>
#include <filesystem>
#include <fstream>
#include <iterator>
#include <string>
#include <system_error>
#include <vector>

#include <gtest/gtest.h>

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

namespace {

namespace fs = std::filesystem;
namespace geo = bcad::adapters::geometry;
namespace io = bcad::adapters::io;
using bcad::hexagon::ports::driving::ExchangeFormat;
using bcad::hexagon::services::ExchangeService;

std::vector<char> readBytes(const fs::path& path) {
    std::ifstream in(path, std::ios::binary);
    return {std::istreambuf_iterator<char>(in),
            std::istreambuf_iterator<char>()};
}

// Re-Export des Golden-Modells über den echten Service + Byte-Vergleich gegen die
// committete Golden-Datei. Bei Abweichung: erste Diff-Position + Größen (die
// Semantik kann unverändert sein → Encoder-Drift; Diff reviewen, dann Regen).
void expectGoldenByteIdentical(ExchangeFormat format, const std::string& name) {
    const geo::OccGeometryAdapter geometry;
    const geo::StlExportAdapter stl(geometry);
    const geo::StepExportAdapter step;
    const io::IfcExportAdapter ifc;
    const io::DxfExportAdapter dxf;
    const io::PdfExportAdapter pdf;
    const io::PngExportAdapter png;
    const ExchangeService service(
        {}, {{ExchangeFormat::Ifc, &ifc},
             {ExchangeFormat::Dxf, &dxf},
             {ExchangeFormat::Step, &step},
             {ExchangeFormat::Stl, &stl},
             {ExchangeFormat::Pdf, &pdf},
             {ExchangeFormat::Png, &png}});

    const fs::path out = fs::temp_directory_path() / ("bcad_golden_" + name);
    std::error_code ec;
    fs::remove(out, ec);
    // slice-046: dieselbe feste Provenance wie golden_gen → das Golden trägt die
    // injizierte Herkunft und bleibt byte-deterministisch.
    service.exportModel(bcad::golden::goldenModel(), out, format,
                        bcad::golden::goldenProvenance());

    const std::vector<char> got = readBytes(out);
    const std::vector<char> want =
        readBytes(fs::path(BCAD_TEST_GOLDEN_DIR) / name);
    fs::remove(out, ec);

    ASSERT_FALSE(want.empty())
        << "Golden fehlt/leer: " << (fs::path(BCAD_TEST_GOLDEN_DIR) / name)
        << " — 'make golden-regen' vergessen?";
    ASSERT_FALSE(got.empty()) << "Export erzeugte keine Bytes: " << name;

    if (got != want) {
        const std::size_t n = std::min(got.size(), want.size());
        std::size_t at = 0;
        while (at < n && got[at] == want[at]) {
            ++at;
        }
        ADD_FAILURE() << name << ": Byte-Drift ggü. Golden — got " << got.size()
                      << " B, want " << want.size() << " B, erste Abweichung @ Offset "
                      << at << ". Semantik evtl. unverändert (Encoder-Drift) → "
                         "Diff reviewen, dann 'make golden-regen'.";
    }
}

TEST(GoldenExport, IfcByteIdentical) {
    expectGoldenByteIdentical(ExchangeFormat::Ifc, "model.ifc");
}
TEST(GoldenExport, DxfByteIdentical) {
    expectGoldenByteIdentical(ExchangeFormat::Dxf, "model.dxf");
}
TEST(GoldenExport, StepByteIdentical) {
    expectGoldenByteIdentical(ExchangeFormat::Step, "model.step");
}
// STL: OCC-tessellations-/versions-gebunden (ADR-0004) — ein OCC-Upgrade bricht
// diesen Byte-Vergleich bewusst (Signal, dass sich die Tessellation änderte).
TEST(GoldenExport, StlByteIdentical) {
    expectGoldenByteIdentical(ExchangeFormat::Stl, "model.stl");
}
TEST(GoldenExport, PdfByteIdentical) {
    expectGoldenByteIdentical(ExchangeFormat::Pdf, "model.pdf");
}
TEST(GoldenExport, PngByteIdentical) {
    expectGoldenByteIdentical(ExchangeFormat::Png, "model.png");
}

// slice-046 (der Produkt-Kernwert): **verschiedene** Export-Herkunft → **verschiedene**
// Datei (unterscheidbar), **gleiche** Herkunft → byte-identisch (deterministisch). Über
// die drei Formate, die in 046a Herkunft tragen: PDF (sichtbarer Footer), STEP (FILE_NAME),
// STL (80-Byte-Header). PNG (sichtbar = 046b) / IFC / DXF tragen sie hier noch nicht.
TEST(GoldenExport, DistinctProvenanceYieldsDistinctOutput) {
    const geo::OccGeometryAdapter geometry;
    const geo::StlExportAdapter stl(geometry);
    const geo::StepExportAdapter step;
    const io::PdfExportAdapter pdf;
    const ExchangeService service({}, {{ExchangeFormat::Step, &step},
                                       {ExchangeFormat::Stl, &stl},
                                       {ExchangeFormat::Pdf, &pdf}});

    const bcad::hexagon::model::ExportProvenance prov_a{"2020-01-01 08:00",
                                                        "haus-a.bcad", "b-cad 0.1.0"};
    const bcad::hexagon::model::ExportProvenance prov_b{"2026-07-24 14:32",
                                                        "haus-b.bcad", "b-cad 0.1.0"};

    // Nur die Unterscheidbarkeit (die neue Aussage): verschiedene Herkunft →
    // verschiedene Datei. Die Determinismus-Seite (gleiche Herkunft → byte-identisch)
    // deckt `make golden-check` (cross-Prozess) + `GoldenExport.*` ab — NICHT hier
    // in-Prozess: OCC hält für STEP globalen Entity-Zähler-State zwischen Exporten,
    // sodass zwei STEP-Exporte im selben Prozess byte-abweichen (cross-Prozess nicht).
    for (const ExchangeFormat fmt :
         {ExchangeFormat::Pdf, ExchangeFormat::Step, ExchangeFormat::Stl}) {
        const fs::path pa = fs::temp_directory_path() / "bcad_prov_a";
        const fs::path pb = fs::temp_directory_path() / "bcad_prov_b";
        std::error_code ec;
        service.exportModel(bcad::golden::goldenModel(), pa, fmt, prov_a);
        service.exportModel(bcad::golden::goldenModel(), pb, fmt, prov_b);
        const std::vector<char> ba = readBytes(pa);
        const std::vector<char> bb = readBytes(pb);
        fs::remove(pa, ec);
        fs::remove(pb, ec);
        EXPECT_NE(ba, bb) << "verschiedene Herkunft muss unterscheidbare Datei ergeben (fmt "
                          << static_cast<int>(fmt) << ")";
    }
}

}  // namespace
