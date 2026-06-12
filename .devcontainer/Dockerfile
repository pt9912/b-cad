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
FROM ubuntu:26.04@sha256:f3d28607ddd78734bb7f71f117f3c6706c666b8b76cbff7c9ff6e5718d46ff64 AS deps
ENV DEBIAN_FRONTEND=noninteractive

# Reproduzierbare apt-Versionen (ADR-0004): Base per @sha256-Digest gepinnt
# (FROM oben) + apt-Quellen auf einen snapshot.ubuntu.com-Timestamp
# umgestellt. Updates erfolgen durch bewusstes Hochsetzen von Digest +
# Timestamp. snapshot.ubuntu.com erzwingt https (http -> 301), daher zuerst
# ca-certificates aus dem Live-Archiv (kein Toolchain-Paket -> Versions-
# Drift hier unkritisch), dann archive + security auf den Snapshot pinnen.
ARG APT_SNAPSHOT=20260601T000000Z
RUN apt-get update \
 && apt-get install -y --no-install-recommends ca-certificates \
 && sed -i -E "s#https?://(archive|security)\.ubuntu\.com/ubuntu/#https://snapshot.ubuntu.com/ubuntu/${APT_SNAPSHOT}/#" \
        /etc/apt/sources.list.d/ubuntu.sources \
 && rm -rf /var/lib/apt/lists/*

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
        xvfb \
    && rm -rf /var/lib/apt/lists/*
# xvfb: Headless-GL für Viewer-Tests/ACC-002-Beleg — die Qt-offscreen-
# QPA trägt kein OpenGL (ADR-0010, präzisiert ADR-0009 (f)).

# --- build: kompiliert die Target-Kette (Kern + Adapter) ---
FROM deps AS build
WORKDIR /src
COPY . .
RUN cmake -S . -B build -G Ninja && cmake --build build

# --- test: GoogleTest (Kern + Adapter-Linkage), REQ-TEC-005 ---
# xvfb-run: GL-Viewer-Tests rendern headless via Xvfb + Mesa/llvmpipe
# (ADR-0010); die QPA-Wahl liegt beim Harness, nicht in den Tests.
FROM build AS test
RUN xvfb-run -a ctest --test-dir build --output-on-failure

# --- lint: clang-tidy + Suppression-Gate (AGENTS.md §2.4) ---
# slice-004: Auf 26.04 ist g++-15 der Compiler (Header in /usr/include/c++/15),
# aber /usr/lib/gcc/x86_64-linux-gnu/ enthält auch ein 16er-Verzeichnis
# (libgcc-16). clang-tidy (clang-21) wählt sonst die HÖCHSTE GCC-Version (16)
# und findet dort keine libstdc++-Header ('string'/'iostream' not found).
# --gcc-install-dir nagelt clang auf die gcc-15-Toolchain fest (= echter
# Compiler). Beim Hochsetzen des Toolchain-Pins (ADR-0004) mitprüfen.
FROM build AS lint
RUN clang-tidy -p build \
        --extra-arg=--gcc-install-dir=/usr/lib/gcc/x86_64-linux-gnu/15 \
        --warnings-as-errors='*' $(find src -name '*.cpp') \
 && bash tools/suppression-gate.sh

# --- coverage-check: bootstrap-aware Coverage (gcov), ARG-kalibriert ---
FROM deps AS coverage-check
ARG BCAD_COVERAGE_THRESHOLD=70
WORKDIR /src
COPY . .
RUN cmake -S . -B build-cov -G Ninja -DBCAD_COVERAGE=ON >/dev/null \
 && cmake --build build-cov >/dev/null \
 && xvfb-run -a ctest --test-dir build-cov >/dev/null \
 && gcovr --root . --filter 'src/' --exclude '.*/main\.cpp' \
        --fail-under-line ${BCAD_COVERAGE_THRESHOLD} --print-summary build-cov

# --- arch-check: hexagonale Schichtung (ADR-0001), rein textbasiert ---
# Eigene schlanke Stage ohne Toolchain — der Gate braucht nur grep/sed.
FROM ubuntu:26.04@sha256:f3d28607ddd78734bb7f71f117f3c6706c666b8b76cbff7c9ff6e5718d46ff64 AS arch-check
WORKDIR /src
COPY . .
RUN bash tools/arch-check.sh

# --- gate-consistency: Doku↔Makefile-Konsistenz (Modul 13), textbasiert ---
FROM ubuntu:26.04@sha256:f3d28607ddd78734bb7f71f117f3c6706c666b8b76cbff7c9ff6e5718d46ff64 AS gate-consistency
WORKDIR /src
COPY . .
RUN bash tools/gate-consistency.sh
