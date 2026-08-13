# Audit Package Contents

This source tree is the complete public-review package for:

- **PVC-RotHash-2** `0.2.0-draft` (active successor; draft freeze), and
- **PVC-RotHash-1** `1.0.0-rc1` (historical baseline).

## Normative artifacts

### RotHash-2

- `SPECIFICATION_ROTHASH2.md`
- `test-vectors/official-v2.json`
- `docs/RELEASE_NOTES_0.2.0_ROTHASH2_DRAFT.md`

### RotHash-1 / shared finalization

- `SPECIFICATION.md`
- `test-vectors/official-v1.json`
- `test-vectors/phase-vectors-v1.json`
- `docs/SPEC_FREEZE.md`

## Conformance implementations

- `include/`, `src/`, and `app/` — C++20 (`RotHash1`, `RotHash2`, CLI `--rothash2`)
- `reference/python/pvc_rothash1.py`, `pvc_rothash2.py`
- `scripts/verify_vectors.py`, `scripts/verify_vectors_v2.py`

## Cryptanalysis material

- `CRYPTANALYSIS_CHALLENGE_ROTHASH2.md` (R2-C1…R2-C9)
- `CRYPTANALYSIS_CHALLENGE.md` (RotHash-1 C1–C8)
- `docs/STAGE4_DEEP_R2.md`, `docs/STAGE4_DIGEST_R2.md`
- `docs/INDEPENDENT_REVIEW.md`
- `docs/KNOWN_CRYPTOANALYSIS.md`
- `docs/SECURITY_TARGET.md`
- `tools/` and `include/pvc/research.hpp` (preset `R5-rothash2`)
- historical result files under `docs/` and `results/`

## Research integrity

- `SECURITY.md`
- `AUTHORS.md`
- `CITATION.cff`
- `LICENSE`
- GitHub issue templates under `.github/ISSUE_TEMPLATE/`
- CI workflow under `.github/workflows/ci.yml`
- Long-horizon acceptance gates in `docs/ACCEPTANCE_ROADMAP.md`
- Suggested issues: `docs/PUBLIC_REVIEW_ISSUE_ROTHASH2.md`,
  `docs/PUBLIC_REVIEW_ISSUE.md`

**New reviewers of the competitive path:** start with
`CRYPTANALYSIS_CHALLENGE_ROTHASH2.md` and `docs/INDEPENDENT_REVIEW.md`.
