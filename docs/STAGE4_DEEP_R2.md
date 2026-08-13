# Stage 4 Deep — PVC-RotHash-2 (first campaign set)

**Date:** 2026-08-13  
**Algorithm:** PVC-RotHash-2 / research preset `R5-rothash2`  
**Harness:** `scripts/stage4_deep_r2.sh` + ad-hoc full three-byte scans  
**Raw logs:** `results/stage4-r2/`  
**Status:** **No structural break in stated budgets**  
**Not:** Stage 4 complete; not a security proof; production forbidden

This campaign points the existing in-house attack tools at RotHash-2 instead of
the RotHash-1 canonical preset. It is the first **deep** Stage 4 entry after the
Stage 4 smoke gate (`docs/STAGE4_SMOKE_R2.md`).

---

## How to reproduce

```bash
cmake -S . -B build -DPVC_BUILD_TOOLS=ON -DPVC_BUILD_TESTS=ON
cmake --build build -j"$(nproc)"

# Standard multi-tool campaign (~2 min on 12 cores for this machine)
scripts/stage4_deep_r2.sh

# Optional modes
scripts/stage4_deep_r2.sh --quick
scripts/stage4_deep_r2.sh --full

# Full three-byte operational-state domains (2^24 messages each)
./build/pvc-three-byte-collision --preset R5-rothash2 --phase forward --first-byte-count 256
./build/pvc-three-byte-collision --preset R5-rothash2 --phase foldback --first-byte-count 256
```

---

## Headline results (standard mode + full 3-byte)

| Class | Tool / domain | Result |
|---|---|---|
| Digest uniqueness | 1-byte + 2-byte exhaustive (`--rothash2`) | 0 collisions |
| Phase uniqueness | 1-byte + 2-byte all phases (`phase-collision`) | 0 collisions; 65536 digests unique |
| Transition | depth-1 and depth-2 from empty start | 0 exact state collisions |
| Return-alias surface | 512 msgs × 8 B, all reverse depths | **0** alias pairs (4608 contexts) |
| Multicollision chain | prefix 4096 | seed pairs **0**; max levels 1 (none) |
| Foldback-aware / dual-return | prefix 4096 | forward pairs **0** |
| Bridged multicollision | levels 8, prefix 4096 | `path_found=no` (seed_pairs=0) |
| Three-byte forward | first-byte 32 (2.1M) + **full 2^24** | **0** exact state pairs |
| Three-byte foldback | first-byte 32 + **full 2^24** | **0** exact state pairs |
| Alignment (Δ=42) | 54784 pairs | exact transition convergences **0**; max equal-move prefix 1 |
| Related-input | reverse/complement/rotate + permutations | digest/state matches **0** |
| Length framing | lengths 0..64 | formula OK; index framing blocks equal full state |
| Differential | 16×16 B single-bit flips | final mean cube ~2048 bits; digest mean ~128.5; 0 exact digests |
| Truncated birthday | 24/32/40 bits, 4 trials, limit 200k | 24 & 32 found near expected; 40 censored |

### Contrast with RotHash-1

Under RotHash-1 / `R5-canonical` the public record includes:

- 3 two-byte forward pairs;
- **1,496** three-byte forward pairs;
- controller deltas 42/126/196;
- bridged multi-level forward families.

Under RotHash-2 / `R5-rothash2` in these same tool classes:

- **no** two-byte or three-byte forward operational merges in full 2^16 / 2^24
  domains;
- **no** seed pairs for multicollision / bridged / dual-return tools;
- Δ=42 alignment shows no multi-move identical transition paths.

This matches Stage 1 Controller **S** injectivity (G1–G3) and Stage 2 ST2, now
confirmed through the C++ research API used by the production attack suite.

---

## Truncated collisions (generic check)

```text
bits,found,censored,mean_messages,expected,mean_ratio
24,4,0,3350.5,5133.6,0.65
32,4,0,91628.8,82137.2,1.12
40,0,4,0,1.31e6,0  (all trials hit limit)
```

24- and 32-bit truncated collisions appear near birthday cost; 40-bit was
under-budgeted. No evidence of cheaper-than-generic truncated collisions in
this sample.

---

## Explicit non-claims

- No 256-bit collision/preimage/second-preimage security claim.
- No PQC claim.
- Exhaustive domains stop at three bytes for operational state; longer messages
  and digest-surface LSH/beam are **not** finished for RotHash-2.
- Budget-limited negatives ≠ proof.

---

## Still open (continue Stage 4)

1. Digest beam / LSH / barrier campaigns with `R5-rothash2`
2. Longer truncated series (40–48+ bit with larger limits)
3. Reduced-round ladder with systematic absorb variants
4. Independent reverse-engineering / SAT of Controller S (optional)
5. Public challenge text for RotHash-2
6. Optional freeze tag `v0.2.0-rothash2-draft` when packaging is ready

---

## Attack-log reference

See `docs/ATTACK_LOG.md` entry **A-R2-001** for the consolidated negative
finding set.
