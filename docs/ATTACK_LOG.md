# Attack Log

Failures are retained as research results even after a later revision changes
the design.

## A-001 — Forward-state convergence collision

**Affected design:** predecessor of 0.1.0
**Status:** specific failure mitigated by reverse foldback

The first forward-only implementation produced:

```text
M1 = 07 0e
M2 = 07 4c

H(M1) = H(M2)
      = 40ef978c7c43ab0050a607ce52b971a6
        b4e19886c379d69944b2f8de2cff0b3b
```

Different second-byte move fragments converged to the same cube, cursor, and
previous axis. The remaining closure was therefore identical.

This disproved the assumption that an intersecting chain must be unique.

A reverse message foldback was introduced. Exhaustive one- and two-byte search
then found no collision in those domains. That result does not establish
collision resistance.

## A-002 — Raw-diagonal multiplicity distinguisher

**Affected version:** PVC-RotHash-0 0.1.0
**Status:** removed from version 1 extraction; broader histogram attacks remain open

Version 0 concatenated 32 cells from one final cube. Since every byte value
occurs exactly twice in that cube, no output byte could occur three times.

A uniformly sampled 32-byte string has a nonzero probability of containing a
byte at least three times. Therefore a digest with such a repetition was an
immediate proof that the value did not come from version 0.

Version 1 derives consecutive bytes from evolving states and combines all four
body diagonals. In the documented 100,000-message experiment, triple-byte
repetitions occurred in 6.800% of digests, consistent with the reference
behavior rather than being impossible.

## A-003 — Canonical-coordinate positional memory

**Affected version:** PVC-RotHash-0 0.1.0
**Status:** not reproduced against 0.2.0 in documented samples

Independent review reported that every output position had a very large
chi-square deviation. In 31 of 32 positions, the most frequent output value was
the byte initially stored at that coordinate; the remaining position preferred
its complement. A full-state experiment showed the same effect across the cube,
so this was a walk-mixing failure rather than only a poor choice of output cells.

Local reproduction confirmed strong short-message positional bias in version 0.

Version 1 added:

- 64 self-fed diagonal closure symbols;
- 128 full-cube orbit symbols covering all 512 cells;
- a 32-state four-diagonal squeeze.

Documented version 1 measurements:

```text
All 65,536 two-byte messages:
mean per-output-position chi-square = 255.4556
maximum output-bit |z|              = 3.1172

100,000 deterministic 16-byte messages:
mean per-output-position chi-square = 254.5854
maximum output-bit |z|              = 2.7891

50,000 deterministic 16-byte messages, final cube:
mean per-cell chi-square             = 255.3289
canonical/complement preferred value = 6 of 512 cells
```

These samples reject the original simple distinguisher. They do not prove that
no higher-order or larger-sample distinguisher exists.

## Reporting format

Future entries should include:

- affected version or commit;
- exact attack class and reproducer;
- measured and expected generic complexity;
- security impact;
- whether the issue is fixed, mitigated, or open.


## A-004 — Canonical forward-state convergence

**Affected component:** canonical forward absorption in PVC-RotHash-1 0.2.0/0.3.0
**Status:** open structural property; separated by foldback in the exhaustive
two-byte domain

The three observed two-byte pairs are:

```text
17 6f / 17 99
25 1c / 25 46
a2 6f / a2 99
```

Each pair reaches the same complete operational state after forward absorption. The state
comparison includes all 512 cube bytes, cursor, previous axis, and symbol index.

Full processing separates them:

```text
after foldback equal = no
final state equal    = no
digest equal         = no
```

Across all 65,536 two-byte messages, three forward-state merges were found and
none survived foldback. Exhaustive common two-byte suffix extension of all
three pairs also found no after-foldback collision. This disproves forward-
transition injectivity but is not a full hash collision.

## A-005 — Reduced-round full-state collapse

**Affected configurations:** research presets R0–R2
**Status:** expected falsification result; canonical algorithm unchanged

Exhaustive two-byte phase enumeration produced:

```text
R0 final-state collisions = 65,340
R1 final-state collisions = 11,845
R2 final-state collisions =    658
R3 final-state collisions =      0
R4 final-state collisions =      0
R5 final-state collisions =      0
```

R0–R2 also produced full digest collisions. The reduced-round suite therefore
provides a measurable transition between clearly broken and not-yet-broken
configurations in this small domain.


## A-006 — Context-dependent move-controller aliases

**Affected component:** canonical forward absorption in PVC-RotHash-1 0.4.0
**Status:** open structural property; no after-foldback collision in the full
three-byte domain

Exhaustive enumeration of all 16,777,216 three-byte messages produced 1,496
exact forward-state pairs. Of these, 768 are the three known two-byte merges
extended by a common third byte. The remaining 728 use the same first two bytes
and different third bytes.

Every new pair generates an identical six-move physical trace. Absolute symbol
differences are 42, 126, or 196, all even multiples of seven. This shows that
the forward non-injectivity is caused by aliases in the move controller rather
than only by distinct rotation paths converging.

No pair survived foldback. A separate exhaustive scan of all three-byte
after-foldback states also found zero collision. The property remains a design
risk because forward uniqueness is absent and the tested collision resistance
therefore depends strongly on foldback.

## A-007 — Independent-suffix constrained MITM attempt

**Affected component:** foldback stage in PVC-RotHash-1 0.4.0
**Status:** attack unsuccessful in the tested domains

For each of the three known two-byte forward-collision prefix pairs, every
two-byte suffix on the left was compared against every two-byte suffix on the
right at the after-foldback state. The meet-in-the-middle search covered 2^32
cross combinations per prefix pair.

No exact state merge was found. Equal after-foldback states for these
equal-length messages would have implied identical remaining finalization and a
full digest collision. Longer suffixes and other collision prefixes remain open.


## A-008 — Foldback-aware dual-alias search

**Affected component:** interaction between forward aliases and reverse foldback
**Status:** attack unsuccessful in the tested structured domains

Every known three-byte forward collision was tested to determine whether the
two differing original bytes also generate an exact return-transition alias
from the common forward state. No direct dual alias was found. Exhaustive common
one-byte suffix extension covered 382,976 cases with zero after-foldback merge.
The first 4,096 two-byte suffixes for every pair covered 6,127,616 additional
cases with zero exact merge.

The minimum return-state cube distance decreased from 172 bits without a suffix
to 114 bits with one-byte extensions and 78 bits in the sampled two-byte run.
This trend motivates distance-guided attacks even though exact equality was not
observed.

## A-009 — Four-message forward multicollisions

**Affected component:** canonical forward absorption in PVC-RotHash-1 0.5.0
**Status:** open structural weakness; separated by foldback in tested families

Fifteen of the 1,496 known three-byte common forward states contain a second
one-symbol controller alias. Chaining the two aliases creates four distinct
messages with one exact complete forward state. One example is:

```text
176f115b
176f1185
1799115b
17991185
```

All four after-foldback states and full digests are distinct. All common one-byte
suffixes and the first 4,096 common two-byte suffixes were tested for every one
of the fifteen four-way families without an after-foldback or digest collision.
No third immediate collision level was found from these states in the bounded
search. Version 0.6.0 superseded this limitation by inserting common bridge
bytes between alias levels.

This demonstrates that forward aliases are composable. A deeper alias tree or a
construction that also satisfies return-symbol constraints remains a direct
collision threat.


## A-010 — Bridged exponential forward multicollisions

**Affected component:** canonical forward absorption in PVC-RotHash-1 0.6.0
**Status:** confirmed structural weakness; materialized families separated by foldback

Allowing one shared bridge byte between controller aliases yields a 32-level
forward collision path. It represents 2^32 distinct messages with one complete
forward state. The first 16 levels were materialized as 65,536 messages; every
message reached the same forward state, while all after-foldback states and
full digests were distinct.

This supersedes the earlier suggestion that multicollision depth was limited to
two. The forward stage has a scalable Joux-like multicollision mechanism.

## A-011 — Complete known-catalogue foldback separation profile

**Affected component:** reverse foldback in PVC-RotHash-1 0.6.0
**Status:** attack unsuccessful in tested catalogue; separation mechanism characterized

All 1,496 known three-byte forward collision pairs were traced step by step.
Every pair diverged exactly when reverse traversal encountered its last
differing original byte. No differing return-symbol pair aliased at that gate,
no pair diverged late, and no pair reconverged later. The first differing
return transition changed at least 49 cube cells.

All independently selected one-byte suffix pairs were also tested for every
forward collision pair, covering 98,041,856 logical cross combinations with
zero exact after-foldback merge.

## v0.7.0 — Guided foldback distance campaign

### Beam-search near state

A beam of 1,024 message pairs was propagated through a fixed 16-level bridged
forward multicollision path. The smallest complete after-foldback operational
state distance was 224 bits at level 11. Later levels increased the distance.
No exact collision was found.

### Independent-suffix LSH

For inherited prefix pair `176f00` / `179900`, 8,192 two-byte suffixes per side
created 67,108,864 logical pairs. Projection bucketing selected 13,040,013
candidates for full-state scoring. Minimum distance was 180 bits; no merge.

For local alias `af671b` / `af67df`, the same bounded domain reached 578 bits;
no merge.

### Reverse-context aliases

Among 2,304 sampled reachable reverse contexts, 24 contained a controller alias.
All observed differences were 42, 126, or 196. Therefore the reverse transition
must not be assumed injective, even though no tested complete message pair has
aligned such an alias into an after-foldback collision.

### Outcome

The attacks improve near-state distances but do not show a stable trajectory to
zero. The design remains unbroken in these finite domains, not proven secure.


## 2026-07-28 — Direct digest-surface searches

**Target:** the v0.7.0 blind spot: collisions between messages that do not share
a forward state, and the possibility that internal-state closeness provides a
gradient toward digest closeness.

**Methods:** digest-guided bridged beam, forward-divergent beam, multi-domain
digest LSH, and phase/digest correlation measurement.

**Result:** no exact digest collision. Observed best Hamming distances matched
generic multiple-comparison references: 96/96 bits in the same-forward beam,
94/93 bits in the divergent beam, and 83–85/84 bits across four LSH domains.
Phase-to-digest correlations stayed close to zero.

**Interpretation:** no measured sub-generic digest attack or useful linear
state-to-digest gradient in these domains. This is bounded negative evidence,
not a security proof.


## A-R2-001 — Stage 4 deep tools on PVC-RotHash-2 (negative)

**Affected design:** PVC-RotHash-2 draft (`R5-rothash2`, Controller S absorb)  
**Date:** 2026-08-13  
**Status:** no structural break in stated budgets (not a security claim)

The in-house attack suite was pointed at RotHash-2 via
`scripts/stage4_deep_r2.sh` (standard mode) plus full three-byte scans.

**Positive for RotHash-1 that did not transfer:**

| RotHash-1 fact | RotHash-2 observation |
|---|---|
| 3 two-byte forward pairs | 0 (exhaustive 2^16 digests + phases) |
| 1,496 three-byte forward pairs | **0** exact state pairs on full **2^24** forward domain |
| Bridged multi-level families | `path_found=no` (seed_pairs=0) |
| Δ∈{42,126,196} controller aliases | return-alias surface 0/4608 contexts; Δ=42 alignment exact convergences 0 |

**Other negatives (budget-limited):**

- Exhaustive 1- and 2-byte digests unique under C++ `RotHash2`
- Phase collisions: 0 at every recorded phase for 1- and 2-byte domains
- Multicollision / foldback-aware / dual-return: zero seed forward pairs
- Related-input reverse/complement/rotate: 0 digest matches (n=256)
- Truncated 24/32-bit near birthday; 40-bit censored at 200k/trial
- Differential single-bit: mean digest Hamming ~128.5; 0 exact matches

**Commands:** `docs/STAGE4_DEEP_R2.md`, logs in `results/stage4-r2/`.

**Interpretation:** Controller S removes the short-domain forward multicollision
surface that dominated RotHash-1 cryptanalysis. Remaining work is longer
domains, digest-surface search, and external review — not replaying R1 alias
catalogues.
