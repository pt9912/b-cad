// Hello-Hexagon (slice-002): beweist den Driving-Port→Service→Driven-Port-
// Roundtrip mit einem Test-Double — ohne jeden Adapter (ADR-0001
// §Testbarkeit). Genau die Eigenschaft, die `a-check` strukturell
// absichert.

#include <gtest/gtest.h>

#include <string>

#include "hexagon/ports/driven/greeting_source_port.h"
#include "hexagon/services/greeting_service.h"

namespace {

class FixedGreetingSource final
    : public bcad::hexagon::ports::driven::GreetingSourcePort {
public:
    std::string greeting_name() const override { return "hexagon"; }
};

}  // namespace

TEST(GreetingService, GruesstUeberDrivenPortDouble) {
    const FixedGreetingSource source;
    const bcad::hexagon::services::GreetingService service(source);
    EXPECT_EQ(service.greet(), "Hello, hexagon.");
}
