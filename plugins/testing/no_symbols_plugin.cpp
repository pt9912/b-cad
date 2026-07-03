// Test-Fixture (AK LH-FA-PLG-001 Boundary / PLG-002): eine valide Shared
// Library OHNE die vertraglichen Eintrittspunkte ("kein Plugin im Sinne
// des Vertrags"). Erwartung: Ablehnung ohne Absturz, ohne jede Wirkung.
extern "C" int bcad_unrelated_symbol() { return 0; }
