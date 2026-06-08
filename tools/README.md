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

Das ist b-cads **erstes reales Gate** (Greenfield-Bootstrap): es prüft
die Doku-Lieferung, die bereits existiert — anders als Code-Gates, die
erst mit `src/` entstehen. Eingebunden als `make docs-check` /
`make gates`.

### Verwendung (Docker, gemäß Hard Rule AGENTS.md §2.3)

```bash
make docs-check          # Image bauen + alle *.md prüfen
# oder direkt:
docker build -t bcad-docs-check tools/
docker run --rm -v "$PWD":/work bcad-docs-check
```

Lokal ohne Docker (nur Entwicklung, kein Vertrag — vgl. Modul 14):

```bash
node tools/docs-check.js
```

### Herkunft und Drift

`docs-check.js` ist **vendored** aus dem AI-Harness-Kurs
(`ai-harness-course`, `tools/docs-check.js`), unverändert. Begründung
und Adaptions-Eintrag: [`../harness/conventions.md` MR-003](../harness/conventions.md#mr-003-docs-check-als-vendored-doku-sensor).
Bei Updates der Quelle hier nachziehen (Entropy Management, Modul 15).
