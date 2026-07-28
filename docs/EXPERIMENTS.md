# Experiments

## E0 — Canonical-state verification

Verify 512 cells, two copies of every byte, and 192 initial line sums of 1020.

## E1 — Move correctness

For every axis and amount `a`:

```text
R(a) followed by R(8-a) restores the exact state.
R(1)^8 restores the exact state.
```

## E2 — Chain geometry

For every consecutive generated move, axes differ and the current line
intersects the previous line.

## E3 — Determinism, regression vectors, and framing

Verify deterministic output, known-answer regression values, and separation of
`M` from `M || 00`.

## E4 — Small-domain collision search

```bash
./build/pvc-collision-probe 1
./build/pvc-collision-probe 2
```

Exhaustively test all one- and two-byte messages. A collision immediately
falsifies the full candidate. No collision is only a limited negative result.

## E5 — Avalanche

Single-message exhaustive bit flips:

```bash
./build/pvc-avalanche "Perfect Value Cube"
```

Multi-message sweep:

```bash
./build/pvc-avalanche-sweep 32000 16
```

Record mean, standard deviation, minimum, and maximum Hamming distance.

## E6 — Digest distribution and structural output tests

```bash
./build/pvc-distribution-probe --exhaustive-two-byte
./build/pvc-distribution-probe 100000 16
./build/pvc-distribution-probe 100000 64
```

Measure:

- chi-square independently for every output byte position;
- support at every position;
- maximum bit-frequency z-score;
- equal-byte pairs;
- complementary-byte pairs;
- frequency of digests containing a byte three or more times.

The deterministic standard-library generator is test infrastructure only.

## E7 — Final-state positional memory

```bash
./build/pvc-state-bias-probe 50000 16
```

Measure all 512 final-cube coordinates separately and count how often their most
frequent value equals the canonical occupant or its complement.

## E8 — Structural invariants

```bash
./build/pvc-structure-probe "Perfect Value Cube"
```

Verify the intersecting chain and preserved full-state histogram. Initial line
balance is expected to be lost.

## Planned experiments

- reduced-state exhaustive models;
- equivalent move-chain search;
- pairwise and higher-order output correlation;
- symmetry-generated related inputs;
- per-stage differential maps;
- cycle and fixed-point search;
- multicollision and expandable-message attempts;
- meet-in-the-middle attacks on message/finalization boundaries.
