# Attack Model

The design is fully public. An attacker knows the canonical cube, all constants,
all equations, the output construction, and the implementation.

## Priority attacks

### 1. Short algebraic relations

Line rotations are permutations with inverses and finite order. Search for
message relations that produce the same operational state or digest.

### 2. Squeeze collisions

Version 1 no longer projects one static 32-cell set. Nevertheless, two distinct
state trajectories may produce the same 32-byte squeeze sequence. Search for
relations that control only squeeze observations without matching the full cube.

### 3. Preserved full-state histogram

Every state remains a permutation of the canonical cube, and every byte still
occurs exactly twice. Determine whether higher-order statistics, state recovery,
or trajectory pruning exploit this invariant even when marginal output tests
look uniform.

### 4. Symmetry and related-input attacks

The canonical cube contains complement and reflection patterns. Test messages
related through coordinate reflection, byte complement, reversal, rotation, and
length transformations.

### 5. Differential and rotational trails

Track differences after every absorbed symbol and squeeze step. Search for:

- low-weight differences;
- line-, plane-, or orbit-confined trails;
- predictable output differences;
- cancellation during foldback, closure, orbit, or squeeze.

### 6. Meet-in-the-middle and state matching

Moves are reversible. Investigate whether message and finalization segments can
be split around a partial cube/cursor/axis state, or whether diagonal
observations permit partial matching with less memory.

### 7. Fixed points and short cycles

Search reduced variants and finalization stages for repeated operational states,
short cycles, invariant cursor paths, or identical squeeze continuations.

### 8. Length-extension and multicollision structures

Explicit length finalization is intended to prevent trivial continuation, but
Joux-style multicollisions, expandable messages, and prefix/suffix relations
must be investigated independently.

### 9. Higher-order distinguishers

Version 1 passes simple marginal byte and bit tests. Continue with:

- byte-pair and bit-pair correlation matrices;
- mutual information between positions;
- multi-byte pattern frequencies;
- complement/reflection statistics;
- large-sample state and digest tests;
- classifier-based distinguishers trained only on public outputs.

## Interpretation rule

Experiments can falsify the construction. They cannot establish cryptographic
security. Any practical shortcut, collision, invariant, or distinguisher is a
successful project result and must be documented.
