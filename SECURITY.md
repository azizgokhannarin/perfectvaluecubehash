# Security Policy

## Experimental status

PVC-RotHash-1 is an unreviewed research prototype. Its statistical results are
not a security proof.

Do not use it for password storage, signatures, certificates, software-update
verification, file integrity, authentication, key derivation, proof-of-work, or
any production security boundary.

## Reporting a weakness

Open a GitHub issue containing:

- affected version or commit;
- attack class;
- exact inputs or reproduction procedure;
- measured complexity;
- expected generic complexity;
- source code or script when available.

A weakness is a successful project result. Distinguishers and negative results
must be reported even when they do not immediately produce collisions.


## Research parameter warning

Reduced-round presets are intentionally weak and several have documented full
collisions. They exist only for cryptanalysis. The presence of an
`R5-canonical` preset does not imply that R5 is secure.

Forward-state equality must not be reported as a full hash collision unless the
pair also survives foldback, finalization, and digest comparison.


## Forward multicollision warning

Version 0.6.0 documents constructible exponentially large forward families. A
fully materialized example contains 65,536 distinct messages that reach one
exact state at the end of forward absorption. This is a forward-pass
multicollision, not a full hash collision: all 65,536 tested paths are distinct
after foldback and produce distinct digests. The search also found a 32-level
path representing a theoretical 2^32-message forward family, which was not
fully materialized. Reports must state the phase at which equality occurs.


## Foldback-distance warning

Version 0.7.0 did not find an exact after-foldback collision, but guided searches
reduced selected state distances to 224 bits in a bridged family and 180 bits in
a bounded independent-suffix domain. These are near-state measurements, not
collisions. Reports must include the search domain, scoring method, and whether
the distance decreases consistently as resources increase.

Reachable reverse contexts can contain move-controller aliases. In the sampled
0.7.0 campaign, 24 of 2,304 contexts contained one alias, with differences 42,
126, or 196. Foldback must not be described as an injective transition system.

## Version 0.8.0 status

The digest-surface campaign directly searched both same-forward and
forward-divergent message families. Observed minimum digest distances tracked
the generic 256-bit Hamming references, and phase-state distance showed near-zero
linear correlation with final digest distance in the tested domains. No exact
full-digest collision was found.

These results support treating closure and squeeze as a second measured
diffusion barrier, but they do not establish collision or preimage resistance.
The design remains unsuitable for production or security-critical use.
