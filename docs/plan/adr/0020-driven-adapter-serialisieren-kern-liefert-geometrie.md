# ADR-0020: Driven-Adapter serialisieren, der Kern liefert abgeleitete Geometrie (Geometrie-Bereitstellung statt Adapter-Ableitung)

**Status:** Proposed

**Datum:** 2026-07-22

**Autor:** Dietmar Burkard (Architektur-Reinheit; ausgearbeitet im AI-Harness-Lauf)

**Bezug:** [ADR-0001](0001-hexagonale-architektur.md) (Schichtung — diese ADR **schärft** die Adapter-Grenze: driven Adapter reichen nicht in den Kern-Rechen-Kernel), [ADR-0002](0002-geometrie-kern-opencascade.md) (OCC = Geometrie-Backend; die OCC-Montage bleibt legitim im Adapter), [ADR-0014](0014-step-stl-export-backend.md) (STEP/STL-Export — die Haupt-Betroffenen), [ADR-0016](0016-pdf-png-backend.md) (PDF/PNG — die 2D-Projektions-Konsumenten), [ADR-0018](0018-drw-2d-zeichen-daten.md) (DRW-Daten), [ADR-0019](0019-drw-2d-canvas.md) (2D-Canvas — baut auf dieser ADR auf; die dort noch offene `io → services_geo`-Kante **entfällt** unter diesem Prinzip), [OBJ-005](../../../spec/lastenheft.md#3-projektziele) (offene Austauschformate)

---

## Kontext

Die Schicht-Konvention ([ADR-0001](0001-hexagonale-architektur.md), `architecture.md` §2) lautet: **Adapter kommunizieren mit dem Kern über Ports; sie greifen nicht in Kern-Interna.** Heute wird das an einer Stelle verletzt: **driven Export-/Persistenz-Adapter leiten Domänen-Geometrie selbst ab**, indem sie die puren Helfer unter `src/hexagon/services/geometry/` (die »Berechnungs-Kerne«-Sub-Schicht) direkt aufrufen. Die `.a-check.yml`-Allow-Liste führt dafür zwei Kanten — `geometry → services_geo` (STEP/STL-Export) und `persistence → services_geo` (ein abgeleiteter Skalar).

Diese Kanten wurden bei der Einführung der Sub-Schicht **hinzugefügt, um eine bestehende Code-Praxis zu legalisieren** (»die Tabelle an den Code angepasst«), statt die Praxis zu beseitigen. Das ist das Anti-Muster »Regel lockern statt Verletzung fixen«: es macht Adapter-Zugriff auf den Kern-Rechen-Kernel dauerhaft legal, und das normative `architecture.md`-§1-Komponenten-Diagramm (das reines Hexagonal ohne Adapter→Kernel zeigt) wird dadurch **unwahr** — es zeigt ein Bild, das die §2-Tabelle widerlegt. Der 2D-Canvas ([ADR-0019](0019-drw-2d-canvas.md)) drohte, das Muster mit einer dritten Kante fortzuschreiben.

**Befund aus der Code-Kartierung (grundlegend für die Entscheidung):**

1. **Die Ableiten→OCC-Grenze ist bereits scharf.** Jede `services/geometry`-Funktion gibt einen **puren Werttyp** zurück (`model::Footprint`, `model::TriangleMesh`, `model::CutPrism`, `double`, oder das `StepBox`-Struct); nichts OCC-Artiges (`TopoDS_Shape`) entsteht in `services/geometry`. Die OCC-Montage ist vollständig in `src/adapters/geometry/occ_solids.*` isoliert (`makeNetSolid`/`meshToSolid`/`makeBoxSolid` + Compound-Bau + `STEPControl_Writer`).
2. **Der Kernel-Adapter ist schon sauber.** `occ_geometry_adapter` (erfüllt `GeometryKernelPort`) importiert **kein** `services/geometry`; er bekommt pure Primitive (`Footprint`+`CutPrism[]`) und liefert `Solid`/`TriangleMesh`. Nur die **Export**-Adapter greifen zum Ableiten in den Kernel.
3. **Die 2D-Projektion ist io-resident (kein heutiger Verstoß).** `plan_geometry.projectPlan(Building) → PlanView` lebt im io-Adapter; DXF/PDF/PNG nutzen sie **intra-Adapter**. Eine `io → services_geo`-Kante existiert **nicht** — sie entstünde nur beim Verschieben in den Kern. Unter diesem Prinzip wird sie **vermieden**.
4. **Ein Typ-Wrinkle:** `StepBox` ist im `services`-Namespace (`stair_geometry.h`) deklariert, **nicht** in `model/`. Ein Port-Vertrag darf nicht auf einen `services/geometry`-Typ hängen → `StepBox` muss nach `model/`.
5. **Formate divergieren.** STEP will **extrudierbare Eingaben** (`Footprint`+`CutPrism[]`+Skalare) + Treppen als `StepBox[]`; STL will dieselben Wand/Decken-Eingaben **tesselliert** (über den bestehenden `GeometryKernelPort`) + Treppen als `TriangleMesh` (mit render-only Geländer); DXF/PDF/PNG wollen die `PlanView`; die Persistenz einen `double` je Treppe. Ein **einziges** Bündel muss diese Vielfalt tragen (oder format-selektiv befüllt sein).

## Entscheidung

**1. Prinzip — driven Adapter serialisieren, sie leiten keine Domänen-Geometrie ab. Der Kern berechnet alle Modell-Geometrie-Ableitungen und stellt sie als pure Werttypen bereit.** `services/geometry` wird **kern-privat**: nur der Kern (`services → services_geo`) ruft die Berechnungs-Kerne; **kein Adapter** importiert sie mehr. Ein driven Adapter erhält die abgeleitete Geometrie als pure Werte und tut nur noch (a) format-spezifische **Serialisierung** und (b) die **Backend-Montage, die zwingend das Backend braucht** (OCC-B-Rep-Bau aus puren Primitiven, Tessellation über den bestehenden `GeometryKernelPort`) — beides legitim adapter-resident, weil es OCC/Format-Wissen erfordert, **nicht** Modell-Ableitung ist.

**2. Bereitstellungs-Mechanismus — ein pures `DerivedGeometry`-Bündel, kern-seitig berechnet, über den Port an den Adapter gereicht.** Der Export-Use-Case-Service (`ExchangeService`) berechnet vor dem Dispatch die **format-relevante** abgeleitete Geometrie (er ruft `services/geometry`, kern-seitig) und übergibt sie dem Exporter; der `ModelExporterPort`-Vertrag wächst um das Bündel (`write(const Building&, const DerivedGeometry&, path)`). Das Bündel ist ein **pures Werttyp-Aggregat** (per Element die Primitive: Wand `{Footprint, height, CutPrism[]}`, Decke `{Footprint, thickness, CutPrism[], baseZ}`, Dach `TriangleMesh`, Treppe `{StepBox[], TriangleMesh, rise}`, plus `PlanView` für die 2D-Formate). **Format-selektiv befüllt** (der Service füllt nur, was das Format braucht — der Exporter deklariert seinen Bedarf) → keine verschwendete Rechnung, **ein** uniformer Vertragstyp, **kein** Varianten-Wildwuchs.
   - **`StepBox` wird nach `model/` gehoben** (Werttyp `model::StepBox`), damit der Port-Vertrag frei von `services/geometry`-Typen ist.
   - **OCC bleibt im Adapter.** Der Adapter bekommt **pre-OCC**-Primitive (kein `TopoDS_Shape` kann einen neutralen Port kreuzen) und baut daraus über `occ_solids`/`GeometryKernelPort` die Solids — genau die heutige `occ_solids.h`-Montage, nur ohne die vorgelagerten `services::`-Aufrufe.
   - **Persistenz:** `ProjectRepositoryPort::save` erhält die abgeleiteten Skalare analog (der Kern liefert den `rise` je Treppe), **oder** — da es nur ein write-abgeleiteter, nie zurückgelesener Wert ist — wird der `rise` kern-seitig vor `save` gesetzt. Der Impl-Slice wählt die schlankere Form; beide entfernen den `persistence → services_geo`-Import.
   - **2D:** `projectPlan` + `PlanView` ziehen in den Kern (`src/hexagon/services/geometry/{plan_geometry}` — die etablierte lib-freie Helfer-Heimat); die 2D-Exporter erhalten die `PlanView` im Bündel (sie **holen** sie nicht) → **keine** `io → services_geo`-Kante. Der **[ADR-0019](0019-drw-2d-canvas.md)-Canvas** zieht dieselbe Projektion über den `PlanViewPort` (lebendes Modell). Eine Quelle, drei Konsum-Wege (Export-Bündel, Canvas-Port, kern-intern), **null** Adapter→Kernel-Kanten.

**3. Kanten-Entfernung + `architecture.md`-Wahrheit.** Der Refactor entfernt **beide** bestehenden `.a-check.yml`-Kanten (`geometry → services_geo`, `persistence → services_geo`) und fügt die hypothetische io-Kante **gar nicht erst** hinzu. `services_geo` behält als einzige Eingangskante `services → services_geo` (kern-intern). **`architecture.md` wird korrigiert — Tabelle UND Diagramm:** §2 streicht `services/geometry` aus den »Darf importieren«-Spalten der Geometrie-/Persistenz-Adapter (und die IO-Zeile bekommt es nie); das §1-Komponenten-Diagramm wird damit **wahr** (reines Hexagonal, **kein** Adapter→Kernel — das Bild, das es heute schon zeigt, stimmt dann). Dies ist eine **Verschärfung** (Allow-Liste **verengt**, [AGENTS.md §2.6](../../../AGENTS.md) n/a — keine Lockerung).

## Verglichene Alternativen

### Zu 1: Kern liefert Geometrie (gewählt) vs. Status quo (Adapter leiten ab, Kanten bleiben) vs. voll-tessellierte Solids liefern

- **Kern liefert pure Primitive (gewählt) — Pro:** driven Adapter werden echte Serialisierer; `services/geometry` kern-privat; alle Adapter→Kernel-Kanten fallen; `architecture.md`-§1-Diagramm wird wahr; die Ableiten→OCC-Grenze ist schon scharf, also **kein Umbau der Geometrie-Logik**, nur ein **Verschieben des Aufruf-Orts** (Kern statt Adapter) + Port-Vertrag. **Contra:** der `ModelExporterPort`-Vertrag + `save` wachsen; `StepBox` muss nach `model/`; mehrere Refactor-Slices.
- **Status quo — Contra (entscheidend):** legalisiert Adapter→Kernel dauerhaft; hält das normative §1-Diagramm unwahr; der Canvas würde eine dritte Kante hinzufügen. **Verworfen** (der Anlass dieser ADR).
- **Kern liefert fertige Solids/`TopoDS_Shape` — Contra (unmöglich):** ein `TopoDS_Shape` ist OCC-typisiert und **kann keinen neutralen Port kreuzen** (Kern bliebe OCC-frei nur, wenn OCC in den Kern zöge — [ADR-0002](0002-geometrie-kern-opencascade.md)-Bruch). Daher muss das Bündel **pre-OCC**-Primitive tragen und der Adapter die OCC-Montage behalten. **Verworfen.**

### Zu 2: Format-selektives `DerivedGeometry`-Bündel (gewählt) vs. Superset-Bündel vs. Varianten-Typen je Format

- **Selektiv befülltes uniformes Bündel (gewählt) — Pro:** ein Vertragstyp (keine Varianten-Komplexität); der Service füllt nur die format-relevanten Teile (Exporter deklariert Bedarf) → keine verschwendete Rechnung; STEP/STL-Divergenz (Boxen vs. Geländer-Mesh) wird durch **beide** Treppen-Felder im Bündel getragen, jeder Exporter liest seins. **Contra:** ein Bündeltyp mit optionalen/leeren Feldern.
- **Superset immer voll berechnen — Contra:** berechnet `PlanView` für STEP und `StepBox[]` für PDF (Verschwendung), sonst wie gewählt. **Verworfen** (unnötige Rechnung).
- **Varianten-Typ je Format — Contra:** Typ-Wildwuchs + Dispatch-Komplexität für wenig Gewinn. **Verworfen.**

## Konsequenzen

- **Positiv:** Die Schichtung wird **prinzipientreu** — driven Adapter serialisieren nur; `services/geometry` ist kern-privat; **alle** Adapter→Kernel-Kanten fallen; das normative `architecture.md`-§1-Diagramm wird **wahr**; der [ADR-0019](0019-drw-2d-canvas.md)-Canvas baut auf eine saubere Basis (seine `io → services_geo`-Frage dissolviert). Die Geometrie-Ableitungs-**Logik** ändert sich **nicht** (die Helfer sind schon pur) — nur ihr **Aufruf-Ort** (Kern) und die **Naht** (Port-Bündel).
- **Negativ / Folgepflicht (Slices, Nummern im [ADR-Index](README.md)):** (a) **`model::StepBox`** + `DerivedGeometry`-Werttyp(en) im Kern; **`ModelExporterPort::write`**-Vertrag + `ExchangeService`-Berechnung; Exporter-Bedarfs-Deklaration. (b) **2D-Projektions-Slice:** `projectPlan`/`PlanView` nach `services/geometry` + `PlanViewPort` + DXF/PDF/PNG auf das Bündel + Canvas-Naht (**entsperrt [ADR-0019](0019-drw-2d-canvas.md)**; die 2D-Export-Decode-Orakel als Netz). (c) **STEP/STL-Slice:** die Export-Adapter auf das Bündel umstellen (die `occ_solids`-Montage bleibt; die `services::`-Aufrufe wandern in die Kern-Berechnung); die STEP/STL-Round-Trip-/B-Rep-Orakel als Netz. (d) **Persistenz-Slice:** `rise` kern-seitig; `stair_geometry`-Import raus. (e) **`.a-check.yml` + `architecture.md` §1+§2:** Kanten raus, Diagramm/Tabellen wahr. [MR-006](../../../harness/conventions.md#mr-006--unabhängiges-plan-review-vor-implementierungs-start) vor jedem Impl-Start; unabhängiges Code-Review (Export-/Persistenz-Latte: die Orakel müssen die Byte-Identität der Export-Artefakte über den Refactor halten).
- **Keine neue Gate-Regel; im Gegenteil — eine Verschärfung.** Die Allow-Liste **verengt** sich (zwei Kanten raus, die dritte nie rein); `architecture.md` wird **strenger** und **wahrer**. [AGENTS.md §2.6](../../../AGENTS.md) n/a (keine Lockerung). `a-check` erzwingt danach die Adapter→Kernel-Freiheit maschinell.
- **Reihenfolge-Sicherheit (Refactor-Netz):** jeder Slice ist ein **verhaltens-invarianter Umzug** (die Geometrie-Logik ist unverändert) — die bestehenden Export-Decode-/Round-Trip-Orakel (DXF/PDF/PNG Erscheint/Fehlt, STEP/STL B-Rep-Zahl, IFC/DXF-Round-Trip) müssen **byte-/struktur-gleich grün** bleiben. Reißt ein Orakel, ist der Umzug nicht verhaltens-invariant → Stopp.
- **[ADR-0001](0001-hexagonale-architektur.md)/0002/0014/0016/0018/0019 bleiben gültig** — diese ADR schärft die Adapter-Grenze aus [ADR-0001](0001-hexagonale-architektur.md), lässt die OCC-Montage ([ADR-0002](0002-geometrie-kern-opencascade.md)) im Adapter, und macht [ADR-0019](0019-drw-2d-canvas.md)s 2D-Lese-Naht sauber.

## Fitness Function

| Tooling | Regel | Make-Target |
|---|---|---|
| Schichtung (Ziel) | **kein** Adapter importiert `services/geometry`; `services_geo` hat nur die Eingangskante `services → services_geo` | `make a-check` (nach Kanten-Entfernung) |
| Kern-Reinheit | `services/geometry` + `DerivedGeometry` + `model::StepBox` ziehen **keine** Library (Kern-Link-Barriere, [ADR-0001](0001-hexagonale-architektur.md)); kein `TopoDS_Shape` im Kern/Port ([ADR-0002](0002-geometrie-kern-opencascade.md)) | `make build` |
| Export-Invarianz je Slice | die Export-Decode-/Round-Trip-Orakel (DXF/PDF/PNG, STEP/STL, IFC/DXF-Round-Trip) bleiben nach dem Umzug **unverändert grün** (reiner Umzug) | `make test` |
| `architecture.md`-Wahrheit | §1-Diagramm + §2-Tabelle stimmen mit `.a-check.yml` überein (kein Adapter→Kernel) | Review (inferential) + `make docs-check` |

## Re-Evaluierungs-Trigger

- **Ein Format braucht eine Ableitung, die kein pures Werttyp-Primitiv ist** (z. B. ein Backend, das echte OCC-`TopoDS_Shape`-Zwischenschritte über die Naht bräuchte) → die Bündel-Grenze neu bewerten (ggf. das Format geometrie-resident wie STEP/STL, [ADR-0014](0014-step-stl-export-backend.md)).
- **Das `DerivedGeometry`-Bündel wird zu breit** (viele optionale Felder je neuem Format) → Varianten-/Bedarfs-Modell neu bewerten.
- **Ein Import-Adapter braucht Kern-Geometrie** (heute nur Export/Persistenz ableiten) → dieselbe Bereitstellungs-Regel auf den Import-Pfad anwenden.

## Geschichte

| Datum | Ereignis | Verweis |
|---|---|---|
| 2026-07-22 | Proposed (Architektur-Reinheit — driven Adapter serialisieren, Kern liefert abgeleitete Geometrie als pures `DerivedGeometry`-Bündel über den Port; `services/geometry` kern-privat; `StepBox` → `model/`; alle Adapter→Kernel-`.a-check.yml`-Kanten entfernt; `architecture.md` §1-Diagramm + §2-Tabelle werden wahr; löst das »Regel-lockern-statt-fixen«-Anti-Muster der Berechnungs-Kern-Sub-Schicht-Einführung auf; entsperrt den sauberen 2D-Lese-Weg für den DRW-Canvas) | Architektur-Reinheit / DRW-Interaktiv-Strang-Vorlauf |
