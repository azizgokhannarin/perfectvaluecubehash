# Security Policy

## Candidate status

PVC-RotHash-1 1.0.0-rc1 is a frozen, unproven research candidate published for
independent cryptanalysis. Statistical and bounded attack results are not a
security proof.

Do not use it for password storage, signatures, certificates, updates, file
integrity, authentication, key derivation, commitments, proof-of-work, or any
production security boundary.

## Reporting

Use the GitHub `Cryptanalysis finding` issue template for public results. A
practical full-candidate break may be reported privately to the repository
maintainer before publication. Include:

- candidate version and commit;
- strongest affected phase;
- exact messages or deterministic generation procedure;
- time, memory, and number of evaluations;
- expected generic complexity;
- exact-state verification after any fingerprint/LSH filter;
- source code and reproduction commands.

A weakness is a successful project result. Reviewers receive public credit in
the attack log and later paper according to their preferred attribution.

## Interpretation rules

- Forward-state equality is known and is not a full hash collision.
- Reduced-round collisions are known and must name the preset.
- Near-state or near-digest measurements are not collisions.
- Foldback is not globally injective; sampled reverse contexts contain sparse
  controller aliases.
- No full 256-bit digest collision, preimage, or second preimage is currently
  known to this project.
- Passing the included tools is not evidence of cryptographic security.

See `docs/SECURITY_TARGET.md`, `docs/KNOWN_CRYPTOANALYSIS.md`, and
`CRYPTANALYSIS_CHALLENGE.md`.
