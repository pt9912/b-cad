---
id: slice-042d
titel: Export-Refactor Persistenz — rise kern-seitig + persistence→services_geo-Kante raus
status: done
welle: welle-5-erweiterung
lastenheft_refs: [[OBJ-005](../../../../spec/lastenheft.md#3-projektziele), [LH-FA-BLD-002](../../../../spec/lastenheft.md#lh-fa-bld-002--projekt-speichern)]
adr_refs: [[ADR-0020](../../adr/0020-driven-adapter-serialisieren-kern-liefert-geometrie.md), [ADR-0003](../../adr/0003-persistenz-sqlite.md), [ADR-0006](../../adr/0006-relationales-schema-design.md)]
---

# Slice 042d: Export-Refactor Persistenz — `rise` kern-seitig geliefert

**Status:** **done (2026-07-23)** — implementiert, `make gates` grün (254 Tests, a-check 0, docs-check 221/0,
Coverage 91,5 %). [MR-006](../../../../harness/conventions.md#mr-006--unabhängiges-plan-review-vor-implementierungs-start)
Plan-Review 0 HIGH / 2 MED / 1 LOW + [MR-009](../../../../harness/conventions.md#mr-009--geometrielastiges-code-review-vor-welle-closure)
Code-Review 0 HIGH / 0 MED / 1 LOW / 1 INFO — alle eingearbeitet bzw. bewusst eingeordnet. **slice-042e in diese
Closure gefaltet + retired** (kein struktureller Diff verblieb). Siehe [§8 Closure-Notiz](#8-closure-notiz-2026-07-23).

**Welle:** welle-5-erweiterung. **Vierter** der fünf [ADR-0020](../../adr/0020-driven-adapter-serialisieren-kern-liefert-geometrie.md)-Folgepflicht-Refactor-Slices
(Familie 042a…e). **Vorgänger:** slice-042c (done).

**Autor:** Dietmar Burkard (AI-Harness-Lauf). **Datum:** 2026-07-23.

**Bezug:** [ADR-0020](../../adr/0020-driven-adapter-serialisieren-kern-liefert-geometrie.md) (Persistenz erhält
den abgeleiteten `rise`-Skalar analog dem Bündel — **save-Signatur-Form**, `model::Stair` hat **kein** `rise`-Feld
[rise ist bewusst abgeleitet, nicht gespeichert]; Kanten-Entfernung), [ADR-0003](../../adr/0003-persistenz-sqlite.md)
(atomare Persistenz), [ADR-0006](../../adr/0006-relationales-schema-design.md) (Schema — `rise_mm` write-derived).
**Achtung [MR-016](../../../../harness/conventions.md) (ADR-Immutabilität)** unberührt — 042d ändert Code, keine Accepted-ADR.

---

## 1. Ziel

Der SQLite-Persistenz-Adapter berechnet den abgeleiteten `rise`-Skalar je Treppe **nicht mehr selbst**
(`services::stairRiseMm`), sondern erhält ihn **kern-geliefert** über die geweitete `save`-Signatur (analog dem
`DerivedGeometry`-Bündel bei STEP/STL). Danach fällt die letzte `.a-check.yml`-Adapter→Kern-Kante
**`persistence → services_geo`** (der Adapter ruft `services/geometry` **nur** für `stairRiseMm`). **Verhaltens-
invariant** auf der beobachtbaren Ebene: der in `rise_mm` geschriebene Wert bleibt identisch, das atomare
Save + der E-IO-Fehlerfall bleiben erhalten.

**Zwei Befunde prägen den Schnitt:**
- **Kein Produktions-`save`-Aufrufer** (kein `ManageProjectPort`-Service, nicht in `main.cpp`) — nur die Tests
  rufen `repo.save` direkt. Es gibt **keinen** Service (wie den `ExchangeService`), durch den die Tests fahren
  könnten; die rise-Berechnung landet in einem **Test-Helfer** (bis ein Save-Use-Case-Service existiert; die
  Naht ist dann bereits richtig geschnitten — Zukunfts-Sicherung).
- **`rise_mm` wird nie zurückgelesen** (`loadStairs` selektiert es nicht; der Kern re-derived es) → **kein** Test
  prüft den `rise_mm`-Wert heute. Der Umzug der Berechnung braucht daher einen **neuen Verifikations-Test**, der
  die Spalte direkt liest (sonst ist der Umzug un-genetzt).

## 2. Definition of Done

- [ ] **`ProjectRepositoryPort::save`-Vertrag geweitet** um die kern-gelieferten Skalare. **Empfohlene Naht-Form:**
      ein kleines pures `model/`-Aggregat (z. B. `model::PersistedDerivations { std::map<model::StairId, double>
      stairRiseMm; }`) — **`map<StairId,…>`** (order-unabhängig, explizit) statt positionalem Vektor; **nicht** die
      volle `DerivedGeometry` (die trägt Export-Geometrie, die die Persistenz nicht braucht — Kopplung vermeiden).
      Alternativen (positionaler `vector<double>` / `DerivedGeometry` wiederverwenden) benannt; Review entscheidet.
      Port bleibt `model`-only (keine services/geometry-Kante am Port).
- [ ] **SQLite-Adapter serialisiert nur.** `insertStairs`: statt `stairRiseMm(stair, fromStoreyHeight(...))` den
      **gelieferten** `rise` je `StairId` an `rise_mm` binden. **Map-Lookup fail-closed** ([MR-006](../../../../harness/conventions.md#mr-006--unabhängiges-plan-review-vor-implementierungs-start)-MED-2,
      datenverlust-nah): fehlt der Skalar zu einer `stair.id`, **`.at()`** (wirft) → landet im bestehenden
      `catch(...)` → Temp entfernt, Zieldatei intakt (Atomarität gewahrt) — **kein** stilles `rise_mm=0`
      (`operator[]`/`.value_or(0)` verboten). Die `services/geometry`-Include (`stair_geometry.h`) + der
      `stairRiseMm`-Aufruf **entfernt**; `fromStoreyHeight` (die adapter-lokale Höhen-Auflösung, **nur** für
      `stairRiseMm` genutzt) **entfernt** — ihre throw-Semantik zieht in die Kern-Berechnung (s. u.). Atomares
      Save + Transaktion + E-IO-Fehlerfall unverändert (generischer `E-IO`-Substring, INFO-2).
- [ ] **Höhen-Auflösung konsolidiert + Semantik reconcilet.** Heute existieren **zwei** Kopien mit
      **verschiedener** Semantik: `ExchangeService::storeyHeight` (042c, anon-ns, **total/Default**
      `kDefaultStoreyHeightMm`) und adapter-`fromStoreyHeight` (**wirft** E-IO). Extrahieren in **einen** geteilten
      Kern-Helfer, der die Roh-Auflösung liefert und die **Semantik dem Aufrufer überlässt** — z. B.
      `std::optional<double> resolveStoreyHeight(Building, StoreyId)` (pure `model/`- oder `services/`-Helfer,
      erreichbar von Export **und** Save). Export-Pfad (042c) nutzt `.value_or(kDefaultStoreyHeightMm)`; der
      Save-seitige rise-Rechner wirft **E-IO** bei `nullopt` (danglendes Geschoss → kein Teil-Save, wie heute).
      Löst zugleich das 042a-LOW-2-/042c-Duplikat endgültig auf. **Heimat-Wahl** = Review.
- [ ] **Rise-Berechnung kern-seitig, im Aufrufer.** Da kein Produktions-`save`-Aufrufer existiert: ein **Test-
      Helfer** `saveProject(repo, building, path)` berechnet je Treppe `rise = stairRiseMm(s, height)` (mit dem
      geteilten Höhen-Helfer + throw-bei-nullopt) und ruft `save(building, {stairRiseMm}, path)`. **Benannte Grenze
      (ehrlich):** anders als bei 042c gibt es keinen Service, durch den das Netz fährt — der Test-Helfer **ist**
      der Aufrufer; die Naht ist für den künftigen `ManageProjectPort`-Save-Service vorbereitet.
- [ ] **Verwaistes `DerivedStair.rise_mm` retiren ([MR-006](../../../../harness/conventions.md#mr-006--unabhängiges-plan-review-vor-implementierungs-start)-LOW-1).**
      042a legte `DerivedStair.rise_mm` **ausdrücklich „für Slice 042d"** an (in der Annahme, `save` bekäme die
      volle `DerivedGeometry`). Da die Persistenz eine **eigene** `map`-Naht bekommt (nicht die Export-`DerivedGeometry`
      — s. Naht-Form), ist das Feld **verwaist** (befüllt in `exchange_service.cpp`, von **keinem** Exporter gelesen).
      In 042d **entfernen**: das Feld aus `derived_geometry.h`, die `stairRiseMm(...)`-Befüllung im Export-Zweig des
      `ExchangeService` (der Export-Pfad ruft `stairRiseMm` dann gar nicht mehr — nur der Save-Pfad), + der
      irreführende Kommentar. Kein Verhaltens-Effekt (das Feld war tot).
- [ ] **`.a-check.yml`-Kante `persistence → services_geo` entfernt.** Nach der Migration ruft der Persistenz-
      Adapter **nichts** mehr aus `services/geometry` (`stairRiseMm` war der einzige Zugriff). **Gegenprobe:**
      `make a-check` grün **nach** der Entfernung. `services_geo` hat danach **nur noch** die Eingangskante
      `services → services_geo` — **alle** Adapter→Kern-Kanten sind weg (der [ADR-0020](../../adr/0020-driven-adapter-serialisieren-kern-liefert-geometrie.md)-Kernanspruch).
      Weil 042d die Kante entfernt, **zieht 042d auch die `architecture.md` §2-persistence-Zeile mit** (Option A,
      [MR-006](../../../../harness/conventions.md#mr-006--unabhängiges-plan-review-vor-implementierungs-start)-MED-1 —
      analog 042c; sonst widerspräche §2 der `.a-check.yml`, die 042d selbst ändert); **nur** das §1-Diagramm
      (grob-Ebene, zeigt schon reines Hexagonal) bleibt dem Abschluss-Slice 042e.
- [ ] **Verhaltens-Invarianz + Rise-Verifikations-Test (Netz-Loch schließen).** `make gates` grün; die
      bestehenden Treppen-Round-Trip-/Crash-Recovery-Tests bleiben **unverändert grün**. **Neuer Test** (weil
      `rise_mm` heute **nie** zurückgelesen/geprüft wird): nach `saveProject` die `rise_mm`-Spalte **direkt per
      SQL lesen** und `== stairRiseMm(stair, storeyHeight)` prüfen (für ≥ 1 Treppe mit bekannter Geschoss-Höhe) —
      belegt, dass der gelieferte Skalar byte-gleich zum früheren adapter-berechneten ist. Plus ein
      Danglender-Storey-Test: `saveProject` mit unbekanntem `from_storey_id` → **E-IO** (kein Teil-Save, Datei
      unberührt) — die throw-Semantik überlebt den Umzug.
- [ ] **Test-Aufrufer-Migration.** Die **25** direkten `repo.save(building, path)`-Aufrufe (`test_sqlite_project_
      repository.cpp` 18 + `test_sqlite_crash_recovery.cpp` 7 — INFO-1) auf den `saveProject`-Helfer umstellen
      (füllt die rise-Skalare aus **derselben** `stairRiseMm`-Quelle → Byte-Identität; Muster 042c `writeStep`).
      **Achtung:** mehrere Crash-Recovery-Saves sitzen in **geforkten/getöteten Kind-Prozessen** — der Helfer muss
      dort ebenso greifen.
- [ ] **Doku.** `spec/spezifikation.md` §1 (Persistenz-Datenfluss-Umfeld) um die
      kern-gelieferten Persistenz-Skalare (token-frei, [MR-011](../../../../harness/conventions.md#mr-011--referenz-integritäts-gate-matrix-ids-spans-hostpaths)/[MR-014](../../../../harness/conventions.md)
      vor Gate greppen) + `spezifikation-historie`; [ADR-Index](../../adr/README.md) Persistenz-Zeile „erfüllt";
      **CHANGELOG** ([MR-004](../../../../harness/conventions.md#mr-004--top-level-changelogmd-keep-a-changelog)).
      **`architecture.md` §2-persistence-Zeile: `services/geometry` entfernen** ([MR-006](../../../../harness/conventions.md#mr-006--unabhängiges-plan-review-vor-implementierungs-start)-MED-1
      — 042d entfernt die Kante, also zieht 042d die §2-Zeile mit, konsistent mit `.a-check.yml`; nur das
      §1-Diagramm bleibt 042e). **[ADR-Index](../../adr/README.md) reconcilen** (042d = persistence-Kante +
      §2-persistence-Zeile; 042e = **nur** §1-Diagramm + Abschluss-Verifikation). Closure-Notiz.
      **[MR-020](../../../../harness/conventions.md) Closure-Disziplin:** vor Closure existiert slice-042e
      (Skelett) → erfüllt.

## 3. Plan (vor Code)

| Datei / Komponente | Änderungs-Art | Begründung |
|---|---|---|
| `src/hexagon/model/{persisted_derivations}.h` | neu | pures Aggregat der kern-gelieferten Persistenz-Skalare (`map<StairId,double> stairRiseMm`) |
| `src/hexagon/model/{storey_query}.h` (o. `services/`) | neu | geteilter `resolveStoreyHeight → optional<double>` (Semantik beim Aufrufer) |
| `src/hexagon/ports/driven/project_repository_port.h` | ändern | `save`-Signatur um die Skalare |
| `src/adapters/persistence/sqlite_project_repository.{h,cpp}` | ändern | `rise` binden statt berechnen; `stairRiseMm`/`fromStoreyHeight`/services-Include raus |
| `src/hexagon/services/exchange_service.cpp` | ändern (klein) | `storeyHeight`-anon-ns auf den geteilten Helfer (Konsolidierung 042c-LOW-2 endgültig); `stairRiseMm`-Befüllung des verwaisten `rise_mm` raus (LOW-1) |
| `src/hexagon/model/derived_geometry.h` | ändern | verwaistes `DerivedStair.rise_mm` entfernen (LOW-1) |
| `.a-check.yml` | ändern | Kante `persistence → services_geo` entfernen (Verschärfung) |
| `spec/architecture.md` §2 | ändern | persistence-Adapter-Zeile: `services/geometry` raus (Option A, [MR-006](../../../../harness/conventions.md#mr-006--unabhängiges-plan-review-vor-implementierungs-start)-MED-1; §1-Diagramm bleibt 042e) |
| `tests/adapters/test_sqlite_project_repository.cpp`, `test_sqlite_crash_recovery.cpp` | ändern | `saveProject`-Helfer + **25** Aufrufe (inkl. Fork-Kinder); **rise_mm-Verifikations-Test** (direkt-SQL) + Danglender-Storey-E-IO-Test |
| `spec/spezifikation.md` + `-historie.md`, [ADR-Index](../../adr/README.md), `CHANGELOG.md` | ändern | Doku-Nachzug (token-frei) |

**Bewusst NICHT Teil:** das **§1-Diagramm** (grob-Ebene → reines Hexagonal) + die Abschluss-Verifikation
(**alle** Kanten weg) → **042e**; ein echter `ManageProjectPort`-Save-Use-Case-Service (existiert nicht, eigener
späterer Slice) — 042d schneidet nur die Naht. (Die §2-persistence-**Zeile** zieht 042d **mit** — MED-1.)

## 4. Trigger

- **slice-042c done** (STEP/STL migriert; das Naht-Muster [Kern liefert Skalare] + der geteilte Höhen-Helfer-Bedarf
  etabliert). [ADR-0020](../../adr/0020-driven-adapter-serialisieren-kern-liefert-geometrie.md) Accepted.
- **[MR-006](../../../../harness/conventions.md#mr-006--unabhängiges-plan-review-vor-implementierungs-start)-Plan-Review vor Start** (Reviewer ≠ Autor), HIGH blockiert.
  Persistenz (nicht geometrieschwer, aber datenverlust-nah) → **Code-Review vor Welle-Closure** ([MR-009](../../../../harness/conventions.md#mr-009--geometrielastiges-code-review-vor-welle-closure)) empfohlen.

## 5. Closure-Trigger

- DoD vollständig, `make gates` grün (**Persistenz-Round-Trip-Orakel unverändert grün** + Rise-Verifikations-Test +
  Danglender-Storey-E-IO-Test; `make a-check` grün nach Kanten-Entfernung — **alle Adapter→Kern-Kanten weg**),
  Closure-Notiz → **slice-042e (Abschluss)** startbar.

## 6. Risiken und offene Punkte

- **Rest-Risiko #1 — Netz-Loch: `rise_mm` wird nie zurückgelesen (der Kern-Prüfstein).** Kein bestehender Test
  prüft den `rise_mm`-Wert (er ist write-derived, `loadStairs` liest ihn nicht). Verschöbe man die Berechnung nur
  und die Test-Helfer bauten die Skalare „irgendwie", bliebe eine falsche rise-Rechnung **unentdeckt**.
  **Mitigation (DoD):** der neue Test **liest die `rise_mm`-Spalte direkt per SQL** und prüft `== stairRiseMm(...)`
  gegen die bekannte Geschoss-Höhe — der Helfer speist aus **derselben** Quelle (Byte-Identität, nicht handgerollt).
- **Rest-Risiko #2 — Fehler-Timing verschiebt sich (Verhaltens-Invarianz-Feinheit).** Heute wirft `fromStoreyHeight`
  **mitten in der Save-Transaktion** (→ Rollback, kein Teil-Save). Nach dem Umzug wirft der **Aufrufer VOR** `save`
  (er berechnet rise zuerst) → `save` wird gar nicht erst gerufen. **Beobachtbar identisch** (E-IO, keine Datei),
  aber der Code-Pfad differiert. Der Danglender-Storey-Test pinnt das beobachtbare Verhalten (E-IO + Datei
  unberührt). Das Review bewertet, ob die Atomaritäts-Aussage gewahrt bleibt.
- **Rest-Risiko #3 — Höhen-Auflösungs-Semantik (throw vs. Default).** Der geteilte Helfer liefert `optional`; Export
  nutzt Default (Totalität), Save wirft (Datenintegrität). **Nicht** eine Semantik beiden aufzwingen. Das Review
  prüft die Extraktion + dass 042c's Export-Verhalten (Default) **unverändert** bleibt (die STEP/STL-Orakel als Netz).
- **Rest-Risiko #4 — Naht-Form.** `map<StairId,double>` (empfohlen, order-unabhängig) vs. positionaler `vector`
  (fragil bei Reordering) vs. `DerivedGeometry`-Wiederverwendung (Kopplung an Export-Geometrie). Review entscheidet;
  Port bleibt `model`-only.
- **Rest-Risiko #5 — kein Produktions-Aufrufer (hohle Naht).** Die Naht wird nur von Tests befahren; ehrlich
  benannt, für den künftigen `ManageProjectPort`-Save-Service vorbereitet. Kein Über-Versprechen.
- **a-check-Gegenprobe:** nach der Entfernung `make a-check` grün; **alle** Adapter→`services_geo`-Kanten sind dann
  weg (Verschärfung, [§2.6](../../../../AGENTS.md) n/a). **Spec token-frei** ([MR-011](../../../../harness/conventions.md#mr-011--referenz-integritäts-gate-matrix-ids-spans-hostpaths)/[MR-014](../../../../harness/conventions.md)).
- **Scope:** mittel — kern-nahe Naht + ~24 Test-Aufrufer + zwei neue Tests + Helfer-Extraktion; überwiegend
  mechanisch, aber datenverlust-nah (Persistenz) → Sorgfalt beim atomaren Save + E-IO-Pfad.

## 7. Sub-Area-Modus-Begründung

### Sub-Area: Kern + Adapter-Code `src/`

- **Modus:** GF; **Dichte:** hoch (Port-Vertrag + Adapter-Serialisierung + Helfer-Konsolidierung + a-check-Kante).
  **Phase-Reife:** welle-5 Refactor. **Risiko:** mittel — **datenverlust-nah** (atomares Save, E-IO); die
  Round-Trip-/Recovery-Orakel + der neue rise-Verifikations-/Dangling-Test sind das Netz; **Code-Review** empfohlen.

### Sub-Area: Spec/Doku `spec/` + `docs/plan/`

- **Modus:** GF; **Dichte:** mittel (Persistenz-Datenfluss token-frei; ADR-Index/CHANGELOG). **Risiko:** niedrig.

## 8. Closure-Notiz (2026-07-23)

**Umgesetzt.** Der SQLite-Persistenz-Adapter berechnet den `rise` nicht mehr selbst, sondern **bindet** den
kern-gelieferten Skalar:

- **Naht-Form:** neues pures Aggregat `model::PersistedDerivations { std::map<StairId,double> stairRiseMm; }`
  (order-unabhängig, **eigene** Naht — nicht die Export-`DerivedGeometry`); `ProjectRepositoryPort::save` +
  `SqliteProjectRepository::save` um den Parameter geweitet. Port bleibt `model`-only.
- **Adapter serialisiert nur:** `insertStairs` bindet `derived.stairRiseMm.at(stair.id)` **fail-closed**
  (`.at()` → `catch(...)` → Temp entfernt, Ziel intakt; kein stilles `rise_mm=0`). `stair_geometry.h`-Include +
  `stairRiseMm`/`fromStoreyHeight` entfernt.
- **Höhen-Helfer konsolidiert:** neuer geteilter Kern-Helfer `model::resolveStoreyHeight → optional<double>`
  (`storey_query.h`); Export nutzt `.value_or(kDefaultStoreyHeightMm)` (total, für slab **und** stair), Save-
  Aufrufer wirft E-IO bei `nullopt`. Löst das 042a-LOW-2-/042c-Duplikat endgültig auf. Die 042c-anon-`storeyHeight`
  ist retired.
- **Verwaistes `DerivedStair.rise_mm` retired** (LOW-1): Feld + Export-Befüllung + Kommentar weg; kein Verhaltens-
  Effekt (war tot).
- **Aufrufer:** Test-Helfer `saveProject` (`tests/adapters/save_project_test_helper.h`) berechnet die Skalare aus
  **derselben** Quelle (`stairRiseMm` + `resolveStoreyHeight`) → Byte-Identität; steht für den künftigen
  `ManageProjectPort`-Save-Use-Case-Service (existiert noch nicht — hohle Naht ehrlich benannt).

**Kanten-Abschluss (die 042e-Faltung).** `.a-check.yml`-Kante `persistence → services_geo` entfernt +
`architecture.md` §2-persistence-Zeile mitgezogen. **Audit-Ergebnis: ALLE Adapter→Kern-Kanten sind weg** —
`services_geo` trägt nur noch `services → services_geo` (`make a-check` = 0 Befunde als Gegenprobe). Das
`architecture.md` **§1-Diagramm ist bereits reines Hexagonal** (die Driven-Adapter tragen nur
`implementiert von`-Kanten zu den Driven Ports, keinen Adapter→Kernel-Pfeil; `services/geometry` ist dort kein
eigener Knoten) → **kein struktureller Diff verbleibt für den ursprünglich reservierten slice-042e**. Daher ist
**042e in diese Closure gefaltet und retired** (Skelett aus `open/` entfernt; [ADR-Index](../../adr/README.md)
entsprechend). Der [ADR-0020](../../adr/0020-driven-adapter-serialisieren-kern-liefert-geometrie.md)-Folgepflicht-
Block ist damit **komplett** (042a…042d).

**Netz + Gates.** 25 `save`-Aufrufer (18 project-repo + 7 crash-recovery, inkl. Fork-Kinder) auf `saveProject`
migriert; neu: `rise_mm`-**Direkt-SQL-Verifikations-Test** (`RiseMmKernGeliefertByteGleich` — die Spalte wird
write-derived nie zurückgelesen) + **Danglender-Storey-E-IO-Test** (`DanglendesGeschossWirftEIOKeinTeilSave`,
`!exists(path) && !exists(tmp)`). Round-Trip- + `kill -9`-Recovery-Orakel **unverändert grün**. `make gates`
**EXIT 0** · 254 Tests · a-check 0 · docs-check 221/0 · Coverage 91,5 %.

**Fehler-Timing (Verhaltens-Invarianz-Feinheit).** Der dangling-Wurf wandert von *mitten in der Transaktion*
(alt: `fromStoreyHeight` → Rollback) nach *vor `save`* (neu: Aufrufer). **Beobachtbar identisch** (E-IO, kein
Temp, Ziel unberührt — der neue Test pinnt es); es entsteht sogar **ein** Temp-Pfad weniger.

**Reviews.** [MR-006](../../../../harness/conventions.md#mr-006--unabhängiges-plan-review-vor-implementierungs-start)
Plan-Review 0 HIGH / 2 MED / 1 LOW (alle eingearbeitet); [MR-009](../../../../harness/conventions.md#mr-009--geometrielastiges-code-review-vor-welle-closure)
Code-Review **0 HIGH / 0 MED / 1 LOW / 1 INFO** — LOW-1 (fehlender Direkt-Include in `sqlite_project_repository.h`)
eingearbeitet; INFO-1 (die fail-closed-`.at()` entkommt als `std::out_of_range` statt `runtime_error`) **bewusst
belassen**: nur bei Aufrufer-Programmierfehler (unvollständige Map) erreichbar, nie durch Daten — Defense-in-depth
([MR-006](../../../../harness/conventions.md#mr-006--unabhängiges-plan-review-vor-implementierungs-start)-MED-2),
der reale Aufrufer füllt die Map aus `building.stairs` oder wirft vorab E-IO.

**Lerneintrag.** Test-lokale Helfer-Header (`tests/…`) liegen **nicht** unter dem `src/`-Include-Root → sibling-
relativ inkludieren (`#include "save_project_test_helper.h"`), nicht `adapters/…`. Und: ein Feld „für einen
Folge-Slice" vorpositionieren (042a `DerivedStair.rise_mm`) verwaist, wenn der Folge-Slice eine **eigene** Naht
bekommt — besser erst schneiden, wenn der Konsument feststeht.

**Folge.** [ADR-0020](../../adr/0020-driven-adapter-serialisieren-kern-liefert-geometrie.md)-Refactor-Familie
**abgeschlossen** (042a…042d; 042e gefaltet). Nächster Schritt der Roadmap: **Canvas-Impl** (DRW-2D-Widget, jetzt
architektonisch entsperrt).
