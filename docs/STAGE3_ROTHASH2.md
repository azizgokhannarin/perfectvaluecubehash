# Stage 3 — PVC-RotHash-2 Candidate Mint (draft)

**Date:** 2026-08-02  
**Algorithm:** `PVC-RotHash-2`  
**Status:** Experimental draft candidate (not production; no security claim)  
**Absorb:** Controller **S** (systematic mixed-radix injectivity)  
**Finalization:** Same shape as PVC-RotHash-1 (foldback, closures, squeeze)

## What shipped in-tree

| Artifact | Path |
|---|---|
| Normative absorb draft | `SPECIFICATION_ROTHASH2.md` |
| C++ API | `pvc::RotHash2` in `include/pvc/hash.hpp` |
| Engine branch | `HashParameters::systematic_absorb` in `src/engine.cpp` |
| Python reference | `reference/python/pvc_rothash2.py` |
| Vectors | `test-vectors/official-v2.json` |
| CLI | `./build/pvc-hash --rothash2 --text ...` |
| Research preset | `R5-rothash2` |

## Anchor digests (C++ ↔ Python match)

```text
H2("")  = 9c4f0899255220a60bac6df7661cc50583f25f076ad37254b270b54685ce864a
H2("abc") = 9c1b502e8eac4ea07e18265ea30f888c4d5fd8ae81fa1ed453c2c099d4d68fdb
```

RotHash-1 anchors unchanged (`7f01…`, `f32b…`).

## Prior gates

- Stage 1: G1–G3 injectivity PASS for S  
- Stage 2: ST1–ST4 smoke PASS (`docs/STAGE2_SMOKE_S.md`)

## Stage 3 polish (done 2026-08-13)

- [x] Expand official-v2 vector set (29 messages; aliases + classic KATs)
- [x] CTest cross-impl verify for v2 (`scripts/verify_vectors_v2.py`,
      test `pvc-cross-implementation-vectors-v2`)
- [x] Stage 4 **smoke** entry (`scripts/stage4_smoke_r2.py`,
      `pvc-collision-probe --rothash2`, test `pvc-stage4-smoke-rothash2`)
      — details in `docs/STAGE4_SMOKE_R2.md`

## Not done yet (continue later)

- [ ] Optional tag e.g. `v0.2.0-rothash2-draft` when ready
- [ ] Stage 4 **deep** falsification on RotHash-2 (multicollision / truncated /
      beam tools with `systematic_absorb` / `R5-rothash2`)
- [ ] Public challenge text for RotHash-2
- [ ] Phase dumps for v2 (optional; absorb differs from RotHash-1 dump format)

## Policy

- RotHash-1 remains frozen historical baseline.
- Production use of RotHash-2 is **prohibited** until trust-path Stage 5+ evidence.
- Algorithm changes require a new identifier.
