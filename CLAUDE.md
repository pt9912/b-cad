# Claude Code Entry Point — b-cad

This repository uses the AI Harness process.

Before editing code or documentation, read:

1. `harness/README.md`
2. `AGENTS.md`
3. `harness/conventions.md`
4. the active slice under `docs/plan/planning/`
5. referenced ADRs under `docs/plan/adr/`
6. referenced requirements under `spec/`

Rules:

- Follow Source Precedence from `AGENTS.md` and `harness/README.md`.
- Use only `make` targets for build, test, lint, and gates.
- Do not run direct host `cmake`, `clang`, `ctest`, `apt`, `vcpkg`, or `conan`.
- Before implementation, identify slice id, requirement ids, ADR ids, affected modules, and required gates.
- Before completion, run `make gates`.
- Do not claim success without actual gate output.
- If sources conflict, report the conflict and follow the higher-ranked source.