#pragma once

#include <string>

namespace bcad::hexagon::ports::driving {

// Driving Port (Skelett-Beispiel, slice-002): Use-Case, den der Kern
// anbietet und den ab slice-003 ein GUI-/Plugin-Adapter aufruft.
class GreetPort {
public:
    virtual ~GreetPort() = default;
    virtual std::string greet() const = 0;
};

}  // namespace bcad::hexagon::ports::driving
