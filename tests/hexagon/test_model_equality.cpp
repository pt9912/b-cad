// slice-052a, Orakel-Zeile 3: **Feld-Vollstaendigkeit der Gleichheit**.
//
// Der Sitzungs-Zustand ist ein WERT-VERGLEICH (kein Flag). Uebersieht ein
// `operator==` ein Feld, meldet die Sitzung "nichts geaendert", waehrend der
// Benutzer sehr wohl etwas geaendert hat — und die Warnung vor Datenverlust
// bleibt aus, WAEHREND ALLE ANDEREN ORAKEL GRUEN STEHEN. Das ist §9 R1.
//
// Deshalb prueft diese Datei je Struct-Typ **jedes Feld einzeln**: eine Kopie
// mit genau einem geaenderten Feld muss ungleich sein. Die Vergleiche sind
// `= default` (Pflicht, kein Stil — nur der compiler-generierte Operator nimmt
// kuenftige Felder automatisch auf); die Zeilen hier sind die Gegenprobe dazu:
// wer einen handgeschriebenen Operator einsetzt und ein Feld vergisst, wird
// hier rot.
//
// Erfasst sind alle 13 Typen: die verschachtelten Koordinaten-Traeger
// Point2D/Segment/Footprint (der eigentliche Ort des Risikos — die
// Element-Typen ersetzen ihre Punkt-Felder als GANZES), die neun Element-Typen
// und `Building` selbst.

#include <optional>
#include <string>
#include <vector>

#include <gtest/gtest.h>

#include "hexagon/model/building.h"
#include "hexagon/model/footprint.h"
#include "hexagon/model/guide_line.h"
#include "hexagon/model/layer.h"
#include "hexagon/model/material.h"
#include "hexagon/model/opening.h"
#include "hexagon/model/point2d.h"
#include "hexagon/model/roof.h"
#include "hexagon/model/segment.h"
#include "hexagon/model/slab.h"
#include "hexagon/model/stair.h"
#include "hexagon/model/storey.h"
#include "hexagon/model/wall.h"

namespace {

namespace model = bcad::hexagon::model;

// Kern-Muster jeder Zeile: Original == Kopie, aber Original != (Kopie mit EINEM
// geaenderten Feld). `feld` benennt das Feld in der Fehlermeldung.
#define ERWARTE_FELD_ERFASST(original, mutation, feld)              \
    do {                                                            \
        auto kopie = (original);                                    \
        EXPECT_TRUE((original) == kopie) << "Selbstgleichheit";      \
        mutation(kopie);                                            \
        EXPECT_FALSE((original) == kopie)                           \
            << "Feld nicht im operator== erfasst: " << (feld);      \
    } while (false)

// --- die verschachtelten Koordinaten-Traeger ------------------------------

TEST(ModelEquality, Point2DErfasstBeideKoordinaten) {
    const model::Point2D p{1.0, 2.0};
    ERWARTE_FELD_ERFASST(p, [](auto& k) { k.x_mm = 9.0; }, "Point2D::x_mm");
    ERWARTE_FELD_ERFASST(p, [](auto& k) { k.y_mm = 9.0; }, "Point2D::y_mm");
}

TEST(ModelEquality, SegmentErfasstBeideEndpunkte) {
    const model::Segment s{{1.0, 2.0}, {3.0, 4.0}};
    ERWARTE_FELD_ERFASST(s, [](auto& k) { k.start.x_mm = 9.0; }, "Segment::start");
    ERWARTE_FELD_ERFASST(s, [](auto& k) { k.end.y_mm = 9.0; }, "Segment::end");
}

TEST(ModelEquality, FootprintErfasstPunktlisteUndPunktInhalt) {
    model::Footprint f;
    f.points = {{0.0, 0.0}, {1000.0, 0.0}, {1000.0, 1000.0}};
    ERWARTE_FELD_ERFASST(
        f, [](auto& k) { k.points.pop_back(); }, "Footprint::points (Laenge)");
    ERWARTE_FELD_ERFASST(
        f, [](auto& k) { k.points.front().x_mm = 5.0; },
        "Footprint::points (Inhalt)");
}

// --- die neun Element-Typen ------------------------------------------------

TEST(ModelEquality, StoreyErfasstAlleFelder) {
    const model::Storey s{model::StoreyId{1}, 2500.0};
    ERWARTE_FELD_ERFASST(s, [](auto& k) { k.id = model::StoreyId{2}; }, "Storey::id");
    ERWARTE_FELD_ERFASST(s, [](auto& k) { k.height_mm = 9.0; }, "Storey::height_mm");
}

model::Wall beispielWand() {
    model::Wall w;
    w.id = model::WallId{1};
    w.storey_id = model::StoreyId{1};
    w.start = {0.0, 0.0};
    w.end = {4000.0, 0.0};
    w.thickness_mm = 240.0;
    w.height_mm = 2500.0;
    w.type = model::WallType::Innen;
    w.material_id = model::MaterialId{3};
    return w;
}

TEST(ModelEquality, WallErfasstAlleAchtFelder) {
    const model::Wall w = beispielWand();
    ERWARTE_FELD_ERFASST(w, [](auto& k) { k.id = model::WallId{2}; }, "Wall::id");
    ERWARTE_FELD_ERFASST(w, [](auto& k) { k.storey_id = model::StoreyId{2}; }, "Wall::storey_id");
    ERWARTE_FELD_ERFASST(w, [](auto& k) { k.start.y_mm = 1.0; }, "Wall::start");
    ERWARTE_FELD_ERFASST(w, [](auto& k) { k.end.x_mm = 1.0; }, "Wall::end");
    ERWARTE_FELD_ERFASST(w, [](auto& k) { k.thickness_mm = 115.0; }, "Wall::thickness_mm");
    ERWARTE_FELD_ERFASST(w, [](auto& k) { k.height_mm = 3000.0; }, "Wall::height_mm");
    ERWARTE_FELD_ERFASST(w, [](auto& k) { k.type = model::WallType::Aussen; }, "Wall::type");
    ERWARTE_FELD_ERFASST(w, [](auto& k) { k.material_id.reset(); }, "Wall::material_id");
}

model::Opening beispielOeffnung() {
    model::Opening o;
    o.id = model::OpeningId{1};
    o.wall_id = model::WallId{1};
    o.kind = model::OpeningKind::Door;
    o.offset_mm = 500.0;
    o.width_mm = 900.0;
    o.height_mm = 2000.0;
    o.sill_height_mm = 0.0;
    o.swing = model::SwingDirection::Left;
    return o;
}

TEST(ModelEquality, OpeningErfasstAlleAchtFelder) {
    const model::Opening o = beispielOeffnung();
    ERWARTE_FELD_ERFASST(o, [](auto& k) { k.id = model::OpeningId{2}; }, "Opening::id");
    ERWARTE_FELD_ERFASST(o, [](auto& k) { k.wall_id = model::WallId{2}; }, "Opening::wall_id");
    ERWARTE_FELD_ERFASST(o, [](auto& k) { k.kind = model::OpeningKind::Window; }, "Opening::kind");
    ERWARTE_FELD_ERFASST(o, [](auto& k) { k.offset_mm = 1.0; }, "Opening::offset_mm");
    ERWARTE_FELD_ERFASST(o, [](auto& k) { k.width_mm = 1.0; }, "Opening::width_mm");
    ERWARTE_FELD_ERFASST(o, [](auto& k) { k.height_mm = 1.0; }, "Opening::height_mm");
    ERWARTE_FELD_ERFASST(o, [](auto& k) { k.sill_height_mm = 800.0; }, "Opening::sill_height_mm");
    ERWARTE_FELD_ERFASST(o, [](auto& k) { k.swing = model::SwingDirection::Right; }, "Opening::swing");
}

model::Roof beispielDach() {
    model::Roof r;
    r.id = model::RoofId{1};
    r.storey_id = model::StoreyId{1};
    r.type = model::RoofType::Sattel;
    r.origin = {0.0, 0.0};
    r.width_mm = 8000.0;
    r.depth_mm = 6000.0;
    r.base_z_mm = 2500.0;
    r.pitch_deg = 30.0;
    r.overhang_mm = 500.0;
    r.thickness_mm = 200.0;
    r.material_id = model::MaterialId{2};
    return r;
}

TEST(ModelEquality, RoofErfasstAlleElfFelder) {
    const model::Roof r = beispielDach();
    ERWARTE_FELD_ERFASST(r, [](auto& k) { k.id = model::RoofId{2}; }, "Roof::id");
    ERWARTE_FELD_ERFASST(r, [](auto& k) { k.storey_id = model::StoreyId{2}; }, "Roof::storey_id");
    ERWARTE_FELD_ERFASST(r, [](auto& k) { k.type = model::RoofType::Pult; }, "Roof::type");
    ERWARTE_FELD_ERFASST(r, [](auto& k) { k.origin.x_mm = 1.0; }, "Roof::origin");
    ERWARTE_FELD_ERFASST(r, [](auto& k) { k.width_mm = 1.0; }, "Roof::width_mm");
    ERWARTE_FELD_ERFASST(r, [](auto& k) { k.depth_mm = 1.0; }, "Roof::depth_mm");
    ERWARTE_FELD_ERFASST(r, [](auto& k) { k.base_z_mm = 1.0; }, "Roof::base_z_mm");
    ERWARTE_FELD_ERFASST(r, [](auto& k) { k.pitch_deg = 45.0; }, "Roof::pitch_deg");
    ERWARTE_FELD_ERFASST(r, [](auto& k) { k.overhang_mm = 1.0; }, "Roof::overhang_mm");
    ERWARTE_FELD_ERFASST(r, [](auto& k) { k.thickness_mm = 1.0; }, "Roof::thickness_mm");
    ERWARTE_FELD_ERFASST(r, [](auto& k) { k.material_id.reset(); }, "Roof::material_id");
}

model::Slab beispielPlatte() {
    model::Slab s;
    s.id = model::SlabId{1};
    s.storey_id = model::StoreyId{1};
    s.type = model::SlabType::Decke;
    s.footprint.points = {{0.0, 0.0}, {1000.0, 0.0}, {1000.0, 1000.0}};
    s.thickness_mm = 200.0;
    model::Footprint cutout;
    cutout.points = {{100.0, 100.0}, {200.0, 100.0}, {200.0, 200.0}};
    s.cutouts.push_back(cutout);
    s.material_id = model::MaterialId{4};
    return s;
}

TEST(ModelEquality, SlabErfasstAlleSiebenFelder) {
    const model::Slab s = beispielPlatte();
    ERWARTE_FELD_ERFASST(s, [](auto& k) { k.id = model::SlabId{2}; }, "Slab::id");
    ERWARTE_FELD_ERFASST(s, [](auto& k) { k.storey_id = model::StoreyId{2}; }, "Slab::storey_id");
    ERWARTE_FELD_ERFASST(s, [](auto& k) { k.type = model::SlabType::Fundament; }, "Slab::type");
    ERWARTE_FELD_ERFASST(s, [](auto& k) { k.footprint.points.front().x_mm = 5.0; }, "Slab::footprint");
    ERWARTE_FELD_ERFASST(s, [](auto& k) { k.thickness_mm = 1.0; }, "Slab::thickness_mm");
    ERWARTE_FELD_ERFASST(s, [](auto& k) { k.cutouts.clear(); }, "Slab::cutouts");
    ERWARTE_FELD_ERFASST(s, [](auto& k) { k.material_id.reset(); }, "Slab::material_id");
}

model::Stair beispielTreppe() {
    model::Stair s;
    s.id = model::StairId{1};
    s.from_storey_id = model::StoreyId{1};
    s.to_storey_id = model::StoreyId{2};
    s.type = model::StairType::Gerade;
    s.start = {0.0, 0.0};
    s.width_mm = 1000.0;
    s.step_count = 15;
    s.tread_mm = 280.0;
    return s;
}

TEST(ModelEquality, StairErfasstAlleAchtFelder) {
    const model::Stair s = beispielTreppe();
    ERWARTE_FELD_ERFASST(s, [](auto& k) { k.id = model::StairId{2}; }, "Stair::id");
    ERWARTE_FELD_ERFASST(s, [](auto& k) { k.from_storey_id = model::StoreyId{3}; }, "Stair::from_storey_id");
    ERWARTE_FELD_ERFASST(s, [](auto& k) { k.to_storey_id = model::StoreyId{3}; }, "Stair::to_storey_id");
    ERWARTE_FELD_ERFASST(s, [](auto& k) { k.start.y_mm = 1.0; }, "Stair::start");
    ERWARTE_FELD_ERFASST(s, [](auto& k) { k.width_mm = 1.0; }, "Stair::width_mm");
    ERWARTE_FELD_ERFASST(s, [](auto& k) { k.step_count = 16; }, "Stair::step_count");
    ERWARTE_FELD_ERFASST(s, [](auto& k) { k.tread_mm = 1.0; }, "Stair::tread_mm");
    // `type` traegt heute nur einen Wert; die Zeile bleibt trotzdem stehen,
    // sobald ein zweiter dazukommt — `= default` nimmt ihn automatisch auf.
}

model::Material beispielMaterial() {
    model::Material m;
    m.id = model::MaterialId{1};
    m.name = "Beton";
    m.category = "Massiv";
    m.u_value = 2.1;
    m.cost_per_m2 = 30.0;
    m.cost_per_m3 = 120.0;
    m.color_hex = std::string{"#808080"};
    m.texture_path = std::string{"beton.png"};
    return m;
}

TEST(ModelEquality, MaterialErfasstAlleAchtFelder) {
    const model::Material m = beispielMaterial();
    ERWARTE_FELD_ERFASST(m, [](auto& k) { k.id = model::MaterialId{2}; }, "Material::id");
    ERWARTE_FELD_ERFASST(m, [](auto& k) { k.name = "Holz"; }, "Material::name");
    ERWARTE_FELD_ERFASST(m, [](auto& k) { k.category = "Leicht"; }, "Material::category");
    ERWARTE_FELD_ERFASST(m, [](auto& k) { k.u_value.reset(); }, "Material::u_value");
    ERWARTE_FELD_ERFASST(m, [](auto& k) { k.cost_per_m2.reset(); }, "Material::cost_per_m2");
    ERWARTE_FELD_ERFASST(m, [](auto& k) { k.cost_per_m3.reset(); }, "Material::cost_per_m3");
    ERWARTE_FELD_ERFASST(m, [](auto& k) { k.color_hex.reset(); }, "Material::color_hex");
    ERWARTE_FELD_ERFASST(m, [](auto& k) { k.texture_path.reset(); }, "Material::texture_path");
}

TEST(ModelEquality, LayerErfasstAlleFuenfFelder) {
    model::Layer l;
    l.id = model::LayerId{1};
    l.name = "Achsen";
    l.visible = true;
    l.locked = false;
    l.color_hex = std::string{"#ff0000"};
    const model::Layer original = l;
    ERWARTE_FELD_ERFASST(original, [](auto& k) { k.id = model::LayerId{2}; }, "Layer::id");
    ERWARTE_FELD_ERFASST(original, [](auto& k) { k.name = "Raster"; }, "Layer::name");
    ERWARTE_FELD_ERFASST(original, [](auto& k) { k.visible = false; }, "Layer::visible");
    ERWARTE_FELD_ERFASST(original, [](auto& k) { k.locked = true; }, "Layer::locked");
    ERWARTE_FELD_ERFASST(original, [](auto& k) { k.color_hex.reset(); }, "Layer::color_hex");
}

TEST(ModelEquality, GuideLineErfasstAlleVierFelder) {
    model::GuideLine g;
    g.id = model::GuideLineId{1};
    g.storey_id = model::StoreyId{1};
    g.layer_id = model::LayerId{1};
    g.segment = {{0.0, 0.0}, {1000.0, 0.0}};
    const model::GuideLine original = g;
    ERWARTE_FELD_ERFASST(original, [](auto& k) { k.id = model::GuideLineId{2}; }, "GuideLine::id");
    ERWARTE_FELD_ERFASST(original, [](auto& k) { k.storey_id = model::StoreyId{2}; }, "GuideLine::storey_id");
    ERWARTE_FELD_ERFASST(original, [](auto& k) { k.layer_id = model::LayerId{2}; }, "GuideLine::layer_id");
    ERWARTE_FELD_ERFASST(original, [](auto& k) { k.segment.end.x_mm = 5.0; }, "GuideLine::segment");
}

// --- Building: alle neun Sammlungen ---------------------------------------

model::Building beispielGebaeude() {
    model::Building b;
    b.storeys.push_back({model::StoreyId{1}, 2500.0});
    b.walls.push_back(beispielWand());
    b.openings.push_back(beispielOeffnung());
    b.roofs.push_back(beispielDach());
    b.slabs.push_back(beispielPlatte());
    b.stairs.push_back(beispielTreppe());
    b.materials.push_back(beispielMaterial());
    model::Layer l;
    l.id = model::LayerId{1};
    l.name = "Achsen";
    b.layers.push_back(l);
    model::GuideLine g;
    g.id = model::GuideLineId{1};
    g.segment = {{0.0, 0.0}, {1000.0, 0.0}};
    b.guide_lines.push_back(g);
    return b;
}

TEST(ModelEquality, BuildingErfasstAlleNeunSammlungen) {
    const model::Building b = beispielGebaeude();
    ERWARTE_FELD_ERFASST(b, [](auto& k) { k.storeys.clear(); }, "Building::storeys");
    ERWARTE_FELD_ERFASST(b, [](auto& k) { k.walls.clear(); }, "Building::walls");
    ERWARTE_FELD_ERFASST(b, [](auto& k) { k.openings.clear(); }, "Building::openings");
    ERWARTE_FELD_ERFASST(b, [](auto& k) { k.roofs.clear(); }, "Building::roofs");
    ERWARTE_FELD_ERFASST(b, [](auto& k) { k.slabs.clear(); }, "Building::slabs");
    ERWARTE_FELD_ERFASST(b, [](auto& k) { k.stairs.clear(); }, "Building::stairs");
    ERWARTE_FELD_ERFASST(b, [](auto& k) { k.materials.clear(); }, "Building::materials");
    ERWARTE_FELD_ERFASST(b, [](auto& k) { k.layers.clear(); }, "Building::layers");
    ERWARTE_FELD_ERFASST(b, [](auto& k) { k.guide_lines.clear(); }, "Building::guide_lines");
}

// Und die TIEFE: eine Aenderung INNERHALB eines Elements muss durchschlagen —
// sonst waere `Building`-Gleichheit ein Laengen-Vergleich.
TEST(ModelEquality, BuildingErfasstAenderungenInnerhalbDerElemente) {
    const model::Building b = beispielGebaeude();
    ERWARTE_FELD_ERFASST(
        b, [](auto& k) { k.walls.front().thickness_mm = 115.0; },
        "Building -> Wall::thickness_mm");
    ERWARTE_FELD_ERFASST(
        b, [](auto& k) { k.guide_lines.front().segment.end.x_mm = 5.0; },
        "Building -> GuideLine -> Segment -> Point2D");
}

}  // namespace
