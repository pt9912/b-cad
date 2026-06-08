#pragma once

#include <string>

#include "hexagon/ports/driven/greeting_source_port.h"
#include "hexagon/ports/driving/greet_port.h"

namespace bcad::hexagon::services {

// Anwendungslogik: implementiert den Driving Port `GreetPort` und nutzt
// den Driven Port `GreetingSourcePort` (Abwärts-Abhängigkeit, ADR-0001).
// Framework-frei — kein Qt/OCC/SQLite.
class GreetingService final : public ports::driving::GreetPort {
public:
    explicit GreetingService(const ports::driven::GreetingSourcePort& source);
    std::string greet() const override;

private:
    const ports::driven::GreetingSourcePort& source_;
};

}  // namespace bcad::hexagon::services
