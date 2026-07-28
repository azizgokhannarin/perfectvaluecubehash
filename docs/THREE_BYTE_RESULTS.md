# Exhaustive Three-Byte and Constrained-Merge Results

Environment: GCC 14.2.0, Release build, warnings as errors.

The canonical algorithm and known-answer vectors are unchanged. This document
records structural attacks added in version 0.4.0.

## Full three-byte forward-state domain

All `2^24 = 16,777,216` three-byte messages were enumerated. Exact state
comparison includes all 512 cube bytes, cursor, previous axis, and symbol
index. A 64-bit research fingerprint was used only to screen candidates; every
reported collision was verified using the complete state.

```text
exact forward-state groups = 1,496
exact forward-state pairs  = 1,496
after-foldback pairs       = 0
```

The forward collisions divide into two classes:

```text
inherited two-byte merges with a common third byte = 768
new context-dependent third-symbol aliases          = 728
```

The 768 inherited pairs are exactly the three known two-byte pairs extended by
all 256 common third bytes:

```text
3 * 256 = 768
```

All 728 new pairs have the same first two bytes and a different third byte.
Every one of these symbol pairs generates the same complete six-move physical
trace from its two-byte context. They are therefore controller aliases, not
pairs of different move paths that later converge.

Absolute third-byte differences:

| Difference | Exact aliases |
|---:|---:|
| 42 | 542 |
| 126 | 89 |
| 196 | 97 |
| **Total** | **728** |

All three differences are even multiples of seven. This supports the
hypothesis that the move controller's parity axis selector and modulo-seven
rotation amount create context-dependent aliases. It does not yet provide a
closed-form generator for all aliases.

## Complete three-byte after-foldback domain

All 16,777,216 three-byte messages were independently enumerated at the state
immediately after reverse foldback.

```text
after-foldback exact state groups = 0
after-foldback exact state pairs  = 0
```

This result is stronger than checking only known forward collisions: no two
messages in the complete three-byte domain merged during or before foldback.
For equal-length messages, an equal after-foldback state would make all
remaining closure and squeeze processing identical and would therefore be a
full digest collision.

The result is finite-domain evidence only. It does not prove that foldback is
injective for longer messages.

## Delta-42 controller alignment

From every one-byte reachable context, all non-wrapping symbol pairs separated
by 42 were tested.

```text
tested pairs                         = 54,784
pairs sharing at least one move      = 45,775
exact six-move transition aliases    = 3
```

The three exact aliases are:

```text
context 17: 6f / 99
context 25: 1c / 46
context a2: 6f / 99
```

An independent report stated 47,520 pairs sharing at least one move. The clean
version-0.4.0 probe obtains 45,775. The exact three full aliases agree; the
intermediate count discrepancy remains documented rather than silently
reconciled.

## Independent-suffix constrained merge search

For each known two-byte forward-collision pair, two-byte suffixes were chosen
independently on the left and right:

```text
left  = collision_prefix_left  || x
right = collision_prefix_right || y
x, y in {0,1}^16
```

A meet-in-the-middle table compared after-foldback states. Each prefix pair
therefore covered `2^16 * 2^16 = 2^32` cross-suffix combinations using 131,072
state evaluations.

| Prefix pair | Cross combinations | Exact after-foldback merges |
|---|---:|---:|
| `176f` / `1799` | 4,294,967,296 | 0 |
| `251c` / `2546` | 4,294,967,296 | 0 |
| `a26f` / `a299` | 4,294,967,296 | 0 |

No equal-suffix or different-suffix merge was found. This tests a meaningful
constrained collision family but does not cover longer suffixes or unrelated
prefixes.

## Interpretation

The forward controller becomes more non-injective as the reachable context
space grows:

```text
two-byte domain:      3 exact forward pairs
three-byte domain: 1,496 exact forward pairs
```

However, the growth is currently explained by deterministic controller aliases:

- 768 inherited extensions of earlier aliases;
- 728 new one-symbol aliases at the third position;
- no distinct physical move paths converging in the three-byte scan.

Foldback separated every forward alias and remained collision-free over the
entire three-byte domain. This is encouraging but also exposes the current
security concentration: forward uniqueness is absent, so collision resistance
must rely heavily on the reverse message-dependent pass and later finalization.

The next work should target four-byte structured families, multi-alias chains,
and chosen relations that attempt to make the foldback return symbols alias as
well as the forward symbols.
