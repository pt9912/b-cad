#include "golden_model.h"

#include "hexagon/model/building.h"

namespace bcad::golden {

namespace model = hexagon::model;

model::Building goldenModel() {
    model::Building b;

    // 1 Geschoss (Höhe 2500 mm).
    b.storeys.push_back({model::StoreyId{1}, 2500.0});

    // 2 Wände (Aussen/Innen) — tragen IFC-Extrusionen, den 2D-Grundriss
    // (DXF/PDF/PNG) und die STEP/STL-Solids.
    b.walls.push_back({model::WallId{1}, model::StoreyId{1}, {0.0, 0.0},
                       {5000.0, 0.0}, 240.0, 2500.0, model::WallType::Aussen});
    b.walls.push_back({model::WallId{2}, model::StoreyId{1}, {0.0, 0.0},
                       {0.0, 4000.0}, 300.0, 2500.0, model::WallType::Innen});

    // Decke (Aufstandshöhe = Geschoss-Oberkante).
    model::Slab slab;
    slab.id = model::SlabId{1};
    slab.storey_id = model::StoreyId{1};
    slab.type = model::SlabType::Decke;
    slab.footprint.points = {
        {0.0, 0.0}, {5000.0, 0.0}, {5000.0, 4000.0}, {0.0, 4000.0}};
    slab.thickness_mm = 200.0;
    b.slabs.push_back(slab);

    // Sattel-Dach (wasserdichtes B-Rep-Solid im STEP, Netz im STL).
    model::Roof roof;
    roof.id = model::RoofId{1};
    roof.storey_id = model::StoreyId{1};
    roof.type = model::RoofType::Sattel;
    roof.origin = {0.0, 0.0};
    roof.width_mm = 5000.0;
    roof.depth_mm = 4000.0;
    roof.base_z_mm = 2500.0;
    roof.pitch_deg = 30.0;
    roof.overhang_mm = 500.0;
    roof.thickness_mm = 200.0;
    b.roofs.push_back(roof);

    // Gerade Treppe (12 Stufen als analytische Box-Solids; Geländer render-only).
    model::Stair stair;
    stair.id = model::StairId{1};
    stair.from_storey_id = model::StoreyId{1};
    stair.to_storey_id = model::StoreyId{2};
    stair.type = model::StairType::Gerade;
    stair.start = {0.0, 0.0};
    stair.width_mm = 1000.0;
    stair.step_count = 12;
    stair.tread_mm = 280.0;
    b.stairs.push_back(stair);

    // Sichtbare Ebene + Hilfslinie — erscheint im 2D-Grundriss-Export
    // (DXF/PDF/PNG). Die Sichtbarkeit hängt an der Ebene (LH-FA-DRW-005.a):
    // `visible = true`, sonst filtern die 2D-Exporter die Hilfslinie weg.
    model::Layer layer;
    layer.id = model::LayerId{1};
    layer.name = "Achsen";
    layer.visible = true;
    b.layers.push_back(layer);

    model::GuideLine guide;
    guide.id = model::GuideLineId{1};
    guide.storey_id = model::StoreyId{1};
    guide.layer_id = model::LayerId{1};
    guide.segment = {{1000.0, 2000.0}, {4000.0, 2500.0}};
    b.guide_lines.push_back(guide);

    return b;
}

}  // namespace bcad::golden
