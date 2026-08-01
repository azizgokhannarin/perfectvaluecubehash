# G3 Failure Mode — Deliberate Analysis (Controller H)

**Date:** 2026-08-01  
**Status:** Analysis complete; **no new prototype in this note**  
**Lead controller:** offline structural **H** (not a candidate)  
**Gates:** `docs/CONTROLLER_REQUIREMENTS.md`  
**Ambition:** competitive hash path (`docs/RESEARCH_GOAL.md`)

This note exists so the next engineering step is chosen by **reasoning**, not by
another automatic “tweak residue and re-scan” cycle.

---

## 1. What we are optimizing

| Gate | Domain | Pass |
|---|---|---|
| G1 | 256 symbols from initial state | 256 distinct operational states |
| G2 | for every 1-byte prefix, 256 next symbols | same |
| G3 | for every 2-byte prefix, 256 next symbols | same |

**Competitive program rule:** G1 ∧ G2 ∧ G3 before any new candidate ID.

**Not the objective:** “make these two known colliding symbol pairs stop
colliding on their sample prefixes.” That is a local fit; H4 proved it can
**worsen** G3 while still splitting the old pairs.

---

## 2. Scoreboard that grounds the analysis

| Controller | G1 | G2 | G3 (instances / unique pairs) | Role |
|---|---|---|---|---|
| canonical | pass | fail | 728 / 10 | frozen RotHash-1 defect |
| E | pass | pass | 183 / 4 | first deep cut |
| G | pass | pass | 187 / 4 | coefficient-only; no win |
| **H** | **pass** | **pass** | **161 / 2** | **lead** |
| H3 | pass | fail 1 | 183 / 3 | over-touch axis |
| H4 | pass | pass | **337 / 5** | local residual kill, global worse |
| H5 | pass | fail 1 | 243 / 3 | simpler residue; regress |

**Judgment:** H is the best reasoned baseline. H3–H5 are **negative experiments**
with a clear lesson (below), not a ladder to climb blindly.

---

## 3. H residual facts (measured)

Full G3 under H finds **only two** colliding ordered pairs (across all 2^16
prefixes):

| Pair | Δ | Approx. prefix hits | Sample prefix |
|---|---:|---:|---|
| `58` / `c5` | 109 | 82 | `0247` |
| `6e` / `c6` | 88 | 79 | `0056` |

Total instances 161. Both are **physical-path aliases**: identical six-move
`(axis, amount)` sequences from the same two-byte context, hence identical
final operational states.

### 3.1 Phase-by-phase pattern (both pairs)

Traced on the sample prefixes:

- `init_control(symbol)` **differs** for the two symbols (nonlinear init works;
  not the old `control = symbol + k` affine rail alone).
- Intermediate `lane` / `lane2` **differ**.
- For **every** phase `0..5`: `axis` equal, `amount` equal, amount-residue equal.
- Cursor trajectory therefore equal → full state collision.

So the failure is not “rare random merge after divergent moves.” It is
**scheduler cloning**: the map

```text
(symbol, control, probe, geometry, phase, prev_axis) → (axis, amount)
```

agrees on both sides for six phases in a row for those symbol pairs in those
contexts.

### 3.2 Where amount collides under H

H amount (conceptual form):

```text
lane  = mix_odd(symbol, control, probe, geometry, phase, axis)
lane2 = lane + rotl(lane,4) + rotl(symbol XOR control, 2)
residue = (mul_odd(lane2, 41) XOR (lane2>>3) XOR (lane2>>5) XOR phase) mod 7
amount  = 1 + residue
```

On residual traces, **residue matches while lane2 differs**. That means the
map

```text
r(lane2, phase) = (41⊙lane2 XOR ⌊lane2/8⌋ XOR ⌊lane2/32⌋ XOR phase) mod 7
```

is **many-to-one** on the lane2 values actually visited. Symbol does enter
`lane`/`lane2`, but not in a way that separates these two residual symbols once
`r(·)` collides.

Axis under H also matches for those pairs (selector LSB agrees). So even a
perfect amount fix is incomplete if axis still clones; however H’s G1/G2 pass
suggests axis is already “good enough” on shallow domains—amount residual is
the dominant G3 story for the two remaining pairs.

---

## 4. Why local patches failed (reasoned)

| Patch | Intent | Outcome | Lesson |
|---|---|---|---|
| H3 | Harden axis + heavy residue mix | G2 breaks; G3 not better | Do not disturb a G1/G2-passing axis lightly |
| H4 | Keep H axis; add `mul_odd(symbol,13)` into residue | H pairs split on samples; **G3 161→337** | Local residual kill ≠ global injectivity |
| H5 | `amount = 1 + mul_odd(lane2⊕symbol,41) % 7` | G2 fails; G3 worse than H | Another `%7` of a short mix redistributes collisions |

**Core lesson:** Under a 7-valued amount, any byte-mix that is not carefully
designed for **global** collision structure will **permute** which symbol pairs
collide. Optimizing against a 2-pair hit list overfits.

---

## 5. Failure mode (compressed statement)

```text
FAILURE MODE F-G3-H:
  For a small set of symbol pairs (s, s'), after some reachable 2-byte contexts,
  the H scheduler produces identical (axis, amount)^6 sequences because:
    (1) amount is a 7-valued function of a derived byte (lane2) that collides
        for (s, s') across all six phases; and
    (2) axis selection simultaneously collides for those pairs/phases.
  Folding s into the residue ad hoc removes (s, s') on sample prefixes but
  creates other pairs elsewhere (H4) or breaks G2 (H3/H5).

NOT the failure mode:
  - foldback (not in G1–G3 harness)
  - cube geometry / line rotation invertibility
  - “need more avalanche testing before injectivity”
```

---

## 6. What would count as a reasoned next design (single hypothesis)

**Hypothesis H\* (one next experiment only):**

> Keep H’s **init_control**, **axis selector**, and **evolve_control** fixed
> (preserve G1/G2). Replace **only** the amount map with a construction that
> is explicitly **symbol-separating on Z/7Z whenever s ≠ s'**, for fixed
> `(control, probe, geometry, phase, axis)` at phase 0, and that remains
> phase-coupled so six-phase cloning is hard—without re-tuning against the
> two known pairs alone.

### Acceptable shapes for that amount map

**Preferred class (must pick one shape before coding):**

1. **Affine over GF(7) in the symbol with proven nonzero coefficient**, after a
   mix that does **not** re-introduce `Δlane ≡ 0 ⇒ Δamount ≡ 0` for free:
   e.g. work in `Z/7Z` with `amount = 1 + (a·σ(s) + b·τ(lane2) + c·phase) mod 7`
   where `σ: {0..255}→Z/7Z` is balanced and **a ≠ 0**, so for fixed other
   inputs, `s ≠ s'` ⇒ amount differs **unless** `σ(s)=σ(s')`. Then require
   that colliding `σ` pairs cannot also share axis for six phases (measure).

2. **Index into a fixed balanced table T[0..255]→{1..7}`** with
   `index = lane2 XOR π(symbol)` where `π` is a fixed byte permutation
   (odd multiply + xor). H2 failed because indexing was weak; the hypothesis
   is that **H’s lane2 + strong π(symbol)** is different—must be tested **once**
   against G1–G3, not iterated table tweaks.

**Out of scope for the next single experiment:**

- changing axis and amount at once (H3-style);
- optimizing only until `58/c5` and `6e/c6` vanish;
- foldback or full-hash work before G3 pass.

### Pass / fail for that one experiment

```text
PASS:  G1 ∧ G2 ∧ G3 all zero aliases
FAIL:  any gate fails → keep H as lead; write why; stop (no H6 cascade)
```

Either outcome is progress: a pass unlocks ST smoke + candidate planning; a
fail rules out that amount class and forces a larger rethink (documented).

---

## 7. Explicit non-decisions (until H\* runs)

- Do **not** mint a successor candidate.
- Do **not** claim PQC / Keccak-class security.
- Do **not** abandon the design family: H shows G1/G2 are achievable with
  PVC-compatible rotations.
- Do **not** restart foldback multicollision distance campaigns (budget-closed
  for that class).

---

## 8. One-sentence summary

**G3 under H fails because a 7-valued amount derived from a colliding lane2
residue lets two symbols clone a full six-move schedule; fixing only the known
pairs redistributes collisions—so the next step is one global amount-map
redesign with G1–G3 as the sole score, not residual whack-a-mole.**

---

## 9. Approval gate for implementation

Implementation of H\* starts only after confirming:

1. Amount shape is (1) or (2) above (written down first).  
2. Axis/init/evolve frozen from H.  
3. Exactly one full G1+G2+G3 run planned.  
4. On FAIL, stop and update this note—no chain of micro-variants.

This is the anti-ezber contract.
