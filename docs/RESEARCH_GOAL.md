# Research Goal — Competitive Hash Ambition

**Adopted:** 2026-08-01  
**Updated:** 2026-08-01  
**Status:** Binding goal for effort, design, and claims

## North star

Build a **real, competitive cryptographic hash** in the Perfect Value Cube /
intersecting-rotation design family:

- strong enough in structure and evidence to be discussed next to serious
  general-purpose hashes (SHA-3 / Keccak class as the **quality bar**, not a
  claim of equality);
- parameterized and documented honestly for the **post-quantum era** (digest
  length and quantum-query notes; no slogan security);
- **not** a “we tried and it failed” paper as the primary product.

A negative scientific write-up remains an honest **fallback** if the family
cannot meet the controller and cryptanalysis bars after serious redesign. It is
not the plan.

## What success looks like

1. A **new candidate identifier** (not silent RotHash-1 drift) whose move
   controller passes `docs/CONTROLLER_REQUIREMENTS.md` gates G1–G4.
2. Smoke statistical and reduced-round batteries (ST1–ST4) without reviving v0
   distinguishers.
3. Dual conforming implementations, official vectors, freeze policy, and a
   public attack surface.
4. Ongoing independent cryptanalysis with **no known structural break** of the
   kinds that killed RotHash-1’s acceptance path (cheap forward multicollisions).
5. Only then: language that inches from “experimental candidate” toward
   carefully scoped recommendations — still without fake proofs.

## What RotHash-1 is

PVC-RotHash-1 `1.0.0-rc1` is a **frozen research baseline**:

- fully specified, dual-implemented, heavily attacked in-tree;
- **does not** meet the competitive controller requirements;
- remains valuable as comparison, regression, and cryptanalytic evidence;
- **must not** be marketed as the path to Keccak-class standing.

## Design judgment

| Layer | Judgment |
|---|---|
| PVC geometry, intersecting rotations, foldback, closures, squeeze | Keep as family DNA unless evidence forces a larger fork |
| RotHash-1 controller | Defective for injectivity; replace for any successor |
| Coefficient-only patches (e.g. G) | Insufficient |
| Structural controller redesign + hard gates | **Main engineering path** |
| Foldback-distance grinding on known forward collisions | Low EV (`docs/EFFORT_POLICY.md`) |

## Claim discipline (unchanged)

- No “collision-resistant”, “PQC-secure”, or “Keccak replacement” claims without
  evidence that would survive expert review.
- Meeting G1–G4 is **necessary, not sufficient**.
- Production use stays prohibited until an explicit later policy change.

## Working order

Full trust path (stages 0–9, gates, time scale, anti-patterns):

→ **`docs/TRUST_PATH_ROADMAP.md`**

Short form:

```text
1. Controller injectivity G1–G3 (H* contract)
2. ST1–ST4 smoke
3. Mint new candidate ID + vectors + dual impl
4. Deep falsification suite
5. Public independent review window
6. PQ/parameter honesty + engineering
7. Only then careful recommendation language
```

See also:

- `docs/CONTROLLER_REQUIREMENTS.md`
- `docs/CONTROLLER_G3_FAILURE_MODE.md`
- `docs/SECURITY_TARGET.md`
- `docs/ACCEPTANCE_ROADMAP.md`
- `docs/EFFORT_POLICY.md`
