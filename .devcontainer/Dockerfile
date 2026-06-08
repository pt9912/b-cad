# b-cad DevContainer / Gates (REQ-TEC-009, Modul 14).
#
# Jeder Gate ist eine Dockerfile-Target-Stage: die Quelle wird per COPY
# ins Image gebacken und der Gate als RUN zur Build-Zeit ausgeführt —
# KEINE Bind-Mounts. Schlägt ein Gate fehl, schlägt `docker build
# --target <gate>` fehl. Vorbild: cmake-xray.
#
#   docker build -f .devcontainer/Dockerfile --target test          .
#   docker build -f .devcontainer/Dockerfile --target lint          .
#   docker build -f .devcontainer/Dockerfile --target coverage-check .
#
# Hinweis (Modul 14): Zielzustand ist Base-Pinning per @sha256 + ein
# Image-Hash-Vertrag (harness/image-hash.txt). Solange der nicht
# etabliert ist, bleibt der Tag — bewusst, dokumentiert.

# --- Toolchain ---
FROM ubuntu:24.04 AS deps
ENV DEBIAN_FRONTEND=noninteractive
RUN apt-get update && apt-get install -y --no-install-recommends \
        build-essential \
        clang-tidy \
        cmake \
        gcovr \
        ninja-build \
        qt6-base-dev \
        libocct-foundation-dev \
        libocct-modeling-data-dev \
        libocct-modeling-algorithms-dev \
        libocct-data-exchange-dev \
        libtbb-dev \
        libsqlite3-dev \
        libgtest-dev \
    && rm -rf /var/lib/apt/lists/*

# --- build: kompiliert die Target-Kette (Kern + Adapter) ---
FROM deps AS build
WORKDIR /src
COPY . .
RUN cmake -S . -B build -G Ninja && cmake --build build

# --- test: GoogleTest (Kern + Adapter-Linkage), REQ-TEC-005 ---
FROM build AS test
RUN ctest --test-dir build --output-on-failure

# --- lint: clang-tidy + Suppression-Gate (AGENTS.md §2.4) ---
FROM build AS lint
RUN clang-tidy -p build --warnings-as-errors='*' $(find src -name '*.cpp') \
 && bash tools/suppression-gate.sh

# --- coverage-check: bootstrap-aware Coverage (gcov), ARG-kalibriert ---
FROM deps AS coverage-check
ARG BCAD_COVERAGE_THRESHOLD=70
WORKDIR /src
COPY . .
RUN cmake -S . -B build-cov -G Ninja -DBCAD_COVERAGE=ON >/dev/null \
 && cmake --build build-cov >/dev/null \
 && ctest --test-dir build-cov >/dev/null \
 && gcovr --root . --filter 'src/' --exclude '.*/main\.cpp' \
        --fail-under-line ${BCAD_COVERAGE_THRESHOLD} --print-summary build-cov

# --- arch-check: hexagonale Schichtung (ADR-0001), rein textbasiert ---
# Eigene schlanke Stage ohne Toolchain — der Gate braucht nur grep/sed.
FROM ubuntu:24.04 AS arch-check
WORKDIR /src
COPY . .
RUN bash tools/arch-check.sh
