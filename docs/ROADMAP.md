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

## Next — Foldback-aware alias construction

- chain multiple controller aliases inside four-byte and longer messages;
- derive chosen symbol differences that alias in both forward and return passes;
- construct pairs satisfying forward and foldback constraints simultaneously;
- search longer equivalent chains without storing complete states;
- calibrate 24-, 32-, 40-, and 48-bit collision distributions over many trials;
- add chosen differential cancellation and boomerang-style message relations;
- search cycles and fixed points in reduced operational state graphs;
- investigate multicollision and expandable-message constructions;
- obtain independent source-level review of the move controller.

## Decision gate

After structural cryptanalysis, choose among:

- abandon the construction;
- revise the move controller;
- introduce a new original state operation;
- retain it only as a negative research result;
- proceed toward a formal specification and independent review.
