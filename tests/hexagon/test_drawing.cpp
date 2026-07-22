// Akzeptanz-Tests für das 2D-Zeichnen (slice-032b, LH-FA-DRW-005/006, ADR-0018)
// — OCC-frei über das GeometryKernelPort-Double (ADR-0001 §Testbarkeit). Ebenen
// und Hilfslinien sind pure Domänen-Werte auf `Building`, editiert über den
// EIGENEN Driving-Port EditDrawingPort (Muster EvaluatePort: eigener Port,
// geteiltes Service-Objekt):
// - DRW-006: Ebene anlegen/umbenennen/sichtbar; Name-Pflicht + projekt-eindeutig
//   (uq_layers_project_name); removeLayer RESTRICT (referenzierte Ebene nicht
//   löschbar, ADR-0018).
// - DRW-005: Hilfslinie anlegen; Entartung (Anfang = Ende) abgelehnt; unbekannte
//   Ebene/unbekanntes Geschoss abgelehnt (E-VAL-001-Rejection, Modell unverändert).
// Die Export-Sichtbarkeit (Hilfslinie „erscheint im Artefakt" / unsichtbare Ebene
// → fehlt) ist slice-032c; hier ruht die Beobachtung modell-seitig (der
// Persistenz-Round-Trip: tests/adapters/test_sqlite_project_repository.cpp).

#include <gtest/gtest.h>

#include <optional>
#include <string>
#include <utility>

#include "analytic_geometry_double.h"
#include "hexagon/model/guide_line.h"
#include "hexagon/model/layer.h"
#include "hexagon/model/point2d.h"
#include "hexagon/services/structure_edit_service.h"

namespace {

namespace model = bcad::hexagon::model;
namespace services = bcad::hexagon::services;
using bcad::testing::AnalyticGeometry;

constexpr model::StoreyId kGroundStorey{1};  // Default-Geschoss bei Service-Anlage

model::Layer makeLayer(std::string name) {
    model::Layer layer;
    layer.name = std::move(name);
    return layer;
}

model::GuideLine makeGuide(model::StoreyId storey, model::LayerId layer,
                           model::Point2D a, model::Point2D b) {
    model::GuideLine guide;
    guide.storey_id = storey;
    guide.layer_id = layer;
    guide.segment = {a, b};
    return guide;
}

}  // namespace

// DRW-006 Happy: Ebene anlegen (mit Farbe), umbenennen, sichtbar/unsichtbar.
TEST(Drawing_LH_FA_DRW_006, EbeneAnlegenUmbenennenSichtbarkeit) {
    const AnalyticGeometry geometry;
    services::StructureEditService svc(geometry);

    model::Layer proto = makeLayer("Bemaßung");
    proto.color_hex = "#00ff00";
    const std::optional<model::LayerId> id = svc.addLayer(proto);
    ASSERT_TRUE(id.has_value());

    ASSERT_EQ(svc.building().layers.size(), 1U);
    EXPECT_EQ(svc.building().layers[0].name, "Bemaßung");
    EXPECT_TRUE(svc.building().layers[0].visible);
    ASSERT_TRUE(svc.building().layers[0].color_hex.has_value());
    EXPECT_EQ(*svc.building().layers[0].color_hex, "#00ff00");

    EXPECT_TRUE(svc.renameLayer(*id, "Hilfsgeometrie"));
    EXPECT_EQ(svc.building().layers[0].name, "Hilfsgeometrie");

    EXPECT_TRUE(svc.setLayerVisible(*id, false));  // Export-Filter
    EXPECT_FALSE(svc.building().layers[0].visible);
}

// DRW-006 Boundary: Ebene ohne Namen (leer/whitespace) abgelehnt.
TEST(Drawing_LH_FA_DRW_006, LeererNameAbgelehnt) {
    const AnalyticGeometry geometry;
    services::StructureEditService svc(geometry);

    EXPECT_FALSE(svc.addLayer(makeLayer("   ")).has_value());
    EXPECT_TRUE(svc.building().layers.empty());  // Modell unverändert
}

// DRW-006 (uq_layers_project_name, Zusatz-Sonde): projekt-doppelter Name
// abgelehnt — beim Anlegen UND beim Umbenennen.
TEST(Drawing_LH_FA_DRW_006, DoppelterNameAbgelehnt) {
    const AnalyticGeometry geometry;
    services::StructureEditService svc(geometry);

    ASSERT_TRUE(svc.addLayer(makeLayer("Layer A")).has_value());
    EXPECT_FALSE(svc.addLayer(makeLayer("Layer A")).has_value());  // Duplikat
    EXPECT_EQ(svc.building().layers.size(), 1U);

    const auto b = svc.addLayer(makeLayer("Layer B"));
    ASSERT_TRUE(b.has_value());
    EXPECT_FALSE(svc.renameLayer(*b, "Layer A"));  // Kollision → abgelehnt
    EXPECT_EQ(svc.building().layers.back().name, "Layer B");  // unverändert
}

// DRW-006 Negative: noch referenzierte Ebene löschen → abgelehnt (restrict).
TEST(Drawing_LH_FA_DRW_006, ReferenzierteEbeneLoeschenAbgelehnt) {
    const AnalyticGeometry geometry;
    services::StructureEditService svc(geometry);

    const auto layer = svc.addLayer(makeLayer("Achsen"));
    ASSERT_TRUE(layer.has_value());
    const auto guide = svc.addGuideLine(
        makeGuide(kGroundStorey, *layer, {0.0, 0.0}, {1000.0, 0.0}));
    ASSERT_TRUE(guide.has_value());

    EXPECT_FALSE(svc.removeLayer(*layer));         // restrict: referenziert
    EXPECT_EQ(svc.building().layers.size(), 1U);   // Modell unverändert

    // Nach Entfernen der Hilfslinie ist die Ebene löschbar.
    EXPECT_TRUE(svc.removeGuideLine(*guide));
    EXPECT_TRUE(svc.removeLayer(*layer));
    EXPECT_TRUE(svc.building().layers.empty());
}

// DRW-005 Happy: Hilfslinie auf sichtbarer Ebene anlegen (Endpunkte/Ebene).
TEST(Drawing_LH_FA_DRW_005, HilfslinieAnlegen) {
    const AnalyticGeometry geometry;
    services::StructureEditService svc(geometry);

    const auto layer = svc.addLayer(makeLayer("Achsen"));
    ASSERT_TRUE(layer.has_value());
    const auto id = svc.addGuideLine(
        makeGuide(kGroundStorey, *layer, {100.0, 200.0}, {900.0, 200.0}));
    ASSERT_TRUE(id.has_value());

    ASSERT_EQ(svc.building().guide_lines.size(), 1U);
    const model::GuideLine& g = svc.building().guide_lines[0];
    EXPECT_EQ(static_cast<int>(g.storey_id), static_cast<int>(kGroundStorey));
    EXPECT_EQ(static_cast<int>(g.layer_id), static_cast<int>(*layer));
    EXPECT_DOUBLE_EQ(g.segment.start.x_mm, 100.0);
    EXPECT_DOUBLE_EQ(g.segment.start.y_mm, 200.0);
    EXPECT_DOUBLE_EQ(g.segment.end.x_mm, 900.0);
    EXPECT_DOUBLE_EQ(g.segment.end.y_mm, 200.0);
}

// DRW-005 Boundary: entartete Hilfslinie (Anfang = Ende) abgelehnt.
TEST(Drawing_LH_FA_DRW_005, EntarteteHilfslinieAbgelehnt) {
    const AnalyticGeometry geometry;
    services::StructureEditService svc(geometry);

    const auto layer = svc.addLayer(makeLayer("Achsen"));
    ASSERT_TRUE(layer.has_value());
    EXPECT_FALSE(
        svc.addGuideLine(
               makeGuide(kGroundStorey, *layer, {500.0, 500.0}, {500.0, 500.0}))
            .has_value());
    EXPECT_TRUE(svc.building().guide_lines.empty());  // Modell unverändert
}

// DRW-005 (Zusatz-Sonde): unbekannte Ebene / unbekanntes Geschoss abgelehnt;
// removeGuideLine mit unbekannter Id → false.
TEST(Drawing_LH_FA_DRW_005, UnbekannteEbeneOderGeschossAbgelehnt) {
    const AnalyticGeometry geometry;
    services::StructureEditService svc(geometry);

    const auto layer = svc.addLayer(makeLayer("Achsen"));
    ASSERT_TRUE(layer.has_value());

    EXPECT_FALSE(svc.addGuideLine(makeGuide(kGroundStorey, model::LayerId{999},
                                            {0.0, 0.0}, {10.0, 0.0}))
                     .has_value());  // unbekannte Ebene
    EXPECT_FALSE(svc.addGuideLine(makeGuide(model::StoreyId{999}, *layer,
                                            {0.0, 0.0}, {10.0, 0.0}))
                     .has_value());  // unbekanntes Geschoss
    EXPECT_TRUE(svc.building().guide_lines.empty());

    EXPECT_FALSE(svc.removeGuideLine(model::GuideLineId{42}));  // unbekannt
}
