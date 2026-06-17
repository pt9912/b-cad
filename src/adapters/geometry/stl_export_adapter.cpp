// STL-Export (ADR-0014, geometrie-resident). Assembliert das Dreiecksnetz aller
// 3D-Bauteile über die Kern-Geometrie-Funktionen + den GeometryKernelPort und
// schreibt eine binäre STL-Datei (atomar). Total: degenerierte Bauteile werden
// übersprungen, 3D-leeres Modell ergibt eine gültige leere STL (kein Wurf).

#include "adapters/geometry/stl_export_adapter.h"

#include <fcntl.h>
#include <unistd.h>

#include <cerrno>
#include <cstddef>
#include <cstdint>
#include <cstring>
#include <stdexcept>
#include <string>
#include <system_error>
#include <utility>
#include <vector>

#include "hexagon/model/constants.h"
#include "hexagon/services/opening_geometry.h"   // wallCutPrisms
#include "hexagon/services/roof_geometry.h"      // roofMesh
#include "hexagon/services/slab_geometry.h"      // slabBaseZ/slabCutPrisms/translateMeshZ
#include "hexagon/services/stair_geometry.h"     // stairMesh
#include "hexagon/services/wall_footprint.h"     // wallFootprint

namespace fs = std::filesystem;
namespace model = bcad::hexagon::model;
namespace services = bcad::hexagon::services;
namespace ports = bcad::hexagon::ports::driven;

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

// --- Bauteil-Netze sammeln (über die Kern-Geometrie wiederverwendet) ---------

void appendWallMeshes(std::vector<model::TriangleMesh>& out,
                      const model::Building& building,
                      const ports::GeometryKernelPort& geometry) {
    for (const model::Wall& w : building.walls) {
        model::TriangleMesh mesh;
        try {
            mesh = geometry.tessellateFootprint(
                services::wallFootprint(w, building.walls), w.height_mm,
                services::wallCutPrisms(w, building.openings));
        } catch (const std::exception&) {
            continue;  // degeneriertes Bauteil überspringen (Totalität, kein Abbruch)
        }
        out.push_back(std::move(mesh));
    }
}

void appendSlabMeshes(std::vector<model::TriangleMesh>& out,
                      const model::Building& building,
                      const ports::GeometryKernelPort& geometry) {
    for (const model::Slab& s : building.slabs) {
        model::TriangleMesh mesh;
        try {
            mesh = geometry.tessellateFootprint(s.footprint, s.thickness_mm,
                                                services::slabCutPrisms(s));
        } catch (const std::exception&) {
            continue;  // degeneriertes Bauteil überspringen (Totalität)
        }
        out.push_back(services::translateMeshZ(
            std::move(mesh), services::slabBaseZ(s, storeyHeight(building, s.storey_id))));
    }
}

void appendRoofMeshes(std::vector<model::TriangleMesh>& out,
                      const model::Building& building) {
    for (const model::Roof& r : building.roofs) {
        model::TriangleMesh mesh = services::roofMesh(r);  // analytisch, total
        if (!mesh.empty()) {
            out.push_back(std::move(mesh));
        }
    }
}

void appendStairMeshes(std::vector<model::TriangleMesh>& out,
                       const model::Building& building) {
    for (const model::Stair& s : building.stairs) {
        model::TriangleMesh mesh =
            services::stairMesh(s, storeyHeight(building, s.from_storey_id));
        if (!mesh.empty()) {
            out.push_back(std::move(mesh));
        }
    }
}

// --- Binäres STL (little-endian; amd64-Zielplattform, REQ-TEC) ---------------

void appendU32LE(std::string& out, std::uint32_t value) {
    for (int i = 0; i < 4; ++i) {
        out.push_back(static_cast<char>((value >> (8 * i)) & 0xFFU));
    }
}

void appendF32LE(std::string& out, float value) {
    std::uint32_t bits = 0;
    std::memcpy(&bits, &value, sizeof(bits));
    appendU32LE(out, bits);
}

// Schreibt die Dreiecke eines Netzes ins STL-Byte-Format (Flat-Shading-Normale
// je Dreieck = Normale des ersten Vertex, TriangleMesh-Konvention).
void appendMeshTriangles(std::string& out, const model::TriangleMesh& mesh) {
    const std::vector<double>& p = mesh.positions;
    const std::vector<double>& n = mesh.normals;
    const std::vector<int>& idx = mesh.indices;
    for (std::size_t t = 0; t + 3 <= idx.size(); t += 3) {
        const std::size_t i0 = static_cast<std::size_t>(idx[t]) * 3;
        appendF32LE(out, static_cast<float>(n[i0]));
        appendF32LE(out, static_cast<float>(n[i0 + 1]));
        appendF32LE(out, static_cast<float>(n[i0 + 2]));
        for (std::size_t k = 0; k < 3; ++k) {
            const std::size_t vi = static_cast<std::size_t>(idx[t + k]) * 3;
            appendF32LE(out, static_cast<float>(p[vi]));
            appendF32LE(out, static_cast<float>(p[vi + 1]));
            appendF32LE(out, static_cast<float>(p[vi + 2]));
        }
        out.push_back('\0');  // attribute byte count (uint16) = 0
        out.push_back('\0');
    }
}

std::string buildStl(const std::vector<model::TriangleMesh>& meshes) {
    std::string out;
    // 80-Byte-Header — NICHT mit "solid" beginnen (sonst ASCII-Heuristik).
    const std::string banner = "b-cad binary STL export";
    out.append(banner);
    out.append(80 - banner.size(), '\0');
    std::uint32_t triangles = 0;
    for (const model::TriangleMesh& m : meshes) {
        triangles += static_cast<std::uint32_t>(m.triangleCount());
    }
    appendU32LE(out, triangles);
    for (const model::TriangleMesh& m : meshes) {
        appendMeshTriangles(out, m);
    }
    return out;
}

// --- Atomares Schreiben (Temp + fsync + Rename) ------------------------------

std::string ioCodeForErrno(int err) {
    switch (err) {
        case EACCES:
        case EPERM:
        case EROFS:
        case EISDIR:
        case ENOTDIR:
        case ENOENT:
            return "E-IO-001";
        default:
            return "E-IO-002";
    }
}

[[noreturn]] void throwIo(int err, const fs::path& at) {
    const std::string code = ioCodeForErrno(err);
    const std::string event =
        code == "E-IO-001" ? "io_no_permission" : "persist_error";
    throw std::runtime_error(code + ": STL-Export fehlgeschlagen ('" +
                             at.string() + "'): " + std::strerror(err) +
                             "; event=" + event);
}

void atomicWrite(const fs::path& path, const std::string& content) {
    const fs::path tmp(path.string() + ".tmp");
    const int fd = ::open(tmp.c_str(), O_WRONLY | O_CREAT | O_TRUNC, 0644);
    if (fd < 0) {
        throwIo(errno, tmp);
    }
    const char* data = content.data();
    std::size_t remaining = content.size();
    while (remaining > 0) {
        const ssize_t written = ::write(fd, data, remaining);
        if (written < 0) {
            const int err = errno;
            ::close(fd);
            std::error_code rm;
            fs::remove(tmp, rm);
            throwIo(err, tmp);
        }
        data += written;
        remaining -= static_cast<std::size_t>(written);
    }
    ::fsync(fd);
    ::close(fd);
    std::error_code ec;
    fs::rename(tmp, path, ec);
    if (ec) {
        std::error_code rm;
        fs::remove(tmp, rm);
        throwIo(ec.value(), path);
    }
}

}  // namespace

StlExportAdapter::StlExportAdapter(const ports::GeometryKernelPort& geometry)
    : geometry_(geometry) {}

void StlExportAdapter::write(const model::Building& building,
                             const fs::path& path) const {
    std::vector<model::TriangleMesh> meshes;
    appendWallMeshes(meshes, building, geometry_);
    appendSlabMeshes(meshes, building, geometry_);
    appendRoofMeshes(meshes, building);
    appendStairMeshes(meshes, building);
    atomicWrite(path, buildStl(meshes));  // vollständig im Speicher, dann atomar
}

}  // namespace bcad::adapters::geometry
