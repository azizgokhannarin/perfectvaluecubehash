# Stage 2 Smoke — Full Research Hash with Controller S

**Date:** 2026-08-02  
**Controller:** offline systematic **S** (`scripts/controller_redesign_prototypes.py`)  
**Harness:** `scripts/stage2_smoke_s.py`  
**Status:** **PASS** (ST1–ST4)  
**Not:** PVC-RotHash-1; not a security proof; not a frozen candidate yet

Stage 1 injectivity (G1–G3) already passed for S. This stage checks that wiring
S into the **full** PVC path (forward, foldback, diagonal/orbit closure,
squeeze) does not immediately revive v0-class failures or digest collapse on
small domains.

---

## Command

```bash
python3 scripts/stage2_smoke_s.py \
  --avalanche-samples 256 \
  --multiplicity-samples 5000
```

(Full ST2 is always exhaustive `2^16` unless `--skip-st2`.)

---

## Results

| Gate | Metric | Result | Pass |
|---|---|---|---|
| Sanity | `hash_S("abc") ≠ RotHash-1("abc")` | distinct digests | yes |
| **ST1** | Single-bit avalanche, 256 trials × 16-byte msgs | mean **128.06** bits (std 7.57, min 106, max 149) | **PASS** |
| **ST2** | All 65536 two-byte messages | **65536** unique digests, **0** collisions | **PASS** |
| **ST3** | Digest has some byte ≥3 times | rate **6.72%** on 5000×16-byte msgs | **PASS** |
| **ST4** | Reduced ladder (4096 two-byte msgs) | full S coll=0; S no-foldback coll=0; R0-like canonical coll=**20** | **PASS** |

### ST1 detail

Mean Hamming distance after one input bit flip is essentially the random
reference half of 256 bits. No avalanche freeze observed in this sample.

### ST2 detail

Exhaustive two-byte domain under the **complete** S-hash path has no digest
collision. This is stronger than G3 (which only tested one-symbol operational
states): ST2 includes foldback, closures, and squeeze.

### ST3 detail

v0 forbade any triple occurrence of a byte in a 32-byte digest. Under S-hash,
triples occur at a healthy nonzero rate (~6.7% in this sample), consistent with
random-like 32-byte strings rather than a hard ban.

### ST4 detail

A deliberately broken R0-like path (canonical absorb, no foldback, no closures,
4-byte squeeze) shows digest collisions on the same 4096-message sample where
full S shows zero. The ladder documents that **weakening the path breaks** while
full S does not in this budget.

Note: `S` with foldback disabled also showed zero collisions in this 4096
sample; ST4 does not claim foldback is required for two-byte uniqueness, only
that a clearly reduced path is measurably weaker than full S.

---

## Stage gate summary

```text
Stage 1 (G1–G3 injectivity): PASS (controller S)
Stage 2 (ST1–ST4 smoke):     PASS (research full-hash with S)
Stage 3 (new candidate ID):  READY TO PLAN — not minted yet
```

---

## Explicit non-claims

- No collision/preimage/PQC security claim.
- Python research path is not the dual-implementation freeze package.
- Official RotHash-1 vectors are unchanged.
- Production use remains prohibited.

---

## Next (Stage 3)

1. Choose candidate name (e.g. PVC-RotHash-2).
2. Normative absorb = Controller S (or equivalent) in SPEC + C++ + Python.
3. Generate official digests and phase vectors; cross-verify.
4. Tag freeze; open deep falsification (Stage 4) and public review (Stage 5).
