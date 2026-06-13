#include "hexagon/services/slab_geometry.h"

#include <cstddef>

#include "hexagon/model/constants.h"

namespace bcad::hexagon::services {

double slabBaseZ(const model::Slab& slab, double storey_height_mm) {
    switch (slab.type) {
        case model::SlabType::Decke:
            return storey_height_mm;  // auf der Geschoss-Oberkante
        case model::SlabType::Bodenplatte:
        case model::SlabType::Fundament:
            return -slab.thickness_mm;  // Oberkante auf Höhe 0, Dicke nach unten
    }
    return 0.0;
}

std::vector<model::CutPrism> slabCutPrisms(const model::Slab& slab) {
    std::vector<model::CutPrism> prisms;
    prisms.reserve(slab.cutouts.size());
    const double eps = model::kOpeningCutOvershootMm;
    for (const model::Footprint& cut : slab.cutouts) {
        model::CutPrism prism;
        prism.polygon = cut;
        prism.z_min_mm = -eps;                       // relativ zum Solid [0,Dicke]
        prism.z_max_mm = slab.thickness_mm + eps;    // Überstand volumen-neutral
        prisms.push_back(prism);
    }
    return prisms;
}

model::TriangleMesh translateMeshZ(model::TriangleMesh mesh, double dz) {
    for (std::size_t i = 2; i < mesh.positions.size(); i += 3) {
        mesh.positions[i] += dz;
    }
    return mesh;
}

}  // namespace bcad::hexagon::services
