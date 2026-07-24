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

#include "hexagon/model/derived_geometry.h"               // DerivedGeometry (ADR-0020, slice-042c)
#include "hexagon/model/mesh_ops.h"                       // translateMeshZ (ADR-0020: model/-Util)

namespace fs = std::filesystem;
namespace model = bcad::hexagon::model;
namespace ports = bcad::hexagon::ports::driven;

namespace bcad::adapters::geometry {
namespace {

// --- Bauteil-Netze aus dem kern-berechneten `DerivedGeometry`-Bündel sammeln -----
// Wände/Decken werden **adapter-seitig tessellert** (über den `GeometryKernelPort`,
// OCC-resident, Regel C) aus den kern-gelieferten Primitiven; Dach/Treppe tragen
// bereits das fertige, kern-berechnete Netz. Der try/catch-Skip bleibt hier, weil
// nur die Tessellation (OCC) werfen kann — die Ableitung ist kern-seitig + total.

void appendWallMeshes(std::vector<model::TriangleMesh>& out,
                      const model::DerivedGeometry& derived,
                      const ports::GeometryKernelPort& geometry) {
    for (const model::DerivedWall& w : derived.walls) {
        model::TriangleMesh mesh;
        try {
            mesh = geometry.tessellateFootprint(w.footprint, w.height_mm, w.cutPrisms);
        } catch (const std::exception&) {
            continue;  // degeneriertes Bauteil überspringen (Totalität, kein Abbruch)
        }
        out.push_back(std::move(mesh));
    }
}

void appendSlabMeshes(std::vector<model::TriangleMesh>& out,
                      const model::DerivedGeometry& derived,
                      const ports::GeometryKernelPort& geometry) {
    for (const model::DerivedSlab& s : derived.slabs) {
        model::TriangleMesh mesh;
        try {
            mesh = geometry.tessellateFootprint(s.footprint, s.thickness_mm, s.cutPrisms);
        } catch (const std::exception&) {
            continue;  // degeneriertes Bauteil überspringen (Totalität)
        }
        // Auf die kern-gelieferte Aufstandshöhe verschieben (nach der Tessellation).
        out.push_back(model::translateMeshZ(std::move(mesh), s.baseZ_mm));
    }
}

void appendRoofMeshes(std::vector<model::TriangleMesh>& out,
                      const model::DerivedGeometry& derived) {
    for (const model::DerivedRoof& r : derived.roofs) {
        if (!r.mesh.empty()) {
            out.push_back(r.mesh);  // kern-berechnetes Netz (analytisch, total)
        }
    }
}

void appendStairMeshes(std::vector<model::TriangleMesh>& out,
                       const model::DerivedGeometry& derived) {
    for (const model::DerivedStair& s : derived.stairs) {
        if (!s.mesh.empty()) {
            out.push_back(s.mesh);  // kern-berechnetes Netz inkl. Geländer
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

std::string buildStl(const std::vector<model::TriangleMesh>& meshes,
                     const model::ExportProvenance& provenance) {
    std::string out;
    // 80-Byte-Header — NICHT mit "solid" beginnen (sonst ASCII-Heuristik). Bei
    // injizierter Herkunft (slice-046) trägt er sie, sonst den festen Banner; in
    // JEDEM Fall auf ≤80 Byte geklemmt — das rohe `80 - size` würde bei > 80 Byte
    // `size_t`-underflowen (OOM/Crash, MR-006-046-MED-2).
    std::string banner =
        provenance.empty()
            ? std::string("b-cad binary STL export")
            : ("b-cad STL " + provenance.version + " " + provenance.source + " " +
               provenance.date);
    if (banner.size() > 80) {
        banner.resize(80);
    }
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

void StlExportAdapter::write(const model::Building& /*building*/,
                             const model::DerivedGeometry& derived,
                             const fs::path& path,
                             const model::ExportProvenance& provenance) const {
    std::vector<model::TriangleMesh> meshes;
    appendWallMeshes(meshes, derived, geometry_);
    appendSlabMeshes(meshes, derived, geometry_);
    appendRoofMeshes(meshes, derived);
    appendStairMeshes(meshes, derived);
    atomicWrite(path, buildStl(meshes, provenance));  // im Speicher, dann atomar
}

}  // namespace bcad::adapters::geometry
