# b-cad Doku-Validator — Build-Stage-Gate, KEIN Bind-Mount.
# Bauen mit Repo-Root als Kontext
#
#   docker build -f tools/Dockerfile --target docs-check -t bcad-docs-check .
#
# Seit MR-007 läuft
# die Validierung über das digest-gepinnte d-check-Image
# (https://github.com/pt9912/d-check, Konfiguration: .d-check.yml);
# Digest-Hebungen
# sind Routine-Pins mit Begründung im Commit-Body.

FROM ghcr.io/pt9912/d-check@sha256:5bccf9fb3d1c54639dec3a541771d2ea43db9a0c1c58c28b3f12f20d38133d1b AS docs-check
WORKDIR /repo
COPY . .

