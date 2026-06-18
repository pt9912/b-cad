// Isolierter DXF-Codec-Test (slice-021b Commit i): symmetrischer Roundtrip
// `DxfWriter` → `DxfReader` + Wohlgeformtheits-/Totalitäts-Grenze, OHNE Adapter/
// Building (Muster `test_ifc_spf_reader`). Deckt die ADR-0015-Fitness-Function
// „symmetrischer Reader+Writer, Roundtrip-prüfbar" auf Codec-Ebene; die
// Mapping-/Integrations-AK (LH-FA-IO-003/004) folgen in Commit ii.

#include "adapters/io/dxf_reader.h"
#include "adapters/io/dxf_writer.h"

#include <stdexcept>
#include <string>

#include <gtest/gtest.h>

namespace {

using bcad::adapters::io::DxfReader;
using bcad::adapters::io::DxfWriter;
using bcad::adapters::io::dxfReal;

// --- Writer→Reader-Roundtrip: zwei LINEs je eigener LAYER -------------------

TEST(DxfCodec, WriterReaderRoundtripPreservesLinesLayersAndCoords) {
    DxfWriter w;
    w.pair(0, "SECTION");
    w.pair(2, "ENTITIES");
    w.pair(0, "LINE");
    w.pair(8, "STOREY_1");
    w.pair(10, 0.0);
    w.pair(20, 0.0);
    w.pair(11, 5000.0);
    w.pair(21, 0.0);
    w.pair(0, "LINE");
    w.pair(8, "STOREY_2");
    w.pair(10, 0.0);
    w.pair(20, 0.0);
    w.pair(11, 0.0);
    w.pair(21, 4000.0);
    w.pair(0, "ENDSEC");
    w.pair(0, "EOF");

    const DxfReader r = DxfReader::parse(w.build());
    const auto lines = r.sectionEntities("ENTITIES");
    ASSERT_EQ(lines.size(), 2U);
    EXPECT_EQ(lines[0]->type, "LINE");
    EXPECT_EQ(lines[0]->str(8).value_or(""), "STOREY_1");
    EXPECT_DOUBLE_EQ(lines[0]->num(10).value(), 0.0);
    EXPECT_DOUBLE_EQ(lines[0]->num(11).value(), 5000.0);
    EXPECT_EQ(lines[1]->str(8).value_or(""), "STOREY_2");
    EXPECT_DOUBLE_EQ(lines[1]->num(21).value(), 4000.0);
}

// --- Real-Formatierung: locale-frei, Dezimalzeichen, round-trip-stabil ------

TEST(DxfCodec, DxfRealIsLocaleFreeDecimalAndRoundTrips) {
    EXPECT_NE(dxfReal(5000.0).find('.'), std::string::npos);
    EXPECT_EQ(dxfReal(5000.0).find(','), std::string::npos);
    EXPECT_DOUBLE_EQ(std::stod(dxfReal(0.0)), 0.0);
    EXPECT_DOUBLE_EQ(std::stod(dxfReal(5000.0)), 5000.0);
    EXPECT_DOUBLE_EQ(std::stod(dxfReal(-1234.5)), -1234.5);
    EXPECT_DOUBLE_EQ(std::stod(dxfReal(1234.5)), 1234.5);
}

// --- Wohlgeformtheits-Grenze: unwohlgeformt → Wurf (Adapter → E-IO-003) -----

TEST(DxfCodec, NonNumericGroupCodeLineThrows) {
    EXPECT_THROW(DxfReader::parse("hello world\nfoo bar\n"), std::runtime_error);
}

TEST(DxfCodec, OddGroupCodePairingThrows) {
    EXPECT_THROW(DxfReader::parse("0\nSECTION\n2\n"), std::runtime_error);
}

// --- Totalität: wohlgeformt-aber-strukturlos / leer → kein Wurf -------------

TEST(DxfCodec, StructurelessButWellFormedYieldsNoEntitiesSection) {
    const std::string doc =
        "0\nSECTION\n2\nHEADER\n9\n$ACADVER\n1\nAC1009\n0\nENDSEC\n0\nEOF\n";
    const DxfReader r = DxfReader::parse(doc);  // kein Wurf
    EXPECT_TRUE(r.sectionEntities("ENTITIES").empty());
}

TEST(DxfCodec, EmptyAndWhitespaceOnlyYieldNoEntities) {
    EXPECT_TRUE(DxfReader::parse("").entities().empty());
    EXPECT_TRUE(DxfReader::parse("   \n  \n").entities().empty());  // kein Wurf
}

// --- LWPOLYLINE-Vertices: aufeinanderfolgende 10/20-Paare -------------------

TEST(DxfCodec, LwpolylineVerticesAreExtractedInOrder) {
    const std::string doc =
        "0\nSECTION\n2\nENTITIES\n"
        "0\nLWPOLYLINE\n8\nSTOREY_1\n90\n3\n"
        "10\n0.0\n20\n0.0\n10\n1000.0\n20\n0.0\n10\n1000.0\n20\n2000.0\n"
        "0\nENDSEC\n0\nEOF\n";
    const DxfReader r = DxfReader::parse(doc);
    const auto ents = r.sectionEntities("ENTITIES");
    ASSERT_EQ(ents.size(), 1U);
    EXPECT_EQ(ents[0]->type, "LWPOLYLINE");
    const auto pts = ents[0]->points();
    ASSERT_EQ(pts.size(), 3U);
    EXPECT_DOUBLE_EQ(pts[0].first, 0.0);
    EXPECT_DOUBLE_EQ(pts[1].first, 1000.0);
    EXPECT_DOUBLE_EQ(pts[2].second, 2000.0);
}

// --- Sektions-Filter: HEADER-Inhalt zählt nicht zu ENTITIES -----------------

TEST(DxfCodec, SectionEntitiesSkipsOtherSections) {
    const std::string doc =
        "0\nSECTION\n2\nHEADER\n9\n$ACADVER\n1\nAC1009\n0\nENDSEC\n"
        "0\nSECTION\n2\nENTITIES\n0\nLINE\n8\nSTOREY_1\n"
        "10\n0.0\n20\n0.0\n11\n1.0\n21\n1.0\n0\nENDSEC\n0\nEOF\n";
    const DxfReader r = DxfReader::parse(doc);
    EXPECT_EQ(r.sectionEntities("ENTITIES").size(), 1U);
    EXPECT_TRUE(r.sectionEntities("HEADER").empty() ||
                r.sectionEntities("HEADER").front()->type != "LINE");
}

}  // namespace
