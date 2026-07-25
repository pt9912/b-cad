# MR-006 Plan-Review (Nachtrag) — slice-050 **Teil D** (`arch-check.sh` → a-check `constructs`)

**Datum:** 2026-07-25 · **Reviewer:** unabhängiger Agent (≠ Autor), read-only ·
**Gegenstand:** ausschließlich das Delta **Teil D (Amendment 2026-07-25)** in
`docs/plan/planning/done/slice-050-tooling-konsolidierung-dcheck.md` — inkl. der von Teil D
neu gefassten Stellen in A/B/C (HIGH-3-Auflösung, MR-Frage).
**Nicht Gegenstand:** A/B/C im Ur-Stand (`docs/reviews/2026-07-24-slice-050-plan.md`).
**Linse:** (a) Quellen-Konsistenz · (b) Plan-Qualität (DoD-Beobachtbarkeit, reale Sensor-Deckung).

**Verdikt: 3 HIGH / 5 MED / 5 LOW / 7 INFO → Teil D blockiert; nach Einarbeitung startbar.**

**Geprüfte kanonische Quellen:** a-check-Benutzerhandbuch 1.35 (im a-check-Repo unter docs/user/,
§3.4 · §4 „Roh-Text-Monopol (`constructs`)" · §2 Abdeckungs-Hinweis · Änderungshistorie 1.33–1.35)
+ a-check-Release-Register (`version.md#aktuell`); d-check-Benutzerhandbuch 1.42 (v0.51.1, §5
`ignore-refs`); `docs/plan/adr/0017-plugin-api-abi.md` (Accepted/immutabel); `AGENTS.md` §2.5/§2.6/§3;
`harness/conventions.md` (MR-006, MR-012, MR-013, MR-020); Repo-Ist (`tools/arch-check.sh`,
`.a-check.yml`, `a-check.mk`, `Makefile`, `.devcontainer/Dockerfile`, `.d-check.yml`).
**Empirie:** a-check v0.16.0 (`@sha256:aef28cfe…`) und der **real gepinnte** Stand
(`@sha256:24939a6b…` = v0.13.0) gegen das Repo und gegen 20 selbst gebaute Fixtures
(`docker run --rm --network none -v <scratch>:/src:ro`), Scratch außerhalb des Projektbaums.
Repo unverändert.

---

## HIGH

### HIGH-1 — D1 hebt den **wirkungslosen** Pin; der effektive Pin bleibt v0.13.0 → `make a-check` bricht mit Exit 2

- **Fundstelle:** Plan D1 (Z. 159–165) und Plan-Tabelle §3 (Z. 249): „**D1 — Pin-Bump `a-check.mk`:**
  `A_CHECK_IMAGE` von `@sha256:203df7ab…` auf **v0.16.0** …".
- **Geprüfte Quelle:** `a-check.mk` Z. 5 (`A_CHECK_IMAGE ?= …@sha256:203df7ab…`) **und** `Makefile`
  Z. 45 (`A_CHECK_IMAGE ?= …@sha256:24939a6b…`, gesetzt **vor** `include a-check.mk` in Z. 46), plus
  der Makefile-Kommentar Z. 38–39 („Release-Digest **hier**, damit `a-check.mk` unverändert aus
  `--print-mk` bleibt").
- **Befund:** GNU-make-`?=` wirkt nur auf **undefinierte** Variablen — die Zuweisung im `Makefile`
  gewinnt, die in `a-check.mk` ist ein No-op (empirisch mit einer Minimal-Makefile-Probe bestätigt).
  Der **effektive** Pin ist damit `24939a6b…` = **v0.13.0** (belegt: `a-check:v0.14.0 --print-mk`
  druckt genau diesen Digest als Vorgänger-Release; a-check-`git log`: „v0.13.0 Re-Pin — Digest
  24939a6b"). `203df7ab…` ist v0.12.0 und im Repo **nirgends** wirksam. Folge: D1 wie geschrieben
  ändert **nichts** am Gate-Lauf; D2 (`constructs`-Block) trifft dann auf einen Prüfer ohne
  `constructs` und a-check dekodiert **streng** (Handbuch §4). **Empirisch reproduziert** mit dem
  exakt gepinnten Image:
  `a-check: /src/.a-check.yml: yaml: unmarshal errors: line 61: field constructs not found` → **Exit 2**,
  d. h. `make a-check` und `make gates` **rot**, ohne dass ein einziger Befund geprüft wurde.
  Der Plan konnte das nicht sehen, weil beide Vorproben (D1/D2) per `docker run` gefahren wurden,
  nicht über `make a-check`.
- **Empfohlene Auflösung:** D1 auf **`Makefile` Z. 45** umschreiben (das ist die kanonische
  Pin-Stelle dieses Repos) und den stale Kommentar Z. 38–39 („digest-gepinnt v0.13.0") mitziehen.
  `a-check.mk` bewusst **nicht** aus `--print-mk` regenerieren: das v0.16.0-Image druckt
  bauartbedingt noch `6425c93a…` (v0.15.0) — der in MR-013 dokumentierte **Selbst-Pin-Lag** besteht
  fort (verifiziert). Entweder `a-check.mk` unangetastet lassen (Status quo, dann in D1 benennen,
  dass sein Default inert **und** veraltet ist) oder ihn ebenfalls auf `aef28cfe…` heben — die
  DoD-Zeile muss aber sagen, dass der **wirksame** Pin `Makefile` Z. 45 ist. **Beobachtbarer Sensor
  ergänzen:** `make a-check` (nicht `docker run`) grün nach dem Bump.

### HIGH-2 — „Der ADR-0017-P2-**Wortlaut** ist damit vollständig gegatet" ist **falsch**; der Verzicht ist größer als (i)/(ii)

- **Fundstelle:** Plan Teil D, Fähigkeits-Tabelle + Folgesatz (Z. 150–157), Rest-Risiko #5 (Z. 299–305),
  D6 (Z. 180–186).
- **Geprüfte Quelle:** ADR-0017 Entscheidung #6 (Z. 215–219) und Fitness-Function-Zeile (Z. 370),
  **wörtlich**: „**P2 (Plugin-Import-Grenze):** Dateien unter `plugins/` inkludieren **nur** den
  Plugin-API-Header-Satz + `src/hexagon/model/` + `src/hexagon/ports/driving/` — keine
  `src/adapters/`-, Qt-, OCC-, SQLite- und keine `dlfcn.h`-Header." Dazu `tools/arch-check.sh`
  Z. 53–70 und a-check-Handbuch §3.4-Kasten/§3.7-Fallstrick (Ziel ohne Schicht ⇒ **unbeurteilt**).
- **Befund:** P2 ist im ADR-Wortlaut eine **geschlossene Allowlist** („nur …") — arch-check.sh setzt
  sie textuell als *deny-by-default* um (jeder Quote-Include, der nicht mit `plugin_api/`,
  `hexagon/model/` oder `hexagon/ports/driving/` beginnt, ist ein Befund). a-check ersetzt das durch
  eine **Kanten**-Prüfung mit *allow-when-unresolvable*: nur die im Plan genannte **Verbots**-Hälfte
  (Adapter/Qt/OCC/SQLite/`dlfcn.h`) und die auflösbaren Schicht-Kanten werden gegatet.
  Fixture-Belege (a-check v0.16.0, b-cads `.a-check.yml` + `constructs`):
  - **gefangen** ✔ — Quote *und* Angle auf `adapters/io` → `lateral-adapter` (2×); Quote *und* Angle
    auf `hexagon/ports/driven` → `port-direction-mismatch` (2×); `hexagon/services` →
    `wrong-direction`; Qt (beide Formen), OCC-`.hxx`, `sqlite3`, `dlfcn.h` → `tech-leak` (5×);
    aus src/plugin_api heraus auf `adapters/io` → `wrong-direction`. Alle positiven
    Tabellen-Behauptungen des Plans sind damit **bestätigt**.
  - **nicht gefangen**, obwohl P2-Wortlaut-Verstoß und heute von arch-check.sh gemeldet:
    (a) ein **plugin-lokaler** Include (`#include "local_helper.h"` bzw. `#include "../testing/other.h"`
    innerhalb von `plugins/`) — 0 Befunde, weil derselbe Layer bzw. unauflösbar;
    (b) ein Ziel **im Repo, aber in keiner `layers`-Schicht** (Fixture: ein Header unter src/hexagon/util)
    — 0 Befunde, nur der **advisory** Abdeckungs-Hinweis (Handbuch 1.35: „kein Befund, kein
    Exit-Code-Wechsel").
    Beides fällt **nicht** unter die im Plan benannten (i) „repo-extern" und (ii) „Angle-Form auf ein
    **erlaubtes** Ziel" — (ii) ist überdies gar kein P2-Wortlaut-Verstoß (der ADR fordert nirgends die
    Quote-Form; das war reine slice-026b-Härtung, insofern korrekt beschrieben).
  Damit stimmt der Kernsatz nicht: der Verzicht umfasst **die Allowlist-Natur von P2**, nicht nur zwei
  sub-kanten Exoten. Das ist genau die Aussage, die ADR-0021 buchen soll — eine falsch geschnittene
  §2.6-Buchung wäre nach Accept selbst immutabel.
- **Empfohlene Auflösung:** Fähigkeits-Tabelle, Rest-Risiko #5 und der D6-Auftrag umschreiben auf:
  „gegatet bleibt die **Verbots**-Hälfte des P2-Wortlauts + alle auflösbaren Schicht-Kanten; **nicht**
  gegatet bleibt die **geschlossene Allowlist** — jedes Include-Ziel, das a-check auf keine Schicht
  auflöst (repo-extern, plugin-lokal/relativ, repo-intern aber schichtlos), sowie die Include-**Form**".
  Als Kompensation empfehle ich zwei Zusätze, die den Verzicht real verkleinern (beide ohne
  Gate-Gaming, weil sie *nichts* umdeuten): (1) den advisory **Abdeckungs-Hinweis** als DoD-Zeile
  („Baum vollständig von `layers`/`composition_root` gedeckt", heute wahr — voll-repo verifiziert),
  (2) einen `constructs`-Eintrag als Roh-Text-Monopol für die verbliebene Form-Achse prüfen (z. B.
  Angle-Include von Projekt-Präfixen), **falls** eine Zone formulierbar ist — sonst ausdrücklich
  als geprüft und verworfen in ADR-0021 benennen.

### HIGH-3 — D8 verlangt genau die MR-013-Edits, die Teil C als unzulässig verwirft (Selbstwiderspruch)

- **Fundstelle:** Plan D8 (Z. 191–195): „… `harness/conventions.md` **Z. 442/455/468**" versus
  Teil C (Z. 199–214): „**Auflösung stattdessen: Tombstone, MR-013 bleibt unangetastet.**"
- **Geprüfte Quelle:** `harness/conventions.md` — MR-013 beginnt in Z. 439 und endet vor MR-014
  (Z. 489). Die Zeilen 442 (Geltungsbereich-Link auf `tools/arch-check.sh`), 448, 455 und 468
  („ihre Sensors-Bindung wandert von `arch-check` auf `a-check`") liegen **alle innerhalb von
  MR-013**. Die Unveränderlichkeits-Praxis ist wörtlich belegt (MR-012 zu MR-010: „nach Aufnahme
  inhaltlich unveränderlich … Nachzug per neuem Eintrag").
- **Befund:** Der Implementierer bekommt zwei einander ausschließende Anweisungen zur selben Datei;
  die naheliegende (D8 abarbeiten) verletzt genau die Regel, deren Beachtung das Amendment als
  Verbesserung gegenüber dem Ur-Plan verkauft. Der Widerspruch ist nicht kosmetisch: MR-013 ist der
  einzige Ort, der die Sensor-Bindung dokumentiert, und ein In-Place-Edit dort ist irreversibel
  falsch (Nachzug gehört in MR-021).
- **Empfohlene Auflösung:** In D8 die Zeilenangabe `harness/conventions.md` Z. 442/455/468 **streichen**
  und durch „MR-013 bleibt unangetastet (Tombstone, Teil C); der Nachzug lebt in MR-021" ersetzen.
  Sicherstellen, dass die D8-Liste danach nur noch **mutable** Dokumente nennt.

---

## MED

### MED-1 — `constructs`-Zone ohne Schrägstrich weitet das `dlopen`-Monopol auf Geschwister-Verzeichnisse

- **Fundstelle:** Plan D2 (Z. 166–168), „byte-genau die verifizierte Fassung":
  `adapter: src/adapters/plugin`.
- **Geprüfte Quelle:** a-check-Handbuch §4: „Der Adapter-Abgleich ist ein **Teilstring**-Vergleich auf
  dem Dateipfad (nicht segmentgrenzen-bewusst — `adapters/config` matcht auch `adapters/configurator`;
  präzise Fragmente wählen)"; `constructs` nutzt „**dieselbe** Mechanik wie `tech`".
  Gegenstück: `tools/arch-check.sh` Z. 42 (`grep -vE '^src/adapters/plugin/'`, **verankert + Slash**).
- **Befund:** Empirisch reproduziert — ein `::dlopen(`-Aufruf in einem Fixture-Verzeichnis
  src/adapters/plugin_helper/ wird von arch-check.sh gemeldet, von der Plan-Fassung **nicht**
  (4 statt 5 `construct-leak`). Mit `adapter: src/adapters/plugin/` (ein Zeichen mehr) meldet a-check
  alle 5 Fälle und die echten Host-Dateien bleiben weiterhin still (verifiziert). Das ist eine stille
  Monopol-Aufweichung, die nicht im Verzicht benannt ist.
- **Empfohlene Auflösung:** D2 auf `adapter: src/adapters/plugin/` ändern und die Gegenprobe
  (Nachbar-Verzeichnis) in D7 aufnehmen.

### MED-2 — D4 belegt seine Behauptung nicht: der zitierte Kommentar-Treffer ist **kein** Falsch-Positiv, und die Richtung ist ein Deckungs-**Verlust**

- **Fundstelle:** Plan D4 (Z. 171–174): „`arch-check.sh` grept roh (ein `dlclose` im **Kommentar** war
  ein Falsch-Positiv, real in `src/adapters/plugin/plugin_host.h:13`); `constructs` strippt
  C-Kommentare … Das ist eine **Verschärfung der Ehrlichkeit**".
- **Geprüfte Quelle:** `src/adapters/plugin/plugin_host.h` Z. 13 (liegt **in** der erlaubten Zone;
  `tools/arch-check.sh` Z. 42 filtert `^src/adapters/plugin/` heraus) + a-check-Handbuch §4
  („Kommentare zählen nicht — außer in Python … bewusste Abweichung von einem `grep`-Skript").
- **Befund:** Die zitierte Stelle ist heute **kein** Befund (weder bei arch-check.sh noch bei
  a-check) — sie kann die Behauptung nicht tragen. Die Differenz wirkt zudem in die **andere**
  Richtung: ein auskommentierter `dlopen(`-Aufruf **außerhalb** der Zone wird von arch-check.sh
  gemeldet, von `constructs` **nicht** (Fixture: src/adapters/ui/comment_leak.cpp — arch-check.sh
  meldet, a-check schweigt). Das ist Entrauschung, aber eben ein weiterer, nicht gebuchter
  Deckungs-Verlust — nicht eine „Verschärfung".
- **Empfohlene Auflösung:** D4 umformulieren: Kommentar-Treffer außerhalb der Zone entfallen
  (bewusst, Handbuch §4) — als **dritter** benannter Verzichts-Punkt in MR-021/ADR-0021 statt als
  Verschärfung; das falsche Beispiel streichen.

### MED-3 — Sensor-Umbindung ist breiter als ADR-0017; die ADR-Index-Folgepflicht-Zeile wird unwahr

- **Fundstelle:** Plan D5 (Z. 175–179): MR-021 trägt „Sensor-Umbindung `make arch-check` → `make a-check`
  **für ADR-0017-Regel P**"; Plan-Tabelle §3 Z. 258: `docs/plan/adr/README.md` ändern nur „ADR-Index um
  ADR-0021".
- **Geprüfte Quelle:** Repo-Grep über `docs/plan/adr/`: **15** ADRs binden Fitness-Functions an
  `make arch-check` (0001 Z. 77/80 · 0002 Z. 73/77/81 · 0003 Z. 64 · 0007 Z. 108 · 0008 Z. 121 ·
  0009 Z. 145/157 · 0011 Z. 220 · 0012 Z. 189 · 0013 Z. 224/225 · 0014 Z. 197 · 0015 Z. 186 ·
  0016 Z. 287 · 0017 Z. 369/370 · 0018 Z. 93 · 0019 Z. 87/89) — alle Accepted/immutabel.
  MR-013 band bisher nur die **A–E-Hälfte** um und ließ `make arch-check` als real existierendes
  Target stehen; nach Teil D existiert das Target **gar nicht mehr**. Dazu MR-020 (ADR-Index-
  Folgepflicht-Zeilen = kanonische „nicht-vergessen"-Liste) und `docs/plan/adr/README.md` Z. 62,
  die Regel P als „**erfüllt** … in `tools/arch-check.sh`" führt.
- **Befund:** (1) MR-021 muss die Umbindung **generisch** aussprechen („`make arch-check` entfällt;
  alle ADR-Fitness-Zeilen, die es nennen, binden auf `make a-check`"), sonst bleiben 14 ADRs mit einem
  Sensor, den es nicht gibt — genau die Honesty-Klasse, die AGENTS §3 adressiert. (2) Die **mutable**
  ADR-Index-Zeile 62 wird sachlich falsch und ist im Plan nicht als Änderung gebucht.
- **Empfohlene Auflösung:** D5 um den generischen Umbindungs-Satz erweitern; Plan-Tabelle §3 und D8
  um „`docs/plan/adr/README.md` Z. 62 (ADR-0017-Folgepflicht: Mechanismus `constructs`/`edges`/`tech`
  statt `tools/arch-check.sh`)" ergänzen.

### MED-4 — Fünf **In-Code**-Kommentare nennen den retireten Sensor; D8 kennt nur `.md`

- **Fundstelle:** Plan D8 (Z. 191–195) — nur Markdown-Dateien.
- **Geprüfte Quelle:** Repo-Grep (nicht-`.md`): `src/adapters/plugin/plugin_host.cpp` Z. 2,
  `src/adapters/plugin/plugin_host.h` Z. 5, `src/adapters/CMakeLists.txt` Z. 80,
  `src/plugin_api/CMakeLists.txt` Z. 5, `plugins/CMakeLists.txt` Z. 10 („arch-check Regel P1/P2").
  Präzedenz: ADR-0017 Z. 313–314 machte den Nachzug eines stale werdenden Kommentars ausdrücklich zur
  Folgepflicht; MR-013 führt „Source-Drift … nachziehen (Entropy Management)".
- **Befund:** Nach dem Retire behaupten fünf produktive Kommentare eine Regel-Quelle, die es nicht mehr
  gibt. Kein Gate meldet das (kein Pfad-Zitat, nur der Name) — es bleibt still stehen.
- **Empfohlene Auflösung:** D8 um eine Zeile „In-Code-Kommentare (5 Stellen) auf `a-check`
  (`constructs`/`edges`/`tech`) umschreiben" erweitern.

### MED-5 — ADR-0021 ist ohne das im Repo durchgängige **unabhängige ADR-Text-Review** vor Accept geplant

- **Fundstelle:** Plan D6 (Z. 180–186) + Tabelle §3 Z. 257/264 (als neue Artefakte sind nur die ADR
  selbst und dieser MR-006-Nachtrag gelistet).
- **Geprüfte Quelle:** `docs/reviews/` — ADR-0013/0014/0015/0016/0017 tragen je ein
  `*-adr-00NN-text-review.md` (Reviewer ≠ Autor) **vor** Accept; ADR-0017 §Geschichte Z. 407–409
  dokumentiert Review + Projektinhaber-Durchsicht als Accept-Vorbedingung.
- **Befund:** Der Plan bucht weder das Review-Artefakt noch den Aufwand; zugleich ist ADR-0021 nach
  Accept immutabel (§2.5) und trägt — s. HIGH-2 — eine inhaltlich noch nicht korrekt geschnittene
  Aussage. Ein Accept ohne Review bräche die Präzedenz an der teuersten Stelle.
- **Empfohlene Auflösung:** ADR-0021 als **Proposed** anlegen, Text-Review-Artefakt in die
  Plan-Tabelle §3 aufnehmen und den Accept aus dem Slice-Umfang herausnehmen (oder den Slice
  ausdrücklich um diesen Schritt erweitern). Alternativ prüfen, ob der Verzicht — korrekt geschnitten —
  überhaupt eine eigene ADR braucht (s. INFO-5).

---

## LOW

### LOW-1 — „Reihenfolge D → A ist **zwingend**" ist überzogen
**Fundstelle:** Plan Amendment-Kasten Z. 29, Teil C Z. 230, §6 Z. 311–313.
**Geprüfte Quelle:** `AGENTS.md` Z. 153 (`make arch-check` ist **dokumentiert**) + d-check-Handbuch §5
(`exempt-targets` adressiert nur die `gate-undocumented`-Richtung).
**Befund:** `arch-check` ist real **und** dokumentiert — es wäre in **keiner** Reihenfolge ein
`exempt-targets`-Kandidat. Läuft A zuerst, bleiben Regel und Tabellenzeile schlicht bis D bestehen und
verschwinden dort **gemeinsam** (ein Commit) — grün in beiden Ordnungen. Real ist nur: die Baseline
(Rest-Risiko #4) müsste zweimal gefahren werden. **Auflösung:** „zwingend" → „empfohlen (spart die
zweite Baseline); Bedingung ist lediglich, dass Makefile-Regel und AGENTS-Zeile im **selben** Commit
fallen".

### LOW-2 — Die Aufzählung der vom Tombstone gedeckten Referenzen ist unvollständig
**Fundstelle:** Plan Teil C Z. 218–220 („Damit gedeckt **ohne** Fremd-Edit: …").
**Geprüfte Quelle:** Repo-Grep auf `tools/arch-check.sh`: zusätzlich `docs/plan/adr/0002-geometrie-kern-opencascade.md`
Z. 80 (**Markdown-Link** → `links`/`target-missing`, ebenfalls Accepted/immutabel),
`docs/plan/adr/README.md` Z. 62 und mehrere `docs/reviews/**` (Inline-Code → `codepaths`).
**Befund:** Mechanisch gedeckt (der `refs`-Glob greift **ziel**-seitig, quellen-unabhängig), aber die
Aufzählung erweckt den Eindruck von Vollständigkeit. **Auflösung:** Aufzählung ergänzen oder als
„u. a." kennzeichnen.

### LOW-3 — Die Hebung der `codepaths.ignore-refs`-Alias-Liste weitet **bestehende** Tombstones still auf `links`/`anchors`
**Fundstelle:** Plan Teil C Z. 215–217.
**Geprüfte Quelle:** d-check-Handbuch §5 (Alias „wirkt wie ein `ignore-refs`-Eintrag ohne `in`/`keep`,
**skopiert auf `codepaths`**") + `.d-check.yml` Z. 26–28 (heutige Alias-Einträge: `tools/Dockerfile`,
zwei `plan_geometry`-Pfade).
**Befund:** Nach der Hebung sind diese drei Ziele auch für `links`/`anchors` stumm — eine
Neben-Lockerung, die niemand beschlossen hat. **Auflösung:** entweder die Alias-Liste stehen lassen und
**einen** neuen Top-Level-Eintrag nur für die drei Skripte anlegen, oder die Weitung bewusst im
`.d-check.yml`-Kommentar benennen.

### LOW-4 — D8 lässt die `make a-check`-Zeile in `harness/README.md` (Z. 73) aus
**Fundstelle:** Plan D8: „`harness/README.md` Z. 22/74/79".
**Geprüfte Quelle:** `harness/README.md` Z. 73 = `make a-check`-Vertragszeile (Pendant zu AGENTS.md
Z. 152, die D8 sehr wohl erweitert).
**Befund:** Asymmetrie — nach dem Retire trägt nur AGENTS.md die `constructs`/P1-Erweiterung.
**Auflösung:** Z. 73 in D8 aufnehmen.

### LOW-5 — ADR-0021 fällt **nicht** unter die `matrix`-Grandfathering-Ausnahme
**Fundstelle:** Plan D6 / Tabelle §3 Z. 257.
**Geprüfte Quelle:** `.d-check.yml` `matrix.exempt-paths`
(`docs/plan/adr/000[1-9]-*.md`, `docs/plan/adr/001[0-7]-*.md`) + Regel `{from: adr, to: slice,
allow: false}` mit `token: 'slice-\d{3}'` (MR-014).
**Befund:** Eine neue ADR 0021 ist **nicht** exempt: jedes bare `slice-050` im ADR-Körper wirft
`matrix`-Befunde (Ausweg nur der Zeilen-Marker `<!-- d-check:status-provenance -->`). Der Plan warnt
nicht davor, obwohl die ADR den Slice-Anlass naheliegenderweise nennen will.
**Auflösung:** Hinweis in D6 aufnehmen (Slice-Bezug in den **mutablen** ADR-Index, nicht in den Körper).

---

## INFO

- **INFO-1 — Versions-/Digest-Angaben stimmen.** `version.md#aktuell`: v0.16.0, 2026-07-25,
  `@sha256:aef28cfe25bb054b1b0eb28420222a45b9f6ce9425b7ffd0f55e6ae56f295b56` — byte-gleich mit D1.
  Handbuch-Änderungshistorie 1.33 bucht `constructs`/`construct-leak` und „**Ausgeliefert mit v0.16.0**";
  1.34/1.35 (Ziel-Glob-Schattenwurf, Abdeckungs-Hinweis) ebenfalls v0.16.0 — die Plan-Zitate sind
  quellentreu.
- **INFO-2 — Beide Vorproben reproduziert.** Voll-Repo (read-only) gegen v0.16.0 mit **unveränderter**
  Config: `gesamt: 0 Befund(e)`, Exit 0, **kein** Abdeckungs-Hinweis (der Baum ist heute vollständig
  von `layers`/`composition_root` gedeckt — die D1-Behauptung trägt). Mit dem D2-`constructs`-Block:
  ebenfalls 0 Befunde. Beim heute gepinnten v0.13.0 ohne den Block: 0 Befunde (kein Regress).
- **INFO-3 — D7 ist wirksam und deckt mehr als geschrieben.** Fixture-Lauf: `construct-leak` feuert je
  einmal für fremden Adapter, `plugins/`-Baum und `src/main.cpp` (mit dem Zusatz
  „(composition_root: forbid)" in der Meldung) — und zusätzlich für den **Kern**. Empfehlung: den
  Kern-Fall (`src/hexagon/…`) als vierten Probe-Ort aufnehmen (ADR-0017 P1 nennt ihn ausdrücklich),
  die Probe mit der gehärteten Zone (MED-1) fahren und um den Nachbar-Verzeichnis-Fall ergänzen.
- **INFO-4 — Zeilennummern/Pfade stichprobenweise geprüft, alle korrekt:**
  `.devcontainer/Dockerfile` Z. 91–96 (arch-check-Stage) · `Makefile` Z. 32/62–63/202 ·
  ADR-0017 Z. 314 (Kommentar-Nachzug) · `spec/architecture.md` Z. 175 („`make arch-check`, geplant") ·
  `spec/spezifikation.md` Z. 579/638/677 · `README.md` Z. 24/143 · `harness/README.md` Z. 22/74/79 ·
  `AGENTS.md` Z. 151/153/158 · `harness/conventions.md` Z. 442/455/468 (existieren — aber s. HIGH-3) ·
  CHANGELOG Z. 301 (arch-check-Gegenprobe als Muster). MR-021 ist frei (höchste vergebene: MR-020).
- **INFO-5 — §2.6-Behandlung: konservativ, mit HIGH-2 aber zwingend.** Für den *ursprünglich*
  beschriebenen Verzicht ((i)/(ii), beide **über** dem ADR-Wortlaut) wäre die MR-013-Lesart
  („kein ADR, weil nichts gelockert wird" — wörtlich verifiziert) vertretbar gewesen; die ADR ist dort
  Überabsicherung, kein Fehler. Sobald der Verzicht korrekt geschnitten ist (HIGH-2: die **Allowlist**
  des P2-Wortlauts fällt), ist §2.6 unstrittig einschlägig und ADR-0021 **erforderlich** — mit
  entsprechend breiterem Text. ADR-0017 selbst bleibt korrekt unangetastet (§2.5).
- **INFO-6 — „neue MR statt Ergänzung an MR-013" ist praxis-konform.** MR-012 zu MR-010 wörtlich:
  „nach Aufnahme inhaltlich unveränderlich … Nachzug per neuem Eintrag (statt In-Place-Edit),
  Lineage-Pointer … analog MR-003 → MR-007". Die Amendment-Korrektur des Ur-Plans („Ergänzung oder
  neue MR" → **neue** MR-021) ist damit richtig. Ebenso trägt der Tombstone-Weg: d-check-Handbuch §5
  nennt genau diesen Fall („ein bewusst entfernter Pfad, den immutable/historische Doku … noch
  zitiert"), `ignore-refs` ist seit v0.49.0 Top-Level und wird von `links`, `anchors` **und**
  `codepaths` gemeinsam honoriert; ohne Eintrag meldet ein fehlendes Ziel weiter. Nach dem Löschen
  meldeten sonst **`links`/`target-missing`** (die Markdown-Links in `harness/conventions.md` Z. 442
  und `docs/plan/adr/0002-…` Z. 80) und **`codepaths`/`codepath-missing`** (Inline-Code in
  conventions.md, ADR-0017 Z. 314, ADR-Index, `docs/reviews/**`, `README.md` Z. 24, `done*/**`,
  im Plan selbst) — `codepaths.roots` enthält `docs`, `harness`, `tools`, `spec`, `src`.
- **INFO-7 — Kosmetische Reste.** `.claude/settings.local.json` führt Allowlist-Einträge für
  `make arch-check` / `bash tools/arch-check.sh` (Z. 53/61/98) — nach dem Retire tote Einträge, kein
  Gate betroffen. Es existiert **kein** `.github/`-Workflow, der nachzuziehen wäre; die
  CI-Befehlsliste lebt in AGENTS.md/README (von D8 abgedeckt).

---

## Bestätigt tragfähig (empirisch)

- **P1 ist durch `constructs` scan-identisch abgedeckt.** `languages.cpp`
  (`src/**/*.{cpp,h}`, `plugins/**/*.{cpp,h}`) und der `grep`-Umfang von `tools/arch-check.sh`
  (`src plugins --include='*.cpp' --include='*.h'`) sind **dieselbe** Dateimenge; `plugins/`-Baum ✔,
  Composition Root via `composition_root: forbid` ✔ (Meldung nennt es explizit), Kern ✔, fremder
  Adapter ✔; `dlmopen` ist vom Muster erfasst; RE2-`\b` funktioniert. **Zwei** Ausnahmen, beide oben
  gebucht: Kommentare (MED-2) und die nicht-verankerte Zone (MED-1).
- **Der `dlfcn.h`-Include** (die andere P1-Hälfte des ADR-Wortlauts) liegt unverändert bei
  a-check `tech` — inkl. `plugins/`-Baum (Fixture-belegt).
- **Alle positiven Zeilen der Fähigkeits-Tabelle** (Adapter-Header, Kern-Umgehung, Qt/OCC/SQLite/dlfcn
  in Quote- **und** Angle-Form) sind reproduziert; auch src/plugin_api ist über `edges` kanten-gegatet
  (`wrong-direction`), obwohl die Schicht dort keine `role` trägt.
- **`constructs` kann P2 strukturell nicht ersetzen** (Monopol vs. inverse Zone) — die Begründung des
  Plans und die Ablehnung einer Pseudo-Zone als Gate-Gaming (Präzedenz Option C in slice-043) sind
  korrekt.
- **Kein Regress aus dem Versionssprung:** Voll-Repo gegen v0.16.0 mit unveränderter Config = 0.

## Fazit

Die **positiven** Fähigkeits-Behauptungen von Teil D halten der Empirie stand — a-check v0.16.0
trägt das `dlopen`-Aufruf-Monopol vollständig, inklusive `plugins/`-Baum und Composition Root.
Blockierend sind drei andere Dinge: der Pin-Bump zeigt auf die **wirkungslose** Datei und ließe
`make gates` mit Exit 2 rot laufen (HIGH-1, empirisch mit dem real gepinnten Image reproduziert);
die Kernaussage „der P2-**Wortlaut** bleibt vollständig gegatet" ist **falsch** — mit arch-check.sh
fällt die *geschlossene Allowlist* von Regel P2, nicht nur zwei sub-kante Exoten (HIGH-2), was den
Zuschnitt der nach Accept immutablen ADR-0021 direkt betrifft; und D8 verlangt genau die MR-013-Edits,
die Teil C zuvor als unzulässig verworfen hat (HIGH-3). Die MED-Ebene ist mechanisch (ein Schrägstrich,
eine falsche Beispiel-Fundstelle, drei vergessene Nachzugs-Orte, das fehlende ADR-Text-Review); die
LOW-Ebene enthält zwei Stellen, an denen der Plan **strenger/aufwendiger** ist als nötig
(Reihenfolge-„zwingend", Alias-Hebung). Nach Einarbeitung der HIGHs ist Teil D startbar.
