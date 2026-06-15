# tools/

Harness-Hilfswerkzeuge für b-cad. Nicht Teil des Produkts (`src/`),
sondern Sensorik für die Doku-Schicht.

## docs-check

`docs-check.js` ist ein **Markdown-Link-Validator** (Node 22, keine
Dependencies). Er prüft repo-weit:

1. Interne Markdown-Links `[text](pfad.md#anker)` — Datei vorhanden? Anker vorhanden?
2. Bild-Referenzen `![alt](pfad)` — Datei vorhanden?
3. Code-/Config-Referenzen `[text](pfad.cpp|.cmake|.json|…)` — Datei vorhanden?
4. Explizite Inline-Code-Pfade (relative `./`/`../`-Pfade in Backticks) — Datei/Verzeichnis vorhanden?
5. Sicherheitsnetz: relative Pfade dürfen nicht aus dem Repo führen.

Das war b-cads **erstes reales Gate** (Greenfield-Bootstrap) und prüft
die Doku-Lieferung, die bereits existiert. Seit slice-002 stehen daneben
die Code-Gates (Liste: `harness/README.md` §Sensors). Eingebunden als
`make docs-check` / `make gates`.

### Verwendung (Docker, gemäß Hard Rule AGENTS.md §2.3)

`docs-check` ist eine **Dockerfile-Target-Stage** (Quelle per `COPY` ins
Image gebacken, Validierung als `RUN`) — **kein** Bind-Mount. Kontext
ist das Repo-Root:

```bash
make docs-check          # baut die docs-check-Stage; schlägt bei Fehlern fehl
# oder direkt:
docker build -f tools/Dockerfile --target docs-check -t bcad-docs-check .
```

Lokal ohne Docker (nur Entwicklung, kein Vertrag — vgl. Modul 14):

```bash
node tools/docs-check.js
```

### Herkunft und Drift

Die Doku-Validierung läuft seit [MR-007](../harness/conventions.md#mr-007--auflösung-von-mr-003-docs-check-via-d-check) über
[d-check](https://github.com/pt9912/d-check) — digest-gepinntes
Container-Image als Build-Stage (kein Bind-Mount), Konfiguration in
[`../.d-check.yml`](../.d-check.yml). Das zuvor hier vendorte
`docs-check.js` ([MR-003](../harness/conventions.md#mr-003--docs-check-als-vendored-doku-sensor), Kurs-Kopie) ist gelöscht; Historie: <!-- d-check:ignore (historisch: geloescht mit [MR-007](../harness/conventions.md#mr-007--auflösung-von-mr-003-docs-check-via-d-check)) -->
[`../harness/conventions.md` MR-007](../harness/conventions.md#mr-007--auflösung-von-mr-003-docs-check-via-d-check).
