// Driven-Adapter (Geometrie) — Skelett-Probe (slice-001).
// Beweist, dass bcad_adapters real gegen OpenCascade linkt. Der echte
// GeometryKernelPort-Adapter (Extrusion, boolesche Operationen,
// ADR-0002) folgt ab slice-003.

#include "adapters/geometry/occ_probe.h"

#include <Standard_Version.hxx>
#include <TCollection_AsciiString.hxx>

namespace bcad::adapters::geometry {

std::string occ_version() {
    // OCC_VERSION_COMPLETE ist ein Compile-Time-Makro und würde für sich
    // KEIN Linken gegen OpenCascade erzwingen. Wir leiten den Wert
    // bewusst durch eine reale TKernel-Laufzeitfunktion
    // (TCollection_AsciiString), damit echte Linkage gegen TKernel
    // erzwungen wird.
    const TCollection_AsciiString v(OCC_VERSION_COMPLETE);
    return std::string(v.ToCString(), static_cast<std::size_t>(v.Length()));
}

}  // namespace bcad::adapters::geometry
