# Candidate Freeze Policy

This repository may hold **more than one** frozen candidate identifier. They
must not be silently mixed.

---

## Frozen candidate A — PVC-RotHash-1 (historical baseline)

```text
Algorithm: PVC-RotHash-1
Candidate: 1.0.0-rc1
Tag:       v1.0.0-rc1
Digest:    256 bits
Role:      Historical baseline; known forward multicollisions
```

Normative definition: `SPECIFICATION.md`. Official digest and phase vectors:
`test-vectors/official-v1.json`, `phase-vectors-v1.json`.

**Not the competitive product path.** Production use prohibited.

### What is frozen (RotHash-1)

- the 512-byte Perfect Value Cube constant;
- coordinate and axis conventions;
- six moves per absorbed symbol;
- **RotHash-1** move-controller formulas;
- forward and reverse-foldback ordering;
- length framing;
- diagonal and orbit closures;
- four-diagonal squeeze;
- 32-byte output;
- all official **v1** test vectors.

---

## Frozen candidate B — PVC-RotHash-2 (draft public-attack freeze)

```text
Algorithm: PVC-RotHash-2
Candidate: 0.2.0-draft
Tag:       v0.2.0-rothash2-draft
Digest:    256 bits
Role:      Active experimental successor (Controller S absorb)
```

Normative absorb: `SPECIFICATION_ROTHASH2.md`. Shared finalization shape:
`SPECIFICATION.md` §§10–15. Official digests: `test-vectors/official-v2.json`.

**Still experimental.** Production use prohibited. No security claim.

### What is frozen (RotHash-2 0.2.0-draft)

- systematic Controller **S** absorb equations as in `SPECIFICATION_ROTHASH2.md`;
- finalization parameters as used by `pvc::RotHash2` (same counts as RotHash-1);
- all official **v2** digest vectors;
- algorithm identifier `PVC-RotHash-2`.

---

## Allowed changes (either candidate)

While a candidate is frozen, the tree may receive:

- documentation clarifications that do not change that candidate's vectors;
- build, portability, analysis, or tooling fixes that do not affect its digests;
- new attacks, negative results, and reproducibility data;
- implementation bug fixes when an implementation conflicts with its frozen
  specification.

Any algorithmic change creates a **new** candidate identifier. It must not
silently replace PVC-RotHash-1 or PVC-RotHash-2.

---

## Break criteria

A frozen candidate is considered broken for its stated research target if a
reproducible result demonstrates any of the following substantially below the
corresponding generic cost:

- a full 256-bit collision;
- a chosen-prefix or practical second-preimage construction;
- a practical preimage construction;
- a scalable full-digest distinguisher;
- a reduced-complexity attack that extrapolates credibly to the canonical
  parameters;
- a specification ambiguity that permits incompatible conforming digests.

For **RotHash-1**, forward-state equality alone is already known and is not a
full collision. For **RotHash-2**, short-domain forward merges were **not**
found in-house; constructing one is a high-value result (see challenge R2-C5).
Reduced-round collisions must name the research preset.
