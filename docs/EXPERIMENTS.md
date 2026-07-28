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

## E-008 — Reduced-round matrix

```bash
./build/pvc-reduced-round-probe
```

Exhausts every one-byte message for all six presets.

## E-009 — Exact transition and phase collision

```bash
./build/pvc-transition-collision --preset R5-canonical --depth 2
./build/pvc-phase-collision --preset R5-canonical --message-bytes 2
```

The first command isolates local transition convergence. The second determines
whether it survives foldback and finalization over the complete two-byte domain.

## E-010 — Truncated birthday scaling

```bash
./build/pvc-truncated-collision \
  --preset R5-canonical --bits 24 --limit 50000 --trial 0
```

Run multiple `--trial` domains and report the complete distribution, not only
the most favorable collision.

## E-011 — Differential phase search

```bash
./build/pvc-differential-search \
  --preset R5-canonical --samples 4 --message-bytes 16 --mode single
./build/pvc-differential-search \
  --preset R5-canonical --samples 4 --message-bytes 16 --mode paired
```

## E-012 — Reachable predecessor and related inputs

```bash
./build/pvc-predecessor-enumerator --preset R5-canonical
./build/pvc-related-input-probe --preset R5-canonical
```


## E-013 — Foldback merge extension

```bash
./build/pvc-foldback-merge-search \
  --preset R5-canonical --left 176f --right 1799 \
  --suffix-bytes 2 --limit 65536
```

Repeat for every documented forward-state pair. Record both exact collisions
and the minimum after-foldback cube distance.
