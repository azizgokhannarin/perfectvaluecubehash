# Paper Preparation Plan

The paper will be drafted after the frozen candidate has been publicly available
for independent analysis. This file records the intended evidence structure so
that later writing does not overstate the results.

## Proposed title

**PVC-RotHash-1: A Falsification-Oriented Hash Candidate Based on Intersecting
Rotations of a Perfect Value Cube**

## Proposed sections

1. Motivation and design constraints (primitive-free, PVC geometry)
2. Perfect Value Cube construction and geometry
3. Normative hash construction (PVC-RotHash-1)
4. Evolution from raw diagonal extraction to four-diagonal squeeze
5. Statistical evaluation (v0 distinguishers removed in tested domains)
6. Reduced-round and transition analysis
7. Forward controller aliases and multicollisions (42/126/196; bridged 2^32)
8. Foldback separation analysis (domains; inductive budget-close, not a proof)
9. Digest-surface and barrier experiments
10. Controller redesign experiments (E vs G injectivity catalogue; negative for
    coefficient-only G)
11. Performance
12. Limitations, design judgment, and open problems
13. Reproducibility and independent review

## Goal note

The paper’s primary product is **scientific clarity**, not a security claim.
A successor candidate appears only if a later controller clears short-domain
aliases; until then RotHash-1 is the attacked baseline.

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
