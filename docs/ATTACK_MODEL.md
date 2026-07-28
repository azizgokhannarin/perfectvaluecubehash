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


## Version 0.4 priority attacks

### Transition injectivity

Search exact operational-state equality after one, two, and longer symbol
sequences from identical or different reachable predecessor states.

### Forward-plus-foldback collision

Forward equality alone is insufficient because the reverse message pass can
separate convergent prefixes. Construct message pairs that collide after both
forward absorption and foldback.

### Truncated collision scaling

Measure whether reduced and canonical configurations follow the generic
birthday curve from 16 through 64 output bits. A consistent sub-birthday trend
would be evidence of exploitable structure.

### Differential cancellation

Search chosen low-weight differences that become small again after foldback,
closures, or squeeze rather than only measuring random one-bit avalanche.

### Reachable predecessor graph

Measure exact in-degree and merging structure in finite reachable subgraphs as
preparation for a constrained meet-in-the-middle attack.


## Version 0.5 priority attacks

### Bidirectional alias construction

Treat each forward controller alias as one side of a constraint system. Search
for common or independent suffixes that transform the common forward state into
a context where the corresponding return symbols also alias exactly.

### Distance-guided return-state search

The minimum measured return-state distance decreased under longer common
suffixes. Use differential scoring, beam search, and meet-in-the-middle tables to
optimize this distance rather than waiting for random exact matches.

### Deeper forward multicollisions

Fifteen exact four-message forward multicollisions are known. Search longer
reachable contexts for third and later alias levels, and determine whether the
number of branchable common states grows with message length.

### Foldback boomerangs

Use two or more differing positions so that the reverse traversal of one
difference changes the state in a way that permits a later return-symbol alias.
This is a chosen-relation attack, not a random avalanche experiment.

### Independent-suffix search over local aliases

The existing 2^32 independent-suffix MITM targeted only the three original
two-byte prefix collisions. Extend the same strategy to the 728 local
three-byte aliases, prioritizing pairs with the smallest measured return-state
distance.

## v0.7 guided-distance attacker

The attacker may construct large exact forward multicollision families, choose
branches and common bridges, independently vary bounded suffixes, and score the
complete after-foldback state. Projection or fingerprint matches are only
candidate filters; exact operational equality is required for a collision.

The attacker also knows that reachable reverse contexts can contain sparse
controller aliases with differences 42, 126, or 196. A successful structural
attack must align those symbol pairs with both message history and reverse
position, or find a different cancellation relation.
