# Stage 4 Digest Surface — PVC-RotHash-2

**Date:** 2026-08-13  
**Preset:** `R5-rothash2`  
**Harness:** `scripts/stage4_digest_r2.sh` + ad-hoc LSH-8192 / truncated-40  
**Logs:** `results/stage4-r2-digest/`  
**Status:** **No exact 256-bit digest collision; distances near generic references**  
**Not:** security proof; production forbidden

Companion to operational-state Stage 4 (`docs/STAGE4_DEEP_R2.md`). This campaign
asks whether structured digest search finds **sub-generic** closeness when
messages do **not** share a cheap forward multicollision seed (RotHash-2 has
none in short domains).

---

## Reproduce

```bash
scripts/stage4_digest_r2.sh              # standard
scripts/stage4_digest_r2.sh --quick
scripts/stage4_digest_r2.sh --full

# R1-parity LSH domain
./build/pvc-digest-lsh-search --preset R5-rothash2 \
  --left 000000 --right 000001 --suffix-bytes 2 --suffix-limit 8192 \
  --projections 32 --projection-bytes 1

# 40-bit truncated with higher limit
./build/pvc-truncated-campaign --preset R5-rothash2 \
  --bits 40 --trials 4 --limit 2000000
```

---

## Results

### 1. Same-forward digest beam (bridged)

```text
pvc-digest-beam-search --preset R5-rothash2 ...
path_found=no
```

**Expected.** The tool builds a bridged forward multicollision path. Under
Controller S there are no three-byte forward seed pairs, so the campaign cannot
start. This is a **positive structural signal**, not a failed test harness.

### 2. Forward-divergent digest beam

Same budgets as the RotHash-1 digest-surface campaign (depth 8, beam 128,
branch 16 → 229,633 cumulative pairs; generic cumulative min ≈ **93** bits).

| Seeds | Global best digest bits | Generic ref | Exact collision | Best forward bits |
|---|---:|---:|---|---:|
| `000000` / `000001` | **90** | 93 | no | 1019 |
| `a5c301` / `5a3cfe` | **91** | 93 | no | 1311 |

A few bits below the cumulative generic minimum is within multi-trial sampling
variation (compare RotHash-1 divergent best **94** vs ref 93). No exact collision.
Phase distances at the best candidates remain large (~2k cube bits after
closures); squeeze does not expose a clear distance gradient to zero.

### 3. Forward-divergent digest LSH

Standard campaign: 4,096×4,096 logical pairs per domain (generic min ≈ **86**).

| Left | Right | Best | Generic | Exact |
|---|---|---:|---:|---|
| `000000` | `000001` | 83 | 86 | no |
| `102030` | `102031` | 88 | 86 | no |
| `abcdef` | `abcdee` | 86 | 86 | no |
| `5a00c3` | `5a00c4` | 85 | 86 | no |

R1-parity domain (8,192×8,192, generic min ≈ **84**):

| Left | Right | Best | Generic | Exact | Candidates scored |
|---|---|---:|---:|---|---:|
| `000000` | `000001` | **83** | 84 | no | 8,389,677 |

`forward_equal_skipped=0` in all domains. Results track the generic reference
within a few bits (same pattern as RotHash-1 LSH table).

### 4. Barrier correlation (phase ↔ digest)

Prefixes `000000` / `000001`, 5,000 samples, 2-byte suffixes.

| Mode | Digest mean | Exact | |corr| max vs digest |
|---|---:|---:|---|---:|
| Independent suffixes | 128.04 | 0 | final ≈ 0.006 |
| Common suffixes | 128.02 | 0 | final ≈ 0.011 |

No useful linear coupling between operational-state Hamming distance and final
digest distance in these samples. Forward-equal samples = 0.

### 5. Longer truncated collisions

Standard multi-width (`limit=400000`, 4 trials):

```text
bits  found  censored  mean_messages  expected     mean_ratio
24    4      0         3350.5         5133.6       0.65
32    4      0         91628.8        82137.2      1.12
40    0      4         —              1.31e6       censored
48    0      4         —              2.10e7       censored
```

Extended 40-bit (`limit=2000000`, 4 trials):

```text
40,4,0,1176177.5,1314195.1,0.8950,min=795593,max=1356285
```

40-bit truncated collisions appear **near birthday cost** (~0.9× expected). No
evidence of a substantially cheaper-than-generic truncated attack in this
trial set. 48-bit remains under-budgeted.

---

## Interpretation

1. Digest-surface tools that **depend on R1-style forward seeds cannot run** —
   consistent with Stage 4 operational results (A-R2-001).
2. Tools that **do not** need those seeds (divergent beam, LSH, barrier) behave
   like random 256-bit strings at these budgets: minima near binomial
   references, correlations ~0, no exact full digest collision.
3. Truncated 24/32/40-bit costs track birthday; still not a 256-bit claim.

**This is budget-limited negative evidence**, parallel to RotHash-1’s
`docs/DIGEST_SURFACE_RESULTS.md` but for the RotHash-2 absorb path.

---

## Still open

- Larger LSH / beam budgets and more seed pairs
- 48-bit+ truncated with multi-million limits
- Independent external digest search
- Optional SAT/algebraic model of S + squeeze
