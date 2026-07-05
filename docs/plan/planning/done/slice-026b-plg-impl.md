---
id: slice-026b
titel: Plugin-System — Implementierung Plugin-Host/API/Beispiel-Plugin + Regel P (parametrisiert auf [ADR-0017](../../adr/0017-plugin-api-abi.md))
status: done
welle: welle-5-erweiterung
lastenheft_refs: [[LH-FA-PLG-001](../../../../spec/lastenheft.md#lh-fa-plg-001), [LH-FA-PLG-002](../../../../spec/lastenheft.md#lh-fa-plg-002), [LH-FA-PLG-003](../../../../spec/lastenheft.md#lh-fa-plg-003), [LH-FA-PLG-004](../../../../spec/lastenheft.md#lh-fa-plg-004)]
adr_refs: [[ADR-0001](../../adr/0001-hexagonale-architektur.md), [ADR-0004](../../adr/0004-toolchain-dependency-pinning.md), [ADR-0008](../../adr/0008-aenderungs-benachrichtigung.md), [ADR-0009](../../adr/0009-gui-framework-qt6.md), [ADR-0017](../../adr/0017-plugin-api-abi.md)]
---

# Slice 026b: Plugin-System — Implementierung (Host, API, Beispiel-Plugin, Regel P)

**Status:** done (2026-07-03). [MR-006](../../../../harness/conventions.md#mr-006--unabhängiges-plan-review-vor-implementierungs-start)-Plan-Review
**0 HIGH / 1 MED / 3 LOW / 2 INFO** (alle vor Start eingearbeitet; alle vier
Entscheidungspunkte bestätigt) —
[Report](../../../reviews/2026-07-03-slice-026b-plan.md). Unabhängiges
**Code-Review 0 HIGH / 3 MED / 3 LOW** (MED-1..3 + LOW-1..2 vor Closure
behoben) — [Report](../../../reviews/2026-07-03-slice-026b-code-review.md).
DoD vollständig, `make gates` grün (228/228, Coverage 90,3 %),
Closure-Notiz §8.

**Welle:** welle-5-erweiterung (PLG-Strang, Implementierungs-Hälfte —
**M5-bindender Strang**; Muster slice-019b/c [IFC] / slice-020b [STEP/STL] /
slice-021b [DXF] / slice-025b/c [PDF/PNG]).

**Bezug:** [LH-FA-PLG-001](../../../../spec/lastenheft.md#lh-fa-plg-001)
(Dynamische Plugins) + [LH-FA-PLG-002](../../../../spec/lastenheft.md#lh-fa-plg-002)
(Plugin-API) + [LH-FA-PLG-003](../../../../spec/lastenheft.md#lh-fa-plg-003)
(Lifecycle) + [LH-FA-PLG-004](../../../../spec/lastenheft.md#lh-fa-plg-004)
(Sandbox) — seit slice-026a auf **AK-Niveau** (Lastenheft 0.1.13) mit
entschiedenem §1-Mapping
([`LH-FA-PLG-001.a`](../../../../spec/lastenheft.md#lh-fa-plg-001), Sammelblock).
**Parametrisiert auf [ADR-0017](../../adr/0017-plugin-api-abi.md)** (accepted
2026-07-02): `dlopen` + versionierter `extern "C"`-Handshake fail-closed +
C++-Port-Facade in-process; Plugin-Host als **Driving Adapter**; Sandbox =
Port-Vermittlung + Fehler-Barriere. Dieser Slice löst die **zwei offenen
[ADR-0017](../../adr/0017-plugin-api-abi.md)-Folgepflichten** ein (Impl +
arch-check-**Regel P**) und trifft die **drei benannten
Impl-Entscheidungen mit Beleg** (Symbol-Naht, Gate-Scope `plugins/`,
Unload-Strategie im Fehlerpfad) **plus das dokumentierte Port-Subset v1**
(Review-INFO-1: nur die Symbol-Naht braucht empirischen Beleg, die übrigen
Dokumentation).
[ADR-0001](../../adr/0001-hexagonale-architektur.md) (Schichtung),
[ADR-0004](../../adr/0004-toolchain-dependency-pinning.md) (keine neue
Dependency — `dlopen` = glibc),
[ADR-0008](../../adr/0008-aenderungs-benachrichtigung.md) (Fehler-Barriere-Muster
„werfender Beobachter"; kein Beobachter-Zugang v1),
[ADR-0009](../../adr/0009-gui-framework-qt6.md) (kein Qt im Host; Threading-Prämisse;
Regel-E-Kommentar-Nachzug).

**Autor:** Dietmar Burkard. **Datum:** 2026-07-03.

**Schnitt-Herkunft:** Implementierungs-Hälfte des PLG-Strangs, wie in
slice-026a §2 („Nicht Teil") und [ADR-0017](../../adr/0017-plugin-api-abi.md)
§Konsequenzen (b) zugeschnitten: Plugin-Host + Plugin-API-Header-Satz +
Beispiel-/Test-Plugin im `plugins/`-Baum + Composition-Root + AK-Tests +
Regel P + benannte Entscheidungen — **ein** Slice, da die Teile nur gemeinsam
testbar sind (der AK-Test lädt eine **reale** `.so` durch den **echten** Host;
ohne Host kein Plugin-Test, ohne Plugin kein Host-Test). **Nicht Teil:**
GUI-Plugin-Verwaltung (Laden/Entladen aus der Oberfläche — UI-Strang der
welle-5, benannte Lücke §6), Beobachter-Zugang/UI-Erweiterungspunkte/
Signierung/Skript-Plugins ([ADR-0017](../../adr/0017-plugin-api-abi.md)
§Trigger), Undo-Mechanismus
([LH-QA-003](../../../../spec/lastenheft.md#lh-qa-003--undoredo) — **nicht
implementiert**, keine Vorbedingung).

---

## 1. Ziel

Das Plugin-System wird **lauffähig**: b-cad lädt eine Shared Library
([REQ-TEC-008](../../../../spec/spezifikation.md#9-technische-rahmenbedingungen-req-tec))
zur Laufzeit über den Plugin-Host, prüft den versionierten Vertrag
**fail-closed**, reicht dem Plugin einen Kontext mit Driving-Port-Referenzen,
isoliert Fehlverhalten an der Fehler-Barriere und entlädt kontrolliert — genau
das §1-Mapping aus slice-026a
([`LH-FA-PLG-001.a`](../../../../spec/lastenheft.md#lh-fa-plg-001)), belegt
durch AK-Tests mit **realen `.so`-Dateien durch den echten Host** (welle-2-Lehre
slice-015b: der echte Adapter-Pfad muss geübt werden — kein Handshake-Stub als
einziges Orakel). Die **arch-check-Regel P** macht das dlfcn-Monopol und die
Plugin-Import-Grenze computational.

**Vier benannte Entscheidungen (dieser Slice; 1–3 mit Beleg, 4 dokumentiert —
Review-INFO-1):**

1. **Symbol-Naht → Executable-Symbol-Export (CMake `ENABLE_EXPORTS`).**
   Der Kern bleibt eine statische Bibliothek; die Executables, die Plugins
   laden (`b-cad`, `bcad_adapter_tests`), exportieren ihre Symbole
   (`ENABLE_EXPORTS ON` ≙ `-rdynamic`). Begründung: `src/hexagon/model/` und
   `src/hexagon/ports/driving/` sind **header-only** (pure Werttypen bzw.
   pure-virtual Interfaces ohne Key-Function) — ein Plugin braucht zur
   Ladezeit fast keine Kern-Symbole; die vtable-/typeinfo-Symbole der
   Port-Interfaces entstehen als **weak symbols** in Host **und** Plugin und
   werden vom dynamischen Lader über die exportierte Symboltabelle des
   Executables **unifiziert** (trägt Exception-Catch über die Modul-Grenze).
   Die Alternative (`bcad_hexagon` als Shared Library) wäre eine
   Build-Topologie-Änderung ohne v1-Nutzen. **Verboten** bleibt statisches
   Kern-Dazulinken ins Plugin ([ADR-0017](../../adr/0017-plugin-api-abi.md)
   §Entscheidung 2): Plugin-Targets linken **nur** gegen das
   Include-INTERFACE-Target `bcad_plugin_api` — keine `bcad_hexagon`-Linkage.
   **Beleg:** der AK-Test fängt eine im Plugin geworfene Exception im Host
   (typeinfo-Unifikation), und die reale `.so` lädt/arbeitet/entlädt durch den
   echten Host (`make test`).
2. **Gate-Scope `plugins/` → lint JA, Coverage NEIN.** `make lint` erfasst
   künftig `find src plugins -name '*.cpp'` (**Verschärfung**, kein
   [AGENTS.md §2.6](../../../../AGENTS.md)-Fall) — `plugins/` ist Repo-Code
   und hält die 0-Befunde-Latte. Die **Coverage** bleibt auf `src/`
   gefiltert: die `plugins/`-Inhalte sind **AK-Fixtures** (Beispiel- und
   Test-Plugins), deren Abdeckung das Testsystem misst, nicht das Produkt;
   die Produkt-Logik (Host, `src/adapters/plugin/`) ist voll im
   Coverage-Scope. Keine Schwellen-Änderung.
3. **Unload-Strategie im Fehlerpfad → Isolieren OHNE `dlclose`
   (kontrolliertes Leak).** Happy-Pfad: Shutdown-Hook → Kontext-Invalidierung
   → `dlclose`. Fehlerpfad (Load-/Handshake-/Init-/Laufzeit-/Shutdown-Fehler):
   Plugin wird **isoliert** (Kontext invalidiert, keine weiteren Hook-Aufrufe),
   aber **nicht** `dlclose`d — die [ADR-0017](../../adr/0017-plugin-api-abi.md)
   benennt die `dlclose`-Restrisiken (statische Destruktoren/TLS, nach Unload
   referenzierte typeinfo-/Exception-Objekte) und erlaubt das Leak explizit
   (UB-Vermeidung schlägt Speicher-Rückgabe im Fehlerfall).
4. **Port-Subset v1 → `EditStructurePort` + `EvaluatePort`** („lesend +
   editierend zuerst", [ADR-0017](../../adr/0017-plugin-api-abi.md)
   §Entscheidung 3). Beide implementiert `StructureEditService` — der Kontext
   reicht zwei Referenzen auf **denselben** Service, ohne das dem Plugin zu
   zeigen. `ExchangeModelPort`/`DetectRoomsPort`/`ViewModelPort` folgen bei
   Bedarf (additive Kontext-Erweiterung, ABI-Version steigt).

**Struktur-Entscheidung (API-Ort):** Der Plugin-API-Header-Satz lebt in
**`src/{plugin_api}/`** (Brace-Form = geplanter Pfad fürs codepaths-Gate; der
Name `plugin_api` steht fest — Review-INFO-2; header-only, neues
Include-INTERFACE-Target) — **nicht**
in `src/adapters/plugin/`, denn Plugins dürfen **keine** `src/adapters/`-Header
inkludieren, auch nicht die des Hosts
([ADR-0017](../../adr/0017-plugin-api-abi.md) §Entscheidung 3); der Host und
die Plugins inkludieren die API beidseitig. `src/{plugin_api}/` unterliegt
derselben Import-Grenze wie `plugins/` (Regel P2: nur `plugin_api`/`model`/
`ports/driving`).

## 2. Definition of Done

- [x] **Plugin-API-Header-Satz `src/{plugin_api}/`** (header-only): (a)
      `plugin_abi.h` — `BCAD_PLUGIN_ABI_VERSION` (Start: 1) + die drei
      `extern "C"`-Eintrittspunkte als deklarierte Signaturen
      (ABI-Versions-Abfrage, Factory, Destroy) mit festen Symbolnamen; (b)
      `plugin.h` — pure-virtual `Plugin`-Interface (Metadaten `name()` +
      Lifecycle-Hooks `onLoad(PluginContext&)` / `onUnload()`); (c)
      `plugin_context.h` — `PluginContext` vermittelt **genau**
      `EditStructurePort&` + `EvaluatePort&` (Port-Subset v1, Entscheidung 4)
      und ist **invalidierbar** (nach Invalidierung wirft jeder Zugriff —
      beobachtbare Vertragsverletzung statt UB). Includes nur
      `hexagon/model/` + `hexagon/ports/driving/`; **kein** Qt/OCC/SQLite/
      `dlfcn.h`, kein `adapters/`.
- [x] **Plugin-Host `src/adapters/plugin/plugin_host.{h,cpp}`** (Driving
      Adapter): `dlopen`/`dlsym`/`dlclose` **nur hier** (P1-Monopol);
      Handshake **vor** jedem C++-Kontakt (exakte Versions-Gleichheit;
      Mismatch / fehlendes Symbol / nicht ladbare Datei → Ablehnung **ohne**
      Factory-/Init-Aufruf); Lifecycle-Zustandsfolge Entdeckt → Geladen →
      Handshake → Initialisiert → Aktiv → Beendet → Entladen; **jeder**
      Host→Plugin-Übergang exception-gesichert (Fehler-Barriere, Muster
      [ADR-0008](../../adr/0008-aenderungs-benachrichtigung.md) „werfender
      Beobachter"); Meldungs-/Ergebnis-Konvention
      [`E-PLG-001`](../../../../spec/spezifikation.md#4-fehler-codes-und-logging-felder)
      mit `event=plugin_rejected` (Laden/Handshake) bzw. `event=plugin_error`
      (Laufzeit/Shutdown) im Meldungstext (Muster `io_atomic_write`); die
      Ablehnungs-Meldung bei Versions-Mismatch **nennt erwartete und
      vorgefundene Version** (Lastenheft-AK PLG-002); Fehlerpfad isoliert
      **ohne** `dlclose` (Entscheidung 3), Happy-Pfad Shutdown → Invalidierung
      → `dlclose`. Kein Qt, kein OCC, kein SQLite im Host.
- [x] **Beispiel-Plugin `plugins/example/`** (CMake-`MODULE`-Target, zugleich
      AK-Fixture): führt in `onLoad` über den Kontext eine **echte
      Edit-Mutation** aus (inkl. eines bewusst außerhalb des Wertebereichs
      liegenden Parameters → Klemm-Beleg: dieselben Prüf-/Klemm-Regeln wie
      manuelle Eingaben, Lastenheft-AK PLG-004) und hält **keine**
      Port-Referenz über `onUnload` hinaus (Vertragspflicht, Testbeleg).
      **Test-Fixtures `plugins/testing/`:** werfendes Plugin (wirft in
      `onLoad` **vor** jeder Mutation), ABI-Mismatch-Plugin (exportiert
      fremde Version; seine Factory ruft `abort()` — würde der Host sie
      fälschlich rufen, scheitert der Test **laut**), symbol-loses Plugin
      (exportiert keine Eintrittspunkte). **Kein Plugin linkt
      `bcad_hexagon`** (nur `bcad_plugin_api`-Include-Target; statisches
      Kern-Dazulinken verboten).
- [x] **CMake/Symbol-Naht:** `ENABLE_EXPORTS ON` für `b-cad` und
      `bcad_adapter_tests` (Entscheidung 1); Plugin-`MODULE`-Targets mit
      definierter Output-Location, den Tests als Compile-Definition
      übergeben; **kein** neuer `find_package`-/apt-Eintrag (`dlopen` =
      glibc; [ADR-0004](../../adr/0004-toolchain-dependency-pinning.md)-Beleg
      = `make build` grün ohne Dockerfile-Änderung).
- [x] **Composition Root `src/main.cpp`:** `--plugin <pfad>` (wiederholbar)
      lädt beim Start über den Host (Kontext = `StructureEditService`),
      druckt Annahme-/Ablehnungs-Meldung, entlädt beim Beenden. **Benannte
      Lücke:** keine GUI-Plugin-Verwaltung (Laden/Entladen aus der laufenden
      Oberfläche) — die Laufzeit-Fähigkeit belegt der AK-Test durch den
      echten Host im laufenden Prozess; die Oberflächen-Anbindung gehört zum
      UI-Strang der welle-5 (§6).
- [x] **AK-Tests `tests/adapters/test_plugin_host.cpp`** (reale `.so` durch
      den echten Host; [ADR-0017](../../adr/0017-plugin-api-abi.md)
      §Fitness): **Happy** Load→Edit→Unload — Handshake ok, `onLoad` mutiert
      via Kontext (geklemmt, Klemm-Beleg über Port-Abfrage), Unload → Plugin
      hat **keine weitere Wirkung** (Modell-Sonde vorher/nachher; zweiter
      Load/Unload-Zyklus funktioniert); **werfendes Plugin** →
      [`E-PLG-001`](../../../../spec/spezifikation.md#4-fehler-codes-und-logging-felder)/`plugin_error`-Meldung,
      Plugin isoliert, **Modell unverändert** (Vorher/Nachher-Sonde über
      Driving-Port-Abfragen), Host lebt weiter (Folge-Operationen
      funktionieren) — die im Plugin geworfene Exception wird **im Host
      gefangen** (Symbol-Naht-Beleg, Entscheidung 1); **ABI-Mismatch** →
      Ablehnung **ohne** Factory-Aufruf (`abort()`-Fixture), Meldung nennt
      erwartete + vorgefundene Version, `event=plugin_rejected`; **keine
      Plugin-Datei / defekte Datei** → Ablehnung ohne Absturz, Modell
      unverändert; **fehlendes Symbol** → Ablehnung. Kontext-Invalidierung:
      Zugriff nach Unload wirft beobachtbar (kein UB-Pfad im Test).
- [x] **arch-check Regel P** (`tools/arch-check.sh`): **P1** `dlfcn.h`-Include
      und `dlopen`/`dlsym`/`dlclose`-Aufrufe nur in `src/adapters/plugin/` —
      geprüft über `src/` **und** `plugins/`; **P2** Dateien unter `plugins/`
      und `src/{plugin_api}/` inkludieren nur `plugin_api/` +
      `hexagon/model/` + `hexagon/ports/driving/` — kein `adapters/`, kein
      Qt/OCC(`.hxx`)/SQLite, kein `dlfcn.h`. Zusätzlich **Regel-E-Kommentar-
      Nachzug**: der stale Vorbehalt „Plugin-Ausnahme erst mit LH-FA-PLG-*"
      wird ersetzt (es gibt **keine** Regel-E-Ausnahme — kein Qt im
      Plugin-Host; [ADR-0009](../../adr/0009-gui-framework-qt6.md)-Prüfpunkt
      beantwortet).
- [x] **Gate-Scope-Entscheidung umgesetzt (Entscheidung 2):** lint-Stage
      erfasst `plugins/` (`find src plugins -name '*.cpp'`,
      `.devcontainer/Dockerfile`); Coverage-Filter bleibt `src/`
      (Begründung dokumentiert, keine Schwellen-Änderung, kein
      [AGENTS.md §2.6](../../../../AGENTS.md)-Fall).
- [x] **Gate-Doku-Nachzug (Review-MED-1, Präzedenz Regel-E/slice-011b):**
      [`AGENTS.md`](../../../../AGENTS.md) §3 und
      [`harness/README.md`](../../../../harness/README.md) §Sensors —
      lint-Vertragszeile auf „0 Befunde in `src/` + `plugins/`", arch-check-
      Vertragszeile um **Regel P** (dlfcn-Monopol / Plugin-Import-Grenze)
      ergänzt; arch-check-Erfolgsmeldung nennt Regel P mit.
- [x] **`tools/idlink.py` um `PLG` angleichen (Review-LOW-2):** der
      Link-Generator kennt nur `E-(IO|VAL|GEO)` — seit 026a validiert das
      Gate aber `E-(IO|VAL|GEO|PLG)` (Generator ≠ Gate, vorgefundene Drift).
      1-Zeilen-Angleich; die neuen
      [`E-PLG-001`](../../../../spec/spezifikation.md#4-fehler-codes-und-logging-felder)-Nennungen
      im §1-Nachzug werden zusätzlich von Hand verlinkt (nicht auf
      `--apply` verlassen).
- [x] **Spec-Nachzug (klein, [MR-008](../../../../harness/conventions.md#mr-008--lastenheft-schärfung-bleibt-lösungsfrei)-zulässig
      im Impl-Slice, Muster 019b/020b):** `spezifikation.md` §1
      [`LH-FA-PLG-001.a`](../../../../spec/lastenheft.md#lh-fa-plg-001) —
      die beiden dort ausdrücklich an diesen Slice delegierten Punkte
      nachgetragen: Port-Subset v1 (`EditStructurePort` + `EvaluatePort`)
      und Unload-Strategie im Fehlerpfad (Isolieren ohne Entladen). +
      `spezifikation-historie.md` + `**Letzte Änderung:**`-Header. **Kein**
      Lastenheft-Touch, kein neuer ADR, kein Schema.
- [x] `make gates` grün (inkl. neuer Regel P und lint über `plugins/`);
      `make schema-check`/`make io-smoke` unberührt; **unabhängiges
      Code-Review vor Closure** (Muster welle-4-Impl-Slices; **vor**
      schreibendem Review committen oder read-only erzwingen — Lehre 024b);
      [MR-009](../../../../harness/conventions.md#mr-009--geometrielastiges-code-review-vor-welle-closure)
      **n/a** (keine neue Solid-/Bauteil-Geometrie — Feststellung im
      Closure); ADR-Index: beide offene
      [ADR-0017](../../adr/0017-plugin-api-abi.md)-Folgepflicht-Zeilen (Impl,
      Regel P) abgehakt; CHANGELOG; Closure-Notiz mit Lerneintrag.

## 3. Plan (vor Code)

| Datei / Komponente | Änderungs-Art | Begründung |
|---|---|---|
| `src/plugin_api/{plugin_abi,plugin,plugin_context}.h` | neu | API-Header-Satz (header-only): ABI-Version + `extern "C"`-Signaturen, `Plugin`-Interface, `PluginContext` (Port-Subset v1, invalidierbar) |
| `src/adapters/plugin/plugin_host.{h,cpp}` | neu | Driving Adapter: dlfcn-Monopol, Handshake fail-closed, Lifecycle, Fehler-Barriere, [`E-PLG-001`](../../../../spec/spezifikation.md#4-fehler-codes-und-logging-felder)-Meldungen |
| `plugins/example/example_plugin.cpp` + `plugins/CMakeLists.txt` | neu | Beispiel-Plugin (MODULE, AK-Fixture: Edit + Klemm-Beleg) |
| `plugins/testing/{throwing,abi_mismatch,no_symbols}_plugin.cpp` | neu | Test-Fixtures (werfend / fremde ABI-Version + `abort()`-Factory / symbol-los) |
| `CMakeLists.txt` · `src/CMakeLists.txt` · `src/adapters/CMakeLists.txt` | ändern | `add_subdirectory(plugins)`, `bcad_plugin_api`-INTERFACE-Target, Host-Quellen, `ENABLE_EXPORTS` für `b-cad` |
| `tests/CMakeLists.txt` · `tests/adapters/test_plugin_host.cpp` | ändern/neu | AK-Tests (reale `.so`), `ENABLE_EXPORTS` für `bcad_adapter_tests`, Plugin-Pfad als Compile-Definition |
| `src/main.cpp` | ändern | `--plugin <pfad>`-Verdrahtung (Laden beim Start, Meldung, Entladen beim Beenden) |
| `tools/arch-check.sh` | ändern | Regel P1/P2 + Regel-E-Kommentar-Nachzug + Erfolgsmeldung |
| `.devcontainer/Dockerfile` | ändern | lint-Stage: `find src plugins -name '*.cpp'` (Entscheidung 2) |
| `AGENTS.md` + `harness/README.md` | ändern | Gate-Doku-Nachzug: lint-Vertrag `src/`+`plugins/`, arch-check-Vertrag + Regel P (Review-MED-1) |
| `tools/idlink.py` | ändern | E-Familie um `PLG` angleichen — Generator == Gate (Review-LOW-2) |
| `spec/spezifikation.md` + `spec/spezifikation-historie.md` | ändern | §1-Nachtrag Port-Subset v1 + Unload-Strategie (delegierte Punkte); Header |
| `docs/plan/adr/README.md` | ändern (Closure) | beide [ADR-0017](../../adr/0017-plugin-api-abi.md)-Folgepflicht-Zeilen (Impl, Regel P) abhaken |
| `CHANGELOG.md` | ändern (Closure) | Unreleased-Eintrag slice-026b |
| `docs/reviews/{2026-07-03-slice-026b-plan,2026-07-03-slice-026b-code-review}.md` | neu | [MR-006](../../../../harness/conventions.md#mr-006--unabhängiges-plan-review-vor-implementierungs-start)-Report + Code-Review-Report |

## 4. Trigger

- Startbar nach [MR-006](../../../../harness/conventions.md#mr-006--unabhängiges-plan-review-vor-implementierungs-start)-Review:
  slice-026a ist done (AK + §1-Mapping entschieden),
  [ADR-0017](../../adr/0017-plugin-api-abi.md) accepted; die zu vermittelnden
  Driving-Ports sind real (`StructureEditService` implementiert
  `EditStructurePort` + `EvaluatePort`).

## 5. Closure-Trigger

- DoD vollständig, `make gates` grün (inkl. Regel P), unabhängiges
  Code-Review **0 HIGH**, Closure-Notiz mit Lerneintrag → **M5-Trigger-Pfad
  frei** ([OBJ-004](../../../../spec/lastenheft.md#3-projektziele)
  „Erweiterung durch Plugins" ist mit lauffähigem Plugin-System erfüllbar;
  ob M5 damit gebucht wird, entscheidet der Projektinhaber bei der
  Welle-Closure — DRW/UI/Mehrsprachigkeit sind Wellen-Inhalt ohne
  Meilenstein-Bindung).

## 6. Risiken und offene Punkte

- **Symbol-Naht-Restrisiko (Entscheidung 1):** `ENABLE_EXPORTS` exportiert
  die dynamische Symboltabelle des Executables (größere Symboltabelle,
  keine Laufzeit-Kosten im relevanten Maß). Die weak-typeinfo-Unifikation
  über die Modul-Grenze ist der kritische Punkt — sie wird **empirisch
  belegt** (Exception aus dem Plugin im Host gefangen, AK-Test), nicht nur
  behauptet. Schlägt der Beleg fehl (Toolchain-Eigenheit), ist der
  dokumentierte Fallback `bcad_hexagon` als Shared Library — die Wahl ist
  im Closure festzuhalten ([ADR-0017](../../adr/0017-plugin-api-abi.md)
  §Entscheidung 2: „mit Beleg").
- **`dlclose`-Restrisiken:** statische Destruktoren/TLS im Plugin; nach
  Unload noch referenzierte typeinfo-/Exception-Objekte. Fehlerpfad
  isoliert deshalb **ohne** `dlclose` (Entscheidung 3, ADR-gedeckt);
  Happy-Pfad-`dlclose` wird durch den Doppel-Zyklus-Test (Load→Unload→
  Load→Unload) geübt. Exceptions werden **im Barriere-Rahmen** gefangen und
  nicht über den `dlclose`-Zeitpunkt hinaus gehalten.
- **Fixture-`abort()` (ABI-Mismatch-Beleg):** die Factory des
  Mismatch-Fixtures ruft `abort()` — ruft der Host sie fälschlich (Bug),
  stirbt der Test **laut** statt still zu bestehen. Bewusst gewählte
  Fail-Loud-Sonde für „Ablehnung **ohne** Initialisierung".
- **GUI-Lücke (benannte Lücke, kein stilles Unter-Liefern):** Laden/Entladen
  „ohne Neustart" ist im Host-Vertrag real und wird im **laufenden Prozess**
  der AK-Tests geübt; die App bietet v1 nur `--plugin` beim Start. Die
  Oberflächen-Verwaltung (Laden/Entladen aus der laufenden GUI) gehört zum
  UI-Strang der welle-5 — im Closure als Lücke festhalten, damit die
  Welle-Verifikation sie gegen
  [OBJ-004](../../../../spec/lastenheft.md#3-projektziele) bewusst bewertet.
- **`--plugin`-CLI-Glue ohne behavioralen Sensor (benannte Sensor-Lücke,
  Review-LOW-1):** `main.cpp` ist coverage-ausgenommen; die IO-CLI-Pfade
  belegt `make io-smoke` — für `--plugin` entsteht v1 **kein** Smoke
  (bewusst: `io-smoke` trägt die LH-FA-IO-Bindung, ein Plugin-Smoke würde
  seinen Vertrag dehnen). Die Host-Logik selbst ist voll AK-getestet; die
  dünne CLI-Verdrahtung (Arg-Parse → Host-Aufruf → Meldung) bleibt manuell
  belegt. **Folge-Kandidat:** eigener Plugin-Smoke (Muster `io-smoke`,
  LH-Bindung LH-FA-PLG-*), wenn die CLI-/GUI-Fläche des Plugin-Systems
  wächst — im Closure festhalten.
- **Coverage-Wirkung des neuen Host-Codes (Review-LOW-3):** `plugin_host.cpp`
  liegt voll im 70-%-Aggregat (`src/`-Filter) und trägt viele Fehler-/
  Isolier-Zweige — die AK-Matrix (Happy/werfend/Mismatch/defekt/symbollos/
  Doppel-Zyklus/Kontext-Invalidierung) deckt genau diese Zweige; keine
  Schwellen-Gefahr erwartet, aber nach dem ersten Coverage-Lauf prüfen.
  **gcov × MODULE:** im Coverage-Build sind auch die Plugin-`.so`
  instrumentiert; `dlclose` einer instrumentierten `.so` kann
  gcov-Randverhalten zeigen (`.gcda`-Flush) — für den `src/`-Filter
  folgenlos (plugins/ nicht gemessen), als Randinteraktion benannt.
- **Kein Undo:** [LH-QA-003](../../../../spec/lastenheft.md#lh-qa-003--undoredo)
  ist **nicht implementiert** ([ADR-0017](../../adr/0017-plugin-api-abi.md)-Review-HIGH-1-Lehre:
  nicht als Ist behaupten) — der Load→Edit→Unload-Test prüft Validierung/
  Klemmung, **keine** Undo-Erfassung.
- **Threading:** Tests und CLI laufen single-threaded (Hooks synchron im
  Hauptthread, §1) — es entsteht **kein** Thread-Test; die Vertragspflicht
  „Plugin-Threads rufen keine Ports" ist technisch nicht erzwingbar
  (benannte Grenze, §1/[ADR-0017](../../adr/0017-plugin-api-abi.md)).
- **Erstmals Repo-Code außerhalb `src/`:** `plugins/` muss in die
  Docker-Build-Kontexte gelangen (COPY `.` deckt es; `.dockerignore`
  prüfen) und wird von Regel P + lint erfasst (Entscheidung 2). Die
  Coverage-Ausnahme für `plugins/` ist **keine** Gate-Lockerung (der
  bisherige Scope `src/` bleibt unverändert; nichts wird aus ihm entfernt).
- **Meldungs- statt Logging-Infrastruktur:** b-cad hat noch kein
  strukturiertes Logging/OTel (§5-Span `bcad.plugin.lifecycle` bleibt
  Spezifikations-Stand; die Span-Emission folgt mit der
  Observability-Anbindung, benannter Punkt seit dem Bootstrap). Der Host
  transportiert `event=…` im Meldungstext (Muster `io_atomic_write`) — die
  AK-Tests prüfen die Meldung, nicht einen Span.

## 7. Sub-Area-Modus-Begründung

### Sub-Area: Plugin-Host

- **Modus:** GF (deklariert in
  [`harness/conventions.md`](../../../../harness/conventions.md)
  §Modus-Deklaration: `src/adapters/plugin/`, `plugins/` — Plugin-API-/
  Lifecycle-/Sandbox-Konvention · ABI-/API-Vertrags-Inventur · struktureller
  Cluster, 3/3). **Konventionen-Dichte:** hoch
  ([ADR-0017](../../adr/0017-plugin-api-abi.md): Handshake/Lifecycle/
  Barriere/Regel P). **Phase-Reife:** erster Code der Sub-Area.
  **Evidenz-Risiko:** mittel (Symbol-Naht empirisch zu belegen).
  **Reconciliation:** keiner.

### Sub-Area: Build & Toolchain / Test-Infrastruktur

- **Modus:** GF; Änderungen additiv (INTERFACE-Target, MODULE-Targets,
  `ENABLE_EXPORTS`, lint-Scope) — keine neue Dependency
  ([ADR-0004](../../adr/0004-toolchain-dependency-pinning.md)-invariant),
  Tests folgen der GoogleTest-/Fixture-Konvention (reale Artefakte statt
  Stubs, Muster IFC/DXF-Integrationstests).

## 8. Closure-Notiz

**Closure-Kriterien (beobachtbar, 2026-07-03):**

- **Plugin-System lauffähig, alle DoD-Zeilen erfüllt:** Plugin-Host
  (`src/adapters/plugin/`, dlfcn-Monopol; Handshake exakt/fail-closed **vor**
  jedem C++-Kontakt; 7-Stufen-Lifecycle; Fehler-Barriere je Übergang mit
  [`E-PLG-001`](../../../../spec/spezifikation.md#4-fehler-codes-und-logging-felder)-Meldungen
  `plugin_rejected`/`plugin_error`) + Plugin-API `src/plugin_api/` (header-only,
  ABI v1, invalidierbarer Kontext) + Beispiel-Plugin und **vier** Test-Fixtures
  (MODULE ohne Kern-Linkage) + `--plugin`-CLI + **8 AK-Tests mit realer `.so`
  durch den echten Host** + arch-check-Regel P + Gate-Doku-Nachzug. `make gates`
  grün (228/228, Coverage 90,3 % — der Host liegt voll im Scope, die
  LOW-3-Schwellen-Sorge des Plan-Reviews blieb unbegründet); `make
  schema-check`/`io-smoke` unberührt; **keine neue Dependency**.
- **Benannte Entscheidungen getroffen und belegt:** (1) Symbol-Naht =
  **`ENABLE_EXPORTS`** (Kern bleibt statisch) — empirisch belegt: die im
  Plugin geworfene `std::runtime_error` wird **im Host gefangen** und ihr Text
  transportiert (typeinfo-Unifikation über die Modul-Grenze; der dokumentierte
  Fallback Shared-Kern blieb unnötig); (2) Gate-Scope = lint ja / Coverage
  nein; (3) Fehlerpfad = Isolieren **ohne** Entladen; (4) Port-Subset v1 =
  `EditStructurePort` + `EvaluatePort` — (1)–(4) in `spezifikation.md` §1
  fixiert (die zwei von 026a delegierten Platzhalter geschlossen).
- **Zwei Reviews:** [MR-006](../../../../harness/conventions.md#mr-006--unabhängiges-plan-review-vor-implementierungs-start)
  0 HIGH (1 MED + 3 LOW + 2 INFO eingearbeitet) + unabhängiges **Code-Review
  0 HIGH** auf committetem Stand (3 MED + 3 LOW: MED-1 Shutdown-Wurf-Fixture,
  MED-2 create-Null-Event-Klassifikation, MED-3 P2-Angle-Include-Härtung,
  LOW-1 `dlmopen`, LOW-2 `--plugin`-ohne-Pfad-Meldung — **alle vor Closure
  behoben**; LOW-3-Orakel-Härtungen als Kandidaten unten).
  [MR-009](../../../../harness/conventions.md#mr-009--geometrielastiges-code-review-vor-welle-closure)
  **n/a** (keine neue Solid-/Bauteil-Geometrie — bestätigt).
- **ADR-Index:** beide [ADR-0017](../../adr/0017-plugin-api-abi.md)-Folgepflichten
  (Impl, Regel P) abgehakt — **alle drei
  [ADR-0017](../../adr/0017-plugin-api-abi.md)-Folgepflichten erfüllt**.

**Lerneintrag:**

- **`bugprone-empty-catch` als Feedforward-Beleg:** der erste lint-Lauf über
  den Host fing zwei leere Destroy-Barrieren — behoben durch **Behandlung**
  (Destroy-Fehlschlag fließt in die E-PLG-Meldung) statt Suppression
  ([AGENTS.md §2.4](../../../../AGENTS.md)). Der aktivierte `bugprone-*`-Satz
  (inkl. `exception-escape` für die Destruktor-Pfade) trägt genau die
  Fehlerklassen des Plugin-Hosts.
- **Review-Kaskade wirkt versetzt:** das Plan-Review sah die Gate-Doku-Lücke
  (MED-1), das Code-Review die Test-Lücke auf dem Shutdown-Pfad (MED-1) und
  die P2-Angle-Lücke (MED-3) — drei Fund-Klassen, die je nur in ihrer Phase
  sichtbar waren; keine Selbst-Bestätigung.
- **Benannte Lücke GUI-Plugin-Verwaltung:** Laden/Entladen zur Laufzeit ist im
  Host-Vertrag real (AK-Tests üben es im laufenden Prozess); die App bietet v1
  `--plugin` beim Start. Oberflächen-Verwaltung → UI-Strang; die
  Welle-Verifikation bewertet die Lücke bewusst gegen
  [OBJ-004](../../../../spec/lastenheft.md#3-projektziele).
- **Benannte Sensor-Lücke `--plugin`-CLI-Glue** (Plan-Review-LOW-1): kein
  Smoke für die dünne main.cpp-Verdrahtung (io-smoke-Vertrag bleibt IO-rein).

**Steering-Kandidaten (Projektinhaber-Anstoß, 2026-07-03):**

- **lint-Härtung als eigener Quergewerk-Slice** (Muster 018/022, kein ADR —
  Verschärfung, kein [AGENTS.md §2.6](../../../../AGENTS.md)-Fall): heutiger
  Satz = `bugprone-*` + `clang-analyzer-*` + 1× readability. Kandidaten-
  Familien laut Projektinhaber: **`cert-*`, `readability-*` (kuratiert),
  `performance-*`, `modernize-*`, `misc-*`, `cppcoreguidelines-*`,
  `portability-*`** — Vorgehen evidence-first: Dry-Run-Bestandslauf je
  Familie über `src/` + `plugins/`, Befundzahlen sichten, kuratierte
  Aktivierungsliste (0-Befunde-Latte muss halten), Befunde fixen, zentrale
  Ausnahmen nur mit Slice-Bezug (bekannter Kollisionspunkt:
  `cppcoreguidelines-pro-type-reinterpret-cast` vs. dlsym-Idiom;
  `readability-magic-numbers` vs. Test-Literale), Gate-Doku-Nachzug.
- **Plugin-Smoke** (Muster io-smoke, LH-Bindung LH-FA-PLG-*), wenn die
  CLI-/GUI-Fläche wächst.
- **Test-Orakel-Härtung** (Code-Review-LOW-3): Re-Load desselben Plugins nach
  Isolation, `activeCount` nach `unloadAll`/Destruktor, strengere
  Happy-Meldungs-Sonde.

**Restrisiko / Nachfolge:** welle-5-Schwester-Stränge (DRW, UI-Themes/Docking,
Mehrsprachigkeit) offen; **M5-Buchung** = Projektinhaber-Entscheidung bei der
Welle-Closure (der [OBJ-004](../../../../spec/lastenheft.md#3-projektziele)-Pfad
ist mit dem lauffähigen Plugin-System frei). „Isolieren ohne `dlclose`" ist
verhaltensseitig nicht getestet (bewusst, UB-Vermeidungs-Präferenz —
Code-Review-INFO-1).
