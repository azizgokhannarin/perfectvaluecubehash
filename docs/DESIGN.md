# PVC-RotHash-1 Design

## 1. Status and identifier

This document specifies a falsification-oriented prototype, not a standard or
security claim.

```text
PVC-RotHash-1 / canonical algorithm unchanged; research package 0.7.0
```

## 2. State

The state is the canonical Perfect Value Cube:

```text
C : {0..7}³ -> {0..255}
```

It has 512 cells and each byte value occurs exactly twice. Its initial 192
axis-parallel lines sum to 1020. Hashing may destroy those line sums.

State mutation remains rotation-only. Consequently the complete 512-cell byte
histogram is invariant throughout hashing.

## 3. Primitive move

A move is `(axis, intersection point, amount)`:

- `axis` is X, Y, or Z;
- the selected line is the axis-parallel line through the intersection point;
- `amount` is 1 through 7;
- the eight cells move cyclically in the positive axis direction.

Every move is invertible. Rotation by `a` is inverted by rotation by `8-a` on
the same line.

## 4. Intersecting chain

The working state holds a cursor and previous axis. Every next axis differs from
the previous axis and its line passes through the current cursor. The cursor
then advances with the rotated intersection value.

Therefore consecutive moves intersect and generally do not commute. Move
selection reads the current cube and is path-dependent.

## 5. Symbol absorption

Every symbol produces six linked moves. The move controller combines:

- the symbol;
- symbol index;
- cursor coordinate;
- byte currently at the cursor;
- micro-move phase;
- previous axis.

It chooses one of the two orthogonal axes and an amount from 1 to 7. No external
cryptographic primitive or pseudorandom generator is called.

## 6. Message path

The input is absorbed in forward order. It is then traversed in reverse through
a foldback symbol derived from the original byte, original byte position, and a
canonical body-diagonal value.

The foldback was introduced after a forward-only predecessor produced an exact
two-byte collision by converging to the same operational state.

## 7. Length and diagonal closure

The finalization absorbs:

1. `1020 mod 256 = 252`;
2. eight little-endian message-length bytes framed by canonical diagonal cells;
3. the cube side, 8;
4. 64 self-fed symbols derived from the current body diagonals, canonical
   diagonals, message length, and step geometry.

Each closure symbol is absorbed immediately, so later symbols observe all prior
closure rotations.

## 8. Full-cube orbit closure

Version 0 showed that short-message states remembered the canonical occupant of
a coordinate. Version 1 therefore adds 128 orbit symbols.

At orbit index `i`, four coordinates are selected from the four 128-cell
quarters of the storage order. The four coordinates are sampled directly. Their current values, one current
body-diagonal value, the length, the cursor, and the orbit index form the next
symbol.

Across the 128 steps, every physical cube cell participates once as a direct
sample. Since each symbol immediately changes the cube, later samples observe
an evolving state rather than one static snapshot.

## 9. Four-diagonal squeeze

The 256-bit digest is produced in 32 squeeze steps. It is not the raw body
diagonal of one final cube.

For output position `i`:

1. read all four current body diagonals;
2. select one lane from each diagonal with position-dependent offsets;
3. combine the four bytes with odd bit rotations `1,3,5,7`, byte addition,
   a cross-diagonal pair, the squeeze chain, and the output index;
4. emit one byte;
5. derive four further symbols, one rooted in each body diagonal;
6. absorb those symbols before producing output byte `i+1`;
7. update the squeeze-chain byte from the emitted byte, new diagonals, cursor
   value, and cursor geometry.

Thus all 32 output bytes are derived from the four body diagonals, but from 32
different cube states. The same byte may appear more than twice in the digest,
although it still appears exactly twice in any individual cube state.

The exact operational definition is `src/engine.cpp`; `src/hash.cpp` fixes the public API to the canonical parameters.

## 10. Streaming API

Multiple `update()` calls are accepted, but the prototype buffers the message
and computes the complete path at finalization. This avoids freezing a streaming
state format while the construction is still under active revision.

## 11. Explicit non-claims

Version 1 does not claim:

- preimage resistance;
- second-preimage resistance;
- collision resistance;
- indifferentiability from a random oracle;
- quantum resistance;
- suitability for production use;
- compatibility with an existing hash standard.


## 12. Reduced-round research boundary

Version 0.3.0 exposes runtime parameters only through `pvc/research.hpp`.
`RotHash1` still invokes the canonical constants and produces the same known
answers as version 0.2.0.

An exact operational snapshot contains the cube, cursor, previous axis, and
symbol index. Checkpoints are emitted at phase boundaries and after each
squeeze byte. These interfaces exist to construct attacks; they are not part of
a proposed stable hash API.

The canonical forward transition is now known not to be injective over all
two-symbol messages. The reverse foldback is therefore an active collision-
separation layer, not merely additional diffusion.


## 13. Version 0.5 structural interpretation

The forward controller is now known to support composable aliases. Fifteen
three-byte common forward states contain a second one-symbol alias, allowing
four distinct messages to reach one exact forward state. This does not change
the canonical algorithm; it changes the threat model.

The reverse foldback is therefore the first tested phase that separates known
forward multicollisions. Research-only APIs expose the return-symbol derivation
and allow foldback to be applied to an explicit common forward state. These APIs
are for attack construction and do not belong to the proposed public hash
interface.

The design still claims no collision resistance. In particular, any message
family that satisfies both the forward controller aliases and the position-
dependent return-symbol aliases would pass identical states into deterministic
closure and squeeze processing.
