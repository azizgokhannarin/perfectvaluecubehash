# Consolidated Known Cryptanalysis

This document summarizes the state of analysis at candidate freeze. Detailed
commands and raw measurements remain in the version-specific result documents.

## Corrected early distinguishers

PVC-RotHash-0 concatenated 32 cells from one final cube. It had two immediate
structural distinguishers:

- no digest byte could appear more than twice;
- output positions retained a strong preference for the value originally at
  the corresponding canonical coordinate.

PVC-RotHash-1 replaced raw extraction with a 32-state four-diagonal squeeze and
added diagonal and full-cube closures. The documented distribution probes no
longer reproduce those distinguishers.

## Forward non-injectivity

The forward pass is conclusively non-injective.

- Three exact two-byte forward-state pairs exist in the full 65,536-message
  domain.
- The full three-byte domain contains 1,496 exact forward-state pairs.
- Controller aliases use byte differences 42, 126, or 196 in the observed
  contexts and execute identical physical move sequences.
- Common bridges allow a 32-level path representing a theoretical `2^32`
  forward multicollision family.
- A `2^16` subset was materialized: all 65,536 messages had one forward state.

Forward equality is not a full hash collision.

## Foldback separation

For the complete known three-byte forward-collision catalogue:

- all 1,496 pairs diverge when foldback processes their last differing original
  byte;
- direct return-transition aliases and later reconvergences were not found;
- the full three-byte after-foldback domain had zero exact collisions;
- 98,041,856 independent one-byte suffix cross pairs had zero after-foldback
  merge;
- three separate `2^32` independent two-byte suffix domains for the original
  two-byte prefixes had zero after-foldback merge;
- the 65,536-message forward family produced 65,536 after-foldback states and
  65,536 digests.

Foldback is not globally injective: aliases were found in 24 of 2,304 sampled
reachable reverse contexts. They again used differences 42, 126, or 196.

## Distance-guided attacks

Targeted search can reduce selected after-foldback state distances:

- bridged-family beam search reached 224 differing cube bits;
- a bounded independent-suffix LSH search reached 180 differing cube bits.

No exact after-foldback merge was found, and distance did not decrease
monotonically with additional multicollision levels.

## Full-digest surface

Digest-guided searches deliberately included pairs with different forward
states.

- Same-forward beam: minimum 96 digest bits among 42,325 evaluated pairs;
  generic reference minimum about 96.
- Forward-divergent beam: minimum 94 among 229,633 pairs; generic reference
  about 93.
- Four logical `8192 x 8192` LSH domains produced minima 83–85; generic reference
  about 84.
- Phase-state Hamming distances had Pearson correlations approximately between
  -0.02 and +0.02 with final digest distance in the tested campaigns.

No exact 256-bit digest collision has been found.

## Truncated collision results

The documented 24- and 32-bit campaigns tracked generic birthday behavior in
small trial sets. Results at 40 and 48 bits were censored below their expected
birthday cost and are not security evidence.

## Interpretation

The evidence rules out a collection of simple distinguishers and bounded
structural attacks. It does not prove collision or preimage resistance. The
strongest known weakness **of PVC-RotHash-1** is abundant forward multicollision
structure; the strongest measured defenses on that design are reverse foldback
separation and the apparent loss of internal-distance information through
closure and squeeze.

## PVC-RotHash-2 (experimental draft)

RotHash-2 replaces the absorb controller with systematic Controller **S** while
keeping the same finalization shape. Stage 4 entry campaigns
(`docs/STAGE4_DEEP_R2.md`, attack log **A-R2-001**) report:

- exhaustive two-byte digests and all recorded phases unique;
- **zero** exact three-byte forward and after-foldback operational pairs on the
  full `2^24` domain (contrast: RotHash-1 had 1,496 forward pairs);
- zero seed pairs for multicollision / bridged / dual-return tools in tested
  budgets;
- truncated 24/32-bit collisions near birthday cost in small trial sets.

**Digest surface (A-R2-002):** divergent beam bests 90/91 vs ~93 generic;
LSH bests 83–88 vs 84–86 generic; barrier correlations ~0; same-forward beam
inapplicable (`path_found=no`). Truncated 40-bit mean cost ~0.9× birthday
(4 trials, limit 2×10⁶). No exact 256-bit collision in these campaigns.

**Status:** experimental; production prohibited; no security claim. Larger
digest budgets, 48-bit+ truncated, and external review remain open.
