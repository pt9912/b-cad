#!/usr/bin/env node
// docs-check — Markdown-Link-Validator für b-cad.
// Vendored aus dem AI-Harness-Kurs (Repo ai-harness-course, tools/docs-check.js),
// unveraendert uebernommen. Adaption dokumentiert in harness/conventions.md (MR-003).
// Drift-Hinweis: bei Updates der Quelle hier nachziehen (Entropy Management, Modul 15).
// Prüft:
//   1. Interne Markdown-Links [text](pfad.md#anker)
//   2. Bild-Referenzen ![alt](pfad.png|jpg|svg|gif|webp)
//   3. Code-/Config-Referenzen [text](pfad.go|.py|...)
//   4. Explizite Inline-Code-Pfade `../foo.md`, `lab/example/...`
// Exit-Code 1 bei Fehlern, 0 sonst.
//
// Bewusste Einschränkungen:
//   - Reference-style Links [text][ref] und [ref]: url werden nicht
//     geprüft. In diesem Lehr-Repo werden ausschließlich Inline-Links
//     verwendet (dokumentiert in tools/README.md).
//   - Anker werden nur in .md-Zielen geprüft. `code.go#L42` wird
//     stillschweigend akzeptiert (Konvention für Source-Line-Anker).
//   - Inline-Code-Pfadprüfung ist konservativ: nur explizite relative
//     Pfade (./, ../) und Repo-Root-Pfade (lab/, kurs/, tools/).
//   - Symlinks werden vor dem Sicherheits-Check auf ihren realpath
//     aufgelöst — Symlinks aus dem Repo-Baum werden erkannt.

import { readFileSync, readdirSync, statSync, existsSync, realpathSync } from "node:fs";
import { join, dirname, resolve, relative, basename, isAbsolute } from "node:path";

// ---- CLI-Parser ----
const args = process.argv.slice(2);
const options = {
  verbose: false,
  noWarn: false,
  help: false,
  ignore: [],
};
const targets = [];

for (let i = 0; i < args.length; i++) {
  const a = args[i];
  if (a === "-v" || a === "--verbose") { options.verbose = true; continue; }
  if (a === "--no-warn") { options.noWarn = true; continue; }
  if (a === "-h" || a === "--help") { options.help = true; continue; }
  if (a === "--ignore") {
    if (i + 1 >= args.length) {
      process.stderr.write("docs-check: --ignore braucht ein Argument.\n");
      process.exit(2);
    }
    options.ignore.push(args[++i]);
    continue;
  }
  if (a.startsWith("--ignore=")) {
    options.ignore.push(a.slice("--ignore=".length));
    continue;
  }
  if (a.startsWith("-")) {
    process.stderr.write(`docs-check: unbekannte Option ${a} (--help für Hilfe).\n`);
    process.exit(2);
  }
  targets.push(a);
}

if (options.ignore.length === 0) {
  // Default: Templates sind by-design symbolisch (Pfade im Ziel-Repo).
  options.ignore.push("lab/templates");
  options.ignore.push(".tmp");
}

if (options.help) {
  process.stdout.write(`docs-check — Markdown-Link-Validator

USAGE:
  docs-check [OPTIONS] [TARGET ...]

OPTIONS:
  -v, --verbose          OK-Items auch ausgeben (an stdout)
      --no-warn          Warnungen unterdrücken (Exit-Code unverändert)
      --ignore PATH      Pfad ausnehmen (mehrfach erlaubt, --ignore=PATH ok)
                         Default: lab/templates
  -h, --help             Diese Hilfe

EINSCHRÄNKUNGEN:
  - Nur Inline-Links: [text](url). Reference-Style nicht geprüft.
  - Anker nur in .md-Zielen geprüft. \`code.go#L42\` wird ignoriert.
  - Symlinks aus dem Repo werden erkannt (realpath).
  - Explizite Inline-Code-Pfade werden geprüft: \`../foo.md\`, \`lab/example/...\`.
`);
  process.exit(0);
}

const ROOT = process.cwd();
const ROOT_REAL = (() => {
  try { return realpathSync(ROOT); } catch { return ROOT; }
})();
const startPaths = targets.length > 0 ? targets : ["."];

// ---- Markdown-Datei-Liste ermitteln ----
const ignoreAbs = options.ignore.map((i) => resolve(i));
const SKIP_DIRS = new Set([
  "node_modules", ".git", "target", "build", ".gradle",
  "dist", ".next", ".venv", "__pycache__", "vendor", "bin", "obj",
]);

function isIgnored(absPath) {
  for (const ig of ignoreAbs) {
    if (absPath === ig || absPath.startsWith(ig + "/")) return true;
  }
  return false;
}

function collectMarkdown(start) {
  const out = [];
  function walk(p) {
    const absP = resolve(p);
    if (isIgnored(absP)) return;
    let st;
    try {
      st = statSync(p);
    } catch (e) {
      if (e.code !== "ENOENT") {
        process.stderr.write(`WARN  Filesystem-Fehler bei ${p}: ${e.code || e.message}\n`);
      }
      return;
    }
    if (st.isFile()) {
      if (p.endsWith(".md")) out.push(p);
      return;
    }
    if (st.isDirectory()) {
      if (SKIP_DIRS.has(basename(p))) return;
      let children;
      try {
        children = readdirSync(p, { withFileTypes: true });
      } catch (e) {
        process.stderr.write(`WARN  readdir fehlgeschlagen ${p}: ${e.code || e.message}\n`);
        return;
      }
      for (const child of children) walk(join(p, child.name));
    }
  }
  walk(start);
  return out;
}

const mdFiles = new Set();
for (const t of startPaths) {
  for (const f of collectMarkdown(t)) mdFiles.add(resolve(f));
}

if (mdFiles.size === 0) {
  process.stderr.write("docs-check: keine Markdown-Dateien gefunden.\n");
  process.exit(0);
}

// ---- Slugify (GitHub-Konvention, Unicode-aware) ----
function slugify(text) {
  return text
    .replace(/\s+#+\s*$/, "")           // ATX-closing "## ## " entfernen
    .toLowerCase()
    .replace(/<[^>]+>/g, "")            // HTML-Tags weg
    .replace(/`[^`]*`/g, "")            // Inline-Code weg
    .replace(/[*_~]/g, "")              // Markdown-Inline-Marker weg
    .replace(/[^\p{Letter}\p{Number}\p{Emoji_Presentation}\s-]/gu, "")
    .trim()
    // GitHub kollabiert NICHT: jedes Whitespace-Zeichen wird ein eigener
    // Bindestrich ("A — B" → "a--b", weil das "—" ersatzlos entfällt).
    .replace(/\s/g, "-");
}

const headingsCache = new Map(); // absPath → { slugs:Set<string> } | "ENOENT" | "EACCES" | ...
function getHeadings(absPath) {
  if (headingsCache.has(absPath)) return headingsCache.get(absPath);
  let text;
  try {
    text = readFileSync(absPath, "utf8");
  } catch (e) {
    headingsCache.set(absPath, e.code || "EREAD");
    return e.code || "EREAD";
  }
  const slugs = new Set();
  const counter = new Map();
  let inFence = false;
  let fenceMarker = "";
  for (const rawLine of text.split("\n")) {
    // CommonMark: bis zu 3 führende Spaces erlaubt
    const indented = rawLine.replace(/^ {0,3}/, "");
    const fm = indented.match(/^(```+|~~~+)/);
    if (fm) {
      if (!inFence) { inFence = true; fenceMarker = fm[1][0]; }
      else if (indented.startsWith(fenceMarker)) { inFence = false; fenceMarker = ""; }
      continue;
    }
    if (inFence) continue;
    const m = indented.match(/^(#{1,6})\s+(.+?)\s*$/);
    if (m) {
      let s = slugify(m[2]);
      if (s === "") continue;
      if (counter.has(s)) {
        const n = counter.get(s) + 1;
        counter.set(s, n);
        s = `${s}-${n}`;
      } else {
        counter.set(s, 0);
      }
      slugs.add(s);
    }
  }
  headingsCache.set(absPath, { slugs });
  return { slugs };
}

// ---- Inline-Code-Spans entfernen (Multi-Backtick-aware) ----
function stripInlineCode(line) {
  // Multi-Backtick: ` `, `` ``, ``` ``` etc. Match das gleiche
  // Backtick-Count am Anfang und am Ende. CommonMark-konform.
  return line.replace(/(`+)[^`]*?\1/g, "");
}

// ---- Link-Parser mit Klammer-Balancing ----
function extractLinks(text) {
  const links = [];
  const lines = text.split("\n");
  let inFence = false;
  let fenceMarker = "";
  for (let i = 0; i < lines.length; i++) {
    const rawLine = lines[i];
    const indented = rawLine.replace(/^ {0,3}/, "");
    const fm = indented.match(/^(```+|~~~+)/);
    if (fm) {
      if (!inFence) { inFence = true; fenceMarker = fm[1][0]; }
      else if (indented.startsWith(fenceMarker)) { inFence = false; fenceMarker = ""; }
      continue;
    }
    if (inFence) continue;
    const stripped = stripInlineCode(rawLine);

    // Token-basiertes Link-Parsing: [text](target ggf. "title")
    // mit Klammer-Balancing in target.
    let idx = 0;
    while (idx < stripped.length) {
      let isImage = false;
      let openBracket = stripped.indexOf("[", idx);
      if (openBracket === -1) break;
      if (openBracket > 0 && stripped[openBracket - 1] === "!") isImage = true;

      // Bracket-Balance für link text
      let depth = 1;
      let textEnd = openBracket + 1;
      while (textEnd < stripped.length && depth > 0) {
        const ch = stripped[textEnd];
        if (ch === "[") depth++;
        else if (ch === "]") depth--;
        if (depth > 0) textEnd++;
      }
      if (depth !== 0 || textEnd >= stripped.length || stripped[textEnd + 1] !== "(") {
        idx = openBracket + 1;
        continue;
      }

      // Parse target with paren balancing
      const linkText = stripped.slice(openBracket + 1, textEnd);
      let p = textEnd + 2;
      let parenDepth = 1;
      let target = "";
      let inTitle = false;
      while (p < stripped.length && parenDepth > 0) {
        const ch = stripped[p];
        if (!inTitle && ch === "(") parenDepth++;
        else if (!inTitle && ch === ")") parenDepth--;
        else if (ch === '"' && !inTitle && /\s/.test(stripped[p - 1] || " ")) {
          inTitle = true;
        } else if (ch === '"' && inTitle) {
          inTitle = false;
        }
        if (parenDepth > 0) {
          if (!inTitle && /\s/.test(ch) && target.length > 0) {
            // Whitespace + Titel beginnt
            inTitle = false;
            while (p < stripped.length && stripped[p] !== ")") p++;
            break;
          }
          if (!inTitle) target += ch;
        }
        p++;
      }
      if (parenDepth === 0 || (parenDepth > 0 && stripped[p] === ")")) {
        if (target) {
          links.push({ image: isImage, text: linkText, target, line: i + 1 });
        }
      }
      idx = p + 1;
    }
  }
  return links;
}

function extractInlineCodePaths(text) {
  const paths = [];
  const lines = text.split("\n");
  let inFence = false;
  let fenceMarker = "";
  for (let i = 0; i < lines.length; i++) {
    const rawLine = lines[i];
    const indented = rawLine.replace(/^ {0,3}/, "");
    const fm = indented.match(/^(```+|~~~+)/);
    if (fm) {
      if (!inFence) { inFence = true; fenceMarker = fm[1][0]; }
      else if (indented.startsWith(fenceMarker)) { inFence = false; fenceMarker = ""; }
      continue;
    }
    if (inFence) continue;
    // Opt-out für Beispiel-Pfade (fremde Repos, Angriffs-Beispiele):
    // ein HTML-Kommentar `docs-check:ignore` auf derselben Zeile.
    if (rawLine.includes("docs-check:ignore")) continue;

    const re = /(`+)([^`]*?)\1/g;
    let m;
    while ((m = re.exec(rawLine)) !== null) {
      if (rawLine[m.index - 1] === "[" && rawLine[re.lastIndex] === "]") {
        continue;
      }
      const value = m[2].trim();
      if (looksLikeExplicitPath(value)) {
        paths.push({ target: normalizeInlinePath(value), line: i + 1 });
      }
    }
  }
  return paths;
}

function normalizeInlinePath(value) {
  return value
    .replace(/^["']|["']$/g, "")
    .replace(/[.,;:]$/g, "");
}

function looksLikeExplicitPath(value) {
  const v = normalizeInlinePath(value);
  if (v === "" || /\s/.test(v)) return false;
  if (v.startsWith("//")) return false;
  if (/[{}<>|*?=]/.test(v)) return false;
  if (v.includes("…") || v.includes("->") || v.includes("→")) return false;
  if (isExternal(v) || v.startsWith("#")) return false;
  return (
    v.startsWith("./") ||
    v.startsWith("../") ||
    v.startsWith("lab/") ||
    v.startsWith("kurs/") ||
    v.startsWith("tools/")
  );
}

function isExternal(target) {
  return /^[a-z][a-z0-9+.-]*:/i.test(target);
}
function isAnchorOnly(target) {
  return target.startsWith("#");
}

function checkLocalTarget({ rel, line, target, fileDir, rootRelative }) {
  const [pathPart, anchor] = target.split("#");

  if (isAbsolute(pathPart)) {
    process.stderr.write(`ERROR ${rel}:${line}: absoluter Pfad "${target}" nicht erlaubt\n`);
    errors++;
    return;
  }

  const targetAbs = rootRelative ? resolve(ROOT, pathPart) : resolve(fileDir, pathPart);

  let realTarget = targetAbs;
  try {
    if (existsSync(targetAbs)) realTarget = realpathSync(targetAbs);
  } catch { /* realpath kann scheitern, dann targetAbs nutzen */ }

  const relFromRoot = relative(ROOT_REAL, realTarget);
  if (relFromRoot.startsWith("..") || isAbsolute(relFromRoot)) {
    process.stderr.write(`ERROR ${rel}:${line}: Ziel "${target}" zeigt aus dem Repo heraus\n`);
    errors++;
    return;
  }

  if (!existsSync(targetAbs)) {
    process.stderr.write(`ERROR ${rel}:${line}: Ziel "${target}" existiert nicht (resolved: ${relative(ROOT, targetAbs)})\n`);
    errors++;
    return;
  }

  if (anchor && targetAbs.endsWith(".md")) {
    const h = getHeadings(targetAbs);
    if (typeof h === "string") {
      if (h === "EACCES" || h === "EPERM") {
        process.stderr.write(`ERROR ${rel}:${line}: Ziel "${target}" nicht lesbar (${h}) — Anker nicht prüfbar\n`);
        errors++;
      } else if (!options.noWarn) {
        process.stderr.write(`WARN  ${rel}:${line}: Ziel "${target}" Heading-Index nicht ermittelbar (${h})\n`);
        warnings++;
      }
    } else if (!h.slugs.has(anchor)) {
      process.stderr.write(`ERROR ${rel}:${line}: Anker "#${anchor}" existiert nicht in ${relative(ROOT, targetAbs)}\n`);
      errors++;
    } else if (options.verbose) {
      process.stdout.write(`OK    ${rel}:${line}: ${target}\n`);
      okCount++;
    }
    return;
  }

  if (options.verbose) {
    process.stdout.write(`OK    ${rel}:${line}: ${target}\n`);
    okCount++;
  }
}

// ---- Hauptlauf ----
let errors = 0;
let warnings = 0;
let okCount = 0;

const collator = new Intl.Collator("en", { sensitivity: "variant" });
const sorted = [...mdFiles].sort(collator.compare);
for (const absMd of sorted) {
  const rel = relative(ROOT, absMd);
  let text;
  try {
    text = readFileSync(absMd, "utf8");
  } catch (e) {
    process.stderr.write(`ERROR ${rel}: lesen fehlgeschlagen (${e.code || e.message})\n`);
    errors++;
    continue;
  }
  const links = extractLinks(text);
  const inlineCodePaths = extractInlineCodePaths(text);
  const fileDir = dirname(absMd);

  for (const link of links) {
    const { target, line } = link;
    if (isExternal(target)) continue;
    if (isAnchorOnly(target)) {
      const own = getHeadings(absMd);
      const anchor = target.slice(1);
      if (typeof own === "string") {
        process.stderr.write(`WARN  ${rel}:${line}: eigener Heading-Index nicht ermittelbar (${own})\n`);
        warnings++;
        continue;
      }
      if (!own.slugs.has(anchor)) {
        process.stderr.write(`ERROR ${rel}:${line}: Anker "#${anchor}" existiert nicht in dieser Datei\n`);
        errors++;
      } else if (options.verbose) {
        process.stdout.write(`OK    ${rel}:${line}: in-file anchor #${anchor}\n`);
        okCount++;
      }
      continue;
    }

    checkLocalTarget({ rel, line, target, fileDir, rootRelative: false });
  }

  for (const codePath of inlineCodePaths) {
    const rootRelative = /^(lab|kurs|tools)\//.test(codePath.target);
    checkLocalTarget({ rel, line: codePath.line, target: codePath.target, fileDir, rootRelative });
  }
}

// ---- Zusammenfassung ----
// Immer auf stderr — Diagnostik-Stream, konsistent für CI.
const summary = `\ndocs-check: ${mdFiles.size} Datei(en) geprüft, ${errors} ERROR, ${warnings} WARN${options.verbose ? `, ${okCount} OK` : ""}.\n`;
process.stderr.write(summary);

process.exit(errors > 0 ? 1 : 0);
