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
#include "hexagon/model/derived_geometry.h"  // DerivedGeometry (ADR-0020, slice-042c)

namespace fs = std::filesystem;
namespace model = bcad::hexagon::model;

namespace bcad::adapters::geometry {
namespace {

// B-Rep-Solids der **pre-OCC-Primitive** aus dem kern-berechneten `DerivedGeometry`-
// Bündel (ADR-0020: der Kern liefert die Ableitung, der Adapter serialisiert + baut
// das B-Rep — die OCC-Montage bleibt adapter-resident, Regel C). Wände/Decken via
// `makeNetSolid`, Decken auf die kern-gelieferte `baseZ` gehoben; Dächer via
// `meshToSolid` (vernäht); Treppen als analytische Box-Solids je `StepBox` (Geländer
// render-only, nicht im STEP). Degenerierte Bauteile werden **beim OCC-Bau**
// übersprungen (Totalität, fail-closed) — der per-Bauteil-try/catch bleibt hier, weil
// die reine Ableitung (kern-seitig, total) schon geschehen ist und nur die OCC-
// Konstruktion werfen kann.
TopoDS_Compound buildSolidCompound(const model::DerivedGeometry& derived) {
    TopoDS_Compound compound;
    const BRep_Builder builder;
    builder.MakeCompound(compound);

    for (const model::DerivedWall& w : derived.walls) {
        TopoDS_Shape solid;
        try {
            solid = makeNetSolid(w.footprint, w.height_mm, w.cutPrisms);
        } catch (const std::exception&) {
            continue;  // degeneriertes Bauteil überspringen (Totalität)
        }
        builder.Add(compound, solid);
    }

    for (const model::DerivedSlab& s : derived.slabs) {
        TopoDS_Shape solid;
        try {
            solid = makeNetSolid(s.footprint, s.thickness_mm, s.cutPrisms);
        } catch (const std::exception&) {
            continue;
        }
        // Platte auf ihre kern-gelieferte Aufstandshöhe heben (base_z).
        gp_Trsf lift;
        lift.SetTranslation(gp_Vec(0.0, 0.0, s.baseZ_mm));
        builder.Add(compound, BRepBuilderAPI_Transform(solid, lift, true).Shape());
    }

    // Dächer: das wasserdichte Netz wird zu einem B-Rep-Solid vernäht. `meshToSolid`
    // ist fail-closed → ein leeres Shape (degeneriert / nicht geschlossen) wird
    // übersprungen (Totalität); der try/catch spiegelt die Wand-/Decken-Schleifen.
    for (const model::DerivedRoof& r : derived.roofs) {
        TopoDS_Shape solid;
        try {
            solid = meshToSolid(r.mesh);
        } catch (const std::exception&) {
            continue;  // degeneriertes/fehlschlagendes Dach überspringen (Totalität)
        }
        if (!solid.IsNull()) {
            builder.Add(compound, solid);
        }
    }

    // Treppen: die Stufen als **analytische** OCC-Box-Solids — das flache Treppen-Netz
    // ist eine **nicht-manifolde** Box-Union (nicht zu *einem* gültigen Solid
    // vernähbar). Das **Geländer** (render-only) bleibt **ausgelassen** (nur im STL).
    // Die berührenden Box-Solids bleiben **getrennte** Compound-Member (kein Fuse).
    for (const model::DerivedStair& s : derived.stairs) {
        try {
            for (const model::StepBox& box : s.boxes) {
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

void StepExportAdapter::write(const model::Building& /*building*/,
                              const model::DerivedGeometry& derived,
                              const fs::path& path) const {
    try {
        const TopoDS_Compound compound = buildSolidCompound(derived);

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
