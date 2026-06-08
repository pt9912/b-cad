// Smoke-Test des dependency-freien Kerns (slice-001). Beweist die
// Test-Verdrahtung (GoogleTest, REQ-TEC-005) gegen bcad_hexagon ohne
// Qt/OCC/SQLite. Akzeptanz-Tests mit LH-IDs folgen ab slice-003.

#include <gtest/gtest.h>

#include "hexagon/services/bootstrap_info.h"

TEST(BootstrapInfo, BannerNenntProduktnamen) {
    const std::string banner = bcad::hexagon::services::application_banner();
    EXPECT_NE(banner.find("b-cad"), std::string::npos);
}
