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
