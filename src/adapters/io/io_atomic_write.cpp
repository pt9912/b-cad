// Geteilter atomarer, binär-treuer Datei-Writer (slice-025b, ADR-0016).
// Extrahiert das seit `ifc_export_adapter`/`dxf_export_adapter` duplizierte
// Temp+fsync+Rename-Muster als eine Wahrheit für die neuen PDF/PNG-Adapter —
// **byte-gleiche** errno-Abbildung + `.tmp`-Namensschema (Review-LOW-2), damit
// die Negative-Test-Technik (`.tmp`-Verzeichnis → EISDIR → E-IO-001) trägt.

#include "adapters/io/io_atomic_write.h"

#include <fcntl.h>
#include <unistd.h>

#include <cerrno>
#include <cstddef>
#include <cstring>
#include <stdexcept>
#include <string>
#include <system_error>

namespace fs = std::filesystem;

namespace bcad::adapters::io {
namespace {

// errno → Spec-Fehlercode (byte-gleich zu dxf_export_adapter::ioCodeForErrno):
// Zielpfad nicht beschreibbar (inkl. EISDIR/ENOTDIR/ENOENT der open-Phase) →
// E-IO-001, sonst (Medium voll / IO) → E-IO-002.
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

[[noreturn]] void throwIo(int err, const char* format_label,
                          const std::string& what, const fs::path& at) {
    const std::string code = ioCodeForErrno(err);
    const std::string event =
        code == "E-IO-001" ? "io_no_permission" : "persist_error";
    throw std::runtime_error(code + ": " + format_label + "-Export " + what +
                             " ('" + at.string() + "'): " + std::strerror(err) +
                             "; event=" + event);
}

}  // namespace

void writeFileAtomically(const fs::path& path, const std::string& content,
                         const char* format_label) {
    const fs::path tmp(path.string() + ".tmp");
    const int fd = ::open(tmp.c_str(), O_WRONLY | O_CREAT | O_TRUNC, 0644);
    if (fd < 0) {
        throwIo(errno, format_label, "Zielpfad nicht beschreibbar", tmp);
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
            throwIo(err, format_label, "Schreibfehler", tmp);
        }
        data += written;
        remaining -= static_cast<std::size_t>(written);
    }
    ::fsync(fd);
    ::close(fd);
    std::error_code ec;
    fs::rename(tmp, path, ec);  // atomarer Ersatz; bei Fehler Zielpfad intakt
    if (ec) {
        std::error_code rm;
        fs::remove(tmp, rm);
        throwIo(ec.value(), format_label, "Rename auf Zielpfad", path);
    }
}

}  // namespace bcad::adapters::io
