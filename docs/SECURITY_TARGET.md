# Security Target and Claim Boundaries

## Research target

PVC-RotHash-1 outputs 256 bits. The aspirational generic targets are:

| Property | Generic reference | Current claim |
|---|---:|---|
| Collision resistance | about `2^128` work | **Not claimed** |
| Second-preimage resistance | about `2^256` work | **Not claimed** |
| Preimage resistance | about `2^256` work | **Not claimed** |
| Random-oracle-like output distribution | statistical ideal | only bounded tests passed |
| Length-extension resistance | construction-specific | no formal claim |
| Quantum collision resistance | about `2^(256/3)` generic query scale | **Not claimed** |
| Quantum preimage resistance | about `2^128` generic query scale | **Not claimed** |

These values are comparison targets, not proven security levels.

## Post-quantum wording (non-claim)

PVC-RotHash-1 does **not** claim post-quantum security. A 256-bit digest is the
current candidate output length only. Under standard quantum query models:

- preimage search scales like Grover's algorithm (about `2^128` queries for a
  256-bit ideal random oracle preimage);
- collision search admits better-than-classical quantum algorithms whose
  generic cost is far below `2^128` work for 256-bit outputs.

Until a full security argument and independent cryptanalysis exist, quantum
remarks are **parameter-comparison notes**, not design guarantees. Any future
longer-output variant (for example 384- or 512-bit squeeze) would be a separate
candidate identifier under the freeze policy. See
`docs/ACCEPTANCE_ROADMAP.md` Gate E for the planned expansion of this section.

## What has been established

The repository provides reproducible evidence that:

- the direct-output version-0 distinguishers were corrected;
- canonical output distributions and single-bit avalanche are close to their
  random references in the documented domains;
- reduced-round variants fail progressively;
- the forward pass admits exact and exponentially extensible multicollisions;
- known forward multicollisions are separated by foldback in tested domains;
- selected internal-state distances do not linearly predict final digest
  distances after closure and squeeze;
- bounded full-digest searches have not outperformed generic reference minima.

## What has not been established

There is no security reduction, proof of indifferentiability, proof of
collision resistance, or proof of preimage resistance. The construction is a
new permutation-of-values design and may have undiscovered algebraic,
combinatorial, invariant, differential, meet-in-the-middle, SAT/SMT, or
long-message attacks.

## Production prohibition

Do not use the candidate for authentication, signatures, certificates, password
storage, KDFs, MACs, integrity verification, commitments, proof-of-work, or any
other real security boundary.
