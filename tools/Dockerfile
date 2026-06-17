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
# Pin: d-check v0.10.0 (gehoben von v0.8.0; MR-007/MR-011 dokumentieren den
# Adoptions-Stand, der Live-Pin lebt hier).

FROM ghcr.io/pt9912/d-check@sha256:ca49d33f22ecadfd08db03e4487b52b3f2a70dec01a41f2d0f472bfc2012797c AS docs-check
WORKDIR /repo
COPY . .

