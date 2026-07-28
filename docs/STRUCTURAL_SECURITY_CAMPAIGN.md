# v0.7.0 Structural Security Campaign

## Scope

This campaign keeps the canonical PVC-RotHash-1 algorithm and its known-answer
vectors unchanged. It targets the remaining concentration of security in the
foldback pass with four independent methods:

1. distance-guided search inside a bridged forward-multicollision family;
2. projection-LSH search over independently selected suffixes;
3. measurement of controller aliases in reachable reverse contexts;
4. multi-trial truncated-collision scaling and unequal-length framing checks.

All reported state collisions require exact equality of the full 512-byte cube,
cursor, previous axis, and symbol index. Screening projections and fingerprints
are never accepted without exact comparison.

## Foldback-distance beam search

Command:

```bash
./build/pvc-foldback-beam-search \
  --levels 16 --beam 1024 --threads 4 --print-limit 3
```

The tool first constructs a bridged 16-level forward multicollision path. At
each collision level it explores branch assignments that retain one exact
forward state, scores the resulting pair after complete foldback, and retains
the best 1,024 candidates.

Observed best operational-state distances by selected levels:

```text
seed level 1 = 812 bits
level 5      = 270 bits
level 10     = 230 bits
level 11     = 224 bits
level 14     = 290 bits
level 15     = 254 bits
```

The global minimum was:

```text
foldback bit distance  = 224
foldback byte distance = 60
digest bit distance    = 119
level                  = 11
exact merge            = no
```

The distance can be optimized substantially, but it does not decrease
monotonically as the bridged path grows. This experiment therefore gives a
near-collision direction, not a convergence law or an exact collision.

## Independent-suffix projection search

The projection-LSH tool generates after-foldback states for independently
selected suffixes on both sides of a known forward collision. Multiple
four-byte state projections create candidate buckets; every candidate is then
scored against the complete operational state.

### Inherited collision prefix

```bash
./build/pvc-foldback-lsh-search \
  --left 176f00 --right 179900 \
  --suffix-bytes 2 --suffix-limit 8192 \
  --projections 16 --projection-bytes 4
```

```text
suffixes per side             = 8,192
logical cross pairs           = 67,108,864
projection candidates scored  = 13,040,013
best foldback bit distance    = 180
best foldback byte distance   = 48
best digest bit distance      = 116
exact after-foldback merge    = no
```

Best pair in this bounded domain:

```text
176f000fcc
1799000fcc
```

### Local third-symbol alias

```bash
./build/pvc-foldback-lsh-search \
  --left af671b --right af67df \
  --suffix-bytes 2 --suffix-limit 8192 \
  --projections 16 --projection-bytes 4
```

```text
logical cross pairs           = 67,108,864
projection candidates scored  = 13,586,377
best foldback bit distance    = 578
best foldback byte distance   = 140
best digest bit distance      = 139
exact after-foldback merge    = no
```

The inherited pair was much more susceptible to distance reduction than the
sampled local alias. This difference is a useful target for future
classification; it is not evidence that all local aliases are stronger.

## Reachable return-alias surface

Command:

```bash
./build/pvc-return-alias-surface \
  --messages 256 --message-bytes 8 --print-limit 8
```

For each deterministic eight-byte message, aliases were enumerated at the
forward endpoint and after every actual reverse transition.

```text
reachable reverse contexts        = 2,304
contexts containing an alias      = 24
context alias probability         = 1.041667%
total alias pairs                 = 24
maximum aliases in one context    = 1
```

Alias differences:

```text
42  -> 13
126 -> 4
196 -> 7
```

The foldback controller therefore has a non-empty but sparse alias surface in
this sample. The same three even multiples of seven that govern forward aliases
also govern the observed reverse-context aliases. An attack must still align a
message pair, a common reachable context, the reverse position, and one of
these sparse symbol pairs.

## Truncated-collision campaign

The campaign uses independent deterministic domains and records first
collisions. Generic first-collision expectation is

```text
sqrt(pi/2) * 2^(n/2)
```

Results:

```text
24 bits, 8 trials, limit 30,000
found                    = 8/8
mean first collision     = 5,400.375
birthday expectation     = 5,133.575
mean/expected            = 1.0520
range                    = 605 .. 12,506

32 bits, 4 trials, limit 150,000
found                    = 4/4
mean first collision     = 92,608.75
birthday expectation     = 82,137.20
mean/expected            = 1.1275
range                    = 74,243 .. 112,833

40 bits, 1 trial, limit 100,000
found                    = 0
birthday expectation     = 1,314,195

48 bits, 1 trial, limit 100,000
found                    = 0
birthday expectation     = 21,027,122
```

The 24- and 32-bit observations remain compatible with generic birthday
scaling. The 40- and 48-bit runs are heavily censored and must not be treated as
meaningful positive security evidence.

## Unequal-length framing

Command:

```bash
./build/pvc-length-framing-probe --max-length 64
```

For canonical parameters:

```text
after forward  symbol_index = n
after foldback symbol_index = 2n
final          symbol_index = 2n + 330
```

All 65 tested lengths had distinct symbol indices at each phase. Therefore a
classic expandable-message construction based on identical complete
operational states at unequal lengths is blocked directly by the state model.
This does not rule out unequal-length digest collisions or more complex
compensation attacks.

## Interpretation

The campaign did not find an exact foldback or digest collision. It did show
that foldback distances are optimizable: 224 bits inside a bridged family and
180 bits in a bounded independent-suffix search. No monotonic route toward zero
was observed.

The return controller itself is not injective in every reachable context. Its
observed aliases are sparse and restricted to differences 42, 126, and 196.
This narrows the attack problem but also confirms that foldback cannot be
modelled as an everywhere-injective primitive.

PVC-RotHash-1 remains an experimental candidate. These finite results justify
continued external cryptanalysis; they do not establish collision, second-
preimage, or preimage resistance.
