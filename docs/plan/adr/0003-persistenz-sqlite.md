# ADR-0003: Projekt-Persistenz mit SQLite, atomar geschrieben

**Status:** Accepted

**Datum:** 2026-06-08

**Autor:** Dietmar Burkard

**Bezug:** REQ-TEC-007, LH-FA-BLD-002, LH-FA-BLD-003, LH-FA-BLD-004, LH-QA-005, ADR-0001

---

## Kontext

Ein b-cad-Projekt (Geschosse, Bauteile, Material, Historie) muss
persistent gespeichert, vollständig wieder geladen und versioniert
werden (LH-FA-BLD-002..004). Datenverlust am Gebäudemodell ist der
schärfste Fehlerfall (Repo-Klasse, siehe
[`harness/conventions.md` §Repo-Klasse](../../../harness/conventions.md#repo-klasse)):
ein Absturz darf den letzten konsistenten Stand nicht zerstören
(LH-QA-005). Die Persistenz steht hinter `ProjectRepositoryPort`
(ADR-0001).

## Entscheidung

Wir wählen **SQLite** als Projektdatei-Format, gekapselt im Driven
Adapter `src/adapters/persistence/` hinter `ProjectRepositoryPort`.
Schreiben erfolgt **atomar** (Schreiben in Temp-Datei, dann Rename).

## Verglichene Alternativen

### Option A — Eigenes Binärformat

- Pro: kompakt, volle Kontrolle.
- Contra: Migrations-, Integritäts- und Teil-Lese-Logik komplett selbst;
  hohes Korruptionsrisiko.

### Option B — XML/JSON-Datei

- Pro: menschenlesbar, einfach zu diffen.
- Contra: bei großen Modellen langsam und speicherhungrig (LH-QA-001/002);
  keine transaktionale Konsistenz; partielles Lesen schwierig.

### Option C — SQLite (gewählt)

- Pro: eine Datei, transaktional, robust; partielles/abfragbares Lesen;
  Schema-Migrationen handhabbar; bewährt als Anwendungs-Dateiformat.
- Contra: Schema-Design und Migrationsdisziplin nötig; binäres Format
  (nicht direkt diffbar).

## Konsequenzen

- Positiv: Transaktionen geben Konsistenz; Historie (LH-FA-BLD-004) als
  Tabelle; gezielte Abfragen für Auswertungen (LH-FA-EVL-*) möglich.
- Negativ: Schema-Versionierung mit getesteter Aufwärts-Migration ist
  Pflicht (siehe [`spec/spezifikation.md` §2](../../../spec/spezifikation.md#2-datenstrukturen-und-schemas)).
- Folgepflicht: Atomares Schreiben (Temp + Rename) als Hard-Anforderung;
  Crash-Recovery-Test (LH-QA-005) `kill -9` zwischen Schreibphasen.

## Fitness Function

| Tooling | Regel | Make-Target |
|---|---|---|
| Architekturtest | SQLite-Header nur in `src/adapters/persistence/` | `make arch-check` **Regel D** (real, slice-008a) |
| Round-Trip | `save`→`load` ergibt feldgleiches Modell (BLD-002/003) | `make test` (real, slice-008a) |
| Crash-Recovery-Test | `kill -9` während `save` → nach Neustart letzter konsistenter Stand ladbar | `make test` (Recovery-Suite, **slice-008b**) |

## Re-Evaluierungs-Trigger

- Wenn Mehrbenutzer-/gleichzeitiger Zugriff (aktuell out-of-scope,
  Lastenheft §6) doch gefordert wird.
- Wenn Projektgrößen das RAM-/Performance-Budget (LH-QA-001/002) reißen.

## Geschichte

| Datum | Ereignis | Verweis |
|---|---|---|
| 2026-06-08 | Proposed (aus Architektur-Outline, Bootstrap) | spec/architecture.md §3 |
| 2026-06-09 | Accepted — umgesetzt in slice-008a: `ProjectRepositoryPort` + SQLite-Adapter (atomar via Temp+Rename), Round-Trip grün, arch-check Regel D. Crash-Recovery-Fitness (LH-QA-005) als Folgepflicht slice-008b | slice-008a |
