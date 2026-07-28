# Initial Reduced-Round Results

Environment: GCC 14.2.0, Release build, warnings as errors.

## One-byte preset matrix

All 256 one-byte messages were exhausted for every preset.

| Preset | Digest bytes | Digest collisions | Final-state collisions | Mean one-bit distance |
|---|---:|---:|---:|---:|
| R0-minimal | 4 | 242 | 242 | 15.947 / 32 |
| R1-foldback | 8 | 42 | 42 | 31.990 / 64 |
| R2-small | 8 | 3 | 3 | 31.891 / 64 |
| R3-medium | 16 | 0 | 0 | 64.068 / 128 |
| R4-near | 16 | 0 | 0 | 63.928 / 128 |
| R5-canonical | 32 | 0 | 0 | 127.738 / 256 |

The reduced configurations provide a useful falsification gradient: R0 through
R2 collapse in a tiny domain, while R3 through R5 do not collide in the same
one-byte experiment.

## Canonical forward convergence

The canonical six-move transition is injective over all 256 single symbols from
the initial state, but not over all two-symbol paths.

```text
17 6f  / 17 99
25 1c  / 25 46
a2 6f  / a2 99
```

Each pair produces the same complete operational state after forward absorption of two
symbols. This confirms that intersecting moves do not make the message path
unique.

The collision does not survive reverse foldback:

```text
after_foldback_equal = no
after_orbit_equal    = no
final_state_equal    = no
digest_equal         = no
```

## Exhaustive canonical two-byte phases

All 65,536 two-byte messages were enumerated.

| Phase | Exact state collisions | Distinct states |
|---|---:|---:|
| after forward | 3 | 65,533 |
| after foldback | 0 | 65,536 |
| after diagonal closure | 0 | 65,536 |
| after orbit closure | 0 | 65,536 |
| final | 0 | 65,536 |
| digest | 0 | 65,536 |

This is evidence that foldback removes the three observed forward convergences
in this finite domain. It is not evidence that foldback is collision-resistant
for longer messages.

## Two-byte phase gradient

| Preset | Forward collisions | After-foldback collisions | Final-state collisions | Digest collisions |
|---|---:|---:|---:|---:|
| R0-minimal | 65,340 | 65,340 | 65,340 | 65,341 |
| R1-foldback | 56,750 | 11,845 | 11,845 | 12,851 |
| R2-small | 24,967 | 658 | 658 | 658 |
| R3-medium | 1,162 | 0 | 0 | 0 |
| R4-near | 20 | 0 | 0 | 0 |
| R5-canonical | 3 | 0 | 0 | 0 |

The number of forward-state merges decreases sharply as moves per symbol rise.
For R3–R5, the tested foldback pass separates every observed two-byte merge.


## Common-suffix foldback merge search

For each of the three canonical forward-collision pairs, all 65,536 common
two-byte suffixes were appended and tested. No after-foldback collision was
found.

| Forward pair | Minimum foldback cube bit distance | Minimum differing cube bytes | Suffix |
|---|---:|---:|---:|
| `176f` / `1799` | 244 | 68 | `a8c7` |
| `251c` / `2546` | 208 | 61 | `0c1d` |
| `a26f` / `a299` | 258 | 68 | `8611` |

This tests 196,608 structured extensions. It does not exclude longer or
different suffix constructions, prefixes, or pairs whose forward collision
occurs later in the message.

## Reachable predecessor sample

For all 256 one-byte prefixes followed by all 256 next symbols under R5:

```text
transitions                         = 65,536
distinct target states              = 65,533
targets with multiple predecessors  = 3
maximum observed indegree           = 2
```

This is the same structural phenomenon as the two-symbol forward collision,
expressed as an in-degree measurement on a finite reachable graph.

## Truncated collision scaling

For R5, 24-bit truncation, 20 deterministic domains:

```text
mean first collision = 4,692.70 messages
birthday expectation = 5,133.57 messages
mean ratio           = 0.9140
minimum ratio        = 0.1900
maximum ratio        = 2.0700
```

The observed spread is broad, as expected for first-collision times. This small
sample does not demonstrate a sub-birthday attack. Full digests differed in the
reported R3–R5 truncated collisions.

Reduced presets R0–R2 produced full-digest collisions far below the 24-bit
birthday scale, which is consistent with their exact-state collapse.

## Differential phase measurements

Four 16-byte base messages, all single-bit changes (512 comparisons):

| Phase | Mean differing cube bits | Mean differing cube bytes |
|---|---:|---:|
| after forward | 1,400.4062 / 4,096 | 339.1152 / 512 |
| after foldback | 1,985.5000 / 4,096 | 490.9688 / 512 |
| after diagonal closure | 2,047.7227 / 4,096 | 510.0469 / 512 |
| after orbit closure | 2,048.0156 / 4,096 | 509.8906 / 512 |
| final | 2,047.8438 / 4,096 | 510.0156 / 512 |

Digest distance:

```text
mean = 127.5742 / 256
min  = 100
max  = 156
exact digest matches = 0
```

Paired differences also converged to approximately half the cube bits and half
the digest bits, with no exact state or digest matches in 512 comparisons.

## Related-input tests

Under R5:

| Relation | Mean digest distance | Digest matches | Final-state matches |
|---|---:|---:|---:|
| reverse | 128.0195 | 0 | 0 |
| byte complement | 128.1914 | 0 | 0 |
| cyclic left rotation | 128.8594 | 0 | 0 |

Complete multiset permutation domains:

| Domain | Unique messages | Digest collisions | Final-state collisions |
|---|---:|---:|---:|
| `ABCDEFGH` | 40,320 | 0 | 0 |
| `AABBCCDD` | 2,520 | 0 | 0 |
| `00001111` | 70 | 0 | 0 |

## Interpretation

The most important new fact is negative but useful: the forward transition is
not injective even in the canonical configuration. The reverse foldback is not
merely cosmetic; it is currently the component that separates the observed
forward convergences.

The next attack should therefore target a collision that satisfies both the
forward and foldback constraints, rather than treating forward-state equality
alone as a complete hash collision.
