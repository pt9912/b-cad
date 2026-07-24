# b-cad — Harness-Gates.
#
# Jeder INTERNE Gate ist eine Dockerfile-Target-Stage (Quelle per COPY ins
# Image gebacken, Gate als RUN) — KEINE Bind-Mounts, maximal reproduzierbar
# (Modul 14, Vorbild cmake-xray). `make gates` aggregiert nur real
# existierende Targets (Kurs-Modul 13).
#
# AUSNAHME (MR-013, slice-030 → erweitert slice-033/MR-014): `a-check`
# (a-check.mk) und `docs-check` (d-check.mk) sind externe, digest-gepinnte
# Gates und laufen als `docker run --network none -v $(CURDIR):/…:ro` — ein
# read-only Bind-Mount statt COPY-Stage (Definition im jeweiligen .mk).
# Reproduzierbarkeit sitzt hier auf Image-Ebene (@sha256), Hermetik via
# `--network none`; dieselbe Klasse wie schema-check (d-migrate).

DOCKER ?= docker
IMAGE ?= bcad
DOCKERFILE ?= .devcontainer/Dockerfile

# ADR-0006: d-migrate als externe Tool-Dependency per @sha256
# gepinnt (ADR-0004-Prinzip auf externe Tools angewandt; ein Floating-Tag
# erzeugte sonst nicht reproduzierbare DDL).
DMIGRATE ?= ghcr.io/pt9912/d-migrate:0.9.12@sha256:1618cdfa54bb5d03f4a5d538b81139d5baabc6275f0dd38cd6741a8f32a75666

# Gate-Kalibrierung (Threshold-as-Variable; "Kalibrierungs-Bindung"
# Modul 13). coverage-gate ist bootstrap-aware: niedrige Anfangsschwelle,
# Ramp-Trigger dokumentiert in AGENTS.md §3.
COVERAGE_THRESHOLD ?= 70

# Gate = Build einer Stage; --target wählt sie, der Kontext ist das Repo.
GATE = $(DOCKER) build -f $(DOCKERFILE)

.PHONY: help dev-image build test lint arch-check coverage-gate docs-check gate-consistency record-gates gates versions schema-check schema-regen acc-002-beleg run io-smoke golden-regen golden-check

help: ## Targets anzeigen
	@grep -E '^[a-zA-Z_-]+:.*?## ' $(MAKEFILE_LIST) | \
		awk 'BEGIN {FS = ":.*?## "}; {printf "  %-16s %s\n", $$1, $$2}'

# a-check — externes Architektur-Gate (a-check.mk, digest-gepinnt v0.13.0;
# Release-Digest hier, damit a-check.mk unverändert aus `--print-mk` bleibt).
# d-check — externes Doku-Referenz-Gate (d-check.mk, digest-gepinnt v0.51.1;
# slice-033/MR-014, löst die tools/Dockerfile-FROM-Stage ab). Der Release-
# Digest lebt bewusst hier, damit d-check.mk unverändert aus `--print-mk`
# übernommen werden kann. Beide Includes nach
# `help` eingebunden, damit `help` das Default-Goal bleibt.
A_CHECK_IMAGE ?= ghcr.io/pt9912/a-check@sha256:24939a6b47bfbfcb9845813cd781a70fba161fb12c6593e188ac51ede5267a9c
include a-check.mk
DCHECK_DIGEST ?= sha256:fede3d027b2ebc1dd8534460853e57b67cc7a9a182cad2e2138c8eebf7a2d03c
include d-check.mk

dev-image: ## Toolchain-Image (deps-Stage) — z. B. für die IDE/DevContainer
	$(GATE) --target deps -t $(IMAGE):deps .

build: ## ADR-0001 — Target-Kette kompilieren (Kern ohne Adapter-Deps)
	$(GATE) --target build -t $(IMAGE):build .

test: ## REQ-TEC-005 — GoogleTest (Kern + Adapter-Linkage)
	$(GATE) --target test -t $(IMAGE):test .

lint: ## clang-tidy + Suppression-Gate (AGENTS.md §2.4)
	$(GATE) --target lint -t $(IMAGE):lint .

arch-check: ## ADR-0017 — Plugin-P-Rest (dlopen/dlsym/dlclose-Aufruf + P2-Import-Allowlist); Schichtung A–E via a-check
	$(GATE) --target arch-check -t $(IMAGE):arch-check .

coverage-gate: ## bootstrap-aware Coverage (Schwelle $(COVERAGE_THRESHOLD)%, gcov; Composition Root ausgenommen)
	$(GATE) --target coverage-check \
		--build-arg BCAD_COVERAGE_THRESHOLD=$(COVERAGE_THRESHOLD) \
		-t $(IMAGE):coverage-check .

docs-check: doc-check ## Doku-Konsistenz — Referenz-Integritäts-Gate via d-check (d-check.mk, digest-gepinnt, .d-check.yml; netzlos read-only Bind-Mount)

gate-consistency: ## Modul 13 — jeder als real dokumentierte make-Befehl existiert (Doku↔Makefile)
	$(GATE) --target gate-consistency -t $(IMAGE):gate-consistency .

record-gates: ## Nachweis schreiben: Working-Tree-Hash (für den Stop-Hook)
	@bash tools/harness/record-gates.sh

# ADR-0004 Fitness Function: gepinnte Toolchain-Versionen. Zwei Läufe
# (gleicher Digest + Snapshot) müssen dieselbe Liste liefern. Beleg-
# Manifest: harness/toolchain-versions.txt (committet).
#   make versions > harness/toolchain-versions.txt   # neu erzeugen
#   diff <(make versions) harness/toolchain-versions.txt  # Drift prüfen
versions: ## ADR-0004 — gepinnte Toolchain-Versionen (Reproduzierbarkeits-Beleg)
	@$(GATE) $(NOCACHE) --target deps -t $(IMAGE):deps . >/dev/null
	@$(DOCKER) run --rm $(IMAGE):deps dpkg-query -W \
		-f='$${Package} $${Version}\n' \
		build-essential clang-tidy cmake gcovr ninja-build qt6-base-dev \
		libocct-foundation-dev libocct-modeling-data-dev \
		libocct-modeling-algorithms-dev libocct-data-exchange-dev \
		libtbb-dev libsqlite3-dev libgtest-dev xvfb ca-certificates | sort

# Interaktiver App-Start (ADR-0009; Docker/make-only, AGENTS §2.9):
# X11/XWayland-Display des Hosts, GPU-Durchreichung wenn /dev/dri
# existiert (sonst Mesa/llvmpipe-Software-Fallback). KEIN Gate.
# Voraussetzung einmalig: `xhost +local:` (X-Zugriff fuer lokale
# Container; Rueckbau: `xhost -local:`).
run: ## App im Container am lokalen Display starten (kein Gate; vorher ggf. xhost +local:)
	$(GATE) --target build -t $(IMAGE):build .
	$(DOCKER) run --rm -e DISPLAY=$$DISPLAY -e LANG=C.UTF-8 \
		$$( [ -e /dev/dri ] && echo "--device /dev/dri" ) \
		-v /tmp/.X11-unix:/tmp/.X11-unix \
		$(IMAGE):build ./build/src/b-cad

# ACC-002-Beleg (ADR-0009 (f)): rendert das ACC-001-Kern-Demo-Projekt
# offscreen (Mesa/llvmpipe) und schreibt das Beleg-Bild. MANUELLER
# Abnahme-Schritt des Projektinhabers — KEIN Gate, bewusst nicht in
# `make gates` aggregiert; das Begleit-.md entsteht kuratiert von Hand.
acc-002-beleg: ## ACC-002 — Beleg-Bild offscreen rendern (manueller Abnahme-Schritt, KEIN Gate)
	$(GATE) --target build -t $(IMAGE):build .
	$(DOCKER) run --rm \
		-v $(CURDIR)/docs/plan/planning/done:/out \
		$(IMAGE):build bash -c "timeout 120 xvfb-run -a ./build/src/b-cad --acc-002-beleg /out/acc-002-beleg.png"
# timeout-Wrapper: xvfb-run haengt als Container-PID-1 nach App-Ende
# (Cleanup-Race, 2x beobachtet); timeout als Prozessgruppen-Leader
# vermeidet das und deckelt den Lauf.

# Behaviour-Smoke des verdrahteten IO-Binaries: belegt die coverage-ausgenommene
# main.cpp-CLI-/Composition-Root-Glue je Austauschformat (IFC/DXF Export+Re-Import,
# STEP/STL Export; exit 0 + nicht-leere Datei, fail-closed je-Aufruf-Guard in
# tools/io-smoke.sh). KEIN gates-Member (GUI-Binary unter xvfb, schwerer +
# Cleanup-Race) -> CI-Befehlsliste, Muster acc-002-beleg. Bindung: LH-FA-IO-001..006
# (LH-Bindung, harness/conventions.md).
io-smoke: ## IO-Binary headless je Format (IFC/DXF Roundtrip, STEP/STL Export) — KEIN Gate, CI/LH-FA-IO-*
	$(GATE) --target build -t $(IMAGE):build .
	$(DOCKER) run --rm -e BCAD_SMOKE_SELFTEST $(IMAGE):build \
		bash -c "timeout 180 xvfb-run -a bash tools/io-smoke.sh"

# ADR-0006-Drift: die committete SQLite-DDL muss exakt das sein, was
# d-migrate aus spec/data-model.yaml erzeugt. BEWUSST NICHT in `make gates`
# (d-migrate bleibt aus dem hermetischen Gate-Build-Pfad) — gehört aber in
# die CI-Befehlsliste, sonst feuert das Drift-Gate nie automatisch.
schema-check: ## ADR-0006 — Drift: schema.sql == d-migrate(data-model.yaml) (NICHT in gates; in CI)
	@$(DOCKER) run --rm -v $(CURDIR)/spec:/work:ro $(DMIGRATE) \
		schema generate --source=/work/data-model.yaml --target=sqlite \
		--deterministic --report=/dev/null \
		| diff -u src/adapters/persistence/schema.sql - \
		&& echo "schema-check ok: schema.sql == d-migrate(data-model.yaml)"
# Hinweis: d-migrate-stderr (u. a. W200-DEZIMAL→REAL-Hinweise) bleibt
# sichtbar — Pull-/Image-Fehler so diagnostizierbar; der Diff vergleicht
# nur stdout (die reine DDL).

# schema.sql aus data-model.yaml NEU erzeugen (dieselbe DMIGRATE-Bild-Version
# wie schema-check, nur schreibend statt diffend). Nach einer data-model.yaml-
# Aenderung aufrufen, dann `make schema-check` zur Byte-Verifikation. BEWUSST
# NICHT in `make gates` (Muster schema-check; d-migrate bleibt aus dem
# hermetischen Gate-Build-Pfad) — haelt die Regen in der "nur ueber make"-
# Disziplin (AGENTS §2.9), statt eines rohen Einmal-docker-run (slice-032b/L2).
schema-regen: ## ADR-0006 — schema.sql aus data-model.yaml neu erzeugen (danach schema-check)
	@$(DOCKER) run --rm -v $(CURDIR)/spec:/work:ro $(DMIGRATE) \
		schema generate --source=/work/data-model.yaml --target=sqlite \
		--deterministic --report=/dev/null \
		> src/adapters/persistence/schema.sql
	@echo "schema-regen ok: schema.sql neu erzeugt — jetzt 'make schema-check' zur Verifikation"

# slice-044a — Export-Golden aller sechs Austauschformate (Muster ADR-0006
# schema-regen/schema-check). Beide BEWUSST NICHT in `make gates`: der Generator
# ist ein GUI-freies, aber OCC-/Adapter-schweres Binary im build-Image (die
# Byte-Golden-Tests selbst laufen als GoogleTests IN `make test`/`gates` — sie
# lesen die committeten Golden, brauchen die Regen nicht). golden-regen schreibt
# über einen writable Mount (Muster acc-002-beleg), NICHT den schema-regen-stdout-
# Redirect (untauglich für 6 Dateien inkl. binärer STL/PNG/PDF). Der dedizierte
# `golden_gen` baut dasselbe goldenModel() wie der Test (kein buildAcc001-Drift,
# MR-006-044-MED-2). Läuft display-frei (PDF/PNG self-rolled, STEP/STL OCC —
# kein xvfb nötig). Beide gehören in die CI-Befehlsliste (Muster schema-check).
golden-regen: ## slice-044a — Export-Golden (6 Formate) neu erzeugen (danach golden-check); KEIN Gate
	$(GATE) --target build -t $(IMAGE):build .
	$(DOCKER) run --rm \
		-v $(CURDIR)/tests/adapters/golden:/out \
		$(IMAGE):build ./build/tests/golden_gen /out
	@echo "golden-regen ok: tests/adapters/golden/model.* neu erzeugt — jetzt 'make golden-check'"

# golden-check: Regen-Drift — die committeten Golden (ins Image gebacken, COPY . .)
# müssen byte-genau dem entsprechen, was `golden_gen` frisch erzeugt. Fängt ein
# vergessenes golden-regen nach einer Encoder-Änderung. Zwei Läufe im selben Image
# müssen zudem run-zu-run identisch sein (Determinismus, u. a. der STEP-HEADER-Fix).
golden-check: ## slice-044a — Drift: committete Golden == golden_gen (NICHT in gates; in CI)
	$(GATE) --target build -t $(IMAGE):build .
	$(DOCKER) run --rm $(IMAGE):build bash -c '\
		mkdir -p /tmp/golden && ./build/tests/golden_gen /tmp/golden \
		&& for f in model.ifc model.dxf model.step model.stl model.pdf model.png; do \
			diff -q tests/adapters/golden/$$f /tmp/golden/$$f || exit 1; \
		done \
		&& echo "golden-check ok: committete Golden == golden_gen (byte-genau)"'

# `build` ist NICHT separat gelistet: test/lint/coverage-gate sind
# Dockerfile-Stages FROM build und kompilieren die Target-Kette bereits
# (Variante B, siehe harness/README §Sensors). record-gates läuft im
# REZEPT statt als letzter Prerequisite — unter `make -j` liefen
# Prerequisites parallel und der Nachweis entstünde trotz roter Gates
# (MR-005); das Rezept läuft erst, wenn ALLE Prerequisites grün sind.
gates: docs-check gate-consistency a-check arch-check lint test coverage-gate ## alle inneren Gates (mandatory vor PR)
	@bash tools/harness/record-gates.sh
