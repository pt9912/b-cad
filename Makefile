# b-cad — Harness-Gates.
#
# Stand: Greenfield-Bootstrap. Es existiert genau EIN echtes Gate
# (docs-check, Doku-Link-Validator). Code-Gates (build, lint, arch-check,
# test, coverage-gate) entstehen mit dem ersten Code-Slice (slice-002,
# Promotion-Trigger) — sie sind hier bewusst NICHT als Targets behauptet,
# damit `make gates` kein halluziniertes Aggregat wird (Kurs-Modul 13).

DOCKER ?= docker
DOCS_CHECK_IMAGE ?= bcad-docs-check
BUILD_IMAGE ?= bcad-build

.PHONY: help build docs-check docs-check-image gates

help: ## Targets anzeigen
	@grep -E '^[a-zA-Z_-]+:.*?## ' $(MAKEFILE_LIST) | \
		awk 'BEGIN {FS = ":.*?## "}; {printf "  %-20s %s\n", $$1, $$2}'

build: ## ADR-0001 — reproduzierbarer Build im DevContainer (CMake-Target-Trennung Kern/Adapter) + ctest
	$(DOCKER) build -f .devcontainer/Dockerfile --target build -t $(BUILD_IMAGE) .

docs-check-image: ## Doku-Validator-Image bauen (tools/Dockerfile)
	$(DOCKER) build -t $(DOCS_CHECK_IMAGE) tools/

docs-check: docs-check-image ## Doku-Konsistenz — interne Markdown-Links/Anker/ID-Pfade (Modul 11/13)
	$(DOCKER) run --rm -v "$(CURDIR)":/work $(DOCS_CHECK_IMAGE)

gates: docs-check ## alle inneren Gates (derzeit nur docs-check; Code-Gates folgen mit slice-002)
