# Design Decisions

## D-001 — No established cryptographic primitive

Accepted. The candidate does not call or embed an existing hash, cipher, S-box,
KDF, MAC, checksum, or pseudorandom generator.

Analysis executables may use standard statistical and test-data generation
facilities because they are not part of the candidate algorithm.

## D-002 — Rotation-only state mutation

Retained for version 1. Cube cells are moved but never replaced. The complete
byte histogram is invariant.

The digest extractor may combine observed diagonal bytes using elementary byte
operations. This does not alter the cube state and is part of the original
four-diagonal squeeze design.

## D-003 — Consecutive lines intersect

Retained. Every move uses a different axis from the prior move and starts on the
prior line through the moving cursor.

## D-004 — Four body diagonals remain the output source

Retained with revision.

Rejected version-0 form:

```text
one final cube -> concatenate 32 raw diagonal cells
```

Accepted version-1 form:

```text
32 evolving cube states -> combine all four diagonals -> one byte per state
```

Reason: direct concatenation created a byte-multiplicity distinguisher and
exposed canonical-coordinate bias.

## D-005 — Full-cube orbit before squeeze

Accepted. A 128-symbol closure directly samples every physical cube cell once
across four 128-cell quarters while the state evolves.

Reason: short-message tests showed that diagonal-only closure did not mix the
full state sufficiently.

## D-006 — Security wording

All versions remain explicitly experimental. Statistical test results are
reported as rejection of specific attacks, never as proof of security.


## D-007 — Freeze canonical version-1 output during cryptanalysis

Accepted. Version 0.3.0 adds research APIs and tools but does not alter the
canonical constants, operational path, or known-answer vectors.

## D-008 — Runtime parameters are research-only

Accepted. Reduced-round parameters are exposed through `pvc/research.hpp`.
Production-facing `RotHash1` always uses `R5-canonical`.

## D-009 — Exact collision verification

Accepted. Analysis tools may index large state sets with compact,
non-cryptographic fingerprints for memory efficiency, but every reported state
collision must be verified against the complete operational snapshot.

## D-010 — Forward equality is not a full collision

Accepted. Any reported message collision must state the deepest equal phase.
Forward convergence that is separated by foldback is recorded as a structural
finding, not presented as a digest collision.

## Decision — do not revise the canonical controller in v0.7.0

The 42/126/196 alias family is now observed in both forward and reachable
reverse contexts. The project nevertheless keeps the canonical algorithm fixed
for this campaign so that cryptanalysis remains comparable across releases.
Controller redesign is deferred until guided searches are repeated across
multiple paths and independently reviewed.
