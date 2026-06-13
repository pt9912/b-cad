// AK-Tests Viewer-Szene (sichtbare Hälfte LH-FA-D3-002 + statische
// Darstellung LH-FA-D3-001) — display-frei über das Szenen-Surrogat
// (ADR-0009 (f)): echter Kern + echter OCC-Adapter, geprüft wird der
// Szenen-Endzustand (gehaltene Netze + Zähler wirksamer Updates).

#include <gtest/gtest.h>

#include <algorithm>
#include <cmath>
#include <limits>
#include <vector>

#include "adapters/geometry/occ_geometry_adapter.h"
#include "adapters/ui/viewer_scene.h"
#include "hexagon/model/constants.h"
#include "hexagon/model/footprint.h"
#include "hexagon/model/roof.h"
#include "hexagon/model/segment.h"
#include "hexagon/model/slab.h"
#include "hexagon/ports/driven/model_changed_port.h"
#include "hexagon/services/structure_edit_service.h"

namespace {

using bcad::adapters::geometry::OccGeometryAdapter;
using bcad::adapters::ui::ViewerScene;
namespace model = bcad::hexagon::model;
namespace services = bcad::hexagon::services;
namespace driven = bcad::hexagon::ports::driven;

// Schreibt gemeldete Ops mit (für die Folge-Meldungs-AK).
class OpRecorder final : public driven::ModelChangedPort {
public:
    void onModelChanged(const driven::ModelChange& change) override {
        ops.push_back(change.op);
    }
    int count(driven::ModelChangeOp op) const {
        return static_cast<int>(std::count(ops.begin(), ops.end(), op));
    }
    std::vector<driven::ModelChangeOp> ops;
};

model::Segment seg(double x1, double y1, double x2, double y2) {
    return model::Segment{model::Point2D{x1, y1}, model::Point2D{x2, y2}};
}

// Bounding-Box-Ausdehnung eines Netzes entlang einer Achse (0=x,1=y,2=z).
double meshExtent(const model::TriangleMesh& mesh, int axis) {
    double lo = std::numeric_limits<double>::max();
    double hi = std::numeric_limits<double>::lowest();
    for (std::size_t i = static_cast<std::size_t>(axis);
         i < mesh.positions.size(); i += 3) {
        lo = std::min(lo, mesh.positions[i]);
        hi = std::max(hi, mesh.positions[i]);
    }
    return hi - lo;
}

class ViewerSceneAk : public ::testing::Test {
protected:
    OccGeometryAdapter geometry_;
    services::StructureEditService service_{geometry_};
    ViewerScene scene_{service_};
    model::StoreyId eg_{service_.building().storeys.front().id};
};

// Split-Hälfte (i) — statische Darstellung: die Szene zeigt den
// extrudierten Stand aller Wände nach Initial-Laden.
TEST_F(ViewerSceneAk, LH_FA_D3_001_StatischeDarstellungZeigtExtrudiertenStand) {
    service_.addWall(eg_, seg(0, 0, 4000, 0));
    service_.addWall(eg_, seg(4000, 0, 4000, 3000));

    scene_.loadAll();

    ASSERT_EQ(scene_.wallMeshes().size(), 2U);
    for (const auto& [id, mesh] : scene_.wallMeshes()) {
        EXPECT_GT(mesh.triangleCount(), 0) << "Wand ohne Netz";
        // Extrusion in +Z: Netz-Höhe == Wandhöhe (Default = Geschosshöhe).
        EXPECT_NEAR(meshExtent(mesh, 2), model::kDefaultStoreyHeightMm, 0.5);
    }
}

// Happy (ii): committete Parameteränderung → dargestellter Stand folgt
// ohne expliziten Benutzer-/Reload-Schritt (nur über den Callback).
TEST_F(ViewerSceneAk, LH_FA_D3_002_Happy_ParameterAenderungFolgtOhneBenutzerSchritt) {
    const auto wall = service_.addWall(eg_, seg(0, 0, 4000, 0));
    ASSERT_TRUE(wall.has_value());
    scene_.loadAll();
    service_.subscribe(scene_);

    const auto result = service_.setWallThickness(*wall, 800.0);
    ASSERT_EQ(result.status, bcad::hexagon::ports::driving::ParamStatus::Accepted);

    // Kein loadAll(): der Stand kam ausschließlich über Push-Notify+Pull.
    const auto& mesh = scene_.wallMeshes().at(*wall);
    EXPECT_NEAR(meshExtent(mesh, 1), 800.0, 0.5);  // Stärke quer zur Wand (y)
    EXPECT_GE(scene_.effectiveUpdates(), 1);
    service_.unsubscribe(scene_);
}

// Boundary (ii): geklemmte Änderung → dargestellter Stand = Grenzwert;
// die Mehrfach-Meldung einer Mutation (Wand-Op + RoomsChanged) erzeugt
// genau EIN wirksames Szenen-Update (kein Flackern; Welle-1-Lerneintrag).
TEST_F(ViewerSceneAk, LH_FA_D3_002_Boundary_GeklemmtUndGenauEinUpdateJeMutation) {
    const auto wall = service_.addWall(eg_, seg(0, 0, 4000, 0));
    ASSERT_TRUE(wall.has_value());
    scene_.loadAll();
    service_.subscribe(scene_);

    const auto result = service_.setWallThickness(*wall, 5000.0);  // > max
    EXPECT_EQ(result.status, bcad::hexagon::ports::driving::ParamStatus::Clamped);
    EXPECT_NEAR(result.applied_mm, model::kWallThicknessMaxMm, 1e-9);

    EXPECT_NEAR(meshExtent(scene_.wallMeshes().at(*wall), 1),
                model::kWallThicknessMaxMm, 0.5);
    EXPECT_EQ(scene_.effectiveUpdates(), 1)
        << "Wand-Op + RoomsChanged dürfen nur ein wirksames Update erzeugen";
    service_.unsubscribe(scene_);
}

// Boundary (ii): identische Mehrfach-Meldung → idempotenter
// Szenen-Endzustand (je Meldung höchstens ein wirksames Update, der
// Stand kippt nicht).
TEST_F(ViewerSceneAk, LH_FA_D3_002_Boundary_MehrfachMeldungIdempotent) {
    const auto wall = service_.addWall(eg_, seg(0, 0, 4000, 0));
    ASSERT_TRUE(wall.has_value());
    scene_.loadAll();

    const bcad::hexagon::ports::driven::ModelChange change{
        .op = bcad::hexagon::ports::driven::ModelChangeOp::WallThicknessChanged,
        .storey_id = eg_,
        .wall_id = *wall};
    scene_.onModelChanged(change);
    const double extent_first = meshExtent(scene_.wallMeshes().at(*wall), 1);
    scene_.onModelChanged(change);  // identische Wiederholung

    EXPECT_EQ(scene_.wallMeshes().size(), 1U);
    EXPECT_NEAR(meshExtent(scene_.wallMeshes().at(*wall), 1), extent_first, 1e-9);
    EXPECT_EQ(scene_.effectiveUpdates(), 2);  // je Meldung genau eines
}

// Negative (ii): verworfene Mutation (Rejected) → keine Meldung, die
// Darstellung bleibt unverändert.
TEST_F(ViewerSceneAk, LH_FA_D3_002_Negative_RejectedAendertDarstellungNicht) {
    const auto wall = service_.addWall(eg_, seg(0, 0, 4000, 0));
    ASSERT_TRUE(wall.has_value());
    scene_.loadAll();
    service_.subscribe(scene_);
    const double extent_before = meshExtent(scene_.wallMeshes().at(*wall), 1);

    const auto result = service_.setWallThickness(
        *wall, std::numeric_limits<double>::quiet_NaN());
    EXPECT_EQ(result.status, bcad::hexagon::ports::driving::ParamStatus::Rejected);

    EXPECT_EQ(scene_.effectiveUpdates(), 0);
    EXPECT_NEAR(meshExtent(scene_.wallMeshes().at(*wall), 1), extent_before, 1e-9);
    service_.unsubscribe(scene_);
}

// LH-FA-WAL-006 (slice-012): die Stärke-Änderung einer Wand zieht den
// Eck-Nachbarn in der Szene nach (WallGeometryChanged → Pull) — die
// Nachbar-Bounding-Box folgt der neuen Eck-Geometrie.
TEST_F(ViewerSceneAk, LH_FA_WAL_006_NachbarFolgtInDerSzene) {
    const auto a = service_.addWall(eg_, seg(0, 0, 4000, 0));
    const auto b = service_.addWall(eg_, seg(4000, 0, 4000, 3000));
    ASSERT_TRUE(a.has_value() && b.has_value());
    scene_.loadAll();
    service_.subscribe(scene_);

    service_.setWallThickness(*a, 500.0);

    // Nachbar B reicht jetzt bis an As neue Außenkante (y = -250).
    const auto& mesh_b = scene_.wallMeshes().at(*b);
    double y_min = std::numeric_limits<double>::max();
    for (std::size_t i = 1; i < mesh_b.positions.size(); i += 3) {
        y_min = std::min(y_min, mesh_b.positions[i]);
    }
    EXPECT_NEAR(y_min, -250.0, 0.5);
    EXPECT_GE(scene_.effectiveUpdates(), 2);  // A + Nachbar B
    service_.unsubscribe(scene_);
}

// LH-FA-DOR-004 (ADR-0011): eine platzierte Tür bricht die Wirtswand
// durch — die Szene folgt der `WallGeometryChanged`-Meldung der Wirtswand
// (Pull) ohne Benutzer-Schritt; das Netz der Wand wird wirksam ersetzt.
TEST_F(ViewerSceneAk, LH_FA_DOR_004_TuerOeffnungFolgtInDerSzene) {
    const auto wall = service_.addWall(eg_, seg(0, 0, 4000, 0));
    ASSERT_TRUE(wall.has_value());
    scene_.loadAll();
    service_.subscribe(scene_);
    const int triangles_before = scene_.wallMeshes().at(*wall).triangleCount();

    const auto door = service_.addDoor(*wall, 1500.0);
    ASSERT_TRUE(door.has_value());

    // Kein loadAll(): der Stand kam über Push-Notify (WallGeometryChanged)
    // + Pull. Die Öffnung erzeugt zusätzliche Begrenzungsflächen → das
    // Netz der Wand ändert sich nachweisbar.
    EXPECT_GE(scene_.effectiveUpdates(), 1);
    EXPECT_GT(scene_.wallMeshes().at(*wall).triangleCount(), triangles_before)
        << "Wand-Netz muss die Öffnung tragen";
    service_.unsubscribe(scene_);
}

// --- Dächer (LH-FA-ROF-*, slice-014b) ---

model::Roof sampleRoof(model::StoreyId storey) {
    model::Roof roof;
    roof.storey_id = storey;
    roof.type = model::RoofType::Sattel;
    roof.origin = {0.0, 0.0};
    roof.width_mm = 8000.0;
    roof.depth_mm = 6000.0;
    roof.base_z_mm = model::kDefaultStoreyHeightMm;
    roof.pitch_deg = model::kDefaultRoofPitchDeg;
    roof.overhang_mm = model::kDefaultRoofOverhangMm;
    return roof;
}

// LH-FA-ROF-001: ein angelegtes Dach folgt in der Szene (RoofChanged →
// Pull); idempotent bei identischer Mehrfach-Meldung; Entfernen leert.
TEST_F(ViewerSceneAk, LH_FA_ROF_001_DachFolgtInDerSzeneUndIdempotent) {
    scene_.loadAll();
    service_.subscribe(scene_);

    const auto roof = service_.addRoof(sampleRoof(eg_));
    ASSERT_TRUE(roof.has_value());
    EXPECT_EQ(scene_.roofMeshes().size(), 1U);
    EXPECT_GE(scene_.effectiveUpdates(), 1);

    // Identische erneute RoofChanged-Meldung → kein weiteres Update (MED-2).
    const int before = scene_.effectiveUpdates();
    scene_.onModelChanged({.op = driven::ModelChangeOp::RoofChanged,
                           .storey_id = eg_});
    EXPECT_EQ(scene_.effectiveUpdates(), before);

    // Entfernen → Szene ohne Dach, wirksames Update.
    EXPECT_TRUE(service_.removeRoof(*roof));
    EXPECT_TRUE(scene_.roofMeshes().empty());
    EXPECT_GT(scene_.effectiveUpdates(), before);
    service_.unsubscribe(scene_);
}

// LH-FA-ROF-004/005: Neigung/Überstand werden geklemmt; ein
// degenerierter Grundriss wird abgelehnt.
TEST_F(ViewerSceneAk, LH_FA_ROF_004_NeigungUeberstandGeklemmt) {
    const auto roof = service_.addRoof(sampleRoof(eg_));
    ASSERT_TRUE(roof.has_value());

    EXPECT_EQ(service_.setRoofPitch(*roof, 90.0).status,
              bcad::hexagon::ports::driving::ParamStatus::Clamped);
    EXPECT_NEAR(service_.roof(*roof).pitch_deg, model::kRoofPitchMaxDeg, 1e-9);
    EXPECT_EQ(service_.setRoofOverhang(*roof, 9000.0).status,
              bcad::hexagon::ports::driving::ParamStatus::Clamped);
    EXPECT_NEAR(service_.roof(*roof).overhang_mm, model::kRoofOverhangMaxMm,
                1e-9);

    model::Roof degenerate = sampleRoof(eg_);
    degenerate.width_mm = 0.0;
    EXPECT_FALSE(service_.addRoof(degenerate).has_value());
}

// LH-FA-ROF-001..003: Formwechsel (setRoofType) folgt in der Szene
// (Sattel 4 → Walm 6 Dreiecke); ein No-op-Wechsel (gleicher Typ) meldet
// nicht und ändert die Szene nicht (MEDIUM-1 Code-Review).
TEST_F(ViewerSceneAk, LH_FA_ROF_002_FormwechselSattelZuWalmFolgt) {
    service_.subscribe(scene_);
    const auto roof = service_.addRoof(sampleRoof(eg_));  // Sattel
    ASSERT_TRUE(roof.has_value());
    EXPECT_EQ(scene_.roofMeshes().at(*roof).triangleCount(), 4);  // Sattel

    service_.setRoofType(*roof, model::RoofType::Walm);
    EXPECT_EQ(scene_.roofMeshes().at(*roof).triangleCount(), 6);  // Walm

    const int after = scene_.effectiveUpdates();
    service_.setRoofType(*roof, model::RoofType::Walm);  // No-op (gleicher Typ)
    EXPECT_EQ(scene_.effectiveUpdates(), after);
    service_.unsubscribe(scene_);
}

// LH-FA-ROF-001: Dach-Mutation meldet RoofChanged, KEINE RoomsChanged
// (Dächer berühren die Raumerkennung nicht).
TEST_F(ViewerSceneAk, LH_FA_ROF_001_MeldetRoofChangedNichtRooms) {
    OpRecorder recorder;
    service_.subscribe(recorder);

    const auto roof = service_.addRoof(sampleRoof(eg_));
    ASSERT_TRUE(roof.has_value());
    service_.setRoofPitch(*roof, 40.0);

    EXPECT_EQ(recorder.count(driven::ModelChangeOp::RoofChanged), 2);  // add + pitch
    EXPECT_EQ(recorder.count(driven::ModelChangeOp::RoomsChanged), 0);
    service_.unsubscribe(recorder);
}

// --- Platten: Decken/Fundament (LH-FA-SLB-*/FND-*, slice-015b) ---

model::Footprint slabRect(double x0, double y0, double x1, double y1) {
    return model::Footprint{{{x0, y0}, {x1, y0}, {x1, y1}, {x0, y1}}};
}

model::Slab sampleSlab(model::StoreyId storey, model::SlabType type) {
    model::Slab slab;
    slab.storey_id = storey;
    slab.type = type;
    slab.footprint = slabRect(0.0, 0.0, 5000.0, 4000.0);
    slab.thickness_mm = model::kDefaultSlabThicknessMm;
    return slab;
}

double meshMin(const model::TriangleMesh& mesh, int axis) {
    double lo = std::numeric_limits<double>::max();
    for (std::size_t i = static_cast<std::size_t>(axis);
         i < mesh.positions.size(); i += 3) {
        lo = std::min(lo, mesh.positions[i]);
    }
    return lo;
}

// LH-FA-SLB-001: eine Decke folgt in der Szene (SlabChanged → Pull),
// liegt auf der Geschoss-Oberkante (base_z = Geschosshöhe); idempotent;
// Entfernen leert. Belegt die reale OCC-Extrusion + base_z-Translation.
TEST_F(ViewerSceneAk, LH_FA_SLB_001_DeckeFolgtUndAufGeschossOberkante) {
    scene_.loadAll();
    service_.subscribe(scene_);

    const auto slab = service_.addSlab(sampleSlab(eg_, model::SlabType::Decke));
    ASSERT_TRUE(slab.has_value());
    ASSERT_EQ(scene_.slabMeshes().size(), 1U);
    EXPECT_GE(scene_.effectiveUpdates(), 1);
    // Unterkante auf Geschoss-Oberkante (base_z-Translation, real OCC).
    EXPECT_NEAR(meshMin(scene_.slabMeshes().at(*slab), 2),
                model::kDefaultStoreyHeightMm, 0.5);

    const int before = scene_.effectiveUpdates();
    scene_.onModelChanged({.op = driven::ModelChangeOp::SlabChanged,
                           .storey_id = eg_});
    EXPECT_EQ(scene_.effectiveUpdates(), before);  // idempotent

    EXPECT_TRUE(service_.removeSlab(*slab));
    EXPECT_TRUE(scene_.slabMeshes().empty());
    service_.unsubscribe(scene_);
}

// LH-FA-FND-003: Bodenplatte — Oberkante auf Höhe 0 (Unterkante = −Dicke).
TEST_F(ViewerSceneAk, LH_FA_FND_003_BodenplatteOberkanteNull) {
    scene_.loadAll();
    service_.subscribe(scene_);
    const auto slab =
        service_.addSlab(sampleSlab(eg_, model::SlabType::Bodenplatte));
    ASSERT_TRUE(slab.has_value());
    EXPECT_NEAR(meshMin(scene_.slabMeshes().at(*slab), 2),
                -model::kDefaultSlabThicknessMm, 0.5);  // Unterkante −Dicke
    service_.unsubscribe(scene_);
}

// LH-FA-SLB-002/003 + Negative: Dicke geklemmt, degenerierter Grundriss
// abgelehnt, Ausschnitt ändert das Netz; SlabChanged, KEINE RoomsChanged.
TEST_F(ViewerSceneAk, LH_FA_SLB_002_DickeGeklemmtAusschnittUndMeldung) {
    OpRecorder recorder;
    service_.subscribe(recorder);

    const auto slab = service_.addSlab(sampleSlab(eg_, model::SlabType::Decke));
    ASSERT_TRUE(slab.has_value());

    EXPECT_EQ(service_.setSlabThickness(*slab, 9000.0).status,
              bcad::hexagon::ports::driving::ParamStatus::Clamped);
    EXPECT_NEAR(service_.slab(*slab).thickness_mm, model::kSlabThicknessMaxMm,
                1e-9);

    // Degenerierter Grundriss → abgelehnt.
    model::Slab bad = sampleSlab(eg_, model::SlabType::Decke);
    bad.footprint = slabRect(0.0, 0.0, 0.0, 0.0);
    EXPECT_FALSE(service_.addSlab(bad).has_value());

    // Ausschnitt setzen (LH-FA-SLB-003).
    EXPECT_TRUE(service_.addSlabCutout(*slab, slabRect(1000, 1000, 2000, 2000)));

    EXPECT_GT(recorder.count(driven::ModelChangeOp::SlabChanged), 0);
    EXPECT_EQ(recorder.count(driven::ModelChangeOp::RoomsChanged), 0);
    service_.unsubscribe(recorder);
}

// Negative (ii): nach unsubscribe folgt die Darstellung nicht mehr.
TEST_F(ViewerSceneAk, LH_FA_D3_002_Negative_UnsubscribeBeendetFolgen) {
    const auto wall = service_.addWall(eg_, seg(0, 0, 4000, 0));
    ASSERT_TRUE(wall.has_value());
    scene_.loadAll();
    service_.subscribe(scene_);
    service_.unsubscribe(scene_);

    service_.setWallThickness(*wall, 800.0);

    EXPECT_EQ(scene_.effectiveUpdates(), 0);
    EXPECT_NEAR(meshExtent(scene_.wallMeshes().at(*wall), 1),
                model::kDefaultWallThicknessMm, 0.5);
}

}  // namespace
