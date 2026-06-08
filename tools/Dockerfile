# b-cad Doku-Validator — Build-Stage-Gate, KEIN Bind-Mount.
# Bauen mit Repo-Root als Kontext; die Validierung läuft als RUN:
#
#   docker build -f tools/Dockerfile --target docs-check -t bcad-docs-check .
#
# Schlägt docs-check fehl, schlägt der Build fehl. Vendored aus dem
# AI-Harness-Kurs (MR-003); Single-Purpose (kein alignment-check).
#
# Hinweis (Modul 14): Base-Pinning per @sha256 ist Zielzustand.

FROM node:22-alpine AS base
COPY tools/docs-check.js /usr/local/bin/docs-check.js
RUN node --check /usr/local/bin/docs-check.js \
 && chmod +x /usr/local/bin/docs-check.js

# --- docs-check: Repo einbacken und Links/Anker/Pfade validieren ---
FROM base AS docs-check
WORKDIR /work
COPY . .
RUN node /usr/local/bin/docs-check.js
