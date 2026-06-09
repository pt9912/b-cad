# b-cad Doku-Validator — Build-Stage-Gate, KEIN Bind-Mount.
# Bauen mit Repo-Root als Kontext; die Validierung läuft als RUN:
#
#   docker build -f tools/Dockerfile --target docs-check -t bcad-docs-check .
#
# Schlägt docs-check fehl, schlägt der Build fehl. Vendored aus dem
# AI-Harness-Kurs (MR-003); Single-Purpose (kein alignment-check).
#
# Hinweis (Modul 14): Base-Pinning per @sha256 ist Zielzustand.

FROM node:24-alpine@sha256:2bdb65ed1dab192432bc31c95f94155ca5ad7fc1392fb7eb7526ab682fa5bf14 AS base
COPY tools/docs-check.js /usr/local/bin/docs-check.js
RUN node --check /usr/local/bin/docs-check.js \
 && chmod +x /usr/local/bin/docs-check.js

# --- docs-check: Repo einbacken und Links/Anker/Pfade validieren ---
FROM base AS docs-check
WORKDIR /work
COPY . .
RUN node /usr/local/bin/docs-check.js
