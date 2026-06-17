// STEP-Export (ADR-0014, geometrie-resident). Baut die B-Rep-Solids der
// OCC-Solid-Bauteile (Wände + Decken/Fundament) über den geteilten makeNetSolid
// und schreibt sie via OCC-DataExchange (STEPControl_Writer, AP214) atomar.
// Dächer/Treppen (analytische Netze) sind eine benannte STEP-Lücke (STL-only).

#include "adapters/geometry/step_export_adapter.h"

#include <stdexcept>
#include <string>
#include <system_error>

#include <BRepBuilderAPI_Transform.hxx>
#include <BRep_Builder.hxx>
#include <IFSelect_ReturnStatus.hxx>
#include <Interface_Static.hxx>
#include <STEPControl_Writer.hxx>
#include <Standard_Failure.hxx>
#include <TopoDS_Compound.hxx>
#include <TopoDS_Shape.hxx>
#include <gp_Trsf.hxx>
#include <gp_Vec.hxx>

#include "adapters/geometry/occ_solids.h"
#include "hexagon/model/constants.h"
#include "hexagon/services/opening_geometry.h"  // wallCutPrisms
#include "hexagon/services/slab_geometry.h"     // slabBaseZ/slabCutPrisms
#include "hexagon/services/wall_footprint.h"    // wallFootprint

namespace fs = std::filesystem;
namespace model = bcad::hexagon::model;
namespace services = bcad::hexagon::services;

namespace bcad::adapters::geometry {
namespace {

double storeyHeight(const model::Building& building, model::StoreyId id) {
    for (const model::Storey& s : building.storeys) {
        if (s.id == id) {
            return s.height_mm;
        }
    }
    return model::kDefaultStoreyHeightMm;
}

// B-Rep-Solids der OCC-Solid-Bauteile (Wände + Decken) als ein Compound.
// Degenerierte Bauteile werden übersprungen (Totalität); Dächer/Treppen sind
// die benannte STEP-Lücke (analytische Netze, kein OCC-Solid).
TopoDS_Compound buildSolidCompound(const model::Building& building) {
    TopoDS_Compound compound;
    BRep_Builder builder;
    builder.MakeCompound(compound);

    for (const model::Wall& w : building.walls) {
        TopoDS_Shape solid;
        try {
            solid = makeNetSolid(services::wallFootprint(w, building.walls),
                                 w.height_mm,
                                 services::wallCutPrisms(w, building.openings));
        } catch (const std::exception&) {
            continue;  // degeneriertes Bauteil überspringen (Totalität)
        }
        builder.Add(compound, solid);
    }

    for (const model::Slab& s : building.slabs) {
        TopoDS_Shape solid;
        try {
            solid = makeNetSolid(s.footprint, s.thickness_mm,
                                 services::slabCutPrisms(s));
        } catch (const std::exception&) {
            continue;
        }
        // Platte auf ihre Aufstandshöhe heben (base_z, wie das Darstellungsnetz).
        gp_Trsf lift;
        lift.SetTranslation(
            gp_Vec(0.0, 0.0, services::slabBaseZ(s, storeyHeight(building, s.storey_id))));
        builder.Add(compound, BRepBuilderAPI_Transform(solid, lift, true).Shape());
    }
    return compound;
}

}  // namespace

void StepExportAdapter::write(const model::Building& building,
                              const fs::path& path) const {
    try {
        const TopoDS_Compound compound = buildSolidCompound(building);

        STEPControl_Writer writer;
        Interface_Static::SetCVal("write.step.schema", "AP214CD");  // §1 LH-FA-IO-005.a
        if (writer.Transfer(compound, STEPControl_AsIs) != IFSelect_RetDone) {
            throw std::runtime_error(
                "E-IO-002: STEP-Transfer fehlgeschlagen; event=persist_error");
        }

        // Atomar: in Temp schreiben, dann umbenennen. Ein nicht beschreibbarer
        // Zielpfad lässt Write scheitern → E-IO-001 (kein Teil-Export).
        const fs::path tmp(path.string() + ".tmp");
        if (writer.Write(tmp.string().c_str()) != IFSelect_RetDone) {
            std::error_code rm;
            fs::remove(tmp, rm);
            throw std::runtime_error(
                "E-IO-001: STEP-Export fehlgeschlagen ('" + path.string() +
                "'): Schreiben des Zielpfads fehlgeschlagen; event=io_no_permission");
        }
        std::error_code ec;
        fs::rename(tmp, path, ec);
        if (ec) {
            std::error_code rm;
            fs::remove(tmp, rm);
            throw std::runtime_error(
                "E-IO-001: STEP-Export fehlgeschlagen ('" + path.string() +
                "'): " + ec.message() + "; event=io_no_permission");
        }
    } catch (const Standard_Failure& failure) {
        // OCC-Ausnahme neutralisieren — kein OCC-Typ verlässt den Adapter.
        throw std::runtime_error(
            std::string("E-IO-002: STEP-Export fehlgeschlagen (OCC): ") +
            failure.GetMessageString() + "; event=persist_error");
    }
}

}  // namespace bcad::adapters::geometry
