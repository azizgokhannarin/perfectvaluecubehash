# Public Independent-Review Release Checklist

## Candidate identity

- [x] Algorithm identifier fixed as `PVC-RotHash-1`.
- [x] Candidate identifier fixed as `1.0.0-rc1`.
- [x] Normative specification present.
- [x] Algorithm-change policy present.
- [x] Production-use prohibition prominent.

## Conformance

- [x] C++20 reference implementation.
- [x] Independent pure-Python implementation.
- [x] 32 official digest vectors.
- [x] Five phase-state vectors with full 512-byte states.
- [x] Automated cross-implementation verification.
- [x] GCC, Clang, Windows, ASan, and UBSan CI coverage.

## Cryptanalysis package

- [x] Reduced-round parameters and exact state snapshots.
- [x] Historical attack tools retained.
- [x] Known forward collisions and multicollisions disclosed.
- [x] Consolidated known-results document.
- [x] Explicit public challenge targets.
- [x] Reproduction commands and issue templates.

## Research integrity

- [x] Security claims separated from measured results.
- [x] AI assistance disclosed.
- [x] Apache-2.0 license.
- [x] Citation metadata.
- [x] External reviewers offered attribution.

## Before GitHub tag

- [ ] Run `scripts/release_check.sh` on a clean checkout.
- [ ] Confirm GitHub Actions is green.
- [ ] Tag `v1.0.0-rc1` without changing vectors.
- [ ] Attach source archive and audit kit.
- [ ] Open a pinned public-review discussion or issue.
- [ ] Announce that the algorithm is experimental and invite attacks.
