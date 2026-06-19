// STEP-Export (ADR-0014, geometrie-resident). Baut die B-Rep-Solids der
// OCC-Solid-Bauteile (Wände + Decken/Fundament) über den geteilten makeNetSolid
// und schreibt sie via OCC-DataExchange (STEPControl_Writer, AP214) atomar.
// Dächer/Treppen (analytische Netze) sind eine benannte STEP-Lücke (STL-only).

#include "adapters/geometry/step_export_adapter.h"

#include <fcntl.h>
#include <unistd.h>

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
#include "hexagon/services/roof_geometry.h"     // roofMesh (slice-024a)
#include "hexagon/services/slab_geometry.h"     // slabBaseZ/slabCutPrisms
#include "hexagon/services/stair_geometry.h"    // stairStepBoxes (slice-024b)
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

// B-Rep-Solids der OCC-Solid-Bauteile (Wände + Decken) sowie der Dächer (seit
// slice-024a: das wasserdichte Dach-Netz wird zu einem Solid vernäht) als ein
// Compound. Degenerierte/nicht-wasserdichte Bauteile werden übersprungen
// (Totalität, fail-closed). **Treppen** bleiben die benannte STEP-Lücke
// (analytische Box-Union, kein OCC-Solid) bis slice-024b.
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

    // Dächer (slice-024a): das seit 023b wasserdichte `roofMesh` wird zu einem
    // B-Rep-Solid vernäht. `meshToSolid` ist fail-closed → ein leeres Shape
    // (degeneriert / nicht geschlossen) wird übersprungen (Totalität). Der
    // try/catch spiegelt die Wand-/Decken-Schleifen (MR-009 MED-2): ein einzelnes
    // problematisches Dach wird übersprungen, nicht der ganze Export abgebrochen.
    for (const model::Roof& r : building.roofs) {
        TopoDS_Shape solid;
        try {
            solid = meshToSolid(services::roofMesh(r));
        } catch (const std::exception&) {
            continue;  // degeneriertes/fehlschlagendes Dach überspringen (Totalität)
        }
        if (!solid.IsNull()) {
            builder.Add(compound, solid);
        }
    }

    // Treppen (slice-024b): die Stufen als **analytische** OCC-Box-Solids — das
    // flache `stairMesh` ist eine **nicht-manifolde** Box-Union (x-benachbarte
    // Stufen-Quader, die sich an gemeinsamen x-Ebenen mit **partiell koinzidenten
    // Flächen** berühren → nicht zu *einem* gültigen Solid vernähbar).
    // `stairStepBoxes` ist die geteilte Box-Wahrheit; das **Geländer** (render-only)
    // bleibt **ausgelassen** (nur im STL). Per-Bauteil-try/catch wie oben (Totalität).
    // Die berührenden Box-Solids bleiben **getrennte** Compound-Member (kein Fuse).
    for (const model::Stair& s : building.stairs) {
        try {
            for (const services::StepBox& box : services::stairStepBoxes(
                     s, storeyHeight(building, s.from_storey_id))) {
                const TopoDS_Shape solid = makeBoxSolid(
                    box.x0_mm, box.y0_mm, box.z0_mm, box.x1_mm, box.y1_mm, box.z1_mm);
                if (!solid.IsNull()) {
                    builder.Add(compound, solid);
                }
            }
        } catch (const std::exception&) {
            continue;  // problematische Treppe überspringen (Totalität)
        }
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
        // Durability: Temp vor dem Rename auf Platte zwingen (Muster STL-/
        // IFC-Export; Code-Review MED-1) — sonst könnte ein Crash zwischen
        // Write und Flush ein leeres/abgeschnittenes Ziel sichtbar machen.
        const int fd = ::open(tmp.string().c_str(), O_RDONLY);
        if (fd >= 0) {
            ::fsync(fd);
            ::close(fd);
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
