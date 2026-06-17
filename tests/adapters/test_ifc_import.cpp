// AK + Integration (slice-019b Commit ii): IFC-Import end-to-end über den
// ECHTEN Pfad `ExchangeService` -> `ModelImporterPort` -> `IfcImportAdapter`
// (welle-3-Lehre slice-015b / ADR-0013 Fitness Function: Integrationspfad
// üben, nicht nur den Codec). AK `LH-FA-IO-001` Happy/Boundary/Negative +
// Subset-Skip + tragende Pflicht-Referenz. Fixtures sind Raw-String-`.ifc`,
// in eine Temp-Datei geschrieben (der Port liest einen Pfad).

#include "adapters/io/ifc_export_adapter.h"
#include "adapters/io/ifc_import_adapter.h"

#include <filesystem>
#include <fstream>
#include <stdexcept>
#include <string>

#include <gtest/gtest.h>

#include "hexagon/model/building.h"
#include "hexagon/ports/driving/exchange_model_port.h"
#include "hexagon/services/exchange_service.h"

namespace {

namespace fs = std::filesystem;
namespace model = bcad::hexagon::model;
using bcad::adapters::io::IfcImportAdapter;
using bcad::hexagon::ports::driving::ExchangeFormat;
using bcad::hexagon::services::ExchangeService;

// Vollständiges, gültiges IFC-SPF-Subset: 2 Geschosse (Elevation 0 / 3000),
// 2 gerade Wände (Achs-Polyline + Body-Extrusion + Material-Layer / Default),
// Containment je Geschoss. Wand #100 trägt einen 300-mm-Materiallayer,
// Wand #150 kein Material (-> Default-Dicke).
std::string sampleIfc() {
    return "ISO-10303-21;\n"
           "HEADER;\n"
           "FILE_DESCRIPTION((''),'2;1');\n"
           "FILE_NAME('s','2026-06-16T00:00:00',(''),(''),'','','');\n"
           "FILE_SCHEMA(('IFC4'));\n"
           "ENDSEC;\n"
           "DATA;\n"
           "#1=IFCPROJECT('p',$,'Proj',$,$,$,$,$,$);\n"
           "#10=IFCBUILDING('b',$,'Bld',$,$,$,$,$,$,$,$,$);\n"
           "#20=IFCBUILDINGSTOREY('s1',$,'EG',$,$,$,$,$,.ELEMENT.,0.);\n"
           "#21=IFCBUILDINGSTOREY('s2',$,'OG',$,$,$,$,$,.ELEMENT.,3000.);\n"
           "#100=IFCWALL('w1',$,'W1',$,$,$,#110,$);\n"
           "#110=IFCPRODUCTDEFINITIONSHAPE($,$,(#120,#130));\n"
           "#120=IFCSHAPEREPRESENTATION($,'Axis','Curve2D',(#121));\n"
           "#121=IFCPOLYLINE((#122,#123));\n"
           "#122=IFCCARTESIANPOINT((0.,0.,0.));\n"
           "#123=IFCCARTESIANPOINT((5000.,0.,0.));\n"
           "#130=IFCSHAPEREPRESENTATION($,'Body','SweptSolid',(#131));\n"
           "#131=IFCEXTRUDEDAREASOLID($,$,$,2500.);\n"
           "#140=IFCMATERIALLAYER($,300.,$);\n"
           "#141=IFCMATERIALLAYERSET((#140),'Standard');\n"
           "#142=IFCMATERIALLAYERSETUSAGE(#141,.AXIS2.,.POSITIVE.,0.);\n"
           "#143=IFCRELASSOCIATESMATERIAL('rm',$,$,$,(#100),#142);\n"
           "#150=IFCWALL('w2',$,'W2',$,$,$,#160,$);\n"
           "#160=IFCPRODUCTDEFINITIONSHAPE($,$,(#170,#180));\n"
           "#170=IFCSHAPEREPRESENTATION($,'Axis','Curve2D',(#171));\n"
           "#171=IFCPOLYLINE((#172,#173));\n"
           "#172=IFCCARTESIANPOINT((0.,0.,0.));\n"
           "#173=IFCCARTESIANPOINT((0.,4000.,0.));\n"
           "#180=IFCSHAPEREPRESENTATION($,'Body','SweptSolid',(#181));\n"
           "#181=IFCEXTRUDEDAREASOLID($,$,$,3000.);\n"
           "#200=IFCRELCONTAINEDINSPATIALSTRUCTURE('rc1',$,$,$,(#100),#20);\n"
           "#250=IFCRELCONTAINEDINSPATIALSTRUCTURE('rc2',$,$,$,(#150),#21);\n"
           "ENDSEC;\n"
           "END-ISO-10303-21;\n";
}

fs::path writeTemp(const std::string& tag, const std::string& content) {
    const fs::path path = fs::temp_directory_path() / ("bcad_ifc_" + tag + ".ifc");
    std::ofstream out(path, std::ios::binary | std::ios::trunc);
    out << content;
    out.close();
    return path;
}

// RAII-Aufräumen der Temp-Fixture.
struct TempIfc {
    fs::path path;
    explicit TempIfc(const std::string& tag, const std::string& content)
        : path(writeTemp(tag, content)) {}
    ~TempIfc() {
        std::error_code ec;
        fs::remove(path, ec);
    }
    TempIfc(const TempIfc&) = delete;
    TempIfc& operator=(const TempIfc&) = delete;
};

// --- Happy Path (LH-FA-IO-001): Anzahl-Treue + Mapping ---------------------

TEST(IfcImport, HappyPathCountsMatchSource) {
    const TempIfc fixture("happy_counts", sampleIfc());
    const IfcImportAdapter adapter;
    const model::Building building = adapter.read(fixture.path);

    EXPECT_EQ(building.storeys.size(), 2U);  // == Quelle (2 IfcBuildingStorey)
    EXPECT_EQ(building.walls.size(), 2U);    // == Quelle (2 IfcWall)
}

TEST(IfcImport, HappyPathMapsWallGeometryAndStoreyHeight) {
    const TempIfc fixture("happy_map", sampleIfc());
    const IfcImportAdapter adapter;
    const model::Building b = adapter.read(fixture.path);

    ASSERT_EQ(b.storeys.size(), 2U);
    // Geschoss-Höhe = Elevation-Differenz; oberstes -> Default (2500).
    EXPECT_DOUBLE_EQ(b.storeys[0].height_mm, 3000.0);
    EXPECT_DOUBLE_EQ(b.storeys[1].height_mm, 2500.0);

    ASSERT_EQ(b.walls.size(), 2U);
    // Wand #100: Achse (0,0)->(5000,0), Layer-Dicke 300, Extrusions-Höhe 2500.
    EXPECT_DOUBLE_EQ(b.walls[0].start.x_mm, 0.0);
    EXPECT_DOUBLE_EQ(b.walls[0].start.y_mm, 0.0);
    EXPECT_DOUBLE_EQ(b.walls[0].end.x_mm, 5000.0);
    EXPECT_DOUBLE_EQ(b.walls[0].end.y_mm, 0.0);
    EXPECT_DOUBLE_EQ(b.walls[0].thickness_mm, 300.0);
    EXPECT_DOUBLE_EQ(b.walls[0].height_mm, 2500.0);
    EXPECT_EQ(b.walls[0].storey_id, b.storeys[0].id);
    EXPECT_EQ(b.walls[0].type, model::WallType::Innen);  // benannte Lücke
    EXPECT_FALSE(b.walls[0].material_id.has_value());
    // Wand #150: Achse (0,0)->(0,4000), kein Material -> Default-Dicke 240.
    EXPECT_DOUBLE_EQ(b.walls[1].end.y_mm, 4000.0);
    EXPECT_DOUBLE_EQ(b.walls[1].thickness_mm, 240.0);
    EXPECT_DOUBLE_EQ(b.walls[1].height_mm, 3000.0);
    EXPECT_EQ(b.walls[1].storey_id, b.storeys[1].id);
    EXPECT_FALSE(b.walls[1].material_id.has_value());
}

TEST(IfcImport, SubsetForeignEntitiesAreSkippedNotRejected) {
    std::string ifc = sampleIfc();
    // Subset-fremde, vorhandene Entität -> übersprungen, KEIN Wurf (LOW-1).
    const std::string door = "#300=IFCDOOR('d',$,'D1',$,$,$,$,$,2100.,900.);\n";
    ifc.insert(ifc.find("ENDSEC;\nEND-ISO"), door);
    const TempIfc fixture("skip_foreign", ifc);
    const IfcImportAdapter adapter;
    const model::Building building = adapter.read(fixture.path);

    EXPECT_EQ(building.storeys.size(), 2U);
    EXPECT_EQ(building.walls.size(), 2U);  // Tür zählt nicht
}

// --- Boundary (LH-FA-IO-001): strukturlos / leer -> leeres Modell, kein Wurf -

TEST(IfcImport, StructurelessFileYieldsEmptyModel) {
    const std::string ifc =
        "ISO-10303-21;\nHEADER;\nFILE_SCHEMA(('IFC4'));\nENDSEC;\n"
        "DATA;\n#1=IFCPROJECT('p',$,'P',$,$,$,$,$,$);\nENDSEC;\n"
        "END-ISO-10303-21;\n";
    const TempIfc fixture("structureless", ifc);
    const IfcImportAdapter adapter;
    const model::Building building = adapter.read(fixture.path);

    EXPECT_TRUE(building.storeys.empty());
    EXPECT_TRUE(building.walls.empty());
}

TEST(IfcImport, EmptyFileYieldsEmptyModel) {
    const TempIfc fixture("empty", "");
    const IfcImportAdapter adapter;
    const model::Building building = adapter.read(fixture.path);
    EXPECT_TRUE(building.storeys.empty());
    EXPECT_TRUE(building.walls.empty());
}

// --- Negative (LH-FA-IO-001): nicht-IFC / tragende Pflicht-Referenz fehlt ---

void expectRejected(const fs::path& path) {
    const IfcImportAdapter adapter;
    try {
        adapter.read(path);
        FAIL() << "erwarteter E-IO-003-Wurf blieb aus";
    } catch (const std::runtime_error& e) {
        const std::string msg = e.what();
        EXPECT_NE(msg.find("E-IO-003"), std::string::npos);
        EXPECT_NE(msg.find("import_rejected"), std::string::npos);
    }
}

TEST(IfcImport, NonIfcFileRejectedWithEIo003) {
    const TempIfc fixture("non_ifc", "Das ist kein IFC, sondern Klartext.\n");
    expectRejected(fixture.path);
}

TEST(IfcImport, WallWithoutContainmentRejected) {
    // Wand ohne IfcRelContainedInSpatialStructure -> tragende Pflicht-Referenz
    // fehlt -> E-IO-003 (kein stiller Teil-Import).
    std::string ifc = sampleIfc();
    const std::string marker =
        "#200=IFCRELCONTAINEDINSPATIALSTRUCTURE('rc1',$,$,$,(#100),#20);\n";
    ifc.erase(ifc.find(marker), marker.size());
    const TempIfc fixture("no_containment", ifc);
    expectRejected(fixture.path);
}

TEST(IfcImport, WallWithoutAxisRejected) {
    // Wand ohne Achs-ShapeRepresentation -> nicht mappbar -> E-IO-003.
    std::string ifc = sampleIfc();
    const std::string axis =
        "#120=IFCSHAPEREPRESENTATION($,'Axis','Curve2D',(#121));\n";
    ifc.erase(ifc.find(axis), axis.size());
    const TempIfc fixture("no_axis", ifc);
    expectRejected(fixture.path);
}

TEST(IfcImport, ValidHeaderButCorruptDataRejected) {
    // Gültiger ISO-Kopf, aber inhaltlich kaputte DATA-Sektion (Statement ohne
    // '#') -> E-IO-003, kein stilles Unter-Zählen (MED-1/LOW-3-Review).
    const std::string ifc =
        "ISO-10303-21;\nHEADER;\nFILE_SCHEMA(('IFC4'));\nENDSEC;\n"
        "DATA;\nKAPUTTE ZEILE OHNE HASH;\nENDSEC;\nEND-ISO-10303-21;\n";
    const TempIfc fixture("corrupt_data", ifc);
    expectRejected(fixture.path);
}

// --- Integration über den ECHTEN Pfad (ExchangeService -> Port -> Adapter) ---

TEST(IfcImportIntegration, ExchangeServiceImportsThroughRealAdapter) {
    const TempIfc fixture("svc_happy", sampleIfc());
    const IfcImportAdapter adapter;
    const bcad::adapters::io::IfcExportAdapter exporter;
    const ExchangeService service(adapter, exporter);  // Driven-Port-Injektion (main-Muster)

    const model::Building building =
        service.importModel(fixture.path, ExchangeFormat::Ifc);
    EXPECT_EQ(building.storeys.size(), 2U);
    EXPECT_EQ(building.walls.size(), 2U);
}

TEST(IfcImportIntegration, ExchangeServicePropagatesEIo003) {
    const TempIfc fixture("svc_reject", "<png-bytes-not-ifc>\n");
    const IfcImportAdapter adapter;
    const bcad::adapters::io::IfcExportAdapter exporter;
    const ExchangeService service(adapter, exporter);

    try {
        service.importModel(fixture.path, ExchangeFormat::Ifc);
        FAIL() << "erwarteter E-IO-003-Wurf blieb aus";
    } catch (const std::runtime_error& e) {
        const std::string msg = e.what();
        EXPECT_NE(msg.find("E-IO-003"), std::string::npos);
        EXPECT_NE(msg.find("import_rejected"), std::string::npos);
    }
}

}  // namespace
