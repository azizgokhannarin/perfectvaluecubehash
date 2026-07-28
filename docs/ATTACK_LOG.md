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
