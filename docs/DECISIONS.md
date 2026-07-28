# Design Decisions

## D-001 — No established cryptographic primitive

Accepted.

The candidate algorithm will not call or embed an existing hash, cipher, S-box,
KDF, MAC, or pseudorandom generator.

Reason: the experiment must measure the Perfect Value Cube rotation design
without inheriting security from another construction.

## D-002 — Rotation-only state mutation

Accepted for version 0.

Cell values are never replaced, combined, substituted, added, or XORed into
other cells. Only their positions change through cyclic line rotations.

Reason: isolate the original rotation hypothesis.

Consequence: the complete byte histogram is invariant and may be exploitable.

## D-003 — Consecutive lines must intersect

Accepted.

Every move uses a different axis from the previous move and passes through the
cursor moved by that prior rotation.

Reason: create a non-commuting, path-dependent chain.

## D-004 — Digest from four body diagonals

Accepted.

The 256-bit output consists exactly of the 32 cells on the four body diagonals
of the final cube.

Consequence: the hash is a projection of a 512-byte state. Projection
collisions are a primary attack target.

## D-005 — Security wording

Accepted.

All versions remain explicitly experimental until substantial independent
cryptanalysis exists.
