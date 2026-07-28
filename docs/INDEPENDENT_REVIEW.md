# Independent Cryptanalysis Guide

This guide is the preferred entry point for external reviewers.

## 1. Reproduce the candidate

```bash
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release \
  -DPVC_WARNINGS_AS_ERRORS=ON
cmake --build build --parallel
ctest --test-dir build --output-on-failure
python3 scripts/verify_vectors.py \
  --cpp build/pvc-hash \
  --vector-dump build/pvc-vector-dump
```

A reviewer should first verify the two anchor vectors and the complete vector
corpus. The Python implementation under `reference/python/` is intentionally
independent of the C++ source structure.

## 2. Read in this order

1. `SPECIFICATION.md` — frozen normative algorithm.
2. `docs/SECURITY_TARGET.md` — exact non-claims and desired targets.
3. `docs/KNOWN_CRYPTANALYSIS.md` — consolidated positive and negative results.
4. `docs/ATTACK_MODEL.md` — terminology and state boundaries.
5. `docs/CRYPTANALYSIS_FRAMEWORK.md` — reduced-round API and tools.
6. Detailed result documents for the attack class of interest.

## 3. High-value attack questions

Priority questions are:

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
