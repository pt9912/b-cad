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

FROM ghcr.io/pt9912/d-check@sha256:871751f9ed13cec8fadb610c538d735ad6c77a838a0384f645da916e829e4550 AS docs-check
WORKDIR /repo
COPY . .

