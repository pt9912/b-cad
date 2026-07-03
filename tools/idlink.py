#!/usr/bin/env python3
"""idlink — slice-018b: Link-Hygiene für die 7 ID-Familien (MR-011).

Normativer Korpus: nackte UND Inline-Code-IDs -> Markdown-Link auf die Definition.
Log-Dateien (CHANGELOG.md, docs/reviews/**, spec/*-historie.md): nackte IDs ->
Inline-Code (Backtick); Inline-Code bleibt (exempt-paths fängt es).
`done-archive/` ist ausgenommen. d-check validiert das Ergebnis (Orakel).

Anker (Weg B): ID mit eigenem ATX-Heading -> präziser Heading-Slug; Tabellen-/
Listen-ID -> umschließendes Kapitel; ADR -> Datei (kein Anker). Cross-Stratum:
das Ziel ist immer die kanonische Definitionsdatei der Familie, unabhängig von
der Fundstelle. Slug = d-check `Slugify` (anchors.go) inkl. Duplikat-Suffix.

Aufruf:  python3 tools/idlink.py --dry-run <datei.md> [...]
         python3 tools/idlink.py --apply  <datei.md> [...]
         python3 tools/idlink.py --apply-all
"""
import re, sys, os, glob, subprocess

ROOT = os.path.abspath(os.path.join(os.path.dirname(__file__), ".."))

# --- Familien: name, regex, target (Datei oder Verzeichnis mit '/') ---
FAMILIES = [
    ("ADR",    re.compile(r'ADR-\d{4}'),                       "docs/plan/adr/"),
    ("MR",     re.compile(r'MR-\d{3}'),                        "harness/conventions.md"),
    ("LH",     re.compile(r'LH-(?:FA-[A-Z][A-Z0-9]*|QA)-\d+'), "spec/lastenheft.md"),
    ("ACC",    re.compile(r'ACC-\d+'),                         "spec/lastenheft.md"),
    ("OBJ",    re.compile(r'OBJ-\d+'),                         "spec/lastenheft.md"),
    ("REQTEC", re.compile(r'REQ-TEC-\d+'),                     "spec/spezifikation.md"),
    ("E",      re.compile(r'E-(?:IO|VAL|GEO|PLG)-\d+'),            "spec/spezifikation.md"),
]
ANY = re.compile("|".join("(?:%s)" % f[1].pattern for f in FAMILIES))
TARGET = {n: t for n, _, t in FAMILIES}

def fam_of(idstr):
    for n, rx, _ in FAMILIES:
        if rx.fullmatch(idstr):
            return n
    return None

def in_target(file_rel, target):
    if target.endswith("/"):
        t = target.rstrip("/")
        return file_rel == t or file_rel.startswith(t + "/")
    return file_rel == target

# ---------------- d-check Slugify (Port von anchors.go:60) ----------------
def strip_heading_links(s):
    out, i = [], 0
    while i < len(s):
        if s[i] == '!' and i + 1 < len(s) and s[i + 1] == '[':
            i += 1; continue
        if s[i] != '[':
            out.append(s[i]); i += 1; continue
        depth, j, te = 0, i, -1
        while j < len(s):
            if s[j] == '[': depth += 1
            elif s[j] == ']':
                depth -= 1
                if depth == 0: te = j; break
            j += 1
        if te == -1 or te + 1 >= len(s) or s[te + 1] != '(':
            out.append(s[i]); i += 1; continue
        depth, k, de = 0, te + 1, -1
        while k < len(s):
            if s[k] == '(': depth += 1
            elif s[k] == ')':
                depth -= 1
                if depth == 0: de = k; break
            k += 1
        if de == -1:
            out.append(s[i]); i += 1; continue
        out.append(s[i + 1:te]); i = de + 1
    return "".join(out)

def slugify(heading):
    t = strip_heading_links(heading).lower()
    b = []
    for ch in t:
        if ch in '`*':
            continue
        if ch.isalpha() or ch.isdigit() or ch in '-_':
            b.append(ch)
        elif ch in ' \t':
            b.append('-')
    return "".join(b)

# ---------------- HTML-Anker (DC-FA-ANCH-001.b, d-check v0.9.0) ----------------
# `id="…"` an beliebigem Tag, `name="…"` nur an <a> — wörtlich (case-sensitiv).
HTML_ID   = re.compile(r'<[a-zA-Z][^>]*?\bid\s*=\s*"([^"]*)"')
HTML_NAME = re.compile(r'<a\b[^>]*?\bname\s*=\s*"([^"]*)"')

# ---------------- ID -> (target_file, anchor|None) ----------------
def build_map():
    idmap = {}
    # ADR: ID -> Datei (kein Anker)
    for p in glob.glob(os.path.join(ROOT, "docs/plan/adr/[0-9]*.md")):
        idmap["ADR-" + os.path.basename(p)[:4]] = ("docs/plan/adr/" + os.path.basename(p), None)
    # Anker-tragende Familien: aus den Definitionsdateien
    for tgt in ["spec/lastenheft.md", "spec/spezifikation.md", "harness/conventions.md"]:
        full = os.path.join(ROOT, tgt)
        if not os.path.exists(full):
            continue
        prec, sect, html = {}, {}, {}
        cur, counts, fenced = None, {}, False
        for line in open(full, encoding="utf-8"):
            if line.lstrip().startswith("```"):
                fenced = not fenced; continue
            if fenced:
                continue
            # Expliziter Inline-HTML-Anker (<a id/name>) -> präziser Per-ID-Anker.
            for av in HTML_ID.findall(line) + HTML_NAME.findall(line):
                fam = fam_of(av.upper())
                if fam and TARGET.get(fam) == tgt:
                    html[av.upper()] = av
            m = re.match(r'^#{1,6}\s+(.*?)\s*$', line)
            if m:
                base = slugify(m.group(1))
                n = counts.get(base, 0); counts[base] = n + 1
                cur = base if n == 0 else "%s-%d" % (base, n)
                for idm in ANY.finditer(m.group(1)):
                    # nur IDs, deren kanonisches Familien-Target DIESE Datei ist
                    # (sonst überschreibt die spezifikation-`.a` die lastenheft-Def)
                    if TARGET.get(fam_of(idm.group(0))) == tgt:
                        prec.setdefault(idm.group(0), cur)
            elif cur:
                for idm in ANY.finditer(line):
                    idv = idm.group(0)
                    if TARGET.get(fam_of(idv)) != tgt:
                        continue
                    # Definitions-Position: ID hinter Tabellen-Pipe/Bullet (Marker
                    # Pflicht), optional vorangestellter `<a …></a>`-Anker (slice-018c),
                    # optional Backtick/Fett (E-Codes stehen als `| `E-IO-001` |`)
                    if re.match(r'^\s*[|*+-]\s*(?:<a\b[^>]*>\s*</a>\s*)?(?:[`*]+)?'
                                + re.escape(idv) + r'\b', line):
                        sect.setdefault(idv, cur)
        for k, v in sect.items():
            idmap.setdefault(k, (tgt, v))
        for k, v in prec.items():
            idmap[k] = (tgt, v)   # Heading-präzise gewinnt
        for k, v in html.items():
            idmap[k] = (tgt, v)   # expliziter Per-ID-Anker gewinnt (höchste Präzedenz)
    return idmap

IDMAP = build_map()

def link_for(src_rel, idstr):
    """Markdown-Linkziel (rel#anchor) von src_rel aus. None wenn nicht auflösbar."""
    if idstr in IDMAP:
        tgt, anchor = IDMAP[idstr]
    else:
        # Fallback: ID matcht eine Familie, aber keine Definition gefunden
        # (z.B. undefiniertes LH-QA-007) -> Datei-Link auf das Familien-Target.
        t = TARGET.get(fam_of(idstr))
        if not t or t.endswith("/"):
            return None  # ADR/Verzeichnis ohne konkrete Datei: nicht auflösbar
        tgt, anchor = t, None
    rel = os.path.relpath(os.path.join(ROOT, tgt), os.path.dirname(os.path.join(ROOT, src_rel)))
    return rel + ("#" + anchor if anchor else "")

# ---------------- Span-Erkennung ----------------
LINK_RE = re.compile(r'!?\[[^\]]*\]\([^)]*\)')
CODE_RE = re.compile(r'`[^`]*`')

def spans(rx, line):
    return [(m.start(), m.end()) for m in rx.finditer(line)]

def covered(a, b, sps):
    return any(s <= a and b <= e for s, e in sps)

# ---------------- Zeilen-Transformation ----------------
def line_code_regions(line, code_open):
    """Inline-Code-Regionen der Zeile mit Carry-over über Zeilengrenzen
    (dokument-weit, fence-aware via process). Behebt die per-Zeile-Desync bei
    mehrzeiligen `…`-Spans (MED-1; analog d-check v0.9.0 dokument-weit).
    Gibt (regionen, offen_am_zeilenende)."""
    regions, cur = [], code_open
    seg = 0 if code_open else None
    for i, ch in enumerate(line):
        if ch == '`':
            if cur:
                regions.append((seg, i + 1)); cur = False
            else:
                seg = i; cur = True
    if cur and seg is not None:
        regions.append((seg, len(line)))  # offener Span läuft in die nächste Zeile
    return regions, cur

NORM_RE = re.compile(r'(?<!!)\[([^\]]+)\]\(([^)]+)\)')

def transform_line(line, src_rel, is_log, codes, stats):
    links = spans(LINK_RE, line)
    actions = []  # (start, end, replacement)
    # 0) bestehende [<ID>](…)-Links auf das kanonische Ziel normalisieren
    #    (self-healing, idempotent): Linktext == genau eine Familien-ID, kein
    #    Bild-Link, nicht inTarget; schreibt nur, wenn Ziel != kanonisch.
    for lm in NORM_RE.finditer(line):
        txt, url = lm.group(1), lm.group(2)
        if not ANY.fullmatch(txt) or in_target(src_rel, TARGET[fam_of(txt)]):
            continue
        canon = link_for(src_rel, txt)
        if canon and canon != url:
            actions.append((lm.start(), lm.end(), "[%s](%s)" % (txt, canon)))
            stats["norm"] += 1
    if not is_log:
        # 1) NUR volle Einzeilen-Code-Spans (hier geöffnet UND geschlossen) als
        #    Linktext wrappen — Carry-over-Spans nicht anfassen.
        for cs0, cs1 in codes:
            if cs1 - cs0 < 2 or line[cs0] != '`' or line[cs1 - 1] != '`':
                continue
            if covered(cs0, cs1, links):
                continue
            inner = line[cs0 + 1:cs1 - 1]
            chosen = None
            for idm in ANY.finditer(inner):
                idv = idm.group(0)
                if in_target(src_rel, TARGET[fam_of(idv)]):
                    continue
                lk = link_for(src_rel, idv)
                if lk:
                    chosen = lk; break
            if chosen:
                actions.append((cs0, cs1, "[%s](%s)" % (line[cs0:cs1], chosen)))
                stats["code"] += 1
    # 2) nackte IDs (außerhalb Code/Link)
    for m in ANY.finditer(line):
        if covered(m.start(), m.end(), links) or covered(m.start(), m.end(), codes):
            continue
        idv = m.group(0)
        if in_target(src_rel, TARGET[fam_of(idv)]):
            continue
        if is_log:
            actions.append((m.start(), m.end(), "`%s`" % idv)); stats["tick"] += 1
        else:
            lk = link_for(src_rel, idv)
            if lk:
                actions.append((m.start(), m.end(), "[%s](%s)" % (idv, lk))); stats["link"] += 1
            else:
                stats["nolink"].add(idv)
    # anwenden: rechts->links, nicht überlappend
    actions.sort(key=lambda a: a[0], reverse=True)
    last = len(line) + 1
    for s, e, rep in actions:
        if e > last:
            continue
        line = line[:s] + rep + line[e:]; last = s
    return line

def process(rel, apply):
    full = os.path.join(ROOT, rel)
    is_log = rel == "CHANGELOG.md" or rel.startswith("docs/reviews/") or \
             (rel.startswith("spec/") and rel.endswith("-historie.md"))
    stats = {"link": 0, "code": 0, "tick": 0, "norm": 0, "nolink": set()}
    out, fenced, code_open, changed = [], False, False, False
    for line in open(full, encoding="utf-8"):
        raw = line.rstrip("\n")
        if raw.lstrip().startswith("```"):
            fenced = not fenced; code_open = False; out.append(line); continue
        if fenced or raw.lstrip().startswith("#"):
            out.append(line); code_open = False; continue  # Heading/Fence bricht Span
        codes, code_open = line_code_regions(raw, code_open)
        new = transform_line(raw, rel, is_log, codes, stats)
        if new != raw:
            changed = True
        out.append(new + "\n")
    if apply and changed:
        open(full, "w", encoding="utf-8").write("".join(out))
    tag = "LOG " if is_log else "norm"
    n = stats["link"] + stats["code"] + stats["tick"] + stats["norm"]
    if n or stats["nolink"]:
        print("[%s] %-55s link=%d code=%d tick=%d norm=%d %s" %
              (tag, rel, stats["link"], stats["code"], stats["tick"], stats["norm"],
               ("NOLINK:" + ",".join(sorted(stats["nolink"]))) if stats["nolink"] else ""))
    return n

def corpus():
    files = subprocess.check_output(["git", "ls-files", "*.md"], cwd=ROOT, text=True).split()
    return [f for f in files if not f.startswith("docs/plan/planning/done-archive/")]

if __name__ == "__main__":
    args = sys.argv[1:]
    apply = "--apply" in args or "--apply-all" in args
    if "--apply-all" in args:
        targets = corpus()
    else:
        targets = [a for a in args if not a.startswith("--")]
    total = sum(process(t, apply) for t in targets)
    print("== %s: %d Ersetzungen über %d Datei(en) ==" %
          ("APPLY" if apply else "DRY-RUN", total, len(targets)))
