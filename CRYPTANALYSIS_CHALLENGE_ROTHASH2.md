# PVC-RotHash-2 Public Cryptanalysis Challenge

PVC-RotHash-2 candidate **0.2.0-draft** is published to be attacked, not trusted.
There is no monetary bounty. Reproducible results receive public credit in the
attack log and any later paper, subject to the reviewer's preferred attribution.

**This is not PVC-RotHash-1.** RotHash-1 remains a frozen historical baseline with
known forward multicollisions. RotHash-2 uses a different absorb controller
(systematic Controller **S**) and the same finalization shape.

**Production use is prohibited.** No collision, preimage, or post-quantum claim.

## Start here

| Artifact | Path |
|---|---|
| Normative absorb | `SPECIFICATION_ROTHASH2.md` |
| Finalization (shared shape) | `SPECIFICATION.md` §§10–15 |
| C++ API | `pvc::RotHash2`, CLI `--rothash2` |
| Python | `reference/python/pvc_rothash2.py` |
| Vectors | `test-vectors/official-v2.json` |
| Freeze policy | `docs/SPEC_FREEZE.md` |
| In-house Stage 4 | `docs/STAGE4_DEEP_R2.md`, `docs/STAGE4_DIGEST_R2.md` |
| Known results | `docs/KNOWN_CRYPTOANALYSIS.md` (R2 section), A-R2-001/002 |

Tag: **`v0.2.0-rothash2-draft`**

## Primary targets

### R2-C1 — Full collision

Find distinct byte strings `M1 != M2` such that:

```text
PVC-RotHash-2(M1) = PVC-RotHash-2(M2)
```

Report complete messages, complexity, memory, and a verification command using
either implementation (C++ `--rothash2` or Python reference).

### R2-C2 — Better-than-generic truncated collision

For one or more output sizes from 24 through 96 bits, demonstrate a scalable
method whose cost is materially below the birthday reference and explain why it
extends with the output length.

In-house 24/32/40-bit trials tracked birthday cost within sampling noise
(`docs/STAGE4_DIGEST_R2.md`). A method that systematically undercuts birthday is
high value.

### R2-C3 — Preimage or second preimage

Find a full or truncated preimage/second preimage with complexity materially
below generic search. State whether the target digest or message was chosen.

### R2-C4 — Full-digest distinguisher

Distinguish PVC-RotHash-2 outputs from uniform 256-bit strings with a practical,
reproducible sample complexity. Correct for multiple testing and explain the
structural cause.

### R2-C5 — Operational-state merge (forward or after-foldback)

Find distinct equal-length messages that reach the **same operational state**
(512 cube bytes + cursor + previous axis + symbol index) after:

- forward absorption only, or
- forward + foldback,

under RotHash-2 absorb. In-house exhaustive domains found **zero** such pairs
for all 2-byte messages and all 2^24 three-byte messages
(`docs/STAGE4_DEEP_R2.md`). A constructive family here would be high value even
before a full digest collision.

### R2-C6 — Controller injectivity break

Break the Controller **S** injectivity channel (G1–G3): exhibit symbol pairs or
multi-symbol paths that share the same six-move operational effect for a fixed
start-of-symbol context, or otherwise show that `s ↦ (amount0,amount1,amount2)`
is not injective as claimed. Reference `docs/CONTROLLER_REQUIREMENTS.md` and
`SPECIFICATION_ROTHASH2.md`.

### R2-C7 — Finalization-only or length/framing attack

Exploit foldback, diagonal/orbit closure, squeeze, or length framing **without**
needing a short-domain forward alias. Chosen-prefix, herding, expandable
messages, and related constructions are in scope.

### R2-C8 — Formal or solver attack

Encode a reduced or canonical transition system in SAT, SMT, MILP, CP, or a
custom solver and demonstrate a collision, low-weight differential, invariant,
or complexity trend better than the existing tools. Prefer naming the research
preset (`R5-rothash2` for full parameters).

### R2-C9 — Specification or implementation discrepancy

Find an ambiguity, undefined arithmetic behavior, portability defect, or mismatch
between `SPECIFICATION_ROTHASH2.md`, C++ `RotHash2`, Python `pvc_rothash2.py`,
and `test-vectors/official-v2.json`.

## Non-results already known (do not re-report as novel full breaks)

- PVC-RotHash-**1** forward collisions, multicollisions, and `42/126/196` aliases
  (those apply to RotHash-1, not automatically to RotHash-2).
- In-house RotHash-2 negatives in stated budgets: A-R2-001, A-R2-002
  (no short-domain operational merges; digest-surface minima near generic).
- Reduced-round collisions on deliberately weakened presets.
- Near-state / near-digest distances consistent with generic multiple-comparison
  effects.

A new method that turns any of the above into a stronger phase or full-digest
result **is** valuable.

## Quick verification

```bash
cmake -S . -B build -DPVC_BUILD_TOOLS=ON -DPVC_BUILD_TESTS=ON
cmake --build build -j"$(nproc)"

# Anchors
./build/pvc-hash --rothash2 --hex ""
# 9c4f0899255220a60bac6df7661cc50583f25f076ad37254b270b54685ce864a
./build/pvc-hash --rothash2 --hex 616263
# 9c1b502e8eac4ea07e18265ea30f888c4d5fd8ae81fa1ed453c2c099d4d68fdb

python3 scripts/verify_vectors_v2.py --cpp ./build/pvc-hash
ctest --test-dir build -R 'vectors-v2|stage4-smoke|pvc-tests' --output-on-failure
```

Research tools: pass `--preset R5-rothash2` where supported.

## Submission

Use the cryptanalysis issue template or contact the maintainer privately before
publishing a practical full-candidate break. Include scripts whenever possible.
Credit preferred attribution in `docs/ATTACK_LOG.md`.
