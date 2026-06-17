// Codec-Ebene (slice-019b Commit i): isolierte Tests des hand-gerollten
// ISO-10303-21 (SPF) Subset-Lesers — Tokenizer + Entitäts-Graph, ohne
// IFC-Domänen-Mapping und ohne model::Building (ADR-0013 Option D). Orakel
// sind analytisch (Entitäts-/Attribut-Werte), schnell, ohne echten Adapter.

#include "adapters/io/ifc_spf_reader.h"

#include <stdexcept>
#include <string>

#include <gtest/gtest.h>

namespace {

using bcad::adapters::io::asNumber;
using bcad::adapters::io::asRef;
using bcad::adapters::io::asString;
using bcad::adapters::io::attributeAt;
using bcad::adapters::io::SpfEntity;
using bcad::adapters::io::SpfReader;
using bcad::adapters::io::SpfValue;

// Minimaler, gültiger SPF-Rumpf mit Kopf, HEADER- und DATA-Sektion.
std::string wrap(const std::string& data_body) {
    return "ISO-10303-21;\n"
           "HEADER;\n"
           "FILE_DESCRIPTION((''),'2;1');\n"
           "FILE_NAME('t','2026-01-01T00:00:00',(''),(''),'','','');\n"
           "FILE_SCHEMA(('IFC4'));\n"
           "ENDSEC;\n"
           "DATA;\n" +
           data_body +
           "ENDSEC;\n"
           "END-ISO-10303-21;\n";
}

TEST(SpfReader, ParsesEntitiesKeywordsAndIds) {
    const SpfReader reader = SpfReader::parse(wrap(
        "#1=IFCBUILDINGSTOREY('guid',$,'EG',$,$,$,$,$,.ELEMENT.,2500.);\n"
        "#2=IFCWALL('guid',$,'W1',$,$,$,#3,$);\n"));

    EXPECT_EQ(reader.byKeyword("IFCWALL").size(), 1U);
    EXPECT_EQ(reader.byKeyword("IFCBUILDINGSTOREY").size(), 1U);
    EXPECT_EQ(reader.byKeyword("IFCDOOR").size(), 0U);

    const SpfEntity* storey = reader.byId(1);
    ASSERT_NE(storey, nullptr);
    EXPECT_EQ(storey->keyword, "IFCBUILDINGSTOREY");
    EXPECT_EQ(reader.byId(99), nullptr);  // dangling
}

TEST(SpfReader, TypedAttributeKinds) {
    const SpfReader reader = SpfReader::parse(wrap(
        "#1=IFCBUILDINGSTOREY('guid',$,'EG',$,$,$,$,$,.ELEMENT.,2500.);\n"
        "#2=IFCWALL('w',$,'W1',$,$,$,#7,$);\n"));

    const SpfEntity* storey = reader.byId(1);
    ASSERT_NE(storey, nullptr);
    EXPECT_EQ(asString(attributeAt(*storey, 2)), "EG");          // Name
    EXPECT_FALSE(asNumber(attributeAt(*storey, 5)).has_value());  // $ -> kein Wert
    ASSERT_TRUE(asNumber(attributeAt(*storey, 9)).has_value());   // Elevation
    EXPECT_DOUBLE_EQ(*asNumber(attributeAt(*storey, 9)), 2500.0);
    EXPECT_EQ(attributeAt(*storey, 42), nullptr);  // außerhalb

    const SpfEntity* wall = reader.byId(2);
    ASSERT_NE(wall, nullptr);
    EXPECT_EQ(asRef(attributeAt(*wall, 6)), 7);  // Representation-Ref
}

TEST(SpfReader, NestedListsAndCartesianCoordinates) {
    const SpfReader reader = SpfReader::parse(
        wrap("#5=IFCCARTESIANPOINT((1000.,-250.5,0.));\n"
             "#6=IFCPOLYLINE((#5,#7));\n"));

    const SpfEntity* point = reader.byId(5);
    ASSERT_NE(point, nullptr);
    const SpfValue* coords = attributeAt(*point, 0);
    ASSERT_NE(coords, nullptr);
    ASSERT_EQ(coords->kind, SpfValue::Kind::List);
    ASSERT_EQ(coords->items.size(), 3U);
    EXPECT_DOUBLE_EQ(*asNumber(&coords->items[0]), 1000.0);
    EXPECT_DOUBLE_EQ(*asNumber(&coords->items[1]), -250.5);

    const SpfEntity* line = reader.byId(6);
    ASSERT_NE(line, nullptr);
    const SpfValue* pts = attributeAt(*line, 0);
    ASSERT_NE(pts, nullptr);
    ASSERT_EQ(pts->items.size(), 2U);
    EXPECT_EQ(asRef(&pts->items[0]), 5);
    EXPECT_EQ(asRef(&pts->items[1]), 7);
}

TEST(SpfReader, StringEscapeAndComments) {
    const SpfReader reader = SpfReader::parse(
        wrap("/* Kommentar mit ; und # drin */\n"
             "#1=IFCWALL('O''Brien',$,'N',$,$,$,$,$);\n"));
    const SpfEntity* wall = reader.byId(1);
    ASSERT_NE(wall, nullptr);
    EXPECT_EQ(asString(attributeAt(*wall, 0)), "O'Brien");  // '' -> '
}

TEST(SpfReader, ByKeywordIsIdAscendingDeterministic) {
    const SpfReader reader = SpfReader::parse(
        wrap("#30=IFCWALL('a',$,$,$,$,$,$,$);\n"
             "#10=IFCWALL('b',$,$,$,$,$,$,$);\n"
             "#20=IFCWALL('c',$,$,$,$,$,$,$);\n"));
    const auto walls = reader.byKeyword("IFCWALL");
    ASSERT_EQ(walls.size(), 3U);
    EXPECT_EQ(walls[0]->id, 10);
    EXPECT_EQ(walls[1]->id, 20);
    EXPECT_EQ(walls[2]->id, 30);
}

TEST(SpfReader, EmptyDataSectionYieldsEmptyGraph) {
    const SpfReader reader = SpfReader::parse(wrap(""));
    EXPECT_TRUE(reader.entities().empty());
}

TEST(SpfReader, MissingIsoHeaderThrows) {
    EXPECT_THROW(SpfReader::parse("DATA;\n#1=IFCWALL($);\nENDSEC;\n"),
                 std::runtime_error);
    EXPECT_THROW(SpfReader::parse("not an ifc file at all"), std::runtime_error);
}

TEST(SpfReader, UnterminatedSyntaxThrows) {
    EXPECT_THROW(SpfReader::parse(wrap("#1=IFCWALL('unterminated\n")),
                 std::runtime_error);
    EXPECT_THROW(SpfReader::parse(wrap("#1=IFCWALL('a',$,$\n")),
                 std::runtime_error);  // fehlendes ')'/';'
}

TEST(SpfReader, GarbageStatementInDataSectionThrows) {
    // Inhaltlich kaputtes (nicht-#) Statement in DATA -> Wurf statt stillem
    // Drop (kein Unter-Zählen, atomarer Import). HEADER-Einträge bleiben ok.
    EXPECT_THROW(SpfReader::parse(wrap("100=IFCWALL('x',$,$,$,$,$,$,$);\n")),
                 std::runtime_error);
    EXPECT_THROW(SpfReader::parse(wrap("KAPUTT OHNE HASH;\n")),
                 std::runtime_error);
}

}  // namespace
