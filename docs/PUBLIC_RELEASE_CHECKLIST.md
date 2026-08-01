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
- [x] GCC, Clang, Windows, ASan, and UBSan CI workflow present (`.github/workflows/ci.yml`).

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

## Before / around GitHub tag

- [x] Run `scripts/release_check.sh` on a clean checkout (passed 2026-08-01, GCC 14.2).
- [ ] Confirm GitHub Actions is green after CI workflow is on `main`.
- [x] Tag `v1.0.0-rc1` exists on `origin` at freeze commit `30f1b78` (algorithm/vectors unchanged).
- [x] Audit kit described in `AUDIT_PACKAGE.md` (source tree is the package).
- [ ] Open a pinned public-review discussion or issue (draft: `docs/PUBLIC_REVIEW_ISSUE.md`).
- [ ] Announce that the algorithm is experimental and invite attacks (`docs/PUBLIC_REVIEW_ANNOUNCEMENT.md`).

## Gate A packaging notes

- Issue templates: `.github/ISSUE_TEMPLATE/`
- Acceptance strategy: `docs/ACCEPTANCE_ROADMAP.md`
- Tag `v1.0.0-rc1` freezes the **algorithm**. Later commits may add tooling, CI,
  and documentation without moving official vectors; they are not a new candidate.
