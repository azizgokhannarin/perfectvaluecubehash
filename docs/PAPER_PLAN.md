# Paper Preparation Plan

The paper will be drafted after the frozen candidate has been publicly available
for independent analysis. This file records the intended evidence structure so
that later writing does not overstate the results.

## Proposed title

**PVC-RotHash-1: A Falsification-Oriented Hash Candidate Based on Intersecting
Rotations of a Perfect Value Cube**

## Proposed sections

1. Motivation and design constraints
2. Perfect Value Cube construction and geometry
3. Normative hash construction
4. Evolution from raw diagonal extraction to four-diagonal squeeze
5. Statistical evaluation
6. Reduced-round and transition analysis
7. Forward controller aliases and multicollisions
8. Foldback separation analysis
9. Digest-surface and barrier experiments
10. Performance
11. Limitations and open cryptanalytic problems
12. Reproducibility and independent review

## Required evidence before submission

- frozen public specification and tag;
- two independent conforming implementations;
- official digest and phase vectors;
- public issue/report channel;
- consolidated attack log;
- at least one external review or a clearly stated period with no external
  review received;
- exact hardware and software details for all reported benchmarks;
- no claim of proven security.

## Claims that must not appear

The paper must not call PVC-RotHash-1 secure, post-quantum secure, production
ready, collision resistant, or preimage resistant without new evidence beyond
the current repository.
