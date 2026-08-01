# External Advice on G3=0 (Consultation Response)

**Received:** 2026-08-02  
**Source:** External consultation (file `yanıt.docx`, removed after integration)  
**Status:** Adopted as the design principle for hypothesis **S** (systematic coding)

This document records the advice and how it revises our G3 strategy. It is not
a security proof.

---

## 1. Diagnosis (statistical)

Under a **random-like** scheduler, each phase has about `2 × 7 = 14` choices
`(axis, amount)`. A six-phase schedule lives in a space of size about
`14^6 ≈ 7.5×10⁶`.

For a fixed context, two distinct symbols clone all six phases with probability
about `14^{-6} ≈ 1.3×10^{-7}`. With `C(256,2)` pairs per context:

| Gate | Contexts | Expected colliding pairs (order of magnitude) |
|---|---:|---:|
| G1 | 1 | ~0.004 → usually PASS |
| G2 | 256 | ~1 → PASS possible by chance |
| G3 | 65536 | **~280** → PASS probability negligible |

Our H result (161 instances) and H4 after local patch (337) sit in this band.
**Whack-a-mole cannot reach G3=0:** patches re-roll a random-like function;
expectation stays hundreds of collisions.

**Conclusion:** G3=0 requires **structural (provable) injectivity**, not a
better scramble of the same lossy `mod 7` / LSB channels.

This matches and strengthens `docs/CONTROLLER_G3_FAILURE_MODE.md`.

---

## 2. Core design principle

Separate two channels (sponge-like discipline):

| Channel | Role | Phases (6-move symbol) |
|---|---|---|
| **Injectivity** | Message enters via a **bijective** encoding into amounts | **0–2** (three amounts) |
| **Diffusion** | Free mixing; collisions here do not carry injectivity burden | **3–5** + all axis choices |

Do **not** hope injectivity “emerges” from diffusion mixes (`f(lane2) mod 7`).
That is the RC5-style data-dependent small-modulus trap.

### Injectivity channel (phases 0–2)

Capacity: `7³ = 343 ≥ 256`. Encode symbol `s` as mixed radix:

```text
t  = Π(s)                    # fixed public byte permutation
d0 = t mod 7
d1 = ⌊t / 7⌋ mod 7
d2 = ⌊t / 49⌋                # 0..5 for t in 0..255; map is injective on 0..255

# c0,c1,c2 computed from START-OF-SYMBOL state only (no dependence on s)
r_i = 1 + ((d_i + c_i) mod 7)   # i = 0,1,2
```

For a fixed reachable context, each `c_i` is constant. Translation on `Z/7Z` is
bijective, so `s ↦ (r0,r1,r2)` is **injective by construction**.

**Critical implementation rule:** All `c_i` must be derived from the state
**before** any of the six moves of this symbol. If `c_1` were taken from the
state after move 0, it would depend on `r0` hence on `s`, and the product
argument would fail.

### Diffusion channel (phases 3–5 + axes)

Free to use probe, control, symbol, lane mixes. Axis need not carry injectivity
(axis alone is only ~6 bits &lt; 8). Optimize axis for diffusion quality.

---

## 3. Answers to our four questions (compressed)

1. **Single** GF(7) form or single `u8→{1..7}` table cannot inject 256→7
   (pigeonhole). Use **vector** of three amounts (mixed radix). Control plane is
   **not** information-theoretically too narrow (`14^6` ≫ 256); the bug was
   **lossy per-phase** symbol mixing.
2. Axis-only cannot separate 256 symbols; with systematic amounts, axis load is
   lifted — use axis for diffusion only.
3. Residual-targeted patches re-sample the same collision distribution (~280).
   Discipline: **build** injectivity; use G1–G3 as verification, not discovery.
4. Positive pattern: sponge absorb via bijective injection (XOR), permutation
   independent of message. Negative pattern: data-dependent rotation amount from
   small modulus (RC5 family) — same class as `f(lane2) mod 7`.

---

## 4. Residual caveat (second-order)

Systematic coding kills “same move schedule for two symbols.” Different
schedules could still meet in cube state (e.g. period-4 lines). Cube
double-histogram limits some cases; **still run G1–G3** after implementation as
verification of second-order aliasing, not as a fishing expedition.

---

## 5. Program decision

| Prior plan H\* | Updated plan **S** |
|---|---|
| One more global amount hash/scramble | **Systematic mixed-radix injectivity channel** |
| Heuristic score G3 | Structural argument + G1–G3 verify |
| Freeze H axis only | New structural controller **S**; may keep H-like diffusion in phases 3–5 |

**Adopted next implementation:** offline controller **S**
(`scripts/controller_redesign_prototypes.py`), then full G1–G2–G3.

### Implementation note (2026-08-02)

First S draft incorrectly fed a **symbol-dependent** `control` into `c_i`,
which violates the “context translation independent of s” argument and can
reintroduce short-domain aliases. Fixed: `c_i` uses only
`(probe0, cursor, previous_axis, symbol_index, phase tag)` via a state-only
`context_seed`. Diffusion still uses a separate symbol-aware `control`.
