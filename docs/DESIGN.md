# PVC-RotHash-0 Design

## 1. Status

This document specifies the first falsification-oriented prototype. It is not a
security claim and not a standard.

Version identifier:

```text
PVC-RotHash-0 / 0.1.0
```

## 2. State

The state is the canonical Perfect Value Cube:

```text
C : {0..7}³ -> {0..255}
```

It has 512 cells, and each byte value occurs exactly twice.

The initial cube has 192 balanced X/Y/Z lines, each with sum 1020. Hashing is
allowed to destroy these line sums. The value histogram is preserved because
the transformation uses rotations only.

## 3. Primitive move

A move is:

```text
(axis, intersection point, amount)
```

where:

- `axis` is X, Y, or Z;
- the line is the axis-parallel line through the intersection point;
- `amount` is from 1 to 7;
- the eight values on that line move cyclically in the positive axis direction.

Every move is invertible. A move by `a` is inverted by a move on the same line
by `8-a`.

## 4. Intersecting chain

The state contains a cursor and the previous axis.

A new axis is always selected from the two axes different from the previous
axis. The new line passes through the cursor. After rotation, the cursor moves
with the value at the intersection point.

Therefore:

1. consecutive axes are different;
2. the next line intersects the previous line;
3. the chain bends at a value moved by the previous rotation;
4. later move selection reads the current cube state.

This is the central original hypothesis under test.

## 5. Absorbing one symbol

Each input symbol creates six linked moves.

For every move, the implementation combines:

- the current symbol;
- its stream position;
- the current cursor coordinates;
- the byte currently at the cursor;
- the micro-move number.

These values determine:

- which of the two orthogonal axes is selected;
- a rotation amount from 1 to 7;
- the controller value used by the next micro-move.

No external hash, cipher, substitution box, or pseudorandom generator is used.

The C++ source in `src/hash.cpp` is the normative operational definition for
version 0.1.0.

## 6. Reverse foldback

After the forward pass, the message is traversed from its last byte back to its
first byte. Each return symbol combines:

- the original message byte;
- its original byte position;
- one canonical body-diagonal byte.

This return traversal continues the same intersecting rotation chain. Its
purpose is to prevent two different forward move fragments that converge to the
same cube/cursor state from automatically sharing the same continuation.

## 7. Length closure

After the forward and foldback passes, the construction absorbs:

1. `1020 mod 256`, which is 252;
2. eight little-endian message-length bytes combined with canonical diagonal
   cells and cube-dimension offsets;
3. the cube dimension, 8.

This distinguishes byte strings with different lengths.

## 8. Self-fed closure

Thirty-two additional symbols are generated one at a time. Before each symbol,
the current four body diagonals are read. The closure symbol combines:

- one current diagonal cell;
- another current diagonal cell;
- one canonical diagonal cell;
- one byte of the message length;
- a geometry-derived step value.

The generated symbol is immediately absorbed, so every later closure symbol
depends on all prior closure rotations.

## 9. Digest extraction

The digest is not computed with a separate algorithm. It is the direct
concatenation of the four body diagonals of the final cube:

```text
D0: ( i,   i,   i)
D1: (7-i,  i,   i)
D2: ( i,  7-i,  i)
D3: ( i,   i,  7-i)
```

for `i = 0..7`.

```text
digest = D0 || D1 || D2 || D3
```

The output is 32 bytes / 256 bits.

## 10. Streaming API caveat

The public API accepts multiple `update()` calls, but version 0 buffers the
message and computes the cube at finalization. This keeps the specification
simple while the construction is still changing.

## 11. Explicit non-goals

Version 0 does not claim:

- preimage resistance;
- second-preimage resistance;
- collision resistance;
- pseudorandom output;
- quantum resistance;
- suitability as a password hash;
- compatibility with any existing hash standard.
