# Independent Cryptanalysis Guide

This guide is the preferred entry point for external reviewers.

**Two candidates** are frozen for attack. Prefer the competitive path unless you
are studying the historical baseline:

| Priority | Candidate | Challenge | Spec |
|---|---|---|---|
| **1 (active)** | PVC-RotHash-2 `0.2.0-draft` | `CRYPTANALYSIS_CHALLENGE_ROTHASH2.md` | `SPECIFICATION_ROTHASH2.md` |
| 2 (baseline) | PVC-RotHash-1 `1.0.0-rc1` | `CRYPTANALYSIS_CHALLENGE.md` | `SPECIFICATION.md` |

Neither is for production. Neither carries a security claim.

## 1. Reproduce the candidates

```bash
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release \
  -DPVC_WARNINGS_AS_ERRORS=ON -DPVC_BUILD_TOOLS=ON -DPVC_BUILD_TESTS=ON
cmake --build build --parallel
ctest --test-dir build --output-on-failure -E 'pvc-stage4-smoke-rothash2'
# or: scripts/release_check.sh

# RotHash-2 (active)
python3 scripts/verify_vectors_v2.py --cpp build/pvc-hash
./build/pvc-hash --rothash2 --hex 616263

# RotHash-1 (baseline)
python3 scripts/verify_vectors.py \
  --cpp build/pvc-hash \
  --vector-dump build/pvc-vector-dump
```

The Python implementations under `reference/python/` are intentionally
independent of the C++ source structure.

## 2. Read in this order (RotHash-2 path)

1. `CRYPTANALYSIS_CHALLENGE_ROTHASH2.md` — targets R2-C1…R2-C9.
2. `SPECIFICATION_ROTHASH2.md` — normative absorb; finalization via SPECIFICATION.md §§10–15.
3. `docs/SECURITY_TARGET.md` — exact non-claims and desired targets.
4. `docs/KNOWN_CRYPTOANALYSIS.md` — consolidated positive and negative results.
5. `docs/STAGE4_DEEP_R2.md` and `docs/STAGE4_DIGEST_R2.md` — in-house budgets.
6. `docs/ATTACK_MODEL.md` — terminology and state boundaries.
7. `docs/CRYPTANALYSIS_FRAMEWORK.md` — reduced-round API and tools (`R5-rothash2`).
8. `docs/ACCEPTANCE_ROADMAP.md` — long-horizon credibility gates (not a claim of security).

For RotHash-1 historical study, start with `SPECIFICATION.md` and
`CRYPTANALYSIS_CHALLENGE.md`.

## 3. High-value attack questions

**RotHash-2 (preferred):**

1. Is there a full 256-bit collision or a sub-generic truncated method?
2. Can equal-length messages merge at the operational state after forward or
   after foldback (challenge R2-C5)?
3. Can Controller S injectivity (G1–G3) be broken in a real multi-symbol path?
4. Do foldback/closure/squeeze alone admit a structural attack without short
   forward aliases?

**RotHash-1 (baseline / lessons):**

1. Can two messages with different forward states collide after foldback,
   closure, orbit, and squeeze?
2. Can the known controller-alias differences `42`, `126`, and `196` be aligned
   in forward and reverse contexts to create an exact after-foldback merge?
3. Is there a non-linear or higher-order relation between internal-state
   differences and digest differences missed by Pearson correlation?
4. Can SAT, SMT, MILP, or constraint programming break reduced variants and
   extrapolate to the canonical six-move controller?
5. Can the permutation-only state mutation be exploited through cycle,
   invariant-subset, group-relation, or histogram-preserving attacks?
6. Can multicollision, herding, expandable-message, or chosen-prefix techniques
   cross the foldback and length-framing boundary?
7. Can a full-digest distinguisher be constructed with materially fewer samples
   than generic statistical testing?
8. Can preimages or second preimages be found for truncated outputs at a cost
   that scales better than generic search?

## 4. Reporting requirements

A report should include:

- candidate identifier and commit;
- exact attack class and target phase;
- complete messages or deterministic generation procedure;
- measured time, memory, and number of candidate evaluations;
- expected generic cost and the comparison method;
- exact-state verification after fingerprint or LSH filtering;
- source code and build instructions when possible;
- whether the result applies to canonical or reduced parameters.

Use the `Cryptanalysis finding` GitHub issue template. Responsible private
contact may be used before public disclosure when a practical full-candidate
break is found.

## 5. Credit policy

Reproducible findings are recorded in `docs/ATTACK_LOG.md` with author credit,
including negative results that materially improve understanding. Independent
reviewers retain authorship of their analyses and should state preferred names,
affiliations, and citation details.
