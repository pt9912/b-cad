#pragma once

#include <string>

namespace bcad::adapters::geometry {

// Skelett-Probe (slice-001): liefert die OpenCascade-Version über eine
// reale TKernel-Laufzeitfunktion (nicht das Compile-Time-Makro), damit
// echte Linkage gegen OpenCascade erzwungen wird.
std::string occ_version();

}  // namespace bcad::adapters::geometry
