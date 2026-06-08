# b-cad — Harness-Gates.
#
# Jeder Gate ist eine Dockerfile-Target-Stage (Quelle per COPY ins Image
# gebacken, Gate als RUN) — KEINE Bind-Mounts, maximal reproduzierbar
# (Modul 14, Vorbild cmake-xray). `make gates` aggregiert nur real
# existierende Targets (Kurs-Modul 13).

DOCKER ?= docker
IMAGE ?= bcad
DOCKERFILE ?= .devcontainer/Dockerfile

# Gate-Kalibrierung (Threshold-as-Variable; "Kalibrierungs-Bindung"
# Modul 13). coverage-gate ist bootstrap-aware: niedrige Anfangsschwelle,
# Ramp-Trigger dokumentiert in AGENTS.md §3.
COVERAGE_THRESHOLD ?= 70

# Gate = Build einer Stage; --target wählt sie, der Kontext ist das Repo.
GATE = $(DOCKER) build -f $(DOCKERFILE)

.PHONY: help dev-image build test lint arch-check coverage-gate docs-check gate-consistency record-gates gates

help: ## Targets anzeigen
	@grep -E '^[a-zA-Z_-]+:.*?## ' $(MAKEFILE_LIST) | \
		awk 'BEGIN {FS = ":.*?## "}; {printf "  %-16s %s\n", $$1, $$2}'

dev-image: ## Toolchain-Image (deps-Stage) — z. B. für die IDE/DevContainer
	$(GATE) --target deps -t $(IMAGE):deps .

build: ## ADR-0001 — Target-Kette kompilieren (Kern ohne Adapter-Deps)
	$(GATE) --target build -t $(IMAGE):build .

test: ## REQ-TEC-005 — GoogleTest (Kern + Adapter-Linkage)
	$(GATE) --target test -t $(IMAGE):test .

lint: ## clang-tidy + Suppression-Gate (AGENTS.md §2.4)
	$(GATE) --target lint -t $(IMAGE):lint .

arch-check: ## ADR-0001 — hexagonale Schichtung (Kern ohne Qt/OCC/SQLite/adapters)
	$(GATE) --target arch-check -t $(IMAGE):arch-check .

coverage-gate: ## bootstrap-aware Coverage (Schwelle $(COVERAGE_THRESHOLD)%, gcov; Composition Root ausgenommen)
	$(GATE) --target coverage-check \
		--build-arg BCAD_COVERAGE_THRESHOLD=$(COVERAGE_THRESHOLD) \
		-t $(IMAGE):coverage-check .

docs-check: ## Doku-Konsistenz — interne Markdown-Links/Anker/ID-Pfade (Modul 11/13)
	$(DOCKER) build -f tools/Dockerfile --target docs-check -t $(IMAGE):docs-check .

gate-consistency: ## Modul 13 — jeder als real dokumentierte make-Befehl existiert (Doku↔Makefile)
	$(GATE) --target gate-consistency -t $(IMAGE):gate-consistency .

record-gates: ## Nachweis schreiben: Working-Tree-Hash (für den Stop-Hook)
	@bash tools/harness/record-gates.sh

# `build` ist NICHT separat gelistet: test/lint/coverage-gate sind
# Dockerfile-Stages FROM build und kompilieren die Target-Kette bereits
# (Variante B, siehe harness/README §Sensors). record-gates läuft als
# LETZTER Prerequisite — der Nachweis entsteht nur, wenn alle Gates grün
# sind (sonst bricht make vorher ab).
gates: docs-check gate-consistency arch-check lint test coverage-gate record-gates ## alle inneren Gates (mandatory vor PR)
