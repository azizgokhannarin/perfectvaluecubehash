# Roadmap

## Completed — Geometry and reference state

- canonical Perfect Value Cube;
- reversible X/Y/Z line rotations;
- intersecting cursor chain;
- body-diagonal access.

## Completed — PVC-RotHash-0 falsification

- forward and foldback absorption;
- raw four-diagonal output;
- immediate collision history;
- avalanche and small-domain collision probes;
- discovery of raw-output multiplicity and positional-memory distinguishers.

## Completed — PVC-RotHash-1 revision

- 64-symbol diagonal closure;
- 128-symbol full-cube orbit closure;
- 32-state four-diagonal squeeze;
- exhaustive two-byte distribution probe;
- large avalanche sweep;
- final-state positional-bias probe.

## Completed — Initial structural cryptanalysis framework

- six parameterized reduced-round presets;
- exact operational snapshots and phase checkpoints;
- local one- and two-symbol transition collision search;
- exhaustive one- and two-byte phase collision enumeration;
- 1–64 bit truncated collision scaling tool;
- phase-by-phase single and paired differential search;
- finite reachable predecessor in-degree measurement;
- reverse, complement, rotation, and multiset-permutation probes.

## Completed — Three-byte and first constrained MITM stage

- exhaustive full three-byte forward-state scan;
- exhaustive full three-byte after-foldback-state scan;
- classification of inherited and new context-dependent aliases;
- exact physical-move trace comparison for all new three-byte aliases;
- delta-42 controller alignment probe;
- independent two-byte suffix MITM over 2^32 cross pairs for each known prefix.

## Completed — First foldback-aware and multicollision stage

- coupled every known three-byte forward alias to its differing return symbols;
- exhausted common zero- and one-byte extensions for all 1,496 pairs;
- sampled 4,096 common two-byte extensions for every pair;
- found 15 chainable second-level aliases and explicit four-message forward
  multicollisions;
- tested common one-byte and sampled two-byte suffixes against all four-way
  families;
- added explicit-state foldback and return-symbol research APIs.


## Completed — Foldback separation and bridged multicollision stage

- profiled every known three-byte collision through each reverse step;
- proved the return-symbol map is a position-dependent XOR bijection;
- measured the exact separation gate, state-component divergence, and absence
  of later reconvergence in the complete known catalogue;
- exhausted independent one-byte suffix cross pairs for all 1,496 pairs;
- introduced common bridge bytes between alias levels;
- constructed a 32-level forward multicollision path;
- materialized and checked a 65,536-message forward multicollision family.

## Completed — Structural security campaign

- optimized bridged multicollision branches for after-foldback distance;
- added projection-LSH search over independent suffix choices;
- mapped controller aliases in reachable reverse contexts;
- calibrated 24- and 32-bit collision timing and recorded censored 40/48-bit runs;
- formalized unequal-length symbol-index framing;
- retained exact-state verification for every collision candidate.

## Next — Independent cryptanalysis and controller decision

- repeat distance-guided searches across many independently generated bridged
  paths rather than one deterministic path;
- classify inherited and local aliases by their minimum achievable foldback
  distance;
- search paired reverse aliases that cancel across two or more reverse positions;
- implement an independent constraint/SAT model for reduced presets without
  sharing the C++ move-controller implementation;
- search cycles and fixed points in reduced operational state graphs;
- expand 32-bit campaigns and raise 40-bit coverage toward its birthday scale;
- obtain independent source-level and mathematical review of the move controller;
- decide whether to freeze the specification or redesign amount/axis derivation
  to remove the 42/126/196 alias family.

## Decision gate

After structural cryptanalysis, choose among:

- abandon the construction;
- revise the move controller;
- introduce a new original state operation;
- retain it only as a negative research result;
- proceed toward a formal specification and independent review.


## Completed in v0.8.0

- Direct final-digest beam search over bridged forward multicollisions.
- Final-digest beam search constrained to different forward states.
- Multi-domain forward-divergent digest LSH.
- Phase-distance versus digest-distance correlation measurement.
- Generic binomial minimum-distance calibration for every search domain.

## Next campaign

- Repeat digest-surface searches over many independently generated bridged paths.
- Build reduced-round SAT/SMT or finite-domain models for exact digest and state
  constraints without importing a cryptographic primitive into the candidate.
- Search nonlinear and higher-order correlations not visible to Pearson analysis.
- Test differential trails chosen to control emitted diagonal bytes rather than
  merely internal-state distance.
- Expand 40/48-bit truncated campaigns to their expected birthday ranges.

## Strategic acceptance path

**Path to a hash people can rationally trust** (stages, gates, time scale):

→ [`docs/TRUST_PATH_ROADMAP.md`](TRUST_PATH_ROADMAP.md)

Long-horizon public-review packaging and related gates also appear in
[`docs/ACCEPTANCE_ROADMAP.md`](ACCEPTANCE_ROADMAP.md). Those documents do not
replace the completed-work log above.

## Active — Phase 1 controller decision campaign

- living log: [`docs/PHASE1_CONTROLLER_CAMPAIGN.md`](PHASE1_CONTROLLER_CAMPAIGN.md);
- independent modular analysis of the `42/126/196` family (double linear use of
  the symbol in control and amount_source; amount mod 7; axis LSB);
- alignment probe: among multiples of 14, only Δ=42 yields exact one-byte-context
  six-move aliases (three pairs); 126/196 appear only in deeper contexts;
- dual-return and multipath tools: no after-foldback merge in tested domains;
  multipath min distance 282 bits across 128 seeds;
- competitive ambition locked (`docs/RESEARCH_GOAL.md`); controller requirements
  in `docs/CONTROLLER_REQUIREMENTS.md` (G1–G3 hard gates);
- external G3 advice → systematic controller **S**: **G1∧G2∧G3 PASS**;
- Stage 2 smoke with S full-hash path: **ST1–ST4 PASS** (`docs/STAGE2_SMOKE_S.md`);
- Stage 3 draft + polish: **PVC-RotHash-2** (`RotHash2`,
  `SPECIFICATION_ROTHASH2.md`, 29× `official-v2.json`, CTest v2 verify) —
  see `docs/STAGE3_ROTHASH2.md`;
- Stage 4 **smoke PASS** (`docs/STAGE4_SMOKE_R2.md`);
- Stage 4 **first deep PASS** budget-limited (`docs/STAGE4_DEEP_R2.md`, A-R2-001):
  full 2^24 three-byte forward/foldback = 0 pairs under `R5-rothash2`;
- next: digest-surface / longer truncated Stage 4 + public challenge draft;
- **Resume pointer:** [`docs/SESSION_HANDOFF.md`](SESSION_HANDOFF.md).
