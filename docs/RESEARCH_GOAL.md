# Research Goal — Science First, Acceptance Contingent

**Adopted:** 2026-08-01  
**Status:** Binding goal ordering for effort and claims

## What we want

1. **Primary (now):** a **publishable scientific result** — honest, reproducible,
   attack-complete enough to stand as research on an original Perfect Value Cube
   rotation hash.
2. **Aspirational (later):** that a **corrected** member of this design family
   might someday earn serious consideration as a general-purpose hash.

These are compatible only in this order. Acceptance is **not** pursued by
testing a known structural defect harder. Acceptance is pursued only if the
design family can be repaired without abandoning its identity.

## Design judgment (current)

| Layer | Judgment |
|---|---|
| Geometry (PVC, intersecting line rotations, foldback, closures, squeeze) | **Worth continuing** as a research architecture |
| RotHash-1 move controller (linear double use of symbol, amount `% 7`, LSB axis) | **Design defect** for injectivity; blocks acceptance |
| Offline prototype E | Shows the 42-family is not inevitable; residual mod-7 lane rail remains |
| Endless foldback / ARX thrash | **Low value** (`docs/EFFORT_POLICY.md`) |

**Continue the design family.** Do **not** claim RotHash-1 is on a short path to
standardization. Publish RotHash-1 as a frozen, attacked baseline; put any
acceptance hope on a **new candidate** after a principled controller fix.

## Decision rule

```text
IF controller redesign achieves zero one- and two-byte one-symbol aliases
   AND cheap statistical smokes do not revive v0 distinguishers
THEN mint a new candidate ID and re-open deep cryptanalysis
ELSE keep RotHash-1 as the scientific object (paper / negative+constructive)
     and do not market acceptance
```

Either branch is a **success** if it is honest and publishable.
Only the first branch keeps the acceptance aspiration alive.

## Paper-shaped outcomes (both valid)

- **Constructive:** original construction + cryptanalysis + redesign rationale.
- **Negative+constructive:** why the first controller fails, what foldback did
  and did not buy, what a repaired controller must satisfy.

See `docs/PAPER_PLAN.md`, `docs/CONTROLLER_DECISION_MEMO.md`,
`docs/EFFORT_POLICY.md`.
