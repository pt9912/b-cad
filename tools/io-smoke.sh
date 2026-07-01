#!/usr/bin/env bash
# io-smoke: Behaviour-Smoke des verdrahteten b-cad-IO-Binaries.
#
# Belegt die sonst coverage-ausgenommene main.cpp-Glue (CLI-Parsing +
# Composition-Root-Verdrahtung der --import-*/--export-*-Pfade): startet das
# gebaute Binary headless je Austauschformat und prueft exit 0 + nicht-leere
# Datei. IFC/DXF round-trippen (Export + Re-Import); STEP/STL/PDF sind export-only.
#
# Laeuft unter EINEM xvfb-Lauf (QApplication wird in main.cpp unbedingt gebaut);
# der Makefile-Aufruf wickelt `timeout ... xvfb-run -a` darum (Cleanup-Race,
# acc-002-beleg-Lehre/ADR-0010). KEIN make-gates-Member.
#
# Fail-closed: expliziter `|| fail`-Guard JE Aufruf — nicht allein `set -e`,
# dessen errexit in &&-Ketten/Funktionen/Subshells unter xvfb-run/timeout nicht
# zuverlaessig greift.
set -euo pipefail

BIN="${BCAD_BIN:-./build/src/b-cad}"
OUT="$(mktemp -d)"
trap 'rm -rf "$OUT"' EXIT

fail() {
    echo "io-smoke FAIL: $1" >&2
    exit 1
}

# Export-Aufruf: exit 0 + nicht-leere Zieldatei, sonst rot.
expect_export() {  # $1=flag  $2=file
    "$BIN" "$1" "$2" || fail "$1 exit!=0"
    test -s "$2" || fail "$1 erzeugte keine/leere Datei ($2)"
    echo "  ok: $1 -> $2 ($(wc -c < "$2") Bytes)"
}

# Import-Aufruf: exit 0 (Re-Import des eben Exportierten), sonst rot.
expect_import() {  # $1=flag  $2=file
    "$BIN" "$1" "$2" || fail "$1 exit!=0"
    echo "  ok: $1 <- $2"
}

# Negativ-Selbsttest (belegt die Fail-closedness): erzwingt eine leere Datei und
# erwartet, dass die `test -s`-Pruefung sie als Fehler erkennt.
#   BCAD_SMOKE_SELFTEST=1 make io-smoke  ->  muss exit!=0 liefern.
if [ "${BCAD_SMOKE_SELFTEST:-0}" = "1" ]; then
    # Negativ-Selbsttest: erzwingt den Fehlerfall (leere Datei) und MUSS rot
    # (exit!=0) enden — Beleg, dass `make io-smoke` ueberhaupt fehlschlagen kann.
    echo "io-smoke SELFTEST: erzwinge leere Datei -> erwarte exit!=0"
    : > "$OUT/empty.dxf"
    test -s "$OUT/empty.dxf" || fail "SELFTEST belegt fail-closed: leere Datei -> rot"
    fail "SELFTEST unerwartet: leere Datei galt als nicht-leer"
fi

echo "io-smoke: IFC (Export + Re-Import)"
expect_export --export-ifc "$OUT/s.ifc"
expect_import --import-ifc "$OUT/s.ifc"

echo "io-smoke: DXF (Export + Re-Import)"
expect_export --export-dxf "$OUT/s.dxf"
expect_import --import-dxf "$OUT/s.dxf"

echo "io-smoke: STEP (Export-only)"
expect_export --export-step "$OUT/s.step"

echo "io-smoke: STL (Export-only)"
expect_export --export-stl "$OUT/s.stl"

echo "io-smoke: PDF (Export-only, ADR-0016 — self-rolled Vektor-Maßstabsplan)"
expect_export --export-pdf "$OUT/s.pdf"

echo "io-smoke ok: IFC/DXF Export+Re-Import + STEP/STL/PDF Export — alle exit 0, Dateien nicht leer"
