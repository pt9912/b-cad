# b-cad DevContainer / Build (REQ-TEC-009, Modul 14).
#
# Multi-Stage: deps (gepinnte Toolchain) -> build (kompiliert + ctest).
# Der `build`-Stage IST der Reproduzierbarkeits-Gate: schlägt die
# Kompilierung oder ein Test fehl, schlägt `make build` fehl.
#
# Hinweis (Modul 14): Zielzustand ist Base-Pinning per @sha256-Digest
# und ein Image-Hash-Vertrag (harness/image-hash.txt). Solange dieser
# Vertrag nicht etabliert ist, bleibt der Tag — bewusst, dokumentiert,
# nicht stillschweigend.

FROM ubuntu:24.04 AS deps
ENV DEBIAN_FRONTEND=noninteractive
RUN apt-get update && apt-get install -y --no-install-recommends \
        build-essential \
        cmake \
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

# --- Build-Stage: kompiliert die Target-Kette und führt ctest aus ---
FROM deps AS build
WORKDIR /src
COPY . .
RUN cmake -S . -B build -G Ninja \
 && cmake --build build \
 && ctest --test-dir build --output-on-failure
