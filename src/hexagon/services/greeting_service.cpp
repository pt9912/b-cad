#include "hexagon/services/greeting_service.h"

namespace bcad::hexagon::services {

GreetingService::GreetingService(const ports::driven::GreetingSourcePort& source)
    : source_(source) {}

std::string GreetingService::greet() const {
    return "Hello, " + source_.greeting_name() + ".";
}

}  // namespace bcad::hexagon::services
