# Candidate Freeze Policy

## Frozen candidate

The public-review candidate is:

```text
Algorithm: PVC-RotHash-1
Candidate: 1.0.0-rc1
Digest:    256 bits
```

The normative definition is `SPECIFICATION.md`. The official digest and phase
vectors are under `test-vectors/`.

## What is frozen

The following are frozen for the independent-review period:

- the 512-byte Perfect Value Cube constant;
- coordinate and axis conventions;
- six moves per absorbed symbol;
- move-controller formulas;
- forward and reverse-foldback ordering;
- length framing;
- diagonal and orbit closures;
- four-diagonal squeeze;
- 32-byte output;
- all official test vectors.

## Allowed changes

The frozen branch may receive:

- documentation clarifications that do not change test vectors;
- build, portability, analysis, or tooling fixes that do not affect the
  candidate digest;
- new attacks, negative results, and reproducibility data;
- implementation bug fixes when the implementation conflicts with the frozen
  specification.

Any algorithmic change creates a new candidate. It must not silently replace
PVC-RotHash-1.

## Break criteria

The candidate is considered broken for its stated research target if a
reproducible result demonstrates any of the following substantially below the
corresponding generic cost:

- a full 256-bit collision;
- a chosen-prefix or practical second-preimage construction;
- a practical preimage construction;
- a scalable full-digest distinguisher;
- a reduced-complexity attack that extrapolates credibly to the canonical
  parameters;
- a specification ambiguity that permits incompatible conforming digests.

Forward-state equality alone is already known and is not a full collision.
Reduced-round collisions are also known and must be reported with the preset.
