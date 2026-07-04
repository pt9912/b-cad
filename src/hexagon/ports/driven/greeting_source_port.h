#pragma once

#include <string>

namespace bcad::hexagon::ports::driven {

// Driven Port (Skelett-Beispiel, slice-002). Demonstriert die
// Port-Abstraktion, an der `a-check` die hexagonale Schichtung prüft,
// und ist mit einem Test-Double erfüllbar (ADR-0001 §Testbarkeit). Wird
// ab slice-003 durch echte Driven Ports (GeometryKernelPort,
// ProjectRepositoryPort …) ersetzt.
class GreetingSourcePort {
public:
    virtual ~GreetingSourcePort() = default;
    virtual std::string greeting_name() const = 0;
};

}  // namespace bcad::hexagon::ports::driven
