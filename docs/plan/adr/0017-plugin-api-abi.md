# ADR-0017: Plugin-API-/ABI-Vertrag und Sandbox-Modell (Plugin-Host als Driving Adapter; dlopen + versionierter C-Handshake + C++-Port-Facade, in-process)

**Status:** Accepted

**Datum:** 2026-07-02

**Autor:** Dietmar Burkard (Vorbereitung welle-5-erweiterung — die Plugin-API-/ABI-ADR ist der benannte Wellen-Trigger; ausgearbeitet im AI-Harness-Lauf)

**Bezug:** [LH-FA-PLG-001](../../../spec/lastenheft.md#modul-plugin-system-plg) (Dynamische Plugins — Laden/Entladen zur Laufzeit), [LH-FA-PLG-002](../../../spec/lastenheft.md#modul-plugin-system-plg) (Plugin-API, stabiler Vertrag — »vgl. ADR-Folge«: **diese** ADR), [LH-FA-PLG-003](../../../spec/lastenheft.md#modul-plugin-system-plg) (Plugin-Lifecycle), [LH-FA-PLG-004](../../../spec/lastenheft.md#modul-plugin-system-plg) (Plugin-Sandbox — Plugin darf das Modell nicht korruptieren), [OBJ-004](../../../spec/lastenheft.md#3-projektziele) (Erweiterbarkeit durch Plugins — Meilenstein-M5-Rahmen), [REQ-TEC-008](../../../spec/spezifikation.md#9-technische-rahmenbedingungen-req-tec) (Plugin-Architektur = Shared Libraries), [`E-PLG-001`](../../../spec/spezifikation.md#4-fehler-codes-und-logging-felder) (Plugin-Fehlverhalten → isoliert/entladen, Modell unverändert), ADR-0001 (Schichtung — der Plugin-Host ist der in der Architektur verortete **zweite Driving Adapter**), [ADR-0003](0003-persistenz-sqlite.md) (atomare Persistenz — das Daten-Netz unter der Sandbox-Grenze), [ADR-0004](0004-toolchain-dependency-pinning.md) (Toolchain-/Dependency-Pinning — `dlopen` ist glibc, **keine** neue Dependency; zugleich Grundlage der ABI-Abwägung: **ein** gepinnter Compiler), ADR-0005 (Drittanbieter-Lizenz-Attribution — keine Neulast), [ADR-0008](0008-aenderungs-benachrichtigung.md) (Kapselungs-Muster »werfender Beobachter« — Vorbild der Fehler-Barriere), [ADR-0009](0009-gui-framework-qt6.md) (Qt = Driving-/UI-Schicht, **Regel E** — warum **kein** Qt-Plugin-Loader, s. §Alternativen/Option Q)

---

## Kontext

Die vier Meilensteine M1–M4 sind erreicht; als nächster steht **M5
„Erweiterbar"** ([OBJ-004](../../../spec/lastenheft.md#3-projektziele)) an. Die zugehörige Welle (welle-5-erweiterung)
hat als benannten Start-Trigger **diese ADR**: den Plugin-API-/ABI-Vertrag
und das Sandbox-Modell für das Modul `PLG`. Die vier Anforderungen
[LH-FA-PLG-001](../../../spec/lastenheft.md#modul-plugin-system-plg)–004 stehen im Lastenheft auf **Outline-Niveau**;
[REQ-TEC-008](../../../spec/spezifikation.md#9-technische-rahmenbedingungen-req-tec) setzt als technische Rahmenbedingung **Shared Libraries**;
die Architektur verortet den **Plugin-Host** als **Driving Adapter** in
`src/adapters/plugin/` (heute leerer Platzhalter), der **Driving Ports
vermittelt** und **keine** Driven Adapter direkt anspricht, mit einem
`plugins/`-Baum für extern ladbare Plugins und dem Fehlermodell
[`E-PLG-001`](../../../spec/spezifikation.md#4-fehler-codes-und-logging-felder) (Plugin isoliert/entladen, Modell unverändert,
`event=plugin_error`).

**Der Kern ist vorbereitet — die Port-Naht existiert bereits.** Anders
als bei den Format-Backends der welle-4 braucht das Plugin-System **keine
neue Kern-Port-Naht**: Plugins sind ein **zweiter Driving-Weg** in
dieselben Driving-Ports (`src/hexagon/ports/driving/`), die die GUI heute
schon nutzt (`EditStructurePort`, `EvaluatePort`, `ExchangeModelPort`, …).
Jede Plugin-Mutation läuft damit durch **dieselbe** Service-Validierung
(Klemmung [`E-VAL-001`](../../../spec/spezifikation.md#4-fehler-codes-und-logging-felder)) und dieselbe atomare Persistenz wie ein
GUI-Aufruf; ein **künftiger** Undo-Mechanismus ([LH-QA-003](../../../spec/lastenheft.md#lh-qa-003--undoredo) — heute
**nicht implementiert**, es existiert nur das `undo_commands`-Schema aus
[ADR-0006](0006-relationales-schema-design.md)) erfasst Plugin-Mutationen automatisch mit, **weil** sie durch
dieselben Ports laufen. Die offenen Fragen sind **Host-seitig**, nicht
Kern-seitig; die einzige **neue** Naht liegt auf der Link-/Symbol-Ebene
(s. Entscheidung #2).

Vier Lösungsfragen, die der Spec-Text nicht entscheidet:

1. **Lade-Mechanik:** Wie werden Shared Libraries ([REQ-TEC-008](../../../spec/spezifikation.md#9-technische-rahmenbedingungen-req-tec)) zur
   Laufzeit geladen/entladen — POSIX `dlopen` direkt, Qt-Plugin-Framework
   (`QPluginLoader`), oder eine Plugin-Bibliothek?
2. **ABI-Grenze:** Was ist der **stabile Vertrag** ([LH-FA-PLG-002](../../../spec/lastenheft.md#modul-plugin-system-plg)) an
   der Binärgrenze — eine reine C-ABI (Funktionstabellen, opake Handles),
   ein C++-Interface (vtable), oder ein Hybrid?
3. **Sandbox-Semantik:** Was heißt „Plugin darf das Modell nicht
   korruptieren" ([LH-FA-PLG-004](../../../spec/lastenheft.md#modul-plugin-system-plg)) für eine **in-process** geladene
   Shared Library — und wo liegt die ehrliche Grenze gegenüber echter
   Prozess-Isolation?
4. **Durchsetzung:** Welche computational Gates (arch-check, Tests)
   machen die Entscheidung zum Constraint statt zur Absichtserklärung?

**Nicht offen** (bewusst außerhalb dieser ADR — Scope-Verengung, Präzedenz
[ADR-0013](0013-ifc-bibliothek.md)/[0014](0014-step-stl-export-backend.md)/[0015](0015-dxf-backend.md)/[0016](0016-pdf-png-backend.md)):

- **Lastenheft-AK-Schärfung** ([LH-FA-PLG-001](../../../spec/lastenheft.md#modul-plugin-system-plg)–004) bleibt **lösungsfrei**
  ([MR-008](../../../harness/conventions.md#mr-008--lastenheft-schärfung-bleibt-lösungsfrei)) und ist ein **eigener Schärfungs-Slice** nach Accept (Präzedenz
  slice-019a/020a/021a/025a) — diese ADR ist die Lösungsschicht, nicht der
  Anforderungstext.
- **Exakte API-Gestalt** (Header-Dateinamen, Signaturen, das v1-**Subset**
  der vermittelten Driving-Ports, Metadaten-Felder eines Plugins,
  Discovery-/Suchpfad-Konvention im `plugins/`-Baum, CMake-Gestalt der
  Plugin-Targets) legen Schärfungs-/Impl-Slice fest.
- **Vertrauens-/Signier-Modell** (signierte Plugins, Marketplace,
  Berechtigungs-Stufen) ist **out of scope** — benannter Re-Eval-Trigger.
- **UI-Erweiterungspunkte aus Plugins** (eigene Docking-Panels/Werkzeuge
  in der Qt-GUI) sind ein **eigener** späterer Beschluss an der
  Regel-E-Grenze — diese ADR regelt den **modell-seitigen** Vertrag
  (Plugin ↔ Kern über Driving-Ports), nicht die GUI-Einbettung.

## Entscheidung

1. **Lade-Mechanik — POSIX `dlopen`/`dlsym`/`dlclose` direkt im
   Plugin-Host (`src/adapters/plugin/`, Driving Adapter). Keine neue
   Abhängigkeit ([ADR-0004](0004-toolchain-dependency-pinning.md)-konform), kein Qt.** Der dynamische Lader ist
   Teil der gepinnten glibc/Runtime des Containers — es entsteht **kein**
   neuer apt-/`find_package`-Eintrag und **keine** Lizenz-Neulast
   (ADR-0005). [REQ-TEC-008](../../../spec/spezifikation.md#9-technische-rahmenbedingungen-req-tec) (Shared Libraries) wird damit wörtlich
   eingelöst. `QPluginLoader` wäre Qt außerhalb von `src/adapters/ui/` +
   `src/main.cpp` — eine **Regel-E-Verletzung** (Option Q, verworfen).

2. **ABI-Grenze — versionierter `extern "C"`-Handshake, dahinter eine
   C++-Port-Facade (Hybrid).** Jedes Plugin exportiert einen schmalen
   `extern "C"`-Einstiegssatz: eine **ABI-Versions-Abfrage** und ein
   **Factory-/Destroy-Paar**. Der Host prüft die ABI-Version **vor** dem
   ersten C++-Aufruf auf **exakte Gleichheit** — Mismatch, fehlendes
   Symbol oder nicht ladbare Datei ⇒ Ablehnung **ohne** Initialisierung
   ([`E-PLG-001`](../../../spec/spezifikation.md#4-fehler-codes-und-logging-felder), fail-closed). Erst nach bestandenem Handshake tauscht
   der Host C++-Schnittstellen aus: das Plugin implementiert ein
   pure-virtual **Plugin-Interface** (Metadaten + Lifecycle-Hooks) und
   erhält einen **Plugin-Kontext**, der Driving-Port-Referenzen vermittelt
   (Entscheidung #3).
   - **Stabilität heißt hier: versionierter Vertrag mit fail-closed
     Handshake — nicht Binärkompatibilität über Compiler-Grenzen.**
     Die Toolchain ist per [ADR-0004](0004-toolchain-dependency-pinning.md) **gepinnt** (ein **Build**-Compiler,
     eine Standardbibliothek im Container; clang-tidy ist lint-only und
     auf dieselbe Toolchain genagelt); Plugins werden gegen denselben
     gepinnten Stand gebaut. Die C++-vtable-Grenze ist unter dieser
     Prämisse beherrscht; die ABI-Version wird bei **jeder**
     inkompatiblen Änderung des Plugin-Vertrags **und** bei einem
     Toolchain-Hub ([ADR-0004](0004-toolchain-dependency-pinning.md)-Beschluss) erhöht. **Benannte Grenze:**
     binär verteilte Dritt-Plugins aus **fremden** Toolchains sind
     **nicht** versprochen — wird das verbindlich, ist die reine C-ABI
     (Option C) der Weg, als Supersedes-ADR (§Trigger).
   - **Symbol-Auflösung — die eine neue Link-Naht (benannte
     Entscheidungsfrage).** Plugins inkludieren Kern-Header
     (Entscheidung #3) und brauchen zur Ladezeit deren Symbole
     (out-of-line-Methoden, vtables, **typeinfo** — Letzteres trägt auch
     die Fehler-Barriere/`dynamic_cast` über die Modul-Grenze).
     **Verboten:** `bcad_hexagon` **statisch** ins Plugin dazulinken —
     zwei Kern-Kopien mit doppelter typeinfo im Prozess brechen
     Exception-Catch über die Grenze und die Eine-Wahrheit-Annahme. Die
     Wahl zwischen (a) **Symbol-Export des Executables** (CMake
     `ENABLE_EXPORTS`/`-rdynamic`) und (b) **`bcad_hexagon` als Shared
     Library** (Build-Topologie-Änderung) trifft der Impl-Slice als
     **benannte Entscheidung mit Beleg** — der ABI-Mismatch-/
     Fehlendes-Symbol-Test lädt eine **reale** `.so` durch den echten
     Host. Auf **Port-Ebene** bleibt der Kern unberührt; **diese**
     Link-Naht ist die einzige neue.

3. **API-Oberfläche — Plugins sehen genau die Importmenge des
   Plugin-Hosts: Domänen-Modell + Driving-Ports. Der Kontext vermittelt
   ausschließlich Driving-Port-Referenzen.** Die Architektur-Zeile des
   Plugin-Hosts (»darf importieren: model, ports/driving«) gilt
   **transitiv** für Plugins: sie inkludieren den Plugin-API-Header-Satz
   plus Kern-Header aus `src/hexagon/model/` und
   `src/hexagon/ports/driving/` — **nie** `src/adapters/`-Header
   (auch nicht die des Hosts selbst), **nie** Driven-Ports, **kein**
   Qt/OCC/SQLite. Damit erreicht ein Plugin das Modell **nur** über
   dieselben Use-Case-Schnittstellen wie die GUI; welches
   **Port-Subset** der Kontext in v1 tatsächlich ausreicht, entscheidet
   der Schärfungs-/Impl-Slice (z. B. lesend + editierend zuerst).
   - **Kein Beobachter-Zugang in v1:** mit dem Driven-Port-Ausschluss
     können Plugins die Beobachter-Schnittstelle (`ModelChangedPort`,
     ein Driven-Port) **nicht** implementieren — der in
     [ADR-0008](0008-aenderungs-benachrichtigung.md) benannte Plugin-Prüfpunkt (fremder Code als Beobachter →
     Kapselung/Re-Entranz-Verbot **technisch** durchsetzen) wird für v1
     **durch Ausschluss beantwortet**: Plugins sind **pull-only** und
     ausschließlich in ihren Lifecycle-Hooks aktiv (der Hook ist der
     einzige Eintrittspunkt). Beobachter-Zugang für Plugins ist ein
     **benannter Re-Eval-Trigger** (§Trigger), keine stille Aufweichung
     dieser Import-Grenze.

4. **Lifecycle ([LH-FA-PLG-003](../../../spec/lastenheft.md#modul-plugin-system-plg)) — deterministische Zustandsfolge,
   jeder Übergang fail-closed.**
   `Entdeckt → Geladen (dlopen) → Handshake (ABI exakt) → Initialisiert
   (Kontext übergeben) → Aktiv → Beendet (Shutdown-Hook) → Entladen
   (dlclose)`. **Entladen zur Laufzeit** ([LH-FA-PLG-001](../../../spec/lastenheft.md#modul-plugin-system-plg)) ist Teil des
   Vertrags: vor `dlclose` ruft der Host den Shutdown-Hook und
   **invalidiert den Kontext** (das Plugin darf danach keine
   Port-Referenz mehr halten — Vertragspflicht des Plugins, vom
   Test-Plugin der AK-Tests belegt). **Jeder** Fehlerpfad (Load-,
   Handshake-, Init-, Laufzeit- oder Shutdown-Fehler) endet identisch:
   Plugin isolieren/entladen, [`E-PLG-001`](../../../spec/spezifikation.md#4-fehler-codes-und-logging-felder) (`event=plugin_error`),
   Modell unverändert, Host läuft weiter.

5. **Sandbox-Modell ([LH-FA-PLG-004](../../../spec/lastenheft.md#modul-plugin-system-plg)) — Port-Vermittlung +
   Fehler-Barriere, in-process; die Speicherschutz-Grenze wird ehrlich
   benannt.**
   - **Architektur-Vermittlung:** Plugins mutieren das Modell **nur**
     über Driving-Ports (Entscheidung #3) — jede Mutation durchläuft
     dieselbe Validierung/Klemmung ([`E-VAL-001`](../../../spec/spezifikation.md#4-fehler-codes-und-logging-felder)) und dieselbe atomare
     Persistenz wie ein GUI-Aufruf (ein künftiger Undo-Mechanismus,
     [LH-QA-003](../../../spec/lastenheft.md#lh-qa-003--undoredo), erfasst sie über dieselben Ports automatisch mit). Es
     gibt **keinen** Nebeneingang (kein Durchgriff auf
     Persistenz-/Geometrie-Adapter oder Modell-Interna).
   - **Fehler-Barriere:** jeder Host→Plugin-Aufruf (Lifecycle-Hooks) ist
     exception-gesichert — ein werfendes Plugin wird isoliert/entladen
     ([`E-PLG-001`](../../../spec/spezifikation.md#4-fehler-codes-und-logging-felder)), das Modell bleibt unverändert, kein
     Plugin-Fehler propagiert als Absturz in den Host (Muster
     [ADR-0008](0008-aenderungs-benachrichtigung.md) »werfender Beobachter«).
   - **Threading-Vertrag:** der Host ruft alle Plugin-Hooks **synchron
     im Hauptthread** — [ADR-0009](0009-gui-framework-qt6.md) (d) beschließt den
     Single-threaded-Betrieb am Qt-Event-Loop; dass der Kern nicht
     thread-sicher ausgelegt ist, ist die (korrekte) **Folgerung**
     daraus, keine (d)-Zitatstelle. Port-Aufrufe sind **nur aus dem
     Hook-Kontext** zulässig.
     Plugin-eigene Threads dürfen **keine** Ports rufen —
     Vertragspflicht des Plugins, **benannte Grenze:** in-process
     technisch nicht erzwingbar (ein Data-Race wäre Modell-Korruption an
     der Fehler-Barriere vorbei).
   - **Benannte Grenze (ehrlich):** eine **in-process** geladene Shared
     Library teilt den Adressraum — gegen wilde Zeiger/UB gibt es
     **keinen** Speicherschutz; ein bösartiges/defektes Plugin kann den
     Prozess crashen. Das **Daten-Netz darunter** ist die atomare
     Persistenz + Crash-Recovery ([ADR-0003](0003-persistenz-sqlite.md), [LH-QA-005](../../../spec/lastenheft.md#lh-qa-005--crash-recovery)): der letzte
     konsistente Stand auf Platte überlebt jeden Prozess-**Crash**.
     **Zweiter UB-Pfad, ebenso benannt:** UB kann das In-Memory-Modell
     auch **ohne** Crash **still** korrumpieren — das nächste
     Speichern/Autosave ([LH-QA-004](../../../spec/lastenheft.md#lh-qa-004--autosave)) schriebe den korrupten Stand atomar
     **über** den letzten guten; das Persistenz-Netz deckt
     konstruktionsbedingt nur den Crash-Pfad. Gegen stille Korruption
     plus anschließendes Speichern gibt es in-process **keinen** Schutz —
     die stärkste Lesart von [LH-FA-PLG-004](../../../spec/lastenheft.md#modul-plugin-system-plg) ist in-process
     **prinzipiell nicht einlösbar**. **Echte Speicher-Sandbox =
     Prozess-Isolation** (Option P) — bewusst verworfen für v1, benannter
     Re-Eval-Trigger (§Trigger). Die benutzer-beobachtbare Sandbox-AK
     bezieht der Schärfungs-Slice daher auf beobachtbares Fehlverhalten
     **wohlgeformter** Plugins (Port-Vermittlung, Fehler-Barriere,
     Isolierung/Entladung), **nicht** auf Speichersicherheit — lösungsfrei
     ([MR-008](../../../harness/conventions.md#mr-008--lastenheft-schärfung-bleibt-lösungsfrei)).

6. **Durchsetzung — neue arch-check-Regel P (Impl-Slice-Folgepflicht).**
   Zwei mechanisch prüfbare Hälften, analog Regel C/D/E:
   - **P1 (Host-Monopol):** `dlfcn.h`-Include und `dlopen`/`dlsym`/
     `dlclose`-Symbole nur in `src/adapters/plugin/` — kein anderer
     Adapter, nicht der Kern, nicht `src/main.cpp` **und auch kein
     Plugin im `plugins/`-Baum** lädt dynamisch (das Monopol schließt
     `plugins/` ein: ein Plugin, das selbst `dlopen` ruft, unterliefe
     Lifecycle und Fehler-Barriere).
   - **P2 (Plugin-Import-Grenze):** Dateien unter `plugins/` inkludieren
     nur den Plugin-API-Header-Satz + `src/hexagon/model/` +
     `src/hexagon/ports/driving/` — keine `src/adapters/`-, Qt-, OCC-,
     SQLite- und keine `dlfcn.h`-Header (P1-Spiegel in der
     P2-Verbotsliste).
   - **Buchstabe P statt F:** „Regel F" ist historisch durch die für
     Option-D-Codecs **gegenstandslose** IO-Header-Regel belegt
     ([ADR-0013](0013-ifc-bibliothek.md)-Folgepflicht, umdatiert auf den externen-Lib-Re-Eval) —
     Wiederverwendung des Buchstabens wäre ein Verwechslungsrisiko.

## Verglichene Alternativen

### Option H — `dlopen` + versionierter C-Handshake + C++-Port-Facade, in-process (gewählt)

- **Pro:** **Null** neue Abhängigkeit ([ADR-0004](0004-toolchain-dependency-pinning.md)-konform, `dlopen` =
  glibc); löst [REQ-TEC-008](../../../spec/spezifikation.md#9-technische-rahmenbedingungen-req-tec) wörtlich ein; **schichttreu** (Plugin-Host =
  Driving Adapter wie in der Architektur verortet, Kern-**Ports**
  unberührt — die Driving-Ports existieren; einzige neue Naht = Symbol-/
  Link-Ebene, benannt in Entscheidung #2); Plugins erben
  Validierung/Atomarität **gratis** über die Port-Vermittlung; der
  fail-closed ABI-Handshake macht Versions-Drift beobachtbar statt
  undefiniert; unter der **gepinnten** Toolchain ([ADR-0004](0004-toolchain-dependency-pinning.md)) ist die
  C++-vtable-Grenze beherrscht, ohne den v1-Aufwand einer vollständigen
  C-Spiegelung aller Ports. (Ein **künftiger** Undo-Mechanismus,
  [LH-QA-003](../../../spec/lastenheft.md#lh-qa-003--undoredo), erfasst Plugin-Mutationen über dieselben Ports
  automatisch mit.)
- **Contra:** keine Binärkompatibilität über Compiler-Grenzen (benannte
  Grenze, Re-Eval → Option C); in-process = kein Speicherschutz (benannte
  Grenze, Re-Eval → Option P); `dlclose`-Entladen verlangt
  Vertragsdisziplin des Plugins (keine überlebenden Referenzen) — vom
  Lifecycle-Test sondiert.

### Option C — reine C-ABI (Funktionstabellen, opake Handles)

- **Pro:** maximale ABI-Stabilität — Plugins aus fremden
  Compilern/Sprachen (C, Rust, …) möglich; der „stabile Vertrag" aus
  [LH-FA-PLG-002](../../../spec/lastenheft.md#modul-plugin-system-plg) in seiner stärksten Form.
- **Contra:** verlangt die **vollständige C-Spiegelung** der vermittelten
  Driving-Ports (Funktionstabellen, Handle-Verwaltung, String-/Fehler-/
  Ownership-Marshalling) — für v1 **unverhältnismäßig**, solange kein AK
  binär verteilte Fremd-Toolchain-Plugins fordert; die Spiegelschicht
  ist selbst drift-anfällig (zwei Vertragsoberflächen statt einer).
  **Verworfen für v1** — als **benannter Aufstiegspfad** geführt: wird
  Fremd-Toolchain-Distribution verbindlich, ersetzt eine Supersedes-ADR
  die Facade durch die C-ABI (§Trigger); der versionierte
  `extern "C"`-Handshake aus Option H bleibt dabei **unverändert** die
  Eintrittstür (vorwärtskompatible Grundlage).

### Option Q — Qt-Plugin-Framework (`QPluginLoader`)

- **Pro:** Qt ist bereits Dependency ([ADR-0009](0009-gui-framework-qt6.md)); `QPluginLoader` bringt
  Metadaten-Handling und Versions-Checks mit.
- **Contra (entscheidend):** Qt ist die **Driving-/UI-Schicht** —
  `arch-check` **Regel E** begrenzt Qt-Header auf `src/adapters/ui/` +
  `src/main.cpp`. Der Plugin-Host ist ein **eigener** Driving Adapter
  (`src/adapters/plugin/`) — Qt dort bräuchte eine Regel-E-Lockerung
  (Gate-Lockerung nur per ADR, [AGENTS.md §2.6](../../../AGENTS.md)); zudem zöge der
  Qt-Meta-Object-Vertrag Qt in die **Plugin-API** selbst — jedes Plugin
  würde Qt-abhängig, auch reine Modell-Plugins. Gegenfolie wie
  [ADR-0016](0016-pdf-png-backend.md)/Option Q: native Nutzung einer vorhandenen Dependency ist nur
  dann richtig, wenn sie **schichttreu** ist. **Verworfen.**

### Option P — Out-of-Process-Plugins (eigener Prozess + IPC)

- **Pro:** **echte** Sandbox (eigener Adressraum, OS-Isolation) — die
  stärkste Lesart von [LH-FA-PLG-004](../../../spec/lastenheft.md#modul-plugin-system-plg); Crash eines Plugins kann den
  Host nie reißen.
- **Contra:** verlangt ein **IPC-Protokoll über den gesamten
  Port-Vertrag** (Serialisierung des Domänen-Modells, Latenz,
  Prozess-Management, Versionierung des Wire-Formats) — ein Vielfaches
  des v1-Scopes und gegen [REQ-TEC-008](../../../spec/spezifikation.md#9-technische-rahmenbedingungen-req-tec) (Shared Libraries) erst per
  Spec-Fortschreibung durchsetzbar. **Verworfen für v1** — benannter
  Re-Eval-Trigger (Sicherheits-/Vertrauens-AK, §Trigger); die
  Port-Vermittlung aus Option H bleibt dabei die wiederverwendbare
  Schnittstellen-Schicht (ein späterer Prozess-Host vermittelt dieselben
  Ports über IPC).

### Option S — statisch registrierte „Plugins" (im Binary eingebaut)

- Erfüllt weder Laden/Entladen **zur Laufzeit** ([LH-FA-PLG-001](../../../spec/lastenheft.md#modul-plugin-system-plg)) noch
  [REQ-TEC-008](../../../spec/spezifikation.md#9-technische-rahmenbedingungen-req-tec) (Shared Libraries) — **disqualifiziert** (kein echtes
  Plugin-System, nur ein Feature-Registry).

## Konsequenzen

- **Positiv:** Das Plugin-System kommt **ohne** Toolchain-Eingriff
  ([ADR-0004](0004-toolchain-dependency-pinning.md)), **ohne** neue Lizenz-Pflicht (ADR-0005) und **ohne**
  Kern-Architektur-Änderung aus — der Kern behält seine Driving-Ports,
  der Plugin-Host ist ein weiterer Adapter am bestehenden Hexagon
  (ADR-0001-Bild vervollständigt: der zweite Driving Adapter der
  Architektur wird real). Plugin-Mutationen erben Validierung
  ([`E-VAL-001`](../../../spec/spezifikation.md#4-fehler-codes-und-logging-felder)) und atomare Persistenz; ein künftiger Undo-Mechanismus
  ([LH-QA-003](../../../spec/lastenheft.md#lh-qa-003--undoredo), eigener Slice — heute nur das `undo_commands`-Schema aus
  [ADR-0006](0006-relationales-schema-design.md)) erfasst sie über dieselben Ports automatisch mit. Der
  ABI-Handshake macht Vertrags-Drift **beobachtbar** (Ablehnung statt
  UB). Zugleich ist der in [ADR-0009](0009-gui-framework-qt6.md) benannte Prüfpunkt »Plugin-Ausnahme
  der Regel E definieren« beantwortet: es entsteht **keine**
  Regel-E-Ausnahme (kein Qt im Plugin-Host, keine Qt-Pflicht in der
  Plugin-API); der Impl-Slice zieht den damit stale werdenden
  Plugin-Vorbehalts-Kommentar in `tools/arch-check.sh` nach.
  [OBJ-004](../../../spec/lastenheft.md#3-projektziele)/M5 wird erfüllbar.
- **Negativ / Folgepflicht (Slices):** (a) **AK-Schärfungs-Slice**:
  [LH-FA-PLG-001](../../../spec/lastenheft.md#modul-plugin-system-plg)–004 von Outline auf AK (lösungsfrei,
  [MR-008](../../../harness/conventions.md#mr-008--lastenheft-schärfung-bleibt-lösungsfrei); benutzer-beobachtbar: Laden/Entladen sichtbar,
  Fehlverhalten-Isolierung sichtbar, Modell-Integrität beobachtbar) +
  **Spec-Nachzug**: `spezifikation.md` **§4** [`E-PLG-001`](../../../spec/spezifikation.md#4-fehler-codes-und-logging-felder)-Zeile
  präzisieren (Load-/Handshake-Ablehnung vs. Laufzeit-Fehlverhalten —
  ein Code, zwei beobachtbare Wege, oder ein zweiter Code: entscheidet
  der Schärfungs-Slice), **§5** um einen Plugin-Lifecycle-Span
  erweitern (Outline dort ist ausdrücklich »ADR-Folge«), **§6** um eine
  Plugin-API-Vertragszeile. (b) **Impl-Slice(s)**: Plugin-Host
  (`src/adapters/plugin/`) + Plugin-API-Header-Satz + **Beispiel-/
  Test-Plugin** im `plugins/`-Baum (eigenes CMake-`MODULE`-Target; dient
  zugleich als AK-Fixture) + Composition-Root-Verdrahtung + **AK-Tests**
  (werfendes Plugin → [`E-PLG-001`](../../../spec/spezifikation.md#4-fehler-codes-und-logging-felder) + Modell unverändert;
  ABI-Mismatch → Ablehnung ohne Init; Load→Edit→Unload-Zyklus →
  Mutation validiert + undo-fähig, danach kein Zugriff mehr) +
  **arch-check-Regel P**. (c) [MR-006](../../../harness/conventions.md#mr-006--unabhängiges-plan-review-vor-implementierungs-start) vor jedem Impl-Start +
  unabhängiges Code-Review (Muster welle-4); [MR-009](../../../harness/conventions.md#mr-009--geometrielastiges-code-review-vor-welle-closure) voraussichtlich
  **n/a** (keine neue Solid-/Bauteil-Geometrie).
- **Neue arch-check-Regel nötig (Kontrast [ADR-0016](0016-pdf-png-backend.md)):** die Option-D-Codecs
  waren header-frei (Regel A+B genügten) — der Plugin-Host bringt mit
  `dlfcn` erstmals ein **neues systemnahes Symbol-Monopol** und mit
  `plugins/` erstmals **Repo-Code außerhalb von `src/`**, der gegen die
  Schichtung gehalten werden muss → **Regel P** (P1 Host-Monopol, P2
  Plugin-Import-Grenze) ist echte Folgepflicht, kein „gegenstandslos".
- **Gate-Scope-Entscheidung für `plugins/` (Impl-Slice, benannt):** nicht
  nur arch-check ist `src/`-verankert — auch der `make lint`-Vertrag
  lautet „0 Befunde in `src/`" und die Coverage misst `src/`. Ob das
  Beispiel-/Test-Plugin **gelintet und gemessen** wird (Aufnahme des
  `plugins/`-Baums in den Lint-/Coverage-Scope oder eine begründete
  Ausnahme), entscheidet der Impl-Slice **explizit** — analog zur
  Symbol-Naht, keine stille Setzung.
- **Negativ / Risiko (benannt):** in-process **kein Speicherschutz**
  (Absturz-Risiko durch defekte Plugins; Daten-Netz = [ADR-0003](0003-persistenz-sqlite.md)-Atomarität
  + [LH-QA-005](../../../spec/lastenheft.md#lh-qa-005--crash-recovery)); keine Cross-Compiler-Binärkompatibilität (Plugins
  bauen gegen den gepinnten Container-Stand); kein Vertrauens-/
  Signier-Modell (jedes geladene Plugin ist so privilegiert wie die GUI —
  Laden ist eine bewusste Nutzer-Entscheidung); `dlclose`-Entladen
  verlangt Plugin-Disziplin (Kontext-Invalidierung, vom Test sondiert)
  **und** trägt bekannte Restrisiken darüber hinaus (statische
  Destruktoren/TLS des Plugins; nach Unload noch referenzierte
  typeinfo-/Exception-Objekte) — der Impl-Slice entscheidet die
  Unload-Strategie im **Fehlerpfad** explizit; isolieren **ohne**
  `dlclose` (kontrolliertes Leak statt UB) ist zulässig.
- **ADR-0001/0003/0004/0005/0008/0009 bleiben unverändert gültig** —
  diese ADR baut auf ihnen auf (Schichtungs-Verortung von ADR-0001,
  Daten-Netz von [ADR-0003](0003-persistenz-sqlite.md), Pinning-Prämisse von [ADR-0004](0004-toolchain-dependency-pinning.md),
  Barriere-Muster von [ADR-0008](0008-aenderungs-benachrichtigung.md), Regel-E-Grenze von [ADR-0009](0009-gui-framework-qt6.md)).

## Fitness Function

| Tooling | Regel | Make-Target |
|---|---|---|
| Schichtung (heute real) | Der Plugin-Host lebt in `src/adapters/plugin/` — **Regel A** (Kern framework-frei) + **Regel B** (kein Adapter→Adapter) gelten unverändert; **Regel E** hält Qt aus dem Host (kein `QPluginLoader`) | `make arch-check` (ADR-0001/0009, real) |
| **Regel P** (Impl-Slice) | **P1:** `dlfcn.h`/`dlopen`/`dlsym`/`dlclose` nur in `src/adapters/plugin/` — **einschließlich** eines Verbots für den `plugins/`-Baum; **P2:** Dateien unter `plugins/` inkludieren nur Plugin-API- + `src/hexagon/model/`- + `src/hexagon/ports/driving/`-Header — keine `src/adapters/`-, Qt-, OCC-, SQLite- und keine `dlfcn.h`-Header | `make arch-check` (Erweiterung im Impl-Slice) |
| Keine neue Dependency (Impl-Slice) | `make build` zieht **keinen** neuen `find_package`-/apt-Eintrag fürs Plugin-System (`dlopen` = glibc; Plugin-Targets = CMake `MODULE` aus dem Repo-Baum) | `make build` |
| Sandbox-/Lifecycle-AK (Impl-Slice) | **Werfendes Test-Plugin** → [`E-PLG-001`](../../../spec/spezifikation.md#4-fehler-codes-und-logging-felder), Modell-Zustand unverändert (Vorher/Nachher-Sonde über Driving-Port-Abfragen), Host lebt weiter; **ABI-Mismatch-Plugin** → Ablehnung **ohne** Init-Aufruf; **Load→Edit→Unload-Zyklus** → Mutation via `EditStructurePort` geklemmt/validiert, nach Unload kein weiterer Effekt (Undo-Erfassung folgt erst mit dem [LH-QA-003](../../../spec/lastenheft.md#lh-qa-003--undoredo)-Slice — **keine** Plugin-Impl-Vorbedingung) | `make test` |
| Integration über den echten Pfad (Impl-Slice) | **Mindestens ein** Test lädt das reale Beispiel-Plugin (echte `.so` via `dlopen`) durch den echten Host — kein Handshake-/Lifecycle-Stub als einziges Orakel (welle-2-Lehre slice-015b: der echte Adapter-Pfad muss geübt werden) | `make test` |

## Re-Evaluierungs-Trigger

- **Binär verteilte Dritt-Plugins aus fremden Toolchains** werden
  verbindlich (AK oder realer Abnahme-Anspruch, der am
  gepinnten-Container-Bau scheitert) → **reine C-ABI** (Option C) als
  Supersedes-ADR; der `extern "C"`-Handshake bleibt die Eintrittstür.
- **Sicherheits-/Vertrauens-AK** (nicht vertrauenswürdige Plugins,
  Store-/Download-Szenario, Härtungs-Anforderung) → **Prozess-Isolation**
  (Option P) und/oder Signier-Modell — eigener Beschluss; die
  Port-Vermittlung bleibt die Schnittstellen-Schicht.
- **Plugin-Beobachter-Zugang** (Plugins reagieren auf Modell-Änderungen —
  heute pull-only in Hooks, `ModelChangedPort` ausgeschlossen) → eigener
  Beschluss an der [ADR-0008](0008-aenderungs-benachrichtigung.md)-Naht: Kapselung **und** Re-Entranz-Verbot
  müssen dann **technisch** durchgesetzt werden (der dort benannte
  Plugin-Prüfpunkt), keine stille Aufweichung der Import-Grenze aus
  Entscheidung #3.
- **UI-Erweiterungspunkte aus Plugins** (Docking-Panels, Werkzeuge in der
  Qt-GUI, [LH-FA-UI-001](../../../spec/lastenheft.md#modul-benutzeroberfläche-ui)/005-Bezug) → eigene Entscheidung an der
  Regel-E-Grenze (GUI-seitiger Erweiterungs-Vertrag), nicht dieser
  modell-seitige Vertrag.
- **Skript-Plugins** (Python/Lua statt Shared Library) → eigener Beschluss —
  ein Interpreter ist eine neue Dependency ([ADR-0004](0004-toolchain-dependency-pinning.md)-Beleg nötig) und
  [REQ-TEC-008](../../../spec/spezifikation.md#9-technische-rahmenbedingungen-req-tec) wäre fortzuschreiben.
- **Toolchain-Hub** ([ADR-0004](0004-toolchain-dependency-pinning.md)-Beschluss, neuer Compiler/libstdc++-Stand)
  → ABI-Version erhöhen (Vertragsbestandteil aus Entscheidung #2) — kein
  eigener ADR, aber ein benannter Prüfpunkt im Hub-Beschluss.

## Geschichte

| Datum | Ereignis | Verweis |
|---|---|---|
| 2026-07-02 | Proposed (Vorbereitung welle-5-erweiterung — benannter Wellen-Trigger; Plugin-Backend = `dlopen` + versionierter `extern "C"`-Handshake + C++-Port-Facade in-process, Plugin-Host als Driving Adapter, Sandbox = Port-Vermittlung + Fehler-Barriere mit ehrlich benannter Speicherschutz-Grenze; Regel P als Folgepflicht) | welle-5-Vorbereitung / Plugin-ADR |
| 2026-07-02 | **Unabhängiges Text-Review** (Reviewer ≠ Autor, read-only, vor Accept): **1 HIGH + 4 MED + 2 LOW + 3 INFO**, alle eingearbeitet — **HIGH-1** (nicht existierender Undo-Stack war als Gegenwarts-Eigenschaft behauptet + Fitness-AK „undo-fähig" daran gebunden → auf **künftigen** [LH-QA-003](../../../spec/lastenheft.md#lh-qa-003--undoredo)-Mechanismus umformuliert, AK-Zeile entkoppelt); **MED-1** (Silent-Corruption-Pfad benannt: UB ohne Crash + Speichern/Autosave überschreibt den letzten guten Stand — Persistenz-Netz deckt nur den Crash-Pfad, PLG-004-AK auf wohlgeformte Plugins bezogen); **MED-2** (Symbol-Auflösung als benannte Link-Naht: statisches Kern-Dazulinken **verboten**, Wahl Export-vs-Shared = Impl-Slice-Entscheidung mit Beleg); **MED-3** (v1 ohne Beobachter-Zugang — ADR-0008-Plugin-Prüfpunkt durch Ausschluss beantwortet, eigener Re-Eval-Trigger); **MED-4** (Threading-Vertrag: Hooks synchron im Hauptthread, keine Port-Rufe aus Plugin-Threads); **LOW-1** (slice-015b = welle-2-Lehre), **LOW-2** (`dlclose`-Restrisiken, Fehlerpfad-Isolierung ohne `dlclose` zulässig); **INFO-1** (ADR-0009-Prüfpunkt „Regel-E-Ausnahme" = keine, arch-check-Kommentar-Nachzug), **INFO-2** („ein **Build**-Compiler" präzisiert), **INFO-3** (ADR-0005-Proposed-Zitat präzedenz-konform, keine Änderung). Grundentscheidung (Option H, Regel P, Optionen-Vergleich) bestätigt; alle geprüften Zitate/IDs/Pfade quellen-treu. **Accept ausstehend (Projektinhaber)** | [`docs/reviews/2026-07-02-adr-0017-text-review.md`](../../reviews/2026-07-02-adr-0017-text-review.md) |
| 2026-07-02 | **Projektinhaber-Durchsicht** (zweite Runde, vor Accept): **0 HIGH/MED**, 2 LOW + 2 INFO eingearbeitet — **LOW-A** (Gate-Scope für `plugins/` bei lint/coverage ist `src/`-verankert und war unbenannt → explizite Impl-Slice-Entscheidung, analog Symbol-Naht); **LOW-B** (P1-Monopol um den `plugins/`-Baum geschlossen + `dlfcn.h` in die P2-Verbotsliste — ein Beispiel-Plugin darf nicht selbst `dlopen` rufen); **INFO-C** („Kern nicht thread-sicher" als Folgerung markiert, nicht als [ADR-0009](0009-gui-framework-qt6.md)-(d)-Zitatstelle); **INFO-D** (Prozess: Folgepflichten-Zeilen im ADR-Index mit dem Accept nachziehen). Quellen-Zitate und Review-Einarbeitung unabhängig bestätigt (u. a. kein Undo-Code in `src/hexagon/`, Symbol-Naht-Analyse trägt) | [`docs/reviews/2026-07-02-adr-0017-text-review.md`](../../reviews/2026-07-02-adr-0017-text-review.md) §Nachtrag |
| 2026-07-02 | **Accepted** (Projektinhaber) — unabhängiges Text-Review (1 HIGH + 4 MED + 2 LOW + 3 INFO) und Projektinhaber-Durchsicht (2 LOW + 2 INFO), alle eingearbeitet. Plugin-Backend = **`dlopen` + versionierter `extern "C"`-Handshake + C++-Port-Facade in-process** (Option H), Plugin-Host als Driving Adapter, Sandbox = Port-Vermittlung + Fehler-Barriere mit ehrlich benannten Grenzen (Speicherschutz, Silent-Corruption, Threading), Symbol-Naht als benannte Impl-Entscheidung. Folgepflichten (AK-Schärfung [LH-FA-PLG-001](../../../spec/lastenheft.md#modul-plugin-system-plg)..004 + Spec-§4/§5/§6-Nachzug, Impl inkl. Beispiel-Plugin + benannter Entscheidungen, **arch-check-Regel P**) im ADR-Index. **welle-5-Trigger erfüllt** → Wellen-Start als Roadmap-Buchung | welle-5-Buchung |
