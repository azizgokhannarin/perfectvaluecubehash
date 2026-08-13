# Public Draft-Freeze Checklist — PVC-RotHash-2 0.2.0-draft

## Candidate identity

- [x] Algorithm identifier fixed as `PVC-RotHash-2`
- [x] Candidate identifier fixed as `0.2.0-draft`
- [x] Normative absorb specification present (`SPECIFICATION_ROTHASH2.md`)
- [x] Freeze policy updated (`docs/SPEC_FREEZE.md`)
- [x] Production-use prohibition prominent

## Conformance

- [x] C++20 `pvc::RotHash2` / CLI `--rothash2`
- [x] Independent pure-Python `pvc_rothash2.py`
- [x] Official digest vectors (`official-v2.json`, 29 KATs; `official-v2.rsp`)
- [x] Automated cross-implementation verification (`verify_vectors_v2.py` / CTest)
- [ ] Phase-state dumps for v2 (optional; not required for draft freeze)
- [x] Release check covers RotHash-2 anchors (`scripts/release_check.sh`)

## Cryptanalysis package

- [x] Research preset `R5-rothash2`
- [x] Attack tools accept `--preset R5-rothash2` / `--rothash2`
- [x] Stage 4 operational + digest-surface logs (A-R2-001, A-R2-002)
- [x] Public challenge targets (`CRYPTANALYSIS_CHALLENGE_ROTHASH2.md`)
- [x] Suggested announcement + issue drafts

## Research integrity

- [x] Security claims separated from measured results
- [x] RotHash-1 baseline not silently replaced
- [x] Apache-2.0 license

## Before / around GitHub tag

- [x] Run `scripts/release_check.sh` on a clean tree before tagging
- [ ] Tag `v0.2.0-rothash2-draft` at the freeze commit
- [ ] Push tag to `origin`
- [ ] Open pinned public-review issue (text: `docs/PUBLIC_REVIEW_ISSUE_ROTHASH2.md`)
- [ ] Announce experimental status (`docs/PUBLIC_REVIEW_ANNOUNCEMENT_ROTHASH2.md`)
