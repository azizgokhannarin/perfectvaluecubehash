# Stage 4 Smoke — PVC-RotHash-2

**Date:** 2026-08-13  
**Algorithm:** `PVC-RotHash-2` (Controller S absorb)  
**Harness:** `scripts/stage4_smoke_r2.py`, `pvc-collision-probe --rothash2`  
**Status:** **PASS** (smoke only)  
**Not:** security proof; not Stage 4 complete; production still forbidden

Stage 3 minted RotHash-2 in dual implementation form. This smoke starts Stage 4
falsification on the **official** RotHash-2 path (C++ `RotHash2` / Python
`pvc_rothash2`), not only the research Controller S harness.

---

## Commands

```bash
# Official v2 cross-implementation vectors
python3 scripts/verify_vectors_v2.py --cpp ./build/pvc-hash

# Stage 4 smoke (Python + optional C++ one-byte probe)
python3 scripts/stage4_smoke_r2.py --cpp ./build/pvc-collision-probe

# Direct C++ probes
./build/pvc-collision-probe --rothash2 1
./build/pvc-collision-probe --rothash2 r1-pair
# slow: ./build/pvc-collision-probe --rothash2 2
```

CTest names: `pvc-cross-implementation-vectors-v2`, `pvc-stage4-smoke-rothash2`.

---

## Results

| Gate | Metric | Result | Pass |
|---|---|---|---|
| Vectors | 29 official-v2 digests, Python ↔ C++ | all match | **PASS** |
| **SF1** | Exhaustive one-byte digests (256) | 256 unique | **PASS** |
| **SF2** | Two-byte grid digests (4096, step 4) | 4096 unique | **PASS** |
| **SF3** | Known RotHash-1 forward / context / four-way families | pairwise distinct R2 digests | **PASS** |
| **SF4** | C++ `collision-probe --rothash2 1` | no collisions | **PASS** |
| Extra | C++ exhaustive two-byte (`--rothash2 2`, 65536) | no collisions (~5.6s) | **PASS** |

Known RotHash-1 two-byte forward pair `17 6f` / `17 99` does **not** collide under
RotHash-2 digests (C++ `r1-pair` mode and SF3).

---

## Relation to deeper Stage 4

This smoke is the **entry** to Stage 4 (`docs/TRUST_PATH_ROADMAP.md`), not the
exit. Remaining deep campaigns:

- Exhaustive / long two-byte and three-byte alias catalogues with
  `systematic_absorb` / preset `R5-rothash2`
- Foldback-aware, bridged, dual-return tools pointed at RotHash-2
- Truncated collision scaling vs birthday
- Digest beam / LSH / barrier campaigns
- Log all findings in attack log / known cryptanalysis docs

---

## Explicit non-claims

- No collision, preimage, or PQC security claim.
- Smoke PASS ≠ Stage 4 complete ≠ production ready.
- RotHash-1 remains the frozen historical baseline with known forward
  multicollisions; do not use either algorithm for security today.
