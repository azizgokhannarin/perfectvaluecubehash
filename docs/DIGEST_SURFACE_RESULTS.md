# Digest-Surface Cryptanalysis Campaign (v0.8.0)

## Purpose

Earlier campaigns concentrated on messages that intentionally collide at the
end of forward absorption. Version 0.8.0 targets the remaining blind spot:

> Can structured search find two messages with different forward states whose
> final 256-bit digests are substantially closer than a generic random search
> would predict?

The canonical PVC-RotHash-1 algorithm is unchanged. This campaign adds only
research executables and regression tests.

## Tools

- `pvc-digest-beam-search`: direct digest-distance optimization inside a
  bridged same-forward-state multicollision family.
- `pvc-divergent-digest-beam`: direct digest-distance optimization while
  explicitly rejecting candidates that converge to the same forward state.
- `pvc-digest-lsh-search`: projection-LSH search across two independently
  suffixed, forward-divergent message families.
- `pvc-barrier-correlation`: measures correlation between phase-state distance
  and final digest distance.

The generic references reported by the tools use the binomial distribution for
the Hamming distance of two independent 256-bit strings. They are comparisons,
not security proofs.

## 1. Same-forward digest-guided beam

Command:

```bash
./build/pvc-digest-beam-search \
  --levels 16 --beam 1024 --threads 8 --print-limit 0
```

Results:

```text
cumulative_pairs_evaluated          = 42,325
generic cumulative minimum          = 96 bits
observed global minimum             = 96 bits
global-best level                   = 13
exact digest collision              = no
```

The search directly optimized final digest distance. Its best result was
exactly the generic minimum predicted for the total number of evaluated pairs.
The corresponding messages still shared one forward state, but their foldback
states were 1,992 bits apart and closure returned the internal state to the
approximately half-different regime.

No persistent digest-distance gradient was observed across levels. Individual
levels sometimes beat or missed their local generic reference by a few bits,
but the cumulative minimum matched the random reference.

## 2. Forward-divergent digest-guided beam

Command:

```bash
./build/pvc-divergent-digest-beam \
  --left 000000 --right 000001 \
  --depth 8 --beam 128 --branch 16 --print-limit 0
```

Results:

```text
cumulative_pairs_evaluated          = 229,633
generic cumulative minimum          = 93 bits
observed global minimum             = 94 bits
forward-state distance at best      = 1,761 bits
exact digest collision              = no
```

Every retained candidate was checked to ensure that the two forward states
were different. The best digest distance was one bit *above* the generic
reference. The search therefore found no evidence that appending chosen byte
pairs creates a useful final-digest gradient.

At the best candidate, phase distances were:

```text
after forward                       = 1,761 bits
after foldback                      = 2,037 bits
after diagonal closure              = 2,047 bits
after orbit closure                 = 2,034 bits
final operational state             = 2,063 bits
final digest                         = 94 bits
```

## 3. Forward-divergent digest LSH

Each campaign used two different three-byte prefixes, 8,192 two-byte suffixes
per side, 32 one-byte digest projections, and a logical cross product of:

```text
8,192 × 8,192 = 67,108,864 pairs
```

The generic predicted minimum for that cross product is 84 digest bits.

| Left prefix | Right prefix | Best digest distance | Generic reference | Exact collision |
|---|---|---:|---:|---|
| `000000` | `000001` | 85 | 84 | no |
| `102030` | `102031` | 83 | 84 | no |
| `abcdef` | `abcdee` | 85 | 84 | no |
| `5a00c3` | `5a00c4` | 83 | 84 | no |

No candidate pair shared the same forward state. The ±1-bit deviations around
the generic reference are consistent with expected sampling variation and do
not show a systematic sub-generic attack.

## 4. Closure and squeeze barrier correlation

### Different prefixes, independent suffixes

```text
samples                              = 10,000
digest mean                          = 128.0378 bits
digest range                         = 100..163
exact digest collisions              = 0

phase                     mean bits     correlation with digest distance
after forward              880.6714       +0.007007
after foldback            1586.7851       +0.019117
after diagonal closure    2053.7657       -0.006632
after orbit closure       2053.2466       +0.011611
final state               2053.3580       -0.014828
```

### Different prefixes, common suffixes

```text
digest mean                          = 127.9466 bits
exact digest collisions              = 0
correlation after forward            = +0.009504
correlation after foldback           = +0.012976
correlation after diagonal closure   = -0.007459
correlation after orbit closure      = +0.015093
correlation at final state           = -0.016216
```

### Same-forward prefixes, common suffixes

Prefixes `176f` and `1799` remain forward-equal after every common suffix in
this domain.

```text
samples                              = 10,000
forward-equal samples                = 10,000
digest mean                          = 128.0139 bits
digest range                         = 97..157
exact digest collisions              = 0
correlation after foldback           = -0.015919
correlation after diagonal closure   = -0.008740
correlation after orbit closure      = +0.019970
correlation at final state           = +0.000690
```

The measured correlations are all close to zero. In these structured domains,
being unusually close or far at a pre-final phase did not predict final digest
closeness. Diagonal closure moved the 4,096-bit cube state to approximately the
half-different regime even when the two messages had an identical forward
state.

## Interpretation

The campaign directly tested the blind spot identified after v0.7.0. Within
its bounded domains:

1. digest-guided search did not outperform the generic Hamming-distance
   minimum;
2. forward-divergent LSH results tracked the generic cross-pair reference;
3. internal-state distance had negligible linear correlation with digest
   distance;
4. no exact full-digest collision was found.

This is positive evidence that diagonal closure, orbit closure, and the
four-diagonal squeeze behave as a second diffusion barrier rather than merely
preserving foldback distance. It is not a proof of collision resistance.
Nonlinear, higher-order, SAT/SMT, differential, and larger-memory attacks remain
open.
