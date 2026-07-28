# PVC-RotHash-1 Public Cryptanalysis Challenge

PVC-RotHash-1 candidate 1.0.0-rc1 is published to be attacked, not trusted.
There is no monetary bounty in this repository. Reproducible results receive
public credit in the attack log and any later paper, subject to the reviewer's
preferred attribution.

## Primary targets

### C1 — Full collision

Find distinct byte strings `M1 != M2` such that:

```text
PVC-RotHash-1(M1) = PVC-RotHash-1(M2)
```

Report complete messages, complexity, memory, and verification command.

### C2 — Better-than-generic truncated collision

For one or more output sizes from 24 through 96 bits, demonstrate a scalable
method whose cost is materially below the birthday reference and explain why it
extends with the output length.

### C3 — Preimage or second preimage

Find a full or truncated preimage/second preimage with complexity materially
below generic search. State whether the target digest or message was chosen.

### C4 — Full-digest distinguisher

Distinguish PVC-RotHash-1 outputs from uniform 256-bit strings with a practical,
reproducible sample complexity. Correct for multiple testing and explain the
structural cause.

### C5 — Foldback-compatible alias

Construct a message pair that exploits the known `42/126/196` controller alias
families to reach an identical after-foldback operational state. Equality must
include all 512 cube bytes, cursor, previous axis, and symbol index.

### C6 — Chosen-prefix, herding, or expandable-message attack

Use the known forward multicollision structure to create a result that survives
foldback, framing, closure, and squeeze.

### C7 — Formal or solver attack

Encode a reduced or canonical transition system in SAT, SMT, MILP, CP, or a
custom solver and demonstrate a collision, low-weight differential, invariant,
or complexity trend better than the existing tools.

### C8 — Specification or implementation discrepancy

Find an ambiguity, undefined arithmetic behavior, portability defect, or mismatch
between the normative specification, C++ implementation, Python implementation,
and official vectors.

## Non-results already known

The following are known and should not be reported as new full breaks:

- forward-state collisions and large forward multicollisions;
- reduced-round collisions;
- controller aliases with differences 42, 126, or 196;
- near-state pairs that remain digest-distinct;
- output-distance minima consistent with generic multiple-comparison effects.

A new method that turns any known weakness into a stronger phase or full-digest
result is valuable.

## Submission

Use the cryptanalysis issue template or contact the maintainer privately before
publishing a practical full-candidate break. Include scripts whenever possible.
