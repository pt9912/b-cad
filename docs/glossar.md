# Glossar — b-cad

Repo-spezifische Begriffe. Kurs-Begriffe (LLM, Agent, Harness, Guide,
Sensor, Slice, Welle, Carveout, …) stehen in der Kurs-Konvention und
werden hier nicht wiederholt.

| Begriff | Bedeutung in b-cad |
|---|---|
| Gebäudemodell | durchgängiges, parametrisches Datenmodell; Quelle für 2D- und 3D-Sicht (OBJ-003). |
| Hexagon / Kern | framework-freier Anwendungskern (`src/hexagon/`): Domain-Modell, Ports, Services. Kennt kein Qt/OCC/SQLite. |
| Driving Port | primäre Use-Case-Schnittstelle, die der Kern anbietet (GUI/Plugins rufen sie). |
| Driven Port | sekundäre Infrastruktur-Schnittstelle, die der Kern braucht (Geometrie, Persistenz, IO). |
| Adapter | konkrete Implementierung eines Ports (`src/adapters/…`): Qt, OpenCascade, SQLite, Format-Bibliotheken. |
| Geometrie-Kern | OpenCascade, hinter `GeometryKernelPort` gekapselt. |
| Composition Root | `src/main.cpp` — einziger Ort, der Adapter mit dem Kern verdrahtet. |
| Wandzug | zusammenhängende Folge von Wandsegmenten. |
| Raum-Autoerkennung | Ableitung geschlossener Raumpolygone aus Wandzügen (LH-FA-ROM-001). |
| Standardprojekt | Referenzprojekt gemäß ACC-001; Messbasis für LH-QA-001/002. |
| Atomares Schreiben | Persistenz schreibt Temp-Datei + Rename; kein halb geschriebenes Projekt (LH-QA-005). |
