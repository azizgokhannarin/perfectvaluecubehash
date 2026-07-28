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
