# tools/

Harness-Hilfswerkzeuge für b-cad. Nicht Teil des Produkts (`src/`),
sondern Sensorik für die Doku-Schicht.

## docs-check

`docs-check` ist b-cads **Doku-Referenz-Integritäts-Gate**: es prüft
repo-weit interne Markdown-Links/Anker, Inline-Code-Pfade, die
**Referenz-Richtung** (Spec ↛ ADR, ADR ↛ Slice — no-downward / Stable
Dependencies) und die ID-Linkpflicht. Das war b-cads **erstes reales Gate**
(Greenfield-Bootstrap) und prüft die bereits existierende Doku-Lieferung; seit
slice-002 stehen daneben die Code-Gates (Liste: `harness/README.md` §Sensors).
Eingebunden als `make docs-check` / `make gates`.

### Verwendung (Docker, gemäß Hard Rule AGENTS.md §2.3)

Die Validierung läuft über [d-check](https://github.com/pt9912/d-check) — ein
**digest-gepinntes Container-Image** (v0.37.1), eingebunden über
[`../d-check.mk`](../d-check.mk) (`include d-check.mk`, erzeugt von
`d-check --print-mk`; Pin über `DCHECK_DIGEST`), Konfiguration in
[`../.d-check.yml`](../.d-check.yml). Aufruf **netzlos, read-only Bind-Mount**:

```bash
make docs-check          # docker run --network none -v REPO:/repo:ro …
```

`d-check.mk` stellt daneben `doc-doctor` (Diagnose), `doc-repair`
(Fix-Patch) und `doc-trace` (RTM) bereit — kein Gate.

### Herkunft und Drift

- **[MR-007](../harness/conventions.md#mr-007--auflösung-von-mr-003-docs-check-via-d-check):**
  Ablösung des zuvor hier vendorten Node-Validators
  ([MR-003](../harness/conventions.md#mr-003--docs-check-als-vendored-doku-sensor)-Kurs-Kopie,
  gelöscht) durch das digest-gepinnte d-check-Image.
- **slice-033 / [MR-014](../harness/conventions.md):** Umstellung von der
  `tools/Dockerfile`-FROM-Stage (COPY, kein Bind-Mount) auf
  [`../d-check.mk`](../d-check.mk) (Bind-Mount, `--network none`,
  `DCHECK_DIGEST` v0.37.1) — Muster a-check.mk/slice-030; zugleich
  Matrix-Verschärfung ADR ↛ Slice. `tools/Dockerfile` ist damit entfallen
  (codepaths-Tombstone für die eingefrorene slice-004-Referenz).
