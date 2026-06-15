# ADR-0005: Drittanbieter-Lizenz-Attribution & Auslieferungs-Layout

**Status:** Proposed

**Datum:** 2026-06-08

**Autor:** Dietmar Burkard

**Bezug:** [LH-QA-007](../../../spec/lastenheft.md) (vorgeschlagen, noch nicht im Lastenheft), [ADR-0002](0002-geometrie-kern-opencascade.md) (löst dessen offene Lizenz-Notiz), [ADR-0004](0004-toolchain-dependency-pinning.md) (gepinnte Dep-Versionen), [slice-006](../planning/open/slice-006-drittanbieter-attribution.md), [`releasing.md`](../../user/releasing.md)

---

## Kontext

b-cad linkt gegen Drittanbieter-Bibliotheken mit Attributions- und
Lizenztext-Pflichten bei der Distribution:

| Dep | Lizenz | Pflicht bei Auslieferung |
|---|---|---|
| OpenCascade (OCC) | LGPL-2.1-only + **OCCT-exception** | Lizenztext + Exception-Text + Relink-Möglichkeit |
| Qt 6 | LGPL-3.0-only | Lizenztext + Relink-Möglichkeit |
| MIT-Teile (Tooling/Libs) | MIT | Copyright + Lizenztext |
| SQLite | Public Domain | keine (Hinweis genügt) |

Aktuell trägt **kein Build** eine Attribution.
[ADR-0002](0002-geometrie-kern-opencascade.md) hat „Lizenz (GPL-Teile)
prüfen" als **offene Notiz** hinterlassen;
[`releasing.md`](../../user/releasing.md) schiebt „Signierung /
Distribution" als „spätere Welle" weg. Beides ist hier aufzulösen.

Zwei Randbedingungen prägen die Entscheidung:

1. **C++ hat kein verlässliches Dep-Manifest** (anders als npm/Cargo).
   Gegen System-/binär gelinkte Libs liefert ein reiner Quell-Scanner
   kein vollständiges Inventar — er sieht nur, was als Quelle vorliegt.
2. **Die Versionen sind via [ADR-0004](0004-toolchain-dependency-pinning.md)
   gepinnt** (Snapshot + Digest). Damit ist die Attributions-Grundlage
   (welche Dep in welcher Version) deterministisch und reproduzierbar.

## Entscheidung

1. **Tooling-Triade.** Ein **kuratiertes Dep-Manifest**
   (`tools/licenses-manifest.*`: Dep → Version → SPDX-ID → Quelle) ist die
   **Single Source of Truth**. **REUSE** füllt die kanonischen
   SPDX-Volltexte nach `LICENSES/`. **ScanCode Toolkit** läuft nur zur
   **Gegenkontrolle** (meldet im Quellbaum erkannte SPDX-Expressions, die
   im Manifest fehlen) — nicht als Inventar-Quelle.
2. **Auslieferungs-Layout** (via CMake-`install()`, erzeugt in
   `make fullbuild`):

   ```
   dist/
   ├── bin/b-cad
   └── share/doc/b-cad/
       ├── LICENSE                  (eigenes Projekt, MIT)
       ├── NOTICE                   (generiert: Copyright-Statements)
       ├── THIRD_PARTY_LICENSES.md  (generiert: Dep → SPDX → Textverweis)
       └── LICENSES/                (kanonische Volltexte)
   ```
3. **OCCT-exception** ist **kein SPDX-Standard-Identifier** → der Volltext
   wird **manuell** als `LICENSES/OCCT-exception.txt` gepflegt und im
   Manifest als Sonderfall markiert (REUSE kann ihn nicht ziehen).
4. **Gate.** `make license-check` prüft **Allowlist** (nur erlaubte
   SPDX-Expressions) **und Vollständigkeit** (jede gelinkte Dep hat
   Manifest-Eintrag + Volltext). Es wird mit slice-006 real und in
   AGENTS §3 von „Geplant" nach „Real" promotet.

## Verglichene Alternativen

### Option A — Manifest-führend + REUSE-Volltexte + ScanCode-Gegenkontrolle (gewählt)

- Pro: deterministisch und C++-tauglich (kein Verlass auf Quell-Scan gegen gelinkte Binär-Deps); reproduzierbar an den ADR-0004-Pin gekoppelt; gate-prüfbar.
- Contra: Manifest muss bei Dep-/Versions-Änderung manuell gepflegt werden (vom Gate erzwungen).

### Option B — reiner ScanCode-Auto-Scan als Wahrheit

- Pro: minimale Handarbeit.
- Contra: findet system-/binär gelinkte Deps ohne vorliegende Quelle nicht → bei C++ unvollständig/irreführend. Erst tragfähig mit Source-basiertem Dep-Manager + Lockfile.

### Option C — vollständig manuelle Attribution (Texte + NOTICE von Hand)

- Pro: kein Tooling.
- Contra: driftet gegen die real gelinkten Deps, nicht prüfbar, kein Gate — widerspricht der Sensor-Disziplin.

## Konsequenzen

- Positiv: LGPL-/Attributions-Pflichten werden erfüllt und **prüfbar**
  (Gate); Ergebnis ist reproduzierbar (an ADR-0004-Snapshot gekoppelt);
  die offene Lizenz-Notiz aus ADR-0002 ist geschlossen.
- Folgepflicht: **[LH-QA-007](../../../spec/lastenheft.md)** (Attributions-Pflicht, abnahmebindend) muss
  ins Lastenheft. Umsetzung in **slice-006**, getriggert durch ein real
  gelinktes Release-Binary (ohne Binary ist Attribution Fiktion).
- Folgepflicht: Bei Dep-Zuwachs oder Snapshot-Bump aus ADR-0004 muss das
  Manifest nachgezogen werden — `license-check` macht die Drift sichtbar.
- Der **LGPL-Relink-Hinweis** (Angebot von Objektdateien/Quelltext zur
  Neuverknüpfung) ist im `NOTICE` zu adressieren — Detail in slice-006.

## Fitness Function

| Tooling | Regel | Make-Target (geplant) |
|---|---|---|
| Vollständigkeit | jede gelinkte Dep hat Manifest-Eintrag **und** `LICENSES/`-Volltext | `make license-check` (slice-006) |
| Allowlist | keine SPDX-Expression außerhalb der erlaubten Menge | `make license-check` |
| Gegenkontrolle | ScanCode meldet keine erkannte SPDX-Lizenz, die im Manifest fehlt | `make license-check` |

## Re-Evaluierungs-Trigger

- Neue Dep mit nicht-allowlisteter Lizenz (GPL-strikt, kommerziell, …) → ADR-Update + Carveout, nicht PR-Kommentar (AGENTS §2.6).
- Wechsel zu Source-Dep-Manager mit Lockfile (vcpkg/Conan) → Option B neu bewerten (Auto-Inventar dann tragfähig).
- Versions-/Snapshot-Bump aus ADR-0004 → Manifest + Volltexte nachziehen.

## Geschichte

| Datum | Ereignis | Verweis |
|---|---|---|
| 2026-06-08 | Proposed | slice-006, ADR-0002 (offene Lizenz-Notiz), ADR-0004 |
