# Changelog

## 0.3.0 — PVC-RotHash-1 cryptanalysis framework

- kept the canonical hash algorithm and known-answer vectors unchanged;
- added runtime research parameters and six reduced-round presets;
- added exact operational snapshots and phase checkpoints;
- added reduced-round, transition-collision, phase-collision, truncated-
  collision, differential, predecessor, and related-input tools;
- documented the canonical two-symbol forward convergence `17 6f` / `17 99`;
- verified that foldback separates all three forward merges in the exhaustive
  canonical two-byte domain;
- documented full-state and digest collapse in reduced presets R0–R2;
- added a foldback merge-extension search and exhausted all common two-byte
  suffixes for the three canonical forward-collision pairs;
- added regression tests preserving both the canonical KATs and the known
  forward-convergence/foldback-separation result;
- expanded CI with research smoke tests.

## 0.2.0 — PVC-RotHash-1

- replaced direct concatenation of one cube's four body diagonals;
- added an original 32-state four-diagonal squeeze;
- made every output byte depend on all four body diagonals;
- inserted four diagonal-rooted rotation symbols between output bytes;
- expanded self-fed diagonal closure from 32 to 64 symbols;
- added a 128-symbol full-cube orbit closure sampling all 512 cells;
- added exhaustive output-distribution analysis;
- added final-state positional-bias analysis;
- added a 32,000-trial avalanche sweep;
- documented version-0 multiplicity and canonical-coordinate distinguishers;
- added regression assertions for empty input and `abc`.

## 0.1.0 — PVC-RotHash-0

- canonical Perfect Value Cube;
- reversible axis-line rotations;
- state-dependent intersecting move chain;
- six moves per absorbed symbol;
- reverse message foldback traversal;
- geometry-derived length closure;
- self-fed 32-symbol closure;
- direct 32-byte output from one final cube's four body diagonals;
- CLI, tests, avalanche probe, collision probe, and structure probe.
