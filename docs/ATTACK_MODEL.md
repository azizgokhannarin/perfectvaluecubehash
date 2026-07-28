# Attack Model

The project follows a public-design model: an attacker knows the complete cube,
all constants, all equations, and the implementation.

## Priority attacks

### 1. Short algebraic relations

Rotations form permutations. Search for different message encodings that
produce the same effective cube permutation or the same four diagonal values.

### 2. Output projection collisions

Only 32 of 512 final cells are returned. Two final cubes may differ in 480
positions and still collide on all four body diagonals.

This is currently one of the strongest structural concerns.

### 3. Preserved histogram

Every final cube is a permutation of the canonical cube. Each byte value still
occurs exactly twice. The output is therefore sampled from a constrained state,
not from an unconstrained 256-bit space.

### 4. Symmetry and related-input attacks

The canonical cube has complement and reflection patterns. Test whether related
messages cause related final cubes or diagonals.

### 5. Differential trails

Flip one input bit and measure, after every move or absorbed symbol:

- changed cube cells;
- changed diagonal cells;
- changed digest bits;
- differences confined to a line, plane, or orbit.

### 6. Meet-in-the-middle

Moves are reversible. Investigate whether the chain can be split around an
intermediate cube state despite state-dependent move selection.

### 7. Fixed points and short cycles

Search reduced variants for messages or closure states that return the cube or
cursor to an earlier state.

### 8. Length-extension-like behavior

The explicit closure is intended to bind length, but the construction must be
tested for relations between `H(M)` and `H(M || X)`.

### 9. State reconstruction

The digest exposes four full body diagonals. Determine whether those 32 cells
leak enough information to predict cursor evolution, reconstruct hidden lines,
or prune message search.

## Interpretation rule

Experiments can quickly falsify the construction. They cannot establish
cryptographic security. Any practical collision, shortcut, invariant, or
distinguishing attack must be documented even if a later version fixes it.
