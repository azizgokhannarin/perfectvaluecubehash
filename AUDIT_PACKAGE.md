# Audit Package Contents

This source tree is the complete public-review package for PVC-RotHash-1
1.0.0-rc1.

## Normative artifacts

- `SPECIFICATION.md`
- `test-vectors/official-v1.json`
- `test-vectors/phase-vectors-v1.json`
- `docs/SPEC_FREEZE.md`

## Conformance implementations

- `include/`, `src/`, and `app/` — C++20 implementation
- `reference/python/pvc_rothash1.py` — independent pure-Python implementation
- `scripts/verify_vectors.py` — cross-implementation verifier

## Cryptanalysis material

- `CRYPTANALYSIS_CHALLENGE.md`
- `docs/INDEPENDENT_REVIEW.md`
- `docs/KNOWN_CRYPTOANALYSIS.md`
- `docs/SECURITY_TARGET.md`
- `tools/` and `include/pvc/research.hpp`
- historical result files under `docs/`

## Research integrity

- `SECURITY.md`
- `AUTHORS.md`
- `CITATION.cff`
- `LICENSE`
- GitHub issue templates under `.github/ISSUE_TEMPLATE/`
- CI workflow under `.github/workflows/ci.yml`
- Long-horizon acceptance gates in `docs/ACCEPTANCE_ROADMAP.md`
- Suggested public-review issue text in `docs/PUBLIC_REVIEW_ISSUE.md`

Start with `docs/INDEPENDENT_REVIEW.md`.
