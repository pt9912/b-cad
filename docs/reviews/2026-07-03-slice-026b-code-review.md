# Unabhängiges Code-Review — slice-026b (Plugin-System-Implementierung, Commit `8d446a9`)

**Datum:** 2026-07-03
**Artefakt:** Commit `8d446a9` (Plugin-Host, Plugin-API, Beispiel-/Test-Plugins, Regel P, CLI, Gate-Scope, Spec-Nachzug)
**Reviewer:** unabhängig (Reviewer ≠ Autor, ohne Entstehungs-Kontext, read-only auf committetem Stand — Lehre 024b)
**Linsen:** (a) Vertrags-Korrektheit Host vs. ADR-0017/§1, (b) Test-Orakel-Qualität, (c) Regel P, (d) CMake/Symbol-Naht, (e) CLI/main.cpp, (f) Spec-Treue

## Verdikt

**0 HIGH — Closure-tauglich.** 3 MED + 3 LOW + 2 INFO. Die Host-Vertrags-
Korrektheit (Handshake-Ordnung, vollständige Fehler-Barriere inkl. `name()`,
Isolation ohne Entladen, Speicher-Sicherheit in allen Pfaden) und die
benutzer-beobachtbaren AK-Verträge (Modell unverändert, Event-Namen,
Reject-ohne-Init via abort-Sonde, Symbol-Naht-Beleg) sind belegt und getestet.
**Alle drei MED und zwei LOW wurden vor der Closure behoben** (Disposition unten).

## Verifikations-Log (bestätigte tragende Eigenschaften)

- Handshake **vor** jedem C++-Kontakt (version_fn vor create/name/onLoad; Mismatch kehrt ohne C++-Berührung zurück). ✓
- Fehler-Barriere **vollständig**: version_fn, create_fn, `name()`, onLoad, onUnload, destroy — kein ungesicherter Host→Plugin-Übergang. ✓
- Isolation korrekt: Kontext-Invalidierung, barriere-gesicherte Instanz-Freigabe, Handle im Fehlerpfad bewusst offen; `dlclose` NUR im regulären Weg; keine Leaks (außer bewusstem Handle-Leak)/Double-Free/Use-after-free. ✓
- Reguläre Reihenfolge onUnload → invalidate → destroy → dlclose ADR-konform (destroy vor dlclose — Instanz-Code liegt in der `.so`). ✓
- `RTLD_NOW|RTLD_LOCAL`, dlerror-Handling korrekt. ✓
- abort()-Sonde wasserdicht (create UND destroy des Mismatch-Fixtures aborten; bei korrektem Host wird keines gerufen). ✓
- Event-Zuordnung §4-treu (plugin_rejected ↔ Load/Handshake, plugin_error ↔ Laufzeit; Mismatch-Meldung nennt erwartet 1 / vorgefunden 1000). ✓ (eine Inkonsistenz → MED-2, behoben)
- Regel P1: fängt dlopen/dlsym/dlclose + dlfcn.h in `src/`+`plugins/` außer Host; 0 False-Positives im Bestand. ✓
- CMake/Symbol-Naht: ENABLE_EXPORTS auf **beiden** Ladern; Plugins linken nur das INTERFACE-Target (keine Kern-Linkage, auch nicht transitiv); Test-Pfad == Output-Dir. ✓
- main.cpp-Lebensdauern: service vor plugin_host deklariert → Host-Destruktor (unloadAll) läuft vor Service-Zerstörung. ✓
- Spec-Treue: §1-Nachzug (Port-Subset v1, Unload-Strategie) deckungsgleich mit dem Code; Klemm-Beleg 20→50 echt (Klemmgrenze [50,1000]); ThrowingPlugin wirft vor jeder Mutation. ✓

## Findings + Disposition

### MED-1 — Shutdown-Fehler-Zweig (werfender onUnload) ohne Test
**Fund:** Keine Fixture wirft im Shutdown-Hook — der `shutdownAndClose`-Fehler-Zweig (eigene try/catch-Stelle; §1 zählt „Shutdown-Fehler" als normativen Fehlerpfad) war unerprobt; eine Regression dort liefe im PluginHost-Destruktor in `std::terminate`, ungefangen.
**Disposition: behoben** — neue Fixture `plugins/testing/unload_throwing_plugin.cpp` + Test `UnloadHookThrowIsolatesPlugin` (isoliert, `event=plugin_error`, „Shutdown-Hook warf", Modell unverändert, Host lebt weiter).

### MED-2 — `create_fn()==nullptr` als plugin_rejected klassifiziert (Init-Fehler)
**Fund:** Der Null-Rückgabe-Fall des Init-Schritts nutzte `plugin_rejected`, die Schwester-Fehler desselben Schritts (create/onLoad werfen) `plugin_error` — inkonsistent zur §4-Phasen-Zuordnung (rejected ↔ Laden/Handshake).
**Disposition: behoben** — `errorMessage` (plugin_error) + Kontext-Invalidierung vor Rückkehr.

### MED-3 — Regel P2 prüfte nur Quote-Includes (Angle-Include-Lücke)
**Fund:** `#include <adapters/…>` hätte die P2-Allowlist umgangen (src/ liegt für Plugins auf dem Include-Pfad); Repo nutzt zwar zu 100 % Quote-Form, aber der P2-Kommentar versprach mehr als die Regex hielt.
**Disposition: behoben** — P2c: Projekt-Präfixe (`adapters/`, `hexagon/`, `plugin_api/`) als Angle-Include in `plugins/`/`src/plugin_api/` sind jetzt verboten (Projekt-Header nur in Quote-Form, die die Allowlist prüft).

### LOW-1 — P1 fing `dlmopen(` nicht
**Disposition: behoben** — Alternation `dl(m?open|sym|close)`.

### LOW-2 — `--plugin` als letztes Argument wurde schweigend verschluckt
**Disposition: behoben** — sichtbarer Hinweis „--plugin ohne Pfad-Argument ignoriert" auf stderr.

### LOW-3 — Kleinere Orakel-Lücken (Happy-Meldung nur Teilstring; Re-Load desselben Plugins nach Isolation; activeCount nach unloadAll/Destruktor)
**Disposition: nicht umgesetzt (Closure-Notiz)** — keine DoD-Vorgabe; als Härtungs-Kandidaten für den Folge-Ausbau festgehalten.

### INFO-1 — „Isolieren ohne dlclose" verhaltensseitig kaum prüfbar
**Disposition: festgehalten** — UB-Vermeidungs-Präferenz, kein benutzer-beobachtbarer Vertrag; kein Test bewacht sie (bewusst).

### INFO-2 — Kontext-Invalidierung beim create-Null-Fall
**Disposition: mit MED-2 erledigt.**
