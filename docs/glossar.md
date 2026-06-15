# Glossar — b-cad

Repo-spezifische Begriffe. Kurs-Begriffe (LLM, Agent, Harness, Guide,
Sensor, Slice, Welle, Carveout, …) stehen in der Kurs-Konvention und
werden hier nicht wiederholt.

| Begriff | Bedeutung in b-cad |
|---|---|
| Gebäudemodell | durchgängiges, parametrisches Datenmodell; Quelle für 2D- und 3D-Sicht ([OBJ-003](../spec/lastenheft.md#3-projektziele)). |
| Hexagon / Kern | framework-freier Anwendungskern (`src/hexagon/`): Domain-Modell, Ports, Services. Kennt kein Qt/OCC/SQLite. |
| Driving Port | primäre Use-Case-Schnittstelle, die der Kern anbietet (GUI/Plugins rufen sie). |
| Driven Port | sekundäre Infrastruktur-Schnittstelle, die der Kern braucht (Geometrie, Persistenz, IO). |
| Adapter | konkrete Implementierung eines Ports (`src/adapters/…`): Qt, OpenCascade, SQLite, Format-Bibliotheken. |
| Geometrie-Kern | OpenCascade, hinter `GeometryKernelPort` gekapselt. |
| Composition Root | `src/main.cpp` — einziger Ort, der Adapter mit dem Kern verdrahtet. |
| Wandzug | zusammenhängende Folge von Wandsegmenten. |
| Raum-Autoerkennung | Ableitung geschlossener Raumpolygone aus Wandzügen ([LH-FA-ROM-001](../spec/lastenheft.md#lh-fa-rom-001--raum-automatisch-erkennen)). |
| Standardprojekt | Referenzprojekt gemäß [ACC-001](../spec/lastenheft.md#7-abnahmekriterien); Messbasis für [LH-QA-001](../spec/lastenheft.md#lh-qa-001--performance-projektöffnung)/002. |
| Atomares Schreiben | Persistenz schreibt Temp-Datei + Rename; kein halb geschriebenes Projekt ([LH-QA-005](../spec/lastenheft.md#lh-qa-005--crash-recovery)). |
