// slice-008b: Persistenz-Härtung.
//  - Crash-Recovery (LH-QA-005): echtes fork()+SIGKILL mitten im save; die
//    Zieldatei ist nach jedem Kill vollständig ladbar (Zustand A ODER B,
//    nie korrupt) — Evidenz für die Atomik (Temp+Rename), keine Inferenz.
//  - E-IO-001 (kein Schreibrecht/kann nicht anlegen) und E-IO-002
//    (Schreibfehler/Medium voll): getrennte Spec-Codes, beide am realen save.

#include "adapters/persistence/sqlite_project_repository.h"

#include <sys/resource.h>
#include <sys/wait.h>
#include <unistd.h>

#include <csignal>
#include <filesystem>
#include <fstream>
#include <stdexcept>
#include <string>

#include <gtest/gtest.h>

#include "hexagon/model/building.h"

namespace {

namespace fs = std::filesystem;
using bcad::adapters::persistence::SqliteProjectRepository;
using bcad::hexagon::model::Building;
using bcad::hexagon::model::StoreyId;
using bcad::hexagon::model::WallId;
using bcad::hexagon::model::WallType;

Building standA() {
    Building b;
    b.storeys.push_back({StoreyId{1}, 2500.0});
    b.walls.push_back(
        {WallId{1}, StoreyId{1}, {0.0, 0.0}, {4000.0, 0.0}, 240.0, 2500.0,
         WallType::Aussen});
    return b;
}

Building standB() {
    Building b;
    b.storeys.push_back({StoreyId{1}, 2500.0});
    b.storeys.push_back({StoreyId{2}, 3000.0});
    b.walls.push_back(
        {WallId{1}, StoreyId{1}, {0.0, 0.0}, {4000.0, 0.0}, 240.0, 2500.0,
         WallType::Aussen});
    b.walls.push_back(
        {WallId{2}, StoreyId{2}, {0.0, 0.0}, {3000.0, 0.0}, 300.0, 3000.0,
         WallType::Trag});
    return b;
}

void cleanup(const fs::path& path) {
    const fs::path tmp = path.string() + ".tmp";
    fs::remove(path);
    fs::remove_all(tmp);
    fs::remove(fs::path(tmp.string() + "-journal"));
    fs::remove(fs::path(tmp.string() + "-wal"));
}

// RAII: setzt RLIMIT_FSIZE (Soft) + ignoriert SIGXFSZ und stellt BEIDES im
// Destruktor wieder her — exception-safe (auch bei nicht-runtime_error-
// Würfen), damit ein gesetztes Limit keine Folge-Tests im selben Prozess
// beeinflusst (Test-Isolation).
class FileSizeLimit {
public:
    explicit FileSizeLimit(rlim_t soft_bytes) {
        ::sigaction(SIGXFSZ, nullptr, &prev_sa_);  // Disposition merken
        struct sigaction ignore {};
        ignore.sa_handler = SIG_IGN;
        sigemptyset(&ignore.sa_mask);
        ::sigaction(SIGXFSZ, &ignore, nullptr);
        ::getrlimit(RLIMIT_FSIZE, &prev_);
        struct rlimit lim {};
        lim.rlim_cur = soft_bytes;
        lim.rlim_max = prev_.rlim_max;
        ::setrlimit(RLIMIT_FSIZE, &lim);
    }
    ~FileSizeLimit() {
        ::setrlimit(RLIMIT_FSIZE, &prev_);
        ::sigaction(SIGXFSZ, &prev_sa_, nullptr);
    }
    FileSizeLimit(const FileSizeLimit&) = delete;
    FileSizeLimit& operator=(const FileSizeLimit&) = delete;

private:
    struct rlimit prev_ {};
    struct sigaction prev_sa_ {};
};

// LH-QA-005: kill -9 zwischen Schreibphasen → letzter konsistenter Stand
// intakt. Eine nicht-atomare Implementierung würde über N Trials eine
// halb-geschriebene Zieldatei hinterlassen → load wirft/liefert Unsinn.
TEST(SqliteCrashRecovery, KillMidSaveLaesstZielKonsistent) {
    const SqliteProjectRepository repo;
    const fs::path path = fs::temp_directory_path() / "bcad_crash.bcad";
    cleanup(path);

    repo.save(standA(), path);  // vollständiges A: Ziel existiert immer
    const Building b = standB();

    constexpr int kTrials = 60;
    for (int trial = 0; trial < kTrials; ++trial) {
        const pid_t pid = fork();
        ASSERT_GE(pid, 0) << "fork fehlgeschlagen";
        if (pid == 0) {
            // Kind: B in Endlosschleife speichern; KEINE GTest-Assertions,
            // kein normales Ende (wird gekillt). Fehler ignorieren.
            for (;;) {
                try {
                    repo.save(b, path);
                } catch (...) {
                    continue;  // Fehler ignorieren, weiter speichern
                }
            }
        }
        // Eltern: nach deterministischem Jitter killen, dann verifizieren.
        const useconds_t delay =
            static_cast<useconds_t>(50 + (trial * 137) % 900);
        ::usleep(delay);
        ::kill(pid, SIGKILL);
        int status = 0;
        ::waitpid(pid, &status, 0);

        Building loaded;
        ASSERT_NO_THROW(loaded = repo.load(path)) << "trial " << trial;
        const bool is_a = loaded.storeys.size() == 1 && loaded.walls.size() == 1;
        const bool is_b = loaded.storeys.size() == 2 && loaded.walls.size() == 2;
        EXPECT_TRUE(is_a || is_b)
            << "trial " << trial << ": storeys=" << loaded.storeys.size()
            << " walls=" << loaded.walls.size();
    }
    cleanup(path);
}

// E-IO-001: Temp-Pfad mit nicht-leerem Verzeichnis besetzt → Anlegen/Öffnen
// der Temp-DB scheitert (SQLITE_CANTOPEN). Voriger Stand A unverändert.
TEST(SqliteCrashRecovery, OpenFehlerWirftEIO001UndLaesstStandIntakt) {
    const SqliteProjectRepository repo;
    const fs::path path = fs::temp_directory_path() / "bcad_eio001.bcad";
    cleanup(path);

    repo.save(standA(), path);
    const auto size_before = fs::file_size(path);

    const fs::path tmp = path.string() + ".tmp";
    fs::create_directory(tmp);
    { std::ofstream(tmp / "block.txt") << "x"; }  // nicht leer

    std::string what;
    try {
        repo.save(standB(), path);
        ADD_FAILURE() << "save haette werfen muessen";
    } catch (const std::runtime_error& e) {
        what = e.what();
    }
    EXPECT_NE(what.find("E-IO-001"), std::string::npos) << what;
    EXPECT_TRUE(fs::exists(path));
    EXPECT_EQ(fs::file_size(path), size_before);  // voriger Stand unveraendert
    EXPECT_NO_THROW((void)repo.load(path));

    cleanup(path);  // raeumt auch das besetzte Verzeichnis (remove_all)
}

// E-IO-002: echter Schreibfehler. RLIMIT_FSIZE begrenzt die Dateigröße; der
// Temp-DB-Write überschreitet das Limit → SQLITE_FULL/IOERR. SIGXFSZ wird
// ignoriert, sonst killt das Überschreiten den Prozess. Voriger Stand intakt.
TEST(SqliteCrashRecovery, WriteVollWirftEIO002UndLaesstStandIntakt) {
    const SqliteProjectRepository repo;
    const fs::path path = fs::temp_directory_path() / "bcad_eio002.bcad";
    cleanup(path);

    repo.save(standA(), path);
    const auto size_before = fs::file_size(path);

    std::string what;
    bool threw = false;
    {
        // 8192 muss < frische Schema-DB-Größe bleiben (sonst kein Write-Fehler
        // → EXPECT_TRUE(threw) schlägt LAUT fehl, kein stiller False-Pass).
        const FileSizeLimit limit(8192);
        try {
            repo.save(standB(), path);
        } catch (const std::runtime_error& e) {
            threw = true;
            what = e.what();
        }
    }  // Limit + SIGXFSZ-Disposition hier garantiert wiederhergestellt

    EXPECT_TRUE(threw);
    EXPECT_NE(what.find("E-IO-002"), std::string::npos) << what;
    EXPECT_TRUE(fs::exists(path));
    EXPECT_EQ(fs::file_size(path), size_before);
    EXPECT_NO_THROW((void)repo.load(path));

    cleanup(path);
}

// Deckt den separaten Rename-Fehlerzweig (`ioCodeForErrno`): die Temp-DB
// wird erfolgreich geschrieben, aber `rename(tmp, ziel)` scheitert, weil das
// Ziel ein nicht-leeres Verzeichnis ist → neutraler E-IO-Code aus dem
// errno-Pfad (statt aus dem SqliteError-Pfad von open/exec).
TEST(SqliteCrashRecovery, RenameFehlerWirftNeutralenEIOCode) {
    const SqliteProjectRepository repo;
    const fs::path path = fs::temp_directory_path() / "bcad_rename.bcad";
    cleanup(path);
    fs::remove_all(path);

    fs::create_directory(path);                    // Ziel ist ein Verzeichnis …
    { std::ofstream(path / "block.txt") << "x"; }  // … nicht leer → rename scheitert

    std::string what;
    try {
        repo.save(standA(), path);
        ADD_FAILURE() << "save haette werfen muessen";
    } catch (const std::runtime_error& e) {
        what = e.what();
    }
    EXPECT_NE(what.find("rename"), std::string::npos) << what;
    EXPECT_NE(what.find("E-IO-"), std::string::npos) << what;

    fs::remove_all(path);
}

}  // namespace
