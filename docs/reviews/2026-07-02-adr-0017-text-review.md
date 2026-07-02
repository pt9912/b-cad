# Unabhängiges Text-Review — ADR-0017 (Plugin-API-/ABI-Vertrag und Sandbox-Modell)

**Datum:** 2026-07-02
**Artefakt:** [`docs/plan/adr/0017-plugin-api-abi.md`](../plan/adr/0017-plugin-api-abi.md) (Status bei Review: Proposed) + Index-Zeile/Bullet in [`docs/plan/adr/README.md`](../plan/adr/README.md)
**Reviewer:** unabhängig (Reviewer ≠ Autor, ohne Entstehungs-Kontext — MR-006-Linse auf eine ADR, Muster ADR-0013/0014/0015/0016 vor Accept)
**Modus:** read-only (Lehre slice-024b: schreibender Review-Agent nur nach Commit — hier bewusst read-only erzwungen); adversariale Verifikation jeder tragenden Behauptung gegen die realen Artefakte (Lastenheft/Spezifikation/architecture/conventions/ADRs/echter Code/Container-Toolchain).

## Verdikt

**1 HIGH, 4 MED, 2 LOW, 3 INFO — nach Einarbeitung accept-fähig.** Die
Grundentscheidung (dlopen + versionierter `extern "C"`-Handshake +
C++-Port-Facade in-process; Plugin-Host als Driving Adapter; Sandbox =
Port-Vermittlung + Fehler-Barriere; Regel P; Optionen-Vergleich H/C/Q/P/S)
ist aus den kanonischen Quellen begründet; alle geprüften Zitate, IDs,
Pfade und Präzedenz-Behauptungen stimmen. Blockierend war allein HIGH-1
(Textkorrektur ohne Entscheidungsumkehr). Alle Findings wurden vor
Accept-Vorlage eingearbeitet (Disposition je Finding unten).

## Verifikations-Log (tragende Behauptungen — alle bestätigt)

- **architecture.md §2:** Plugin-Host-Zeile wörtlich „darf importieren: model, ports/driving", verboten „Driven Adapter direkt"; Plugin-Host als zweiter Driving Adapter im §1-Diagramm; `plugins/`-Baum in §2.1; §5-Fehlermodell „Plugin-Host isoliert; Modell unverändert (Sandbox)".
- **Driving-Ports real:** `edit_structure_port.h`, `evaluate_port.h`, `exchange_model_port.h` u. a. in `src/hexagon/ports/driving/`.
- **Spezifikation:** `E-PLG-001` in §4 exakt wie zitiert (isoliert/entladen, Modell unverändert, `event=plugin_error`); `E-VAL-001` (Klemmung) korrekt; §5-Span-Outline ausdrücklich „ADR-Folge"; REQ-TEC-008 = Shared Libraries, §9-Vorspann „fortschreibbar".
- **arch-check** (`tools/arch-check.sh`): Regeln A–E real und so beschriftet; C/D/E greppen nur `src/` → die ADR-Begründung, dass `plugins/` (erstmals Repo-Code außerhalb `src/`) eine neue Regel P2 braucht, trägt; Z. 16-Kommentar „Plugin-Ausnahme erst mit LH-FA-PLG-*" (→ INFO-1).
- **„Regel F" historisch belegt:** ADR-0015-Prägung („Regel F gegenstandslos"), ADR-0013-Folgepflicht umdatiert — „Buchstabe P statt F" ist begründet.
- **dlopen = glibc:** Ubuntu-26.04-Container (ADR-0004, Digest + apt-Snapshot), kein neuer apt-/`find_package`-Eintrag nötig; `src/adapters/plugin/` ist realer leerer Platzhalter (nur `.gitkeep`), kein `dlfcn` bisher im Repo (P1 startet sauber).
- **Toolchain-Pin:** g++-15 einziger Build-Compiler; clang-21 nur als clang-tidy-Frontend, per `--gcc-install-dir` auf die gcc-15-Toolchain genagelt (→ INFO-2 Präzisierung „ein **Build**-Compiler").
- **welle-5-Trigger & M5:** Roadmap nennt „welle-4 done + Plugin-API-/ABI-ADR" und „M5 — Erweiterbar / OBJ-004".
- **Index:** Zeile 0017 + Offene-Themen-Bullet konsistent zum ADR-Text.
- **MADR-Qualität:** Optionen H/C/Q/P/S fair verglichen (S sauber über LH-FA-PLG-001 + REQ-TEC-008 disqualifiziert); Gates ehrlich „real" vs. „Impl-Slice" markiert; Re-Eval-Trigger beobachtbar; nur Aufwärts-Referenzen; Lastenheft unberührt (MR-008); keine Kollision mit einer Accepted-ADR; Regel E respektiert statt gelockert.

## Findings (schärfstes zuerst) + Disposition

### HIGH-1 — „Denselben Undo-Stack" gibt es nicht: nicht existierender Mechanismus als Gegenwarts-Eigenschaft behauptet, Fitness-AK daran gebunden
**Fund:** Vier Stellen (Kontext, Option-H-Pro, Konsequenzen-Positiv, Fitness-AK „undo-fähig") behaupteten im Präsens, Plugin-Mutationen liefen durch „denselben Undo-Stack (LH-QA-003)" bzw. „erben … Undo gratis". Im gesamten Kern existiert **kein** Undo/Redo (kein Treffer in `src/hexagon/`, keine Undo-Operation an den Driving-Ports, nichts in `tests/`); real sind nur die Anforderung LH-QA-003 und das `undo_commands`-Schema (ADR-0006) — beides unimplementiert. Die Fitness-AK hätte den Impl-Slice an einen nicht existierenden Mechanismus gebunden (verdeckte Scope-Explosion oder unerfüllbare Testzeile) — die Klasse „behauptete Deckung ohne Substrat".
**Disposition: eingearbeitet.** Alle vier Stellen auf den ehrlichen Stand umformuliert („ein **künftiger** Undo-Mechanismus — heute nur das `undo_commands`-Schema aus ADR-0006 — erfasst Plugin-Mutationen automatisch mit, **weil** sie durch dieselben Ports laufen"); die Fitness-AK von Undo entkoppelt („Undo-Erfassung folgt erst mit dem LH-QA-003-Slice — keine Plugin-Impl-Vorbedingung").

### MED-1 — Sandbox-Ehrlichkeit unvollständig: Silent-Corruption-Pfad (UB ohne Crash → Speichern/Autosave persistiert korrupt) fehlte
**Fund:** Die „benannte Grenze" nannte nur den Crash-Pfad und bot als Netz die atomare Persistenz/Crash-Recovery an. Der zweite UB-Pfad fehlte: stille In-Memory-Korruption ohne Crash, die das nächste Speichern/Autosave (LH-QA-004) atomar **über** den letzten guten Stand schriebe — das Persistenz-Netz deckt konstruktionsbedingt nur den Crash-Pfad; die stärkste Lesart von LH-FA-PLG-004 ist in-process **prinzipiell** nicht einlösbar.
**Disposition: eingearbeitet.** Entscheidung #5 um den zweiten UB-Pfad ergänzt; Schärfungs-Slice angewiesen, die PLG-004-AK auf beobachtbares Fehlverhalten **wohlgeformter** Plugins (Port-Vermittlung, Fehler-Barriere, Isolierung) zu beziehen, nicht auf Speichersicherheit.

### MED-2 — Symbol-Auflösung Plugin ↔ Host war ungeklärt und als „CMake-Gestalt" unterschätzt
**Fund:** `bcad_hexagon` ist eine **statische** Library im Executable; kein `ENABLE_EXPORTS`/`-rdynamic` im Repo. Plugins brauchen zur Ladezeit Kern-Symbole (out-of-line-Methoden, vtables, typeinfo — Letzteres trägt die Fehler-Barriere über die Modul-Grenze). Drei strukturell verschiedene Lösungen: (a) Executable exportiert Symbole, (b) Kern wird Shared Library, (c) Plugin linkt den Kern statisch dazu — (c) erzeugt zwei Kern-Kopien/doppelte typeinfo (bricht Exception-Catch und die Eine-Wahrheit-Annahme). „Keine Kern-Naht" gilt nur auf Port-Ebene; auf Link-Ebene entsteht eine echte neue Naht.
**Disposition: eingearbeitet.** Entscheidung #2 um den Bullet „Symbol-Auflösung — die eine neue Link-Naht" ergänzt: (c) **verboten**; Wahl (a) vs. (b) = benannte Impl-Slice-Entscheidung mit Beleg (realer `.so`-Ladetest); Kontext/Option-H auf „Kern-**Port**-Naht" präzisiert.

### MED-3 — ADR-0008-Plugin-Prüfpunkt (fremder Code als Beobachter) wurde nicht bedient
**Fund:** ADR-0008 §Trigger verlangt für den Plugin-Fall, Kapselung/Re-Entranz-Verbot **technisch** durchzusetzen. Entscheidung #3 schließt Driven-Ports (und damit `ModelChangedPort`) aus — der Fall ist implizit ausgeschlossen, das ADR sprach es aber nicht aus; Aktivierungsmodell („Aktiv"-Zustand) blieb vage.
**Disposition: eingearbeitet.** Entscheidung #3 um „Kein Beobachter-Zugang in v1" ergänzt (durch Ausschluss beantwortet; pull-only, Hook = einziger Eintrittspunkt); neuer Re-Eval-Trigger „Plugin-Beobachter-Zugang" an der ADR-0008-Naht.

### MED-4 — Threading-Vertrag fehlte: Plugin-eigene Threads = zweiter unbenannter Sandbox-Kanal
**Fund:** Per ADR-0009 (d) läuft die Anwendung single-threaded am Qt-Event-Loop; der Kern ist nicht thread-sicher. Ein Plugin-Thread, der Ports ruft, erzeugte ein Data-Race → Modell-Korruption an der Fehler-Barriere vorbei.
**Disposition: eingearbeitet.** Entscheidung #5 um den Threading-Vertrag ergänzt: Hooks synchron im Hauptthread; Port-Aufrufe nur aus dem Hook-Kontext; Plugin-Threads dürfen keine Ports rufen (Vertragspflicht, benannte Grenze: in-process nicht erzwingbar).

### LOW-1 — „welle-3-Lehre slice-015b" ist eine welle-2-Lehre
**Fund:** slice-015b (Decken/Fundament) gehört zu welle-2 (MR-009-Zähler, Index-Datum 2026-06-14); der Etikettfehler stammt aus ADR-0013 (dort immutable).
**Disposition: eingearbeitet** („welle-2-Lehre slice-015b").

### LOW-2 — dlclose-Restrisiken über die Referenz-Disziplin hinaus unbenannt
**Fund:** Statische Destruktoren/TLS des Plugins, nach Unload referenzierte typeinfo-/Exception-Objekte; gängige Milderung: im Fehlerpfad isolieren **ohne** `dlclose` (kontrolliertes Leak statt UB).
**Disposition: eingearbeitet** (Konsequenzen-Risiko + explizite Impl-Slice-Pflicht zur Unload-Strategie im Fehlerpfad).

### INFO-1 — ADR-0009-Prüfpunkt „Plugin-Ausnahme der Regel E definieren" wird implizit beantwortet
**Disposition: eingearbeitet.** Konsequenzen: es entsteht **keine** Regel-E-Ausnahme; Impl-Slice zieht den stale werdenden Kommentar in `tools/arch-check.sh` nach.

### INFO-2 — „ein Compiler" → präziser „ein Build-Compiler"
**Disposition: eingearbeitet** (clang-tidy lint-only, auf dieselbe Toolchain genagelt).

### INFO-3 — ADR-0005 wird als Basis zitiert, ist selbst Proposed
**Disposition: keine Änderung** — identisches Zitat-Muster wie ADR-0013/0014/0015/0016 (präzedenz-konform), nur notiert.

## Gesamturteil

Nach Einarbeitung aller Findings: **accept-reif** (Entscheidung beim
Projektinhaber). Die gewählte Option blieb durch alle Findings unverändert;
die Einarbeitung schärfte Ehrlichkeit (HIGH-1, MED-1), Vollständigkeit
(MED-2/3/4) und Präzision (LOW/INFO).

## Nachtrag — Projektinhaber-Durchsicht (2026-07-02, zweite Runde vor Accept)

**Eigenständige Verifikation des Projektinhabers** (bestätigt): alle
Quellen-Zitate (LH-FA-PLG-001–004 Outline, E-PLG-001/E-VAL-001 wörtlich,
REQ-TEC-008, architecture.md-Plugin-Host-Zeile, existierende Driving-Ports,
leerer `src/adapters/plugin/`-Platzhalter, kein dlfcn/ENABLE_EXPORTS/-rdynamic
im Repo, `bcad_hexagon` statisch → Symbol-Naht-Analyse trägt); ADR-0009-(d)-
Threading-Prämisse, ADR-0008-Plugin-Trigger, „Regel F gegenstandslos"-Historie,
ADR-0004-Compiler-Pin, Roadmap-Trigger; alle 10 Dispositionen der ersten
Review-Runde an den benannten Stellen wiedergefunden — kein HIGH-1-Rückstand
(kein Undo-Code in `src/hexagon/`); kein Konflikt mit einer Accepted-ADR,
Lastenheft unberührt (MR-008), Regel E respektiert, Optionen-Vergleich fair.

**Eigene Findings (alle unterhalb MED) + Disposition:**

### LOW-A — Gate-Scope für `plugins/` bei lint/coverage unbenannt
**Fund:** Die ADR leitet aus „`plugins/` = erstmals Repo-Code außerhalb `src/`"
nur die arch-check-Regel P2 ab; `make lint` („0 Befunde in `src/`") und die
Coverage-Messung sind aber ebenfalls `src/`-verankert — ob das Beispiel-/
Test-Plugin gelintet/gemessen wird, war nirgends entschieden.
**Disposition: eingearbeitet** — neuer Konsequenzen-Bullet
„Gate-Scope-Entscheidung für `plugins/`": explizite Impl-Slice-Entscheidung
(Aufnahme in den Scope oder begründete Ausnahme), analog Symbol-Naht, keine
stille Setzung.

### LOW-B — P1-„Monopol" deckte `plugins/` selbst nicht
**Fund:** P1 verbot dlfcn nur „anderem Adapter, Kern, `src/main.cpp`" —
Repo-Code unter `plugins/` fiel weder unter P1 noch unter die P2-Blockliste
(dort fehlte `dlfcn.h`); ein Beispiel-Plugin hätte selbst `dlopen` rufen
können, ohne dass ein Gate anschlägt.
**Disposition: eingearbeitet** — P1 um den `plugins/`-Baum geschlossen
(inkl. Begründung: Selbst-dlopen unterliefe Lifecycle + Fehler-Barriere);
`dlfcn.h` in die P2-Verbotsliste (P1-Spiegel); Fitness-Function-Zeile
entsprechend nachgezogen.

### INFO-C — „Kern nicht thread-sicher" war als ADR-0009-(d)-Zitat lesbar
**Fund:** (d) beschließt wörtlich nur den Single-threaded-Betrieb; die
Kern-Aussage ist eine (korrekte) Folgerung, keine Zitatstelle.
**Disposition: eingearbeitet** — als Folgerung markiert.

### INFO-D — Prozess: Folgepflichten-Zeilen erst mit Accept
**Fund:** Die Folgepflichten-Tabelle im ADR-Index führt konventionsgemäß nur
akzeptierte ADRs — die 0017-Zeilen (Regel P; AK-Schärfung inkl. Spec-Nachzug
§4/§5/§6; Impl inkl. benannter Entscheidungen; arch-check-Kommentar-Nachzug)
sind mit dem Accept nachzuziehen, plus Index-Status + Geschichte-Zeile.
**Disposition: mit der Accept-Buchung ausgeführt** (kein ADR-Text-Finding).

**Ergebnis der Durchsicht:** 0 HIGH, 0 MED — **Accept erteilt** nach
Einarbeitung von LOW-A/LOW-B/INFO-C.
