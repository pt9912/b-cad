# b-cad Doku-Validator — Build-Stage-Gate, KEIN Bind-Mount.
# Bauen mit Repo-Root als Kontext; die Validierung läuft als RUN:
#
#   docker build -f tools/Dockerfile --target docs-check -t bcad-docs-check .
#
# Schlägt docs-check fehl, schlägt der Build fehl. Seit MR-007 läuft
# die Validierung über das digest-gepinnte d-check-Image
# (https://github.com/pt9912/d-check, Konfiguration: .d-check.yml);
# das vendorte docs-check.js (MR-003) ist abgelöst. Digest-Hebungen
# sind Routine-Pins mit Begründung im Commit-Body.

FROM ghcr.io/pt9912/d-check@sha256:7bdef75be26e808a137ad817cd7fec8bf4eccd282fa78932d2e3d3bbc0e050eb AS docs-check
WORKDIR /repo
COPY . .
RUN ["/d-check", "/repo"]
