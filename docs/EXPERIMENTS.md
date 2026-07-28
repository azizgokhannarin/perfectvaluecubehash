# Experiments

## E0 — Canonical-state verification

Pass conditions:

- 512 cells;
- each value 0..255 appears exactly twice;
- all 192 axis-parallel lines sum to 1020.

## E1 — Move correctness

For every axis and amount `a`:

```text
R(a) followed by R(8-a) returns the exact prior cube.
R(1)^8 returns the exact prior cube.
```

## E2 — Chain geometry

For every consecutive pair of generated moves:

- axes differ;
- the next intersection point lies on the previous line.

## E3 — Determinism and framing

Verify:

- repeated hashing is identical;
- `M` differs from `M || 00`;
- empty and non-empty samples differ.

## E4 — Small-domain collision search

Exhaustively hash:

- all 256 one-byte inputs;
- all 65,536 two-byte inputs.

A collision immediately falsifies collision resistance for the full design.

No collision in these small domains is only a basic implementation result.

## E5 — Avalanche measurement

For a selected message, flip every input bit independently and record:

- mean/min/max changed output bits out of 256;
- mean/min/max changed output bytes out of 32.

The ideal-binomial reference center is 128 changed bits, but proximity to 128
does not prove security.

## E6 — Hidden-state comparison

For pairs of messages, compare:

- digest equality;
- complete final-cube equality;
- number and location of differing cells.

This distinguishes full-state collisions from projection collisions.

## Planned experiments

- reduced cube sizes with exhaustive state exploration;
- automatic search for equivalent move chains;
- diagonal-only collision optimization;
- symmetry-generated related inputs;
- per-round differential maps;
- cycle and fixed-point search;
- multicollision and expandable-message attempts.
