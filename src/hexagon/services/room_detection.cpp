#include "hexagon/services/room_detection.h"

#include <algorithm>
#include <cmath>
#include <cstddef>
#include <utility>
#include <vector>

#include "hexagon/model/constants.h"
#include "hexagon/model/segment.h"

namespace bcad::hexagon::services {

namespace {

using model::Point2D;
using model::Ring;

// Flächen unterhalb dieser Schwelle gelten als degeneriert (kollabierter
// Offset / Zyklus unterhalb der Toleranz) — kein Raum, kein Fehler.
constexpr double kMinAreaMm2 =
    model::kGeometryToleranceMm * model::kGeometryToleranceMm;

double segmentLength(const model::Segment& seg) {
    const double dx = seg.end.x_mm - seg.start.x_mm;
    const double dy = seg.end.y_mm - seg.start.y_mm;
    return std::sqrt((dx * dx) + (dy * dy));
}

// Vorzeichenbehaftete Fläche (Shoelace); > 0 bei CCW-Orientierung.
double signedArea(const Ring& ring) {
    double twice = 0.0;
    for (std::size_t i = 0; i < ring.size(); ++i) {
        const Point2D& a = ring[i];
        const Point2D& b = ring[(i + 1) % ring.size()];
        twice += (a.x_mm * b.y_mm) - (b.x_mm * a.y_mm);
    }
    return twice / 2.0;
}

// Punkt-in-Polygon (Ray-Casting). Randlagen sind hier unkritisch:
// getestet werden Mittellinien-Repräsentanten klar getrennter Zyklen.
bool pointInRing(const Ring& ring, Point2D p) {
    bool inside = false;
    for (std::size_t i = 0, j = ring.size() - 1; i < ring.size(); j = i++) {
        const Point2D& a = ring[i];
        const Point2D& b = ring[j];
        const bool crosses = (a.y_mm > p.y_mm) != (b.y_mm > p.y_mm);
        if (!crosses) {
            continue;
        }
        const double x_at =
            a.x_mm + ((p.y_mm - a.y_mm) * (b.x_mm - a.x_mm) / (b.y_mm - a.y_mm));
        if (p.x_mm < x_at) {
            inside = !inside;
        }
    }
    return inside;
}

// Gerade in Parameterform für den Innen-/Außenkanten-Offset (ADR-0007).
// Eigener Typ mit Member-Schnitt statt freier (Line, Line)-Funktion —
// hält Signaturen frei von vertauschbaren gleichtypigen Nachbarn.
struct OffsetLine {
    Point2D origin{};
    Point2D dir{};  // normiert

    // Schnittpunkt mit `other`. Bei (nahezu) parallelen Geraden der
    // Fallback `other.origin`: für kollineare Nachbarkanten GLEICHER
    // Stärke exakt (gleiche Offset-Gerade); bei UNGLEICHER Stärke eine
    // dokumentierte Welle-1-Näherung — die Ecke springt auf den
    // Offset-Punkt der Folgekante (lineare Überblendung statt exakter
    // Stufenkontur; spez. §1, Code-Review M2). Exakte Stufen erst mit
    // der Wandverschneidung (LH-FA-WAL-006).
    Point2D intersectionWith(const OffsetLine& other) const {
        const double cross =
            (dir.x_mm * other.dir.y_mm) - (dir.y_mm * other.dir.x_mm);
        if (std::abs(cross) < 1e-9) {
            return other.origin;
        }
        const double dx = other.origin.x_mm - origin.x_mm;
        const double dy = other.origin.y_mm - origin.y_mm;
        const double s = ((dx * other.dir.y_mm) - (dy * other.dir.x_mm)) / cross;
        return Point2D{origin.x_mm + (s * dir.x_mm), origin.y_mm + (s * dir.y_mm)};
    }
};

// Geschlossener Wandzug auf Mittellinien-Basis: Punktfolge plus Stärke
// der jeweils AUSGEHENDEN Kante (Index i -> Kante i -> i+1).
struct LoopVertex {
    Point2D point{};
    double outgoing_thickness_mm{};
};
using Loop = std::vector<LoopVertex>;

Ring centerlineRing(const Loop& loop) {
    Ring ring;
    ring.reserve(loop.size());
    for (const LoopVertex& v : loop) {
        ring.push_back(v.point);
    }
    return ring;
}

// Kanten-Graph über toleranz-verschmolzene Segment-Endpunkte
// (spez. §1 Schritt 1).
struct Graph {
    std::vector<Point2D> nodes;
    std::vector<std::pair<std::size_t, std::size_t>> edge_nodes;
    std::vector<double> edge_thickness;
};

std::size_t nodeIndexFor(Graph& graph, Point2D p) {
    for (std::size_t i = 0; i < graph.nodes.size(); ++i) {
        if (segmentLength(model::Segment{graph.nodes[i], p}) <
            model::kGeometryToleranceMm) {
            return i;
        }
    }
    graph.nodes.push_back(p);
    return graph.nodes.size() - 1;
}

Graph buildGraph(const std::vector<const model::Wall*>& walls) {
    Graph graph;
    for (const model::Wall* wall : walls) {
        const std::size_t a = nodeIndexFor(graph, wall->start);
        const std::size_t b = nodeIndexFor(graph, wall->end);
        if (a == b) {
            continue;  // Null-Länge — verwirft bereits LH-FA-WAL-001 Boundary
        }
        graph.edge_nodes.emplace_back(a, b);
        graph.edge_thickness.push_back(wall->thickness_mm);
    }
    return graph;
}

// Halbkante einer planaren Flächen-Traversierung: jede Wand-Kante
// erzeugt zwei gerichtete Halbkanten (Twin = Index ^ 1).
struct HalfEdge {
    std::size_t from{};
    std::size_t to{};
    std::size_t edge{};   // Index in Graph::edge_thickness
    double angle{};       // atan2 der Richtung from -> to
};

std::vector<HalfEdge> buildHalfEdges(const Graph& graph) {
    std::vector<HalfEdge> half;
    half.reserve(2 * graph.edge_nodes.size());
    for (std::size_t e = 0; e < graph.edge_nodes.size(); ++e) {
        const auto& [a, b] = graph.edge_nodes[e];
        const double dx = graph.nodes[b].x_mm - graph.nodes[a].x_mm;
        const double dy = graph.nodes[b].y_mm - graph.nodes[a].y_mm;
        half.push_back(HalfEdge{a, b, e, std::atan2(dy, dx)});
        half.push_back(HalfEdge{b, a, e, std::atan2(-dy, -dx)});
    }
    return half;
}

// Entfernt Stichkanten (Sackgassen) aus einem Flächen-Umlauf: eine
// Halbkante, der unmittelbar ihr Twin folgt, ist ein Hin-und-zurück
// ohne Flächenbeitrag. Die Wand selbst bleibt im Modell — nur das
// Raumpolygon ignoriert den Stich (Welle-1-Einschränkung, spez. §1).
void pruneSpurs(std::vector<std::size_t>& face) {
    bool changed = true;
    while (changed && face.size() >= 2) {
        changed = false;
        for (std::size_t i = 0; i < face.size(); ++i) {
            const std::size_t j = (i + 1) % face.size();
            if (face[j] == (face[i] ^ 1U)) {
                // j zuerst entfernen, falls j > i; sonst i zuerst.
                face.erase(face.begin() + static_cast<std::ptrdiff_t>(std::max(i, j)));
                face.erase(face.begin() + static_cast<std::ptrdiff_t>(std::min(i, j)));
                changed = true;
                break;
            }
        }
    }
}

// Minimale Zyklen über planare Flächen-Traversierung (spez. §1
// Schritt 2): an jedem Knoten sind die ausgehenden Halbkanten nach
// Winkel sortiert; der Flächen-Umlauf nimmt am Zielknoten die im
// Uhrzeigersinn nächste Halbkante nach dem Twin. Innen liegende
// Flächen laufen dadurch CCW (positive Fläche); die unbeschränkte
// Außenfläche läuft CW (negativ) und entfällt. Geteilte Knoten
// (Grad >= 3, Räume mit gemeinsamer Wand) sind damit abgedeckt —
// Code-Review-Finding M1.
std::vector<Loop> extractClosedLoops(const Graph& graph) {
    const std::vector<HalfEdge> half = buildHalfEdges(graph);

    // Ausgehende Halbkanten pro Knoten, nach Winkel sortiert; Position
    // jeder Halbkante in der Liste ihres Startknotens.
    std::vector<std::vector<std::size_t>> outgoing(graph.nodes.size());
    for (std::size_t h = 0; h < half.size(); ++h) {
        outgoing[half[h].from].push_back(h);
    }
    std::vector<std::size_t> position(half.size(), 0);
    for (auto& list : outgoing) {
        std::sort(list.begin(), list.end(), [&half](std::size_t a, std::size_t b) {
            return half[a].angle < half[b].angle;
        });
        for (std::size_t p = 0; p < list.size(); ++p) {
            position[list[p]] = p;
        }
    }

    const auto nextInFace = [&half, &outgoing, &position](std::size_t h) {
        const std::size_t twin = h ^ 1U;
        const auto& out = outgoing[half[h].to];
        const std::size_t p = position[twin];
        return out[(p + out.size() - 1) % out.size()];
    };

    std::vector<Loop> loops;
    std::vector<bool> used(half.size(), false);
    for (std::size_t start = 0; start < half.size(); ++start) {
        if (used[start]) {
            continue;
        }
        std::vector<std::size_t> face;
        std::size_t h = start;
        do {
            used[h] = true;
            face.push_back(h);
            h = nextInFace(h);
        } while (h != start);

        pruneSpurs(face);
        if (face.size() < 3) {
            continue;
        }
        Loop loop;
        loop.reserve(face.size());
        for (const std::size_t he : face) {
            loop.push_back(LoopVertex{graph.nodes[half[he].from],
                                      graph.edge_thickness[half[he].edge]});
        }
        // Nur innen liegende Flächen (CCW, positive Fläche) sind Zyklen;
        // die Außenfläche (negativ) und degenerierte Flächen entfallen.
        if (signedArea(centerlineRing(loop)) >= kMinAreaMm2) {
            loops.push_back(std::move(loop));
        }
    }
    return loops;
}

// Offset-Ring eines CCW-Wandzugs (ADR-0007): side = +1 -> Innenkante
// (Raumpolygon), side = -1 -> Außenkontur (Loch-Ring im umschließenden
// Raum). Jede Kante wird um die halbe Stärke ihres Segments versetzt;
// Ecken sind Schnittpunkte benachbarter Offset-Geraden.
Ring offsetRing(const Loop& loop, double side) {
    const std::size_t k = loop.size();
    std::vector<OffsetLine> lines(k);
    for (std::size_t i = 0; i < k; ++i) {
        const Point2D a = loop[i].point;
        const Point2D b = loop[(i + 1) % k].point;
        const double len = segmentLength(model::Segment{a, b});
        const Point2D dir{(b.x_mm - a.x_mm) / len, (b.y_mm - a.y_mm) / len};
        const Point2D inward{-dir.y_mm, dir.x_mm};  // CCW: Inneres liegt links
        const double shift = side * loop[i].outgoing_thickness_mm / 2.0;
        lines[i] = OffsetLine{Point2D{a.x_mm + (inward.x_mm * shift),
                                      a.y_mm + (inward.y_mm * shift)},
                              dir};
    }
    Ring ring(k);
    for (std::size_t i = 0; i < k; ++i) {
        const std::size_t prev = (i + k - 1) % k;
        ring[i] = lines[prev].intersectionWith(lines[i]);
    }
    return ring;
}

// Prüft, ob der Offset-Ring die Kantenrichtungen des Wandzugs erhält.
// Kollabiert der Innen-Offset (Zyklus kleiner als die Wandstärken),
// kehrt sich mindestens eine Kante um — bei Inversion in BEIDEN Achsen
// entstünde sonst ein scheinbar gültiges Phantom-Polygon mit positiver
// Fläche. Richtungs-Erhalt ist das verlässliche Kollaps-Kriterium
// (spez. §1 §Degenerierte Zyklen: kein Raum, kein Fehler).
bool preservesEdgeDirections(const Loop& loop, const Ring& ring) {
    const std::size_t k = loop.size();
    for (std::size_t i = 0; i < k; ++i) {
        const Point2D a = loop[i].point;
        const Point2D b = loop[(i + 1) % k].point;
        const Point2D va = ring[i];
        const Point2D vb = ring[(i + 1) % k];
        const double dot = ((b.x_mm - a.x_mm) * (vb.x_mm - va.x_mm)) +
                           ((b.y_mm - a.y_mm) * (vb.y_mm - va.y_mm));
        if (dot <= 0.0) {
            return false;
        }
    }
    return true;
}

// Zyklus-Kandidat: Mittellinien-Ring (Verschachtelungs-Test), Innenkanten-
// Ring (Raumpolygon) und Außenkontur (Loch-Ring im umschließenden Raum).
// Kandidaten haben per Konstruktion eine nicht-degenerierte Mittellinie
// (extractClosedLoops filtert Außen- und Null-Flächen); kollabiert nur
// der INNEN-Offset, bleibt der Kandidat als Loch-Lieferant erhalten.
// Mittellinien-degenerierte Züge (kollinear/überlappend) sind welle-1
// out-of-scope und liefern auch keine Löcher (spez. §1, Review M3).
struct Candidate {
    Ring centerline;
    Ring inner;
    Ring outer_contour;
    double inner_area{};

    bool isInside(const Candidate& outer) const {
        return pointInRing(outer.centerline, centerline.front());
    }
};

std::vector<Candidate> buildCandidates(const std::vector<Loop>& loops) {
    std::vector<Candidate> candidates;
    for (const Loop& loop : loops) {
        Candidate c;
        c.centerline = centerlineRing(loop);
        c.inner = offsetRing(loop, +1.0);
        c.outer_contour = offsetRing(loop, -1.0);
        // Kollabierter Innen-Offset -> kein Raumpolygon (inner_area 0
        // hält den Kandidaten unterhalb der Raum-Schwelle); die
        // Außenkontur bleibt als Loch-Ring im umschließenden Raum gültig.
        c.inner_area = preservesEdgeDirections(loop, c.inner)
                           ? signedArea(c.inner)
                           : 0.0;
        candidates.push_back(std::move(c));
    }
    return candidates;
}

// Direktes Kind: in `parent_index` enthalten, ohne dazwischenliegenden
// Zyklus. Index-Parameter statt zweier Candidate-Referenzen — die
// Rollen (child, parent) sind über die Namen gebunden (Review L1).
bool isDirectChild(const std::vector<Candidate>& candidates,
                   std::size_t child_index, std::size_t parent_index) {
    if (child_index == parent_index ||
        !candidates[child_index].isInside(candidates[parent_index])) {
        return false;
    }
    for (std::size_t mid = 0; mid < candidates.size(); ++mid) {
        if (mid == child_index || mid == parent_index) {
            continue;
        }
        if (candidates[child_index].isInside(candidates[mid]) &&
            candidates[mid].isInside(candidates[parent_index])) {
            return false;
        }
    }
    return true;
}

}  // namespace

std::vector<model::Room> detectRooms(const model::Building& building,
                                     model::StoreyId storey) {
    std::vector<const model::Wall*> walls;
    for (const model::Wall& wall : building.walls) {
        if (wall.storey_id == storey) {
            walls.push_back(&wall);
        }
    }
    const std::vector<Candidate> candidates =
        buildCandidates(extractClosedLoops(buildGraph(walls)));

    std::vector<model::Room> rooms;
    int next_id = 1;
    for (std::size_t i = 0; i < candidates.size(); ++i) {
        if (candidates[i].inner_area < kMinAreaMm2) {
            continue;  // kollabierter Innen-Offset -> kein Raum, kein Fehler
        }
        model::Room room;
        room.storey_id = storey;
        room.outer = candidates[i].inner;
        double net = candidates[i].inner_area;
        // Direkte Kinder werden als Loch-Ringe geführt — auch dann, wenn
        // ihr Innen-Offset kollabiert ist und sie selbst keinen Raum
        // ergeben (ihre Wände stehen trotzdem im umschließenden Raum).
        // Spez. §1 Schritt 4.
        for (std::size_t j = 0; j < candidates.size(); ++j) {
            if (!isDirectChild(candidates, j, i)) {
                continue;
            }
            room.holes.push_back(candidates[j].outer_contour);
            net -= signedArea(candidates[j].outer_contour);
        }
        if (net < kMinAreaMm2) {
            continue;  // Löcher füllen den Raum vollständig -> degeneriert
        }
        room.net_area_mm2 = net;
        room.id = static_cast<model::RoomId>(next_id++);
        rooms.push_back(std::move(room));
    }
    return rooms;
}

}  // namespace bcad::hexagon::services
