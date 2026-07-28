# Foldback-Aware Alias and Forward Multicollision Results

Environment: GCC 14.2.0, Release build, warnings as errors.

Version 0.5.0 does not change the canonical `PVC-RotHash-1` algorithm or its
known-answer vectors. It adds attacks that combine the known forward controller
aliases with the return-symbol equations used by foldback.

## Collision catalogue used by the attacks

The complete three-byte forward catalogue contains 1,496 exact pairs:

```text
local third-symbol aliases = 728
inherited two-byte aliases = 768
total                       = 1,496
```

The forward symbol differences, expressed as unsigned byte differences, are:

| Difference | Pairs |
|---:|---:|
| 42 | 1,310 |
| 126 | 89 |
| 196 | 97 |

The count for difference 42 includes the 768 inherited pairs as well as local
third-symbol aliases.

## Direct forward/return dual-alias test

For every three-byte forward collision pair, the attack starts from the exact
common forward state, applies the two different return symbols at the position
where the messages differ, and compares the complete resulting operational
states.

Without an appended suffix:

```text
forward collision pairs tested      = 1,496
return-transition exact merges      = 0
exact after-foldback merges          = 0
minimum return cube bit distance     = 172
minimum return cube byte distance    = 49
closest pair                         = a26ffe / a299fe
```

Thus none of the known forward aliases is simultaneously a one-step return
alias in its original three-byte context.

## Exhaustive common one-byte extension

Every forward collision pair was extended by every common one-byte suffix:

```text
1,496 * 256 = 382,976 structured extension cases
```

After processing the common suffix forward, processing its return symbol, and
then applying the two differing return symbols:

```text
return-transition exact merges      = 0
exact after-foldback merges          = 0
minimum return cube bit distance     = 114
minimum return cube byte distance    = 39
closest pair                         = 251c8d97 / 25468d97
```

This is also a targeted four-byte foldback-aware collision search.

## Sampled common two-byte extension

The first 4,096 two-byte suffixes were applied to every collision pair:

```text
1,496 * 4,096 = 6,127,616 structured extension cases
return-transition exact merges      = 0
exact after-foldback merges          = 0
minimum return cube bit distance     = 78
minimum return cube byte distance    = 22
closest pair                         = 909d870a39 / 909db10a39
```

The complete two-byte common-suffix domain would contain 98,041,856 cases and
is not claimed to have been exhausted by this run. The decreasing minimum
distance shows that longer controlled suffixes can bring return paths closer
even when exact equality is not found.

## Forward multicollision construction

Each of the 1,496 common forward states was searched for another one-symbol
controller alias. Fifteen states contained one additional alias pair.

```text
three-byte forward collision seeds  = 1,496
seeds with a next symbol alias       = 15
next symbol alias pairs              = 15
maximum observed collision levels    = 2
```

The next-alias differences are:

| Difference | Branches |
|---:|---:|
| 42 | 11 |
| 126 | 3 |
| 196 | 1 |

One explicit four-message forward multicollision is:

```text
176f115b
176f1185
1799115b
17991185
```

All four messages reach exactly the same complete forward state. Their four
after-foldback states and four full digests are all distinct.

No third collision level was found from any of the fifteen four-way common
states in the search bounded to five levels. This is an empirical statement
about these states, not a proof that longer multicollisions do not exist.

## Common-suffix tests against the four-way families

All fifteen four-way families were extended by all 256 common one-byte suffixes:

```text
four-way extension cases             = 3,840
cases with an after-foldback collision= 0
cases with a digest collision        = 0
minimum foldback pair cube distance  = 190
```

The first 4,096 common two-byte suffixes were also tested for every family:

```text
four-way extension cases             = 61,440
cases with an after-foldback collision= 0
cases with a digest collision        = 0
minimum foldback pair cube distance  = 170
```

## Interpretation

The forward controller supports a real multicollision mechanism: independent
controller aliases can be chained, producing four distinct messages with one
exact forward state. Therefore the forward pass alone cannot provide collision
resistance.

The tested multicollisions do not survive the reverse pass. Foldback currently
acts as a domain-separated second traversal because each differing original
byte produces a position-dependent return symbol. The new attacks directly
couple the forward and return constraints rather than merely checking random
messages.

These results strengthen the finite evidence for foldback, but also reinforce
the design concentration risk: a constructive family that aliases both passes
would immediately bypass all deterministic closure and squeeze processing.

The next stage should search longer chosen-difference paths where left and
right suffixes are independent, optimize toward the decreasing return-state
distance, and model foldback constraints as a bidirectional state-search
problem.
