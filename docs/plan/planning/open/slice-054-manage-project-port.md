---
id: slice-054
titel: ManageProjectPort realisieren — die deklarierte Ziel-Form des Projekt-Use-Case
status: open
welle: welle-5-erweiterung
lastenheft_refs: [[LH-FA-BLD-002](../../../../spec/lastenheft.md#lh-fa-bld-002--projekt-speichern), [LH-FA-BLD-003](../../../../spec/lastenheft.md#lh-fa-bld-003--projekt-laden), [ACC-005](../../../../spec/lastenheft.md#7-abnahmekriterien)]
adr_refs: [[ADR-0001](../../adr/0001-hexagonale-architektur.md), [ADR-0012](../../adr/0012-evaluations-architektur.md), [ADR-0019](../../adr/0019-drw-2d-canvas.md)]
---

# Slice 054: `ManageProjectPort` realisieren (Struktur-Vorläufer)

**Status:** open — **Struktur-Vorläufer**, verhaltens-invariant. Eigenes
[MR-006](../../../../harness/conventions.md#mr-006--unabhängiges-plan-review-vor-implementierungs-start)
vor dem Start. **Sequenz: 054 → [`slice-053`](slice-053-fenster-als-adapter.md) →
[`slice-052a`](slice-052a-sitzungs-zustand-und-speichern.md) →
[`slice-052b`](slice-052b-neues-projekt.md).**

**Welle:** welle-5-erweiterung (Quergewerk / Struktur-Vorbereitung).
**Autor:** Dietmar Burkard (AI-Harness-Lauf). **Datum:** 2026-07-26.

## Auslöser — die Wurzel einer vierfach wiederholten Klasse

Seit slice-047 landet in jedem GUI-Slice mindestens ein Review-Finding derselben Bauart: eine
**Entscheidung** liegt im coverage-ausgenommenen `src/main.cpp` und ist damit orakel-los.

| Wann | Finding |
|---|---|
| slice-047 | Verify **B4** — Neu-Auflösung des Zeichen-Ziels als Lambda in `main.cpp`, kein Sensor |
| slice-052, [Lauf 1](../../../reviews/2026-07-26-slice-052-plan.md) | MEDIUM-2/-3 — Ziel-Wahl und Antwort-Auswertung im Menü-Handler |
| slice-052a, [Lauf 3](../../../reviews/2026-07-26-slice-052a-plan-3.md) | **HIGH-1** — zugesagtes Orakel für das Schließ-Ereignis nicht herstellbar |
| slice-053, [Lauf 1](../../../reviews/2026-07-26-slice-053-plan.md) | **HIGH-1** — „Handler ziehen mit um" **und** „keine neue Kante" sind nicht gleichzeitig einlösbar |

**Die Ursache ist strukturell, nicht disziplinarisch.** `.a-check.yml` erlaubt `ui_command` nur
`model`/`ui_view`/`ports_driving` und `ui_view` nur `model`/`ports_driven` — **kein** Adapter darf
`hexagon/services/` rufen. Die Projekt-Use-Cases `services::openProject`/`saveProject` sind aber genau
das: **Services ohne Port**. Damit ist `main.cpp` der **einzige** Ort, der sie aufrufen darf
(`composition_root`), und jede Entscheidung, die an ihnen hängt, landet dort — im orakel-losen Bereich.

**`spec/architecture.md`:76 benennt die Auflösung seit dem Bootstrap selbst:**

> `ManageProjectPort` (**Ziel-Form, noch nicht realisiert**) … ein Treiber-Adapter spricht sie
> **direkt** an statt über eine Port-Abstraktion. **Die Port-Form bleibt das Ziel, sobald ein zweiter
> Treiber sie braucht.**

**Der Bedarf des zweiten Treibers ist belegt — eingetreten ist die Bedingung erst mit
[`slice-053`](slice-053-fenster-als-adapter.md).** 053 legt `src/adapters/ui/command/`-Handler an, die
Öffnen und Speichern rufen; ohne Port bleibt ihnen nur die Kante, die
[`.a-check.yml`](../../../../.a-check.yml) verbietet — das ist genau HIGH-1 des
[053-Plan-Reviews](../../../reviews/2026-07-26-slice-053-plan.md). 054 ist der **Vorläufer, der diesen
Treiber möglich macht**, nicht der Slice, der ihn liefert. Nach 054 hat der Port **einen** Treiber
(den Composition-Root, jetzt über die Port-Abstraktion), nach 053 den zweiten.

**Konsequenz für die Doku-Zusage** ([MR-006](../../../../harness/conventions.md#mr-006--unabhängiges-plan-review-vor-implementierungs-start)-Lauf 1, MEDIUM-5): `architecture.md` §1.1 wird auf
**realisiert** gezogen — das ist wahr, sobald Port und Implementierung existieren und der
Composition-Root über sie spricht. Die Zahl der Treiber ist **nicht** Teil der Zeile und wird auch
nicht behauptet.

## 1. Ziel

Die Projekt-Use-Cases stehen hinter einem **Driving Port** `ManageProjectPort`
(`src/hexagon/ports/driving/`), implementiert von einem Kern-Service. Der GUI-Composition-Root ruft sie
über den Port statt direkt.

**Was dieser Slice belegt und was nicht** ([MR-006](../../../../harness/conventions.md#mr-006--unabhängiges-plan-review-vor-implementierungs-start)-Lauf 1, HIGH-2): 054 stellt die **Aufrufbarkeit aus
einem Adapter her** — die Kante `ui_command → ports_driving` besteht in
[`.a-check.yml`](../../../../.a-check.yml):32, und der Port-Vertrag ist so geschnitten, dass ein
`ui/command/`-Handler ihn **ohne** `StructureEditService` und **ohne** `ProjectRepositoryPort` rufen
kann (§2.1). **Den Beleg dafür führt [`slice-053`](slice-053-fenster-als-adapter.md)**, das den Handler
anlegt und ihn schon heute als eigene Orakel-Zeile führt; erst dort sieht `make a-check` eine reale
`ui_command`-Datei am Port. 054 behauptet den Adapter-Beleg **nicht** — sein Binary `bcad_tests` linkt
nur `bcad_hexagon` (`tests/CMakeLists.txt`:9–29), und a-check scannt keine Tests
([`.a-check.yml`](../../../../.a-check.yml):3).

**Verhaltens-invariant:** dieselben Wirkungen, dieselben Fehler, dieselben Meldungen. Kein neues
Verhalten, keine neue Anforderung.

## 2. Bewusst NICHT Teil

- **Jede neue Funktion.** Kein „Speichern", kein „Neu", keine Rückfrage — das sind 052a/052b.
- **Projekt anlegen/versionieren** als *Verhalten*: der Port **darf** die Operationen deklarieren, die
  [`LH-FA-BLD-001`](../../../../spec/lastenheft.md#lh-fa-bld-001--projekt-anlegen)/[`004`](../../../../spec/lastenheft.md#lh-fa-bld-004--projektversionierung)
  später brauchen — geliefert wird in diesem Slice **nur**, was heute existiert (Öffnen, Speichern).
  Ein Port mit unimplementierten Methoden wäre eine Lüge im Vertrag.
- **Die CLI-Seite umbauen.** `runHeadlessCli` **bleibt bei den freien Funktionen** — entschieden, nicht
  offen gelassen ([MR-006](../../../../harness/conventions.md#mr-006--unabhängiges-plan-review-vor-implementierungs-start)-Lauf 1, MEDIUM-3). Grund am Artefakt: der CLI-Pfad speichert ein `Building`,
  das **kein Service hält** — mit `--open` stammt es aus `repository.load(open_path)` in eine lokale
  Variable (`src/main.cpp`:212/:224), während der Port-`save` sein `Building` aus dem injizierten
  `StructureEditService` bezieht (§2.1). Der CLI-`--open`-Pfad lädt zudem bewusst **ohne** Sitzung
  (spez. §1 zu [`LH-FA-BLD-002`](../../../../spec/lastenheft.md#lh-fa-bld-002--projekt-speichern)/[`003`](../../../../spec/lastenheft.md#lh-fa-bld-003--projekt-laden)) und bleibt
  semantisch, wie er ist. **Die DoD sagt deshalb „GUI-Composition-Root", nicht „`main.cpp`".**
- **Kein neuer ADR** — Begründung auf das normative Stratum gestellt ([MR-006](../../../../harness/conventions.md#mr-006--unabhängiges-plan-review-vor-implementierungs-start)-Lauf 1, MEDIUM-4):
  [ADR-0001](../../adr/0001-hexagonale-architektur.md) macht Use-Cases hinter Driving Ports zur
  **Regel**; ein Use-Case, der heute portlos ist, in die Regelform zu bringen, ist deren **Erfüllung**
  — keine Abweichung ([AGENTS §2.6](../../../../AGENTS.md) n/a, **keine** Gate-Lockerung: die genutzten
  Kanten `services → ports_driving` und `ui_command → ports_driving` bestehen).
  `spec/architecture.md`:76 ist dabei **Beleg für die Benennung**, nicht die normative Basis
  ([AGENTS §2.7](../../../../AGENTS.md): derivatives Sicht-Stratum).
  **Abgrenzung zur Präzedenz [ADR-0012](../../adr/0012-evaluations-architektur.md):** dort war offen,
  **ob** ein neuer Port entsteht oder ein bestehender erweitert wird — eine echte Lösungswahl. Hier ist
  der Port seit dem Bootstrap namentlich zugeordnet; offen war allein die **Signatur**, und die ist
  durch die Schicht-Regel determiniert (§2.1). Der **Zuschnitt** (zwei der vier in
  `architecture.md`:76 genannten Aufgaben) wird im Closure-Text explizit protokolliert (R1).

## 2.1 Die Port-Signatur — entschieden, nicht offen (MR-006-Lauf 1, HIGH-1 / R2)

**Die Zwangsbedingung.** [`.a-check.yml`](../../../../.a-check.yml):26 führt für `ports_driving`
**genau eine** ausgehende Kante — `{from: ports_driving, to: model}`; identisch die kanonische Quelle
[`spec/architecture.md`](../../../../spec/architecture.md):103 („Darf importieren: model | Darf NICHT
importieren: services, adapters"). Alle acht Bestands-Ports halten das ein. Die heutigen Signaturen
führen aber `ports::driven::ProjectRepositoryPort&` und `StructureEditService&`
(`src/hexagon/services/manage_project.h`:51–53/:72–75) — **ein Port-Header mit diesen Parametern wäre
`wrong-direction`.** Die ursprüngliche DoD-Formulierung „die **heute existierenden** Operationen" und
die Zusage „keine neue Kante" waren damit unvereinbar; die Kanten-Zusage gewinnt, die Signatur wird
umgeschnitten.

**Der Schnitt.** Infrastruktur nach innen (Konstruktor), Aufruf-Daten in der Methode:

```cpp
// src/hexagon/ports/driving/manage_project_port.h — nur <std> + model
namespace bcad::hexagon::ports::driving {

struct DrawingTargetSinks { /* zwei std::function über model::StoreyId/LayerId */ };
enum class DrawingTargetResolution { Resolved, NoStorey, NoLayer };

class ManageProjectPort {
  public:
    virtual ~ManageProjectPort() = default;
    virtual DrawingTargetResolution openProject(const std::filesystem::path& path,
                                                const DrawingTargetSinks& sinks = {}) = 0;
    virtual void saveProject(const std::filesystem::path& path) = 0;
};
}  // namespace
```

- **`DrawingTargetSinks`/`DrawingTargetResolution` ziehen in den Port-Header um.** Sie sind heute in
  `services/manage_project.h`:25/:34 definiert und brauchen nur `<functional>` + `model::StoreyId`/
  `model::LayerId` — im Port-Header **kantenfrei**. `services/` inkludiert den Port (Kante
  `services → ports_driving` besteht, [`.a-check.yml`](../../../../.a-check.yml):22) und hält für beide
  Typen einen **`using`-Alias** im Namensraum `services`, damit die bestehenden Tests **wortgleich**
  weiterkompilieren — das ist der Invarianz-Beleg aus R3, nicht Bequemlichkeit.
- **Die Sinks bleiben Methoden-Parameter** (Projektinhaber-Entscheidung 2026-07-27 zu R2). `std::function`
  ist framework-frei und gate-legal; der Sitzungs-Zustand des Service bleibt kleiner, dafür führt der
  053-Handler die Sinks selbst und reicht sie durch. **Konsequenz für 053 hier festgehalten**, damit sie
  dort nicht neu entschieden wird.
- **`saveProject` führt kein `Building` mehr.** Der Service bezieht es aus dem injizierten
  `StructureEditService` (`service.building()`, heute `src/main.cpp`:337–338 im GUI-Lambda) — genau das
  löst HIGH-2: ein `ui_command`-Handler darf `StructureEditService` nicht sehen und braucht ihn nun auch
  nicht.

**Die freien Funktionen bleiben** ([MR-006](../../../../harness/conventions.md#mr-006--unabhängiges-plan-review-vor-implementierungs-start)-Lauf 1, MEDIUM-2 — entschieden). `ManageProjectService`
**delegiert** an sie; sie sind die geteilte Ableitungs-Logik von CLI, GUI und **drei
Adapter-Testdateien**, die der ursprüngliche Datei-Plan nicht nannte:
`tests/adapters/save_project_test_helper.h`:14–18, `tests/adapters/test_sqlite_crash_recovery.cpp`,
`tests/adapters/test_sqlite_project_repository.cpp` (18 Aufrufe). Diese drei bleiben **unberührt** —
sie speichern ad-hoc gebaute `Building`-Werte, die kein Service hält, und ihr Gegenstand ist die
**Atomarität des Persistenz-Adapters**, nicht der Port. **Benannte Konsequenz:** die Atomarität wird
damit **nicht** über den Port geprüft; §3-Zeile 3 sagt deshalb nur zu, was sie belegen kann
(Delegation + fail-closed-Wurf), und die Atomaritäts-Orakel bleiben, wo sie hingehören.

## 3. Orakel-Schnitt

| # | Zusicherung | Diskriminierende Gegenprobe |
|---|---|---|
| 1 | **Der Port-Vertrag ist allein aus der Port-Sicht bedienbar**: ein Aufrufer, der **nur** `ManageProjectPort&` hält (kein `StructureEditService`, kein `ProjectRepositoryPort`, keine `services`-Typen), öffnet **und** speichert vollständig — genau die Sicht, die ein `ui_command`-Handler hat | die Port-Methode delegiert nicht mehr (leerer Rumpf / falscher Pfad) ⇒ rot |
| 2 | **Öffnen über den Port** ersetzt den Sitzungs-Stand und löst das Zeichen-Ziel neu auf (die slice-047-Zusagen gelten unverändert weiter) | Zeichen-Ziel-Auflösung entfernt ⇒ rot (heutiges Orakel bleibt gültig) |
| 3 | **Speichern über den Port** erreicht denselben Schreibweg wie die freie Funktion (gleiche `PersistedDerivations`, gleicher `save`-Aufruf) und wirft **fail-closed** bei danglendem `from_storey` | Wurf entfernt ⇒ rot; Delegation umgangen ⇒ rot. **Nicht** hier belegt: die Atomarität — sie ist Adapter-Eigenschaft und bleibt im unveränderten `test_sqlite_crash_recovery.cpp` (§2.1) |
| 4 | **Fehler kommen neutral durch** (keine Framework-Typen im Port-Vertrag; `std::runtime_error` wie heute) | Fehler geschluckt ⇒ rot |
| 5 | **`make a-check` grün ohne neue Kante**: der Port-Header importiert nur `<std>` + `hexagon/model/*`, `services → ports_driving` besteht | **in-slice diskriminierend:** probeweise `hexagon/ports/driven/project_repository_port.h` in den Port-Header ⇒ `wrong-direction` (a-check-Handbuch §3.4). Die frühere Gegenprobe („eine `ui_*` → `services`-Kante") konnte nicht eintreten — 054 fasst keine Adapter-Datei an ([MR-006](../../../../harness/conventions.md#mr-006--unabhängiges-plan-review-vor-implementierungs-start)-Lauf 1, HIGH-2) |

**Die bestehenden Orakel bleiben der Maßstab — durch Nicht-Änderung** (Präzisierung aus [MR-006](../../../../harness/conventions.md#mr-006--unabhängiges-plan-review-vor-implementierungs-start)-Lauf 1,
LOW-1): `tests/hexagon/test_manage_project.cpp` (15 Tests, davon 6 über die reinen
Ableitungs-Funktionen) und `tests/adapters/test_project_open_handler.cpp` prüfen heute Verhalten, das
sich **nicht ändern darf**. Sie ziehen **nicht** auf den Port um — sie bleiben auf den freien
Funktionen und damit **wortgleich**; der `using`-Alias (§2.1) hält sie ohne Textänderung
kompilierbar. **Das ist der stärkere Invarianz-Beleg:** ein Test, der mitumgebaut wird, kann seine
eigene Regression nicht mehr zeigen (R3). Der **Port-Weg** bekommt seine eigene Datei
(`tests/hexagon/test_manage_project_port.cpp`, §3-Zeilen 1/3/4) — additiv, nicht ersetzend.

## 4. Definition of Done

- [ ] **`src/hexagon/ports/driving/manage_project_port.{h}`**: Driving Port mit den zwei heute
      existierenden Use-Cases (Öffnen, Speichern) **in der Signatur nach §2.1** — nur `<std>` +
      `hexagon/model/*`, framework-frei, neutrale Fehler; `DrawingTargetSinks`/
      `DrawingTargetResolution` hierher umgezogen.
- [ ] **Kern-Implementierung**: `ManageProjectService` in `src/hexagon/services/manage_project.{h,cpp}`
      erfüllt den Port, hält `StructureEditService&` + `const ProjectRepositoryPort&` als
      Konstruktor-Abhängigkeiten und **delegiert** an die erhalten bleibenden freien Funktionen (§2.1).
      `services`-seitige `using`-Aliase für die zwei umgezogenen Typen.
- [ ] **Aufrufer umgestellt**: der **GUI-Composition-Root** in `src/main.cpp` (Öffnen- und
      Speichern-Lambda, heute :306 und :337–338) ruft ausschließlich über den Port.
      **`runHeadlessCli` bleibt bei den freien Funktionen** — begründet in §2, nicht offen (MEDIUM-3);
      `make io-smoke` belegt, dass der CLI-Pfad unverändert funktioniert.
- [ ] **Orakel §3-1..5** je mit roter Gegenprobe im Closure-Text.
- [ ] **`spec/architecture.md` §1.1**: die `ManageProjectPort`-Zeile von „**Ziel-Form, noch nicht
      realisiert**" auf **realisiert** ziehen, samt der `Ist-Zustand`-Prosa; §2.1-Baum-Klammer
      „(ManageProjectPort: Ziel-Form, s. §1.1)" nachziehen. **Das ist die eigentliche Doku-Zusage
      dieses Slice** — die Datei behauptet den Zielzustand seit dem Bootstrap.
- [ ] **`spec/spezifikation.md`**: der §1-Block [`LH-FA-BLD-002`](../../../../spec/lastenheft.md#lh-fa-bld-002--projekt-speichern)`.a`/[`003`](../../../../spec/lastenheft.md#lh-fa-bld-003--projekt-laden)`.a` beschreibt die Aufruf-Wege;
      prüfen, ob die Port-Naht dort eine Aussage berührt — **nachziehen oder begründet unberührt
      lassen** (die Entscheidung steht im Closure-Text, nicht implizit).
- [ ] **CHANGELOG** [Unreleased]-Eintrag (Struktur, verhaltens-invariant).
- [ ] **Kein** Lastenheft-Eintrag, **kein** Handbuch-Eintrag — nichts wird benutzer-sichtbar. (Zeile
      bewusst gesetzt: geprüft und **verneint**, nicht vergessen — Lehre aus slice-047 V1.)
- [ ] **`make gates` grün**, **`make io-smoke` grün** (der CLI-Pfad muss unverändert funktionieren),
      `make schema-check` byte-unberührt.
- [ ] **Ruhe-Marker-Toggle** ([MR-017](../../../../harness/conventions.md)): 054 ist der **erste**
      Slice in `in-progress/` seit der slice-049-Closure — der reservierte Sentinel im
      `## Aktuelle Welle`-Block von [`roadmap.md`](../in-progress/roadmap.md) wird **im selben Commit
      wie der `git mv`** entfernt und bei der Closure wieder gesetzt; `planning-drift` ist
      `make gates`-Member. (Zeile aus [MR-006](../../../../harness/conventions.md#mr-006--unabhängiges-plan-review-vor-implementierungs-start)-Lauf 1, LOW-3.)

## 5. Plan (vor Code)

| Datei / Komponente | Art | Begründung |
|---|---|---|
| `src/hexagon/ports/driving/manage_project_port.{h}` | neu | der deklarierte Driving Port (Signatur §2.1); header-only wie alle acht Bestands-Ports. **Klammerform beibehalten** — `codepaths.roots` enthält `src` ([`.d-check.yml`](../../../../.d-check.yml):16), ein expliziter Pfad auf eine geplante Datei meldet `codepath-missing` (gemessen 2026-07-27; damit ist LOW-2 des Reviews **widerlegt**) |
| `src/hexagon/services/manage_project.{h,cpp}` | ändern | `ManageProjectService` erfüllt den Port und delegiert; `using`-Aliase für die umgezogenen Typen |
| `src/main.cpp` | ändern | die **zwei GUI-Lambdas** rufen über den Port (:306, :337–338); `runHeadlessCli` unverändert |
| **`tests/CMakeLists.txt`** | **ändern** | zählt die Kern-Testquellen **explizit** auf (:10–27) — **ohne Eintrag würde die neue Datei stumm nicht gebaut und `make gates` bliebe grün** ([MR-006](../../../../harness/conventions.md#mr-006--unabhängiges-plan-review-vor-implementierungs-start)-Lauf 1, MEDIUM-1) |
| `tests/hexagon/test_manage_project_port.cpp` | neu | §3-Zeilen 1/3/4: Aufruf **allein über `ManageProjectPort&`** |
| `spec/architecture.md` | ändern | §1.1 „Ziel-Form" → realisiert; §2.1-Klammer |
| `spec/spezifikation.md`, `spec/spezifikation-historie.md` | ändern o. begründet unberührt | §1-Aufruf-Wege |
| `CHANGELOG.md` | ändern | [Unreleased] |
| `docs/reviews/`-Report | **liegt** | [MR-006-Lauf 1](../../../reviews/2026-07-27-slice-054-plan.md) (2 HIGH · 5 MED · 4 LOW), eingearbeitet — s. §11 |

**Nicht berührt — begründet:**

- `.a-check.yml` (**keine** neue Kante — `services → ports_driving` und `ui_command → ports_driving`
  existieren), `data-model.yaml`/`schema.sql`, `docs/plan/adr/`, `spec/lastenheft.md`, `docs/user/`.
- **`src/hexagon/CMakeLists.txt`** — die Liste enthält ausschließlich `.cpp`-Dateien (:8–20); der Port
  ist header-only, eine neue `.cpp` entsteht nicht ([MR-006](../../../../harness/conventions.md#mr-006--unabhängiges-plan-review-vor-implementierungs-start)-Lauf 1, MEDIUM-1, zweite Hälfte).
- **`tests/hexagon/test_manage_project.cpp`, `tests/adapters/test_project_open_handler.cpp`** — bleiben
  **wortgleich** auf den freien Funktionen; das ist der Invarianz-Beleg (§3, LOW-1).
- **`tests/adapters/save_project_test_helper.h`, `tests/adapters/test_sqlite_crash_recovery.cpp`,
  `tests/adapters/test_sqlite_project_repository.cpp`** — tragen die **Atomaritäts**-Orakel über die
  freien Funktionen und bleiben unberührt (§2.1, MEDIUM-2). Ihre Nennung hier ist Teil der Zusage:
  sie wurden geprüft, nicht übersehen.
- **`src/adapters/ui/**`** — 054 legt **keine** Adapter-Datei an; der Adapter-Beleg gehört zu 053 (§1).

## 6. Risiken

- **R1 — Port-Zuschnitt.** `ManageProjectPort` ist in `architecture.md` mit **vier** Aufgaben
  beschrieben (anlegen, speichern, laden, versionieren). Nur zwei existieren. Der Port darf **nicht**
  vorgeben, was es nicht gibt (§2) — die Abweichung zur Architektur-Zeile ist beim Nachziehen
  **explizit** zu formulieren, sonst entsteht dieselbe Doku-Unwahrheit, die slice-047 als F4 fand.
- **R2 — die `openProject`-Signatur trägt bereits eine Naht. → ENTSCHIEDEN (2026-07-27, §2.1).** Die
  `DrawingTargetSinks` bleiben **Methoden-Parameter** des Ports und ziehen samt
  `DrawingTargetResolution` in den Port-Header um. Restrisiko: der 053-Handler muss sie führen und
  durchreichen — hier festgehalten, damit 053 es nicht neu entscheidet.
- **R3 — Invarianz-Beleg hängt an den bestehenden Tests.** Neu gefasst nach [MR-006](../../../../harness/conventions.md#mr-006--unabhängiges-plan-review-vor-implementierungs-start)-Lauf 1 (LOW-1): die
  bestehenden Tests ziehen **nicht** um, sie bleiben wortgleich auf den freien Funktionen. Das Risiko
  kehrt sich damit um — nicht „wer sie umformuliert, verliert den Beleg", sondern: **wer die freien
  Funktionen entfernt oder ihre Semantik verschiebt, verliert ihn.** Der `using`-Alias (§2.1) ist der
  Mechanismus, der die Wortgleichheit trägt; bricht er, ist der Schnitt falsch.
- **R4 — der Port ist nach 054 einfach-getrieben** (nur der GUI-Composition-Root). Der zweite Treiber
  kommt erst mit [`slice-053`](slice-053-fenster-als-adapter.md). Bleibt 053 liegen, steht ein Port mit
  einem Treiber — vertretbar (die Testbarkeit ist der Gewinn), aber die Roadmap-Buchung muss das sagen.

## 7. Trigger

- **[MR-006 zu slice-053](../../../reviews/2026-07-26-slice-053-plan.md) HIGH-1** (Kanten-Widerspruch)
  + die dreifache Vorgeschichte (Tabelle oben). Projektinhaber-Entscheidung 2026-07-26: **die
  deklarierte Ziel-Form realisieren**, statt die Klasse ein fünftes Mal einzeln zu behandeln.

## 8. Closure-Trigger

- §3-Zeilen grün + je diskriminierend belegt (Zeile 5 **mit** dem `wrong-direction`-Beleg);
  `tests/hexagon/test_manage_project.cpp` + `tests/adapters/test_project_open_handler.cpp`
  **byte-unverändert** (`git diff --stat` im Closure-Text); `make gates` + `make io-smoke` grün;
  `architecture.md` §1.1 sagt die Wahrheit; R1-Zuschnitt (2 von 4 Aufgaben) protokolliert; Ruhe-Marker
  zurückgesetzt; Closure-Notiz.

## 9. Sub-Area-Modus-Begründung

### Sub-Area: Domänen-Modell + Ports + Services (Hexagon-Kern)

- **Modus:** GF; **Dichte:** mittel — ein Port, eine Implementierung, ein Aufrufer-Umbau; der Aufwand
  steckt im **Zuschnitt** (R1/R2), nicht im Umfang.
- **Phase-Reife:** die Use-Cases sind seit slice-047 real und getestet; dieser Slice gibt ihnen die
  Form, die die Architektur seit dem Bootstrap vorsieht.
- **Risiko:** niedrig-mittel — verhaltens-invariant mit vorhandenem Vorher-Sensor (anders als bei
  einem reinen `main.cpp`-Umzug).

## 10. MR-006-Einarbeitung — Lauf 1 (2026-07-27)

**Report:** [`2026-07-27-slice-054-plan.md`](../../../reviews/2026-07-27-slice-054-plan.md) —
**2 HIGH · 5 MEDIUM · 4 LOW · 2 INFO**, Verdikt „nicht startbar". Unabhängiger Reviewer ≠ Plan-Autor,
ohne Autoren-Kontext ([`.harness/skills/reviewer.md`](../../../../.harness/skills/reviewer.md) v1.0).
**Beide HIGH am Artefakt nachgeprüft und bestätigt**, bevor der Plan geändert wurde.

| Finding | Auflösung | Wo im Plan |
|---|---|---|
| **HIGH-1** Port-Signatur vs. Kanten-Zusage | Signatur umgeschnitten: Infrastruktur in den Konstruktor, nur `<std>`+`model` im Header | **§2.1** (neu) |
| **HIGH-2** Adapter-Beleg ohne Sensor | 054 stellt die Aufrufbarkeit **her**, 053 **belegt** sie; §3-1 auf die Port-Sicht reduziert, §3-5 mit in-slice erreichbarer Gegenprobe | **§1**, §3-1/-5 |
| **MED-1** Build-Liste | `tests/CMakeLists.txt` aufgenommen, `src/hexagon/CMakeLists.txt` als begründet unberührt | §5 |
| **MED-2** Atomarität am Port vorbei | freie Funktionen bleiben, drei Adapter-Testdateien namentlich unberührt, §3-3 sagt nur zu, was sie belegt | §2.1, §3-3, §5 |
| **MED-3** DoD ↔ §2 widersprüchlich | entschieden: **GUI**-Composition-Root stellt um, `runHeadlessCli` nicht — mit Artefakt-Begründung | §2, §4 |
| **MED-4** „kein ADR" auf derivativem Stratum | Begründung auf [ADR-0001](../../adr/0001-hexagonale-architektur.md) gestellt; Abgrenzung zur [ADR-0012](../../adr/0012-evaluations-architektur.md)-Präzedenz | §2 |
| **MED-5** Auslöse-Bedingung nicht eingetreten | im Perfekt formulierte Behauptung ersetzt; Doku-Zusage auf das reduziert, was wahr wird | §Auslöser |
| **LOW-1** „ziehen um" trifft 6/15 Tests nicht | Invarianz-Beleg auf **Nicht-Änderung** umgestellt | §3 |
| **LOW-2** `.{h}`-Klammerform | **widerlegt** — `make docs-check` am 2026-07-27 gemessen: der explizite Pfad meldet `codepath-missing`, die Klammer ist das Ventil. Notation bleibt, Begründung steht jetzt im Datei-Plan | §5 |
| **LOW-3** [MR-017](../../../../harness/conventions.md)-Toggle fehlt | DoD-Zeile ergänzt | §4 |
| **LOW-4** [ACC-005](../../../../spec/lastenheft.md#7-abnahmekriterien) nicht im Frontmatter | ergänzt | Frontmatter |

**Projektinhaber-Entscheidungen 2026-07-27** (die drei Fragen, die der Report offen ließ): Sinks
**bleiben Methoden-Parameter** (nicht konstruktor-injiziert) · Adapter-Beleg **wandert nach 053**
(Sequenz 054 → 053 unverändert) · **kein neuer ADR**, Begründung neu abgestützt.

**Startbar:** ja — beide HIGH sind aufgelöst, alle MEDIUM/LOW eingearbeitet. Ein zweiter [MR-006](../../../../harness/conventions.md#mr-006--unabhängiges-plan-review-vor-implementierungs-start)-Lauf
ist **nicht** erforderlich ([MR-006](../../../../harness/conventions.md#mr-006--unabhängiges-plan-review-vor-implementierungs-start) verlangt Einarbeitung vor dem Start, keinen erneuten Lauf); die
Änderungen sind Präzisierungen des bestehenden Schnitts, keine neue Lösungsrichtung.

## 11. Closure-Notiz

_(bei Ausführung auszufüllen)_
