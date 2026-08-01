# Security Target and Claim Boundaries

## Ambition versus claim

**Ambition (program goal):** a general-purpose hash in this design family that
can stand public cryptanalysis at a quality bar comparable to serious modern
hashes (SHA-3 / Keccak as reference class), including honest post-quantum
parameter discussion.

**Current claim for PVC-RotHash-1:** **none** of the competitive properties
below are claimed. RotHash-1 is an experimental frozen baseline with known
forward multicollision structure.

Successor candidates inherit the same table as **targets** until evidence and
review justify carefully worded statements. Targets are not theorems.

## Aspirational generic targets (256-bit digest)

| Property | Generic reference | RotHash-1 claim | Successor program target |
|---|---:|---|---|
| Collision resistance | about `2^128` work | **Not claimed** | Meet this scale vs known attacks |
| Second-preimage resistance | about `2^256` work | **Not claimed** | Meet this scale vs known attacks |
| Preimage resistance | about `2^256` work | **Not claimed** | Meet this scale vs known attacks |
| Output distribution | statistical ideal | bounded tests only | No simple distinguisher in large samples |
| Length-extension resistance | construction-specific | no formal claim | No trivial extension |
| Quantum collision (query model) | far below `2^128` for 256-bit ideal | **Not claimed** | Document; consider 384/512-bit variants as separate IDs if needed |
| Quantum preimage (Grover) | about `2^128` queries for 256-bit ideal | **Not claimed** | Document; parameterize honestly |

## Structural gate (hard, before any competitive language)

Any successor **MUST** pass `docs/CONTROLLER_REQUIREMENTS.md` injectivity gates
G1–G3 (and G4 regressions). Cheap forward multicollisions of the RotHash-1 type
are **disqualifying** for the competitive program.

## Post-quantum wording

No candidate in this repository currently claims post-quantum security.

Under standard quantum query models, a 256-bit ideal digest has:

- preimage cost on the order of Grover’s algorithm (`~2^128` queries);
- collision cost below the classical `2^128` birthday scale for some quantum
  algorithms.

These are **parameter notes**. A future 384- or 512-bit squeeze would be a
**new candidate identifier**. See Gate E in `docs/ACCEPTANCE_ROADMAP.md`.

## What has been established (RotHash-1 baseline)

Reproducible evidence includes:

- correction of v0 raw-diagonal multiplicity and strong short-message positional
  memory distinguishers under documented tests;
- near-ideal avalanche and distribution samples in stated domains;
- **forward non-injectivity** and scalable bridged multicollisions;
- foldback separation of known forward collisions in large finite domains
  (not a proof of foldback injectivity);
- digest-surface searches not beating generic references in documented budgets;
- offline redesign: E and G reduce but do not eliminate two-byte one-symbol
  aliases (`docs/PHASE1_CONTROLLER_CAMPAIGN.md`).

## What has not been established

No security reduction, indifferentiability proof, or proof of collision /
preimage resistance for any candidate. No successor has yet passed the
controller injectivity program.

## Production prohibition

Do not use PVC-RotHash-1 (or unreleased prototypes) for authentication,
signatures, certificates, password storage, KDFs, MACs, integrity verification,
commitments, proof-of-work, or any real security boundary.
