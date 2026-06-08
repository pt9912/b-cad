#include "hexagon/services/bootstrap_info.h"

namespace bcad::hexagon::services {

std::string application_banner() {
    // BCAD_VERSION wird vom Build aus der project()-Version gesetzt;
    // Fallback hält die Übersetzungseinheit auch ohne Define übersetzbar
    // (Host-Verifikation des dependency-freien Kerns, ADR-0001).
#ifdef BCAD_VERSION
    return std::string("b-cad ") + BCAD_VERSION;
#else
    return std::string("b-cad (dev)");
#endif
}

}  // namespace bcad::hexagon::services
