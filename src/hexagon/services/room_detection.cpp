#include "hexagon/services/room_detection.h"

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

    // Schnittpunkt mit `other`; bei (nahezu) parallelen Geraden der
    // Fallback `other.origin` (kollineare Nachbarkanten gleicher Stärke
    // liegen auf derselben Offset-Geraden).
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
    // pro Knoten: (Kanten-Index, Nachbar-Knoten)
    std::vector<std::vector<std::pair<std::size_t, std::size_t>>> adjacency;
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
    graph.adjacency.emplace_back();
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
        const std::size_t edge = graph.edge_thickness.size();
        graph.edge_thickness.push_back(wall->thickness_mm);
        graph.adjacency[a].emplace_back(edge, b);
        graph.adjacency[b].emplace_back(edge, a);
    }
    return graph;
}

// Läuft einen Grad-2-Zyklus ab Startknoten ab und sammelt Punkte plus
// Stärke der ausgehenden Kante.
Loop traceLoop(const Graph& graph, std::size_t start) {
    Loop loop;
    const auto& [first_edge, first_next] = graph.adjacency[start].front();
    loop.push_back(LoopVertex{graph.nodes[start], graph.edge_thickness[first_edge]});
    std::size_t via_edge = first_edge;
    std::size_t current = first_next;
    while (current != start) {
        const auto& [e0, n0] = graph.adjacency[current][0];
        const auto& [e1, n1] = graph.adjacency[current][1];
        const std::size_t out_edge = (e0 == via_edge) ? e1 : e0;
        const std::size_t out_node = (e0 == via_edge) ? n1 : n0;
        loop.push_back(LoopVertex{graph.nodes[current], graph.edge_thickness[out_edge]});
        via_edge = out_edge;
        current = out_node;
    }
    return loop;
}

// Geschlossene Wandzüge: Zusammenhangskomponenten, in denen jeder Knoten
// Grad 2 hat (welle-1: nur endpunkt-verbundene Zyklen — Schnittpunkte/
// T-Stöße erst mit LH-FA-WAL-006; offene Züge haben Grad-1-Enden und
// erzeugen keinen Raum, LH-FA-ROM-001 Negative).
std::vector<Loop> extractClosedLoops(const Graph& graph) {
    std::vector<Loop> loops;
    std::vector<bool> visited(graph.nodes.size(), false);
    for (std::size_t start = 0; start < graph.nodes.size(); ++start) {
        if (visited[start]) {
            continue;
        }
        std::vector<std::size_t> stack{start};
        visited[start] = true;
        std::size_t component_size = 0;
        bool all_degree_two = true;
        while (!stack.empty()) {
            const std::size_t node = stack.back();
            stack.pop_back();
            ++component_size;
            if (graph.adjacency[node].size() != 2) {
                all_degree_two = false;
            }
            for (const auto& [edge, other] : graph.adjacency[node]) {
                if (!visited[other]) {
                    visited[other] = true;
                    stack.push_back(other);
                }
            }
        }
        if (all_degree_two && component_size >= 3) {
            loops.push_back(traceLoop(graph, start));
        }
    }
    return loops;
}

// Normalisiert auf CCW-Orientierung (Inneres links der Kanten); beim
// Umdrehen wandert die Kanten-Stärke mit: t'[i] = t[(2k-2-i) % k].
void normalizeCounterClockwise(Loop& loop) {
    if (signedArea(centerlineRing(loop)) >= 0.0) {
        return;
    }
    const std::size_t k = loop.size();
    Loop reversed(k);
    for (std::size_t i = 0; i < k; ++i) {
        reversed[i].point = loop[k - 1 - i].point;
        reversed[i].outgoing_thickness_mm =
            loop[((2 * k) - 2 - i) % k].outgoing_thickness_mm;
    }
    loop = std::move(reversed);
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
struct Candidate {
    Ring centerline;
    Ring inner;
    Ring outer_contour;
    double inner_area{};
};

std::vector<Candidate> buildCandidates(std::vector<Loop>&& loops) {
    std::vector<Candidate> candidates;
    for (Loop& loop : loops) {
        normalizeCounterClockwise(loop);
        Candidate c;
        c.centerline = centerlineRing(loop);
        if (signedArea(c.centerline) < kMinAreaMm2) {
            continue;  // Zyklus selbst degeneriert
        }
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

bool containedIn(const Candidate& inner, const Candidate& outer) {
    return pointInRing(outer.centerline, inner.centerline.front());
}

// Direktes Kind: in `parent` enthalten, ohne dazwischenliegenden Zyklus.
bool isDirectChild(const std::vector<Candidate>& candidates, std::size_t child,
                   std::size_t parent) {
    if (child == parent || !containedIn(candidates[child], candidates[parent])) {
        return false;
    }
    for (std::size_t mid = 0; mid < candidates.size(); ++mid) {
        if (mid == child || mid == parent) {
            continue;
        }
        if (containedIn(candidates[child], candidates[mid]) &&
            containedIn(candidates[mid], candidates[parent])) {
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
        // sie selbst keinen Raum ergeben (ihre Wände stehen trotzdem im
        // umschließenden Raum). Spez. §1 Schritt 4.
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
