# Release notes — PVC-RotHash-2 0.2.0-draft

**Tag:** `v0.2.0-rothash2-draft`  
**Date:** 2026-08-13  
**Status:** Experimental public-attack draft freeze  
**Production:** **Prohibited**  
**Security claims:** **None**

## What this is

First public draft freeze of **PVC-RotHash-2**, the successor candidate on the
competitive research path. Absorb uses systematic Controller **S**. Finalization
shape matches PVC-RotHash-1 (foldback, diagonal/orbit closures, four-diagonal
squeeze).

PVC-RotHash-1 `1.0.0-rc1` remains frozen and unchanged as the historical
baseline.

## Package contents

| Item | Location |
|---|---|
| Normative absorb | `SPECIFICATION_ROTHASH2.md` |
| C++ API | `pvc::RotHash2`, CLI `--rothash2` |
| Python reference | `reference/python/pvc_rothash2.py` |
| Official digests | `test-vectors/official-v2.json` (29 KATs) |
| Cross-impl verify | `scripts/verify_vectors_v2.py`, CTest |
| Public challenge | `CRYPTANALYSIS_CHALLENGE_ROTHASH2.md` |
| Stage 4 (in-house) | `docs/STAGE4_*.md`, A-R2-001 / A-R2-002 |
| Research preset | `R5-rothash2` |

## Anchor digests

```text
H2("")    = 9c4f0899255220a60bac6df7661cc50583f25f076ad37254b270b54685ce864a
H2("abc") = 9c1b502e8eac4ea07e18265ea30f888c4d5fd8ae81fa1ed453c2c099d4d68fdb
```

## In-house evidence (not a proof)

- Stage 1: Controller S G1∧G2∧G3 PASS  
- Stage 2: ST1–ST4 smoke PASS  
- Stage 3: dual impl + vectors  
- Stage 4: operational deep + digest-surface budget-limited negatives  

## What is frozen by this tag

For the draft review window of **PVC-RotHash-2 0.2.0-draft**:

- absorb equations in `SPECIFICATION_ROTHASH2.md`;
- shared finalization parameters as used by `RotHash2`;
- official-v2 digest vectors;
- algorithm identifier `PVC-RotHash-2`.

Any algorithmic change requires a **new** candidate identifier (for example
`0.3.0-draft` or a later freeze). Documentation, tooling, and attack logs may
grow without moving digests.

## Non-claims

- Not production-ready  
- Not collision/preimage/PQC secure by assertion  
- Not a replacement recommendation for SHA-2/SHA-3/BLAKE3/etc.  
