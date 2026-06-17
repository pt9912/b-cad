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
# Pin: d-check v0.11.0 (MR-007/MR-011 dokumentieren den Adoptions-Stand v0.8.0;
# der Live-Pin lebt hier, Hebungen sind Routine mit Begründung im Commit-Body).

FROM ghcr.io/pt9912/d-check@sha256:6ec1c463b5276b3314881839bd800b5e9aab12fa624a35d31618cecb62f17795 AS docs-check
WORKDIR /repo
COPY . .

