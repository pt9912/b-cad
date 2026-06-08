// Driven-Adapter (Geometrie) — Skelett-Probe (slice-001).
// Beweist, dass bcad_adapters real gegen OpenCascade linkt. Der echte
// GeometryKernelPort-Adapter (Extrusion, boolesche Operationen,
// ADR-0002) folgt ab slice-003.

#include <Standard_Version.hxx>

#include <string>

namespace bcad::adapters::geometry {

std::string occ_version() {
    return OCC_VERSION_COMPLETE;
}

}  // namespace bcad::adapters::geometry
