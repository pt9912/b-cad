// slice-047b (LH-FA-BLD-002/003): der GUI-Weg „Datei → Speichern/Öffnen"
// über den **Handler**, nicht über den modalen Dialog (Plan-Review MED-1 —
// `QFileDialog` lebt im coverage-ausgenommenen main und wird bewusst nicht
// getestet). Geprüft wird der real beobachtbare Teil: ein gespeichertes
// Projekt wird über `openProject` geladen, der Service ersetzt das Modell,
// und die **abonnierten** Sichten spiegeln den geladenen Stand — der
// per-op-Viewer nur, weil `ModelReplaced` ihn voll neu aufbauen lässt
// (Plan-Review HIGH-1).
//
// Echter Adapter-Pfad (welle-2-Lehre slice-015b): echte SQLite-Datei, echter
// OCC-Geometrie-Adapter — kein Stub als einziges Orakel.

#include <gtest/gtest.h>

#include <QApplication>

#include <cstdlib>
#include <filesystem>
#include <string>

#include "adapters/geometry/occ_geometry_adapter.h"
#include "adapters/persistence/sqlite_project_repository.h"
#include "adapters/ui/command/view_model_mesh_source.h"
#include "adapters/ui/command/edit_drawing_guide_line_sink.h"
#include "adapters/ui/command/edit_structure_wall_sink.h"
#include "adapters/ui/view/canvas_widget.h"
#include "adapters/ui/view/viewer_scene.h"
#include "hexagon/model/layer.h"
#include "hexagon/model/segment.h"
#include "adapters/ui/command/project_menu_handler.h"
#include "hexagon/services/manage_project.h"
#include "hexagon/services/project_session.h"
#include "hexagon/services/structure_edit_service.h"

namespace {

namespace model = bcad::hexagon::model;
namespace services = bcad::hexagon::services;
namespace fs = std::filesystem;

model::Segment seg(double x1, double y1, double x2, double y2) {
    return model::Segment{model::Point2D{x1, y1}, model::Point2D{x2, y2}};
}

fs::path tempProjectPath(const std::string& name) {
    return fs::temp_directory_path() / name;
}

// Speichern → Öffnen über die Handler; die abonnierte Szene spiegelt den
// geladenen Stand (Full-Refresh statt stale Netze).
TEST(ProjectOpenHandler_LH_FA_BLD_003, SavedProjectOpensAndViewsFollow) {
    const bcad::adapters::persistence::SqliteProjectRepository repository;
    const fs::path path = tempProjectPath("bcad_047b_roundtrip.bcad");
    fs::remove(path);

    // --- Quell-Projekt bauen und speichern (Handler-Weg) ---
    bcad::adapters::geometry::OccGeometryAdapter geometry;
    {
        services::StructureEditService source(geometry);
        const auto eg = source.building().storeys.front().id;
        ASSERT_TRUE(source.addWall(eg, seg(0, 0, 4000, 0)).has_value());
        ASSERT_TRUE(source.addWall(eg, seg(4000, 0, 4000, 3000)).has_value());
        services::saveProject(repository, source.building(), path);
    }
    ASSERT_TRUE(fs::exists(path)) << "Speichern muss eine Datei erzeugen";

    // --- Ziel-Service mit ABWEICHENDEM Stand + abonnierter Szene ---
    // Bewusst MEHR Wände als das geladene Projekt (3 statt 2): nur so ist die
    // Zusicherung diskriminierend. Bliebe der Full-Refresh aus, hielte die
    // Szene weiterhin 3 Netze — die Id-Mengen überlappen sich (beide Services
    // vergeben ab 1), ein Vergleich einzelner Ids würde also nichts beweisen.
    services::StructureEditService target(geometry);
    const auto other_storey = target.building().storeys.front().id;
    ASSERT_TRUE(target.addWall(other_storey, seg(0, 0, 1000, 0)).has_value());
    ASSERT_TRUE(target.addWall(other_storey, seg(1000, 0, 1000, 800)).has_value());
    const auto stale_wall = target.addWall(other_storey, seg(1000, 800, 0, 800));
    ASSERT_TRUE(stale_wall.has_value());

    bcad::adapters::ui::command::ViewModelMeshSource mesh_source(target);
    bcad::adapters::ui::view::ViewerScene scene(mesh_source);
    target.subscribe(scene);
    scene.loadAll();
    ASSERT_EQ(scene.wallMeshes().size(), 3U) << "Ausgangs-Szene: drei Wände";

    // --- Öffnen über den Handler ---
    services::openProject(target, repository, path);

    // Modell entspricht dem gespeicherten Stand.
    ASSERT_EQ(target.building().walls.size(), 2U);

    // Die Szene spiegelt GENAU ihn — OHNE ModelReplaced-Fall stünden hier noch
    // drei Netze (der per-op-Pfad pullt nur gemeldete Elemente).
    EXPECT_EQ(scene.wallMeshes().size(), 2U)
        << "ModelReplaced muss den Viewer voll neu aufbauen";
    EXPECT_EQ(scene.wallMeshes().count(*stale_wall), 0U)
        << "das Netz der dritten (nur im alten Stand vorhandenen) Wand muss weg sein";
    for (const auto& w : target.building().walls) {
        EXPECT_EQ(scene.wallMeshes().count(w.id), 1U)
            << "jede geladene Wand muss ein Netz haben";
    }

    target.unsubscribe(scene);
    fs::remove(path);
}

// Fehlerpfad des Öffnen-Handlers: nicht existente Datei → neutraler Wurf,
// Modell + Szene unverändert (der GUI-Aufrufer zeigt das als Dialog).
TEST(ProjectOpenHandler_LH_FA_BLD_003, MissingFileThrowsAndLeavesStateIntact) {
    const bcad::adapters::persistence::SqliteProjectRepository repository;
    bcad::adapters::geometry::OccGeometryAdapter geometry;
    services::StructureEditService target(geometry);
    const auto eg = target.building().storeys.front().id;
    ASSERT_TRUE(target.addWall(eg, seg(0, 0, 2000, 0)).has_value());

    bcad::adapters::ui::command::ViewModelMeshSource mesh_source(target);
    bcad::adapters::ui::view::ViewerScene scene(mesh_source);
    target.subscribe(scene);
    scene.loadAll();

    const auto walls_before = target.building().walls.size();
    const auto meshes_before = scene.wallMeshes().size();

    EXPECT_THROW(
        services::openProject(target, repository,
                              tempProjectPath("bcad_047b_gibt_es_nicht.bcad")),
        std::runtime_error);

    EXPECT_EQ(target.building().walls.size(), walls_before);
    EXPECT_EQ(scene.wallMeshes().size(), meshes_before);
    target.unsubscribe(scene);
}


// MEDIUM-7 + Verify-Finding B4: die HIGH-3-Auflösung selbst — nach dem Öffnen
// zeigt der Canvas das GELADENE Geschoss und eine Hilfslinie landet auf dem
// geladenen Stand. Ohne die Neu-Auflösung stünden Canvas und Sink auf den Ids des
// alten Modells: der Canvas filterte jede Plan-Zeile weg (leer) und
// `addGuideLine` würde abgelehnt.
TEST(ProjectOpenHandler_LH_FA_BLD_003, CanvasAndSinkFollowLoadedIds) {
    int argc = 1;
    char arg0[] = "bcad_adapter_tests";
    char* argv[] = {static_cast<char*>(arg0), nullptr};
    const QApplication app(argc, static_cast<char**>(argv));

    const bcad::adapters::persistence::SqliteProjectRepository repository;
    const fs::path path = tempProjectPath("bcad_047b_canvas.bcad");
    fs::remove(path);

    bcad::adapters::geometry::OccGeometryAdapter geometry;
    model::StoreyId saved_storey{};
    model::LayerId saved_layer{};
    {
        services::StructureEditService source(geometry);
        saved_storey = source.addStorey(2800.0);  // NICHT das Default-Geschoss
        ASSERT_TRUE(source.addWall(saved_storey, seg(0, 0, 5000, 0)).has_value());
        model::Layer l;
        l.name = "Achsen";
        const auto lid = source.addLayer(l);
        ASSERT_TRUE(lid.has_value());
        saved_layer = *lid;
        services::saveProject(repository, source.building(), path);
    }

    services::StructureEditService target(geometry);
    const auto stale_storey = target.building().storeys.front().id;
    bcad::adapters::ui::command::EditDrawingGuideLineSink sink(
        target, stale_storey, model::LayerId{});
    // slice-058: die Wand-Senke haengt am DEMSELBEN veralteten Geschoss — ohne
    // Neu-Aufloesung wuerfe `addWall` nach jedem Laden (unbekannte Geschoss-Id).
    bcad::adapters::ui::command::EditStructureWallSink wall_sink(
        target, stale_storey, {});  // NICHT const: setTarget zieht das Ziel nach
    bcad::adapters::ui::view::CanvasWidget canvas(
        [&target]() { return target.planView(); },
        [&sink](model::Point2D a, model::Point2D b) {
            return sink.addGuideLine(a, b);
        },
        [&wall_sink](model::Point2D a, model::Point2D b) {
            wall_sink.addWall(a, b);
        },
        static_cast<int>(stale_storey));

    // Die **echten** Senken des Composition-Root (main.cpp verdrahtet dieselben
    // zwei Callables) — NICHT im Test nachgestellt. Vorher rief der Test die
    // Setter selbst; damit prüfte er die Setter, nicht die Entscheidung, sie
    // nach dem Laden zu rufen, und der Verlust des Produktions-Aufrufs blieb
    // grün (Verify-Finding B4, Gegenprobe CP-5).
    const services::DrawingTargetSinks sinks{
        [&canvas](model::StoreyId s) {
            canvas.setActiveStorey(static_cast<int>(s));
        },
        [&sink, &wall_sink](model::StoreyId s, model::LayerId l) {
            sink.setTarget(s, l);
            // slice-058: dieselbe Senken-Zeile wie in `main.cpp` — das Wand-Ziel
            // folgt demselben Geschoss.
            wall_sink.setTarget(s);
        },
    };
    const auto resolution = services::openProject(target, repository, path, sinks);

    EXPECT_EQ(resolution, services::DrawingTargetResolution::Resolved)
        << "geladenes Projekt hat Geschoss UND Ebene";
    const auto loaded_storey = target.building().storeys.front().id;

    // Orakel: die Hilfslinie wird auf dem GELADENEN Stand angenommen …
    const auto guide = sink.addGuideLine(model::Point2D{0.0, 0.0},
                                         model::Point2D{1000.0, 0.0});
    ASSERT_TRUE(guide.has_value())
        << "ohne Neu-Aufloesung wuerde der Sink die Ebene/das Geschoss ablehnen";
    ASSERT_EQ(target.building().guide_lines.size(), 1U);
    EXPECT_EQ(target.building().guide_lines.front().storey_id, loaded_storey);
    EXPECT_EQ(target.building().guide_lines.front().layer_id, saved_layer);

    // … und die geladene Wand des zweiten Geschosses ist da (Beleg, dass wirklich
    // das gespeicherte Projekt im Service liegt und nicht der Ausgangs-Stand).
    ASSERT_EQ(target.building().storeys.size(), 2U);
    EXPECT_EQ(target.building().walls.size(), 1U);
    EXPECT_EQ(target.building().walls.front().storey_id, saved_storey);

    // slice-058: … und die WAND wird auf dem geladenen Stand angenommen. Ohne
    // den Ziel-Nachzug wuerfe `addWall` (unbekannte Geschoss-Id); die Barriere
    // faenge es, aber der Benutzer bekaeme statt einer Wand einen Hinweis.
    const auto drawn_wall = wall_sink.addWall(model::Point2D{0.0, 0.0},
                                              model::Point2D{2000.0, 0.0});
    ASSERT_TRUE(drawn_wall.has_value())
        << "ohne Neu-Aufloesung wuerde addWall auf der veralteten Id werfen";
    ASSERT_EQ(target.building().walls.size(), 2U);
    EXPECT_EQ(target.building().walls.back().storey_id, loaded_storey);

    // Grenze, ehrlich benannt: `CanvasWidget` veroeffentlicht sein aktives
    // Geschoss nicht — `setActiveStorey` ist hier nur auf "nimmt den Wert
    // ohne Wurf an" geprueft. Das sichtbare Verhalten (leerer Canvas ohne
    // Neu-Aufloesung) haengt am Paint-Pfad und ist display-gebunden.
    fs::remove(path);
}

// MEDIUM-8: Öffnen ist LESEND — ein Projekt ohne Zeichen-Ebene bleibt ohne.
// Früher legte der GUI-Pfad hier eine Ebene „Canvas" an; der Stand im Speicher
// wich dann vom Dateiinhalt ab und ein erneutes Speichern schrieb sie mit.
TEST(ProjectOpenHandler_LH_FA_BLD_003, OpeningDoesNotMutateLoadedModel) {
    const bcad::adapters::persistence::SqliteProjectRepository repository;
    const fs::path path = tempProjectPath("bcad_047b_readonly.bcad");
    fs::remove(path);

    bcad::adapters::geometry::OccGeometryAdapter geometry;
    {
        services::StructureEditService source(geometry);
        const auto eg = source.building().storeys.front().id;
        ASSERT_TRUE(source.addWall(eg, seg(0, 0, 3000, 0)).has_value());
        ASSERT_TRUE(source.building().layers.empty())
            << "Fixture-Annahme: das Quell-Projekt hat keine Ebene";
        services::saveProject(repository, source.building(), path);
    }

    services::StructureEditService target(geometry);
    services::openProject(target, repository, path);

    EXPECT_TRUE(target.building().layers.empty())
        << "Oeffnen darf keine Ebene anlegen (LH-FA-BLD-003: vollstaendig "
           "wiederhergestellt, nicht ergaenzt)";

    // Roundtrip-Beleg: erneutes Speichern schreibt keinen Zusatz-Inhalt.
    const fs::path again = tempProjectPath("bcad_047b_readonly_2.bcad");
    fs::remove(again);
    services::saveProject(repository, target.building(), again);
    const model::Building reread = repository.load(again);
    EXPECT_TRUE(reread.layers.empty());
    EXPECT_EQ(reread.walls.size(), target.building().walls.size());

    fs::remove(path);
    fs::remove(again);
}


// --- slice-052a, Orakel-Zeile 10 -----------------------------------------
//
// "Speichern" schreibt in die GEMERKTE Datei — belegt am ECHTEN Repository,
// nicht an einem Doppel: oeffnen, aendern, `save()` ohne Pfad-Angabe, und der
// Dateiinhalt traegt die Aenderung. Der Pfad kommt allein aus der Sitzung, die
// ihn beim Oeffnen gemerkt hat.
TEST(ProjectSessionRoundTrip, SpeichernSchreibtInDieGemerkteDatei) {
    const fs::path path =
        fs::temp_directory_path() / "bcad_052a_gemerkte_datei.bcad";
    fs::remove(path);

    const bcad::adapters::persistence::SqliteProjectRepository repository;
    bcad::adapters::geometry::OccGeometryAdapter geometry;

    // Quelle: ein Projekt mit einer Wand, auf die Platte geschrieben.
    {
        services::StructureEditService source(geometry);
        const auto storey = source.building().storeys.front().id;
        source.addWall(storey, seg(0, 0, 4000, 0));
        services::saveProject(repository, source.building(), path);
    }

    // Sitzung: oeffnen (merkt den Pfad), aendern, ueber den Handler speichern.
    services::StructureEditService target(geometry);
    services::ProjectSessionService session(target.building());
    services::ManageProjectService project(target, repository, &session);
    bcad::adapters::ui::command::ProjectMenuHandler handler(
        project, session,
        [&target]() -> const model::Building& { return target.building(); }, {});

    ASSERT_NO_THROW((void)handler.open(path));
    ASSERT_EQ(session.saveTarget().kind,
              bcad::hexagon::ports::driving::SaveTargetKind::KnownPath)
        << "das Oeffnen merkt den Pfad";

    const auto storey = target.building().storeys.front().id;
    const auto neue_wand = target.addWall(storey, seg(0, 0, 0, 3000));
    ASSERT_TRUE(neue_wand.has_value());
    ASSERT_TRUE(session.isDirty(target.building()));

    ASSERT_TRUE(handler.save()) << "ohne Pfad-Argument in die gemerkte Datei";

    // Der Dateiinhalt traegt die Aenderung.
    const model::Building geladen = repository.load(path);
    EXPECT_EQ(geladen.walls.size(), 2U)
        << "die zweite Wand ist in der GEMERKTEN Datei gelandet";
    EXPECT_FALSE(session.isDirty(target.building()))
        << "nach dem Speichern ist die Sitzung sauber";

    fs::remove(path);
}

}  // namespace
