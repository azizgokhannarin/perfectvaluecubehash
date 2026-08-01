# Move-Controller Requirements — Competitive Hash Path

**Status:** Binding for any successor to PVC-RotHash-1  
**Ambition:** general-purpose, PQC-era competitive hash (see `docs/RESEARCH_GOAL.md`)  
**Frozen RotHash-1:** does **not** meet these requirements (known forward aliases)

This document defines the **scheduler** that turns each absorbed symbol into six
intersecting line rotations. The rest of the hash (cube geometry, foldback,
closures, squeeze) may stay in the design family only if this layer is sound.

No security claim is made by meeting these gates. Failing them **blocks** minting
a new candidate identifier.

---

## 1. Design boundary (identity constraints)

A compliant controller **MUST**:

1. Use only the Perfect Value Cube state mutated by **axis-parallel line
   rotations** of amount in `{1,...,7}`.
2. Keep consecutive moves on **different axes**, with the next line through the
   current cursor (intersecting chain).
3. Import **no** SHA, Keccak, AES, ChaCha, BLAKE, external S-box, KDF, MAC, or
   CSPRNG into the candidate algorithm.
4. Remain fully specified by small fixed tables and elementary byte/word ops if
   tables are used (project-local constants, not an external primitive).

A compliant controller **MUST NOT** rely on foldback, length framing, or squeeze
to “fix” one-symbol forward non-injectivity. Those layers are additional; they
are not a substitute for a sound absorb step.

---

## 2. Hard injectivity gates (automated)

Let `T(S, m)` be the operational state after absorbing symbol sequence `m` from
state `S` under the controller (canonical six moves per symbol unless a future
spec changes that parameter under a new candidate ID).

### G1 — Initial one-symbol injectivity

From the canonical initial state `S0`:

```text
|{ T(S0, [b]) | b in 0..255 }| = 256
```

### G2 — One-byte-context injectivity

For every first symbol `a in 0..255`, with `S_a = T(S0, [a])`:

```text
|{ T(S_a, [b]) | b in 0..255 }| = 256
```

### G3 — Two-byte-context injectivity (full domain)

For every prefix `p in {0,1}^16`, with `S_p = T(S0, p)`:

```text
|{ T(S_p, [b]) | b in 0..255 }| = 256
```

**Pass criterion:** G1 ∧ G2 ∧ G3 with **zero** exact operational-state collisions
(cube + cursor + previous_axis + symbol_index).

**Fail criterion:** any pair of distinct symbols (or distinct messages of equal
length in these domains) mapping to the same operational state.

Reproduction (research harness):

```bash
python3 scripts/controller_redesign_prototypes.py \
  --variants H --deep --two-byte-full
```

Expected for a passing variant: all reported alias counts are `0`.

### G4 — Known anti-families

A pass of G1–G3 already excludes the frozen `42/126/196` family and the E/G
residual pairs. Explicit regression checks **SHOULD** still assert absence of:

- the three one-byte-context pairs `17:6f/99`, `25:1c/46`, `a2:6f/99`;
- E residuals `36/71`, `2b/2c`, `61/80`, `2d/ae`;
- G residuals `3c/ac`, `87/fb`, `3c/bc`, `4b/a8`.

---

## 3. Structural requirements (design rules)

These rules exist so we do not rediscover the same modular rails.

### S1 — No double linear use of the symbol

The message byte **MUST NOT** enter both an additive `control` accumulator and
an additive `amount_source` in a way that yields

```text
Δamount_source ≡ α · Δsymbol  (mod 7)   with systematic solutions Δsymbol ≠ 0
```

for long phase runs. RotHash-1 violated this (double add → `2Δ ≡ 0 (mod 7)`).

### S2 — Amount is not “sum then % 7” alone

If amount is reduced modulo 7, the preimage byte **MUST** be produced by a mix
that is **nonlinear in the symbol relative to control** (for example XOR/rotate
and odd multiplications on `Z/256Z`, or a fixed 256→{0..6} table with
documented balance). Coefficient-only linear forms over GF(7) (variant G) are
**insufficient** as a sole fix.

### S3 — Axis selection is symbol-sensitive

Axis choice **MUST** depend on a mix that includes the current symbol (or a
control value that is an injective function of the symbol at phase 0) with more
than a trivial correlation to a single LSB of a symbol-independent field.

### S4 — Phase coupling

Each of the six micro-phases **MUST** re-mix phase index so that a collision at
phase 0 does not automatically clone the remaining five moves.

### S5 — Local tables allowed

A fixed public table of size at most a few hundred bytes (for example a
balanced map `u8 → {1..7}`) **MAY** be used. It is part of the candidate
constant, not an “imported cipher”.

---

## 4. Soft gates before minting a candidate

After G1–G4 pass, before assigning a new algorithm identifier:

| ID | Gate | Minimum bar |
|---|---|---|
| ST1 | Single-bit avalanche sample | mean digests distance compatible with ~128/256 |
| ST2 | Two-byte digest uniqueness | all `2^16` digests distinct under full hash with new controller |
| ST3 | No v0 multiplicity ban | triple-byte repeats occur at ~random rate in a large sample |
| ST4 | Reduced-round ladder | document first unbroken preset; R0–R2 may still break |

ST1–ST4 are **smoke tests**, not a security proof.

---

## 5. Candidate minting rule

```text
IF G1 ∧ G2 ∧ G3 ∧ G4
AND ST1–ST4 smoke pass
AND dual independent implementations + official vectors exist
THEN a new candidate ID MAY be frozen (not “RotHash-1”)
ELSE do not mint; iterate controller only
```

RotHash-1 remains the public attacked baseline until that day.

---

## 6. Explicit non-goals of this document

- Proving collision resistance.
- Matching Keccak performance on day one.
- Claiming PQC security by slogan.

The goal is a controller that **does not fail the first serious structural
test**, so a competitive hash program can honestly begin.

---

## 7. Status of prototypes (harness, exact state equality)

| ID | G1 | G2 | G3 | Notes |
|---|---|---|---|---|
| canonical (RotHash-1) | pass | **fail** (3) | **fail** (728 / 10 pairs) | frozen baseline |
| E | pass | pass | **fail** (183 / 4) | residual mod-7 lane |
| G | pass | pass | **fail** (187 / 4) | coefficient-only |
| **H** | **pass** | **pass** | **fail** (161 / **2** pairs) | **lead**; residuals `58/c5`, `6e/c6` |
| H2 | pass | **fail** (57) | **fail** (10890 / 78) | discarded |
| H3 | pass | **fail** (1) | **fail** (183 / 3) | over-hardened; regressed |
| H4 | pass | pass | **fail** (337 / 5) | kills H residuals; G3 worse |
| H5 | pass | **fail** (1) | **fail** (243 / 3) | simpler residue; regressed G2 |

**Lead remains H.** See `docs/CONTROLLER_G3_FAILURE_MODE.md` for the reasoned
next experiment (hypothesis H\*: one global amount-map redesign; stop on fail).
