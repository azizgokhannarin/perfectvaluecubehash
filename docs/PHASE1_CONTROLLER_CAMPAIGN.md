# Phase 1 — Controller Decision Campaign

**Status:** Active  
**Started:** 2026-08-01  
**Frozen algorithm:** PVC-RotHash-1 `1.0.0-rc1` (unchanged)  
**Strategy:** Parallel attack + offline redesign (`docs/ACCEPTANCE_ROADMAP.md`)

This campaign exists to force a Gate B decision:

- **Keep-1** — defend RotHash-1 with strong separation evidence; or  
- **Redesign-2** — mint a new candidate that removes cheap one-symbol aliases; or  
- **Negative** — stop the acceptance push and publish as a negative result.

No security claim is made by any finding below.

---

## 1. Why the controller is the acceptance bottleneck

Forward absorption admits exact one-symbol aliases and scalable bridged
multicollisions. Observed absolute symbol differences concentrate on:

```text
42, 126, 196
```

These are **not** a curiosity: they are a Joux-style multicollision engine.
Acceptance as a general-purpose hash is impossible while this surface is cheap
and unexplained, unless foldback separation is upgraded from “holds in tested
domains” to a credible, attacked argument — or the aliases are removed.

---

## 2. Independent modular analysis (P1-d)

Equations from `SPECIFICATION.md` §9 (informative re-derivation; not a second
normative source).

### 2.1 Move controller (one phase)

```text
control_0 = u8(symbol + u8(symbol_index) + coord_code(cursor))

selector  = ROTL8(control, phase) XOR probe_before XOR geometry
axis      = CHOOSE_OTHER_AXIS(previous_axis, selector)   // depends on selector LSB

amount_source = control + probe_before + geometry + symbol + 11*axis   // integer sum
amount        = 1 + (amount_source % 7)
```

Then the line rotates, the cursor advances, and:

```text
control ← u8(ROTL8(control, 1+(axis&1)) + probe_after + amount
              + coord_code(cursor) + 7*phase)
```

### 2.2 Phase-0 necessary conditions for an identical first move

Compare symbols `s` and `s+d` from the **same** operational state.

- Initial controls differ by `d` (mod 256):  
  `control_0(s+d) = u8(control_0(s) + d)`.
- Axis depends on `selector & 1`. At phase 0, `ROTL8(control,0) = control`, so
  same axis requires **`d` even** (same LSB of control).
- Amount uses the **integer** sum of byte values, then `% 7`. With identical
  probe, geometry, and axis:

```text
Δ amount_source = (control_R - control_L) + d
```

where `control_R` and `control_L` are the integers in `{0,...,255}`.  
At phase 0, `control_R - control_L = d` (no wrap past the representatives when
both are `c` and `c+d` with `c+d ≤ 255`, or `c+d-256` when wrap occurs).

When both lie in range without mixed wrap pathology,  
`Δ amount_source = 2d`, so same amount requires:

```text
2d ≡ 0 (mod 7)  ⇒  d ≡ 0 (mod 7)
```

(since 2 is invertible mod 7). Combined with “d even”:

```text
d ≡ 0 (mod 14)
```

**Check:** `42`, `126`, and `196` are all multiples of 14.  
All multiples of 14 are phase-0 *candidates*; almost none become full six-move
aliases.

### 2.3 Why naive `(u8_delta + d) mod 7` is the wrong multi-phase model

After phase 0, controls diverge as 8-bit values. Amount matching requires:

```text
(control_R - control_L) + d ≡ 0  (mod 7)
```

with **integer** subtraction of the `0..255` representatives — not the `u8`
difference alone. Because `256 ≡ 4 (mod 7)`, a wrap in the control trajectory
changes the modular condition by 4. Traced known alias `17||6f` / `17||99`
(`d = 42`):

| phase | control_L | control_R | u8 Δ | integer (cR−cL)+d | (mod 7) | amount match |
|---:|---:|---:|---:|---:|---:|---|
| 0 | 62 | 104 | 42 | 84 | 0 | yes |
| 1 | 138 | 222 | 84 | 126 | 0 | yes |
| 2 | 73 | 241 | 168 | 210 | 0 | yes |
| 3 | 206 | 31 | 81 | −133 | 0 | yes |
| 4 | 33 | 194 | 161 | 203 | 0 | yes |
| 5 | 209 | 20 | 67 | −147 | 0 | yes |

Physical six-move traces are identical; final operational states are equal.
The alias is a **true controller collision**, not later path reconvergence.

### 2.4 Alignment-probe facts (one-byte context → second symbol)

`pvc-alignment-probe` over all first symbols and non-wrapping second pairs:

| Δ | tested pairs | exact six-move state aliases |
|---:|---:|---:|
| 14,28 | large | **0** |
| **42** | 54784 | **3** (`17:6f/99`, `25:1c/46`, `a2:6f/99`) |
| 56…182 | large | **0** |
| **126** | 33280 | **0** in this domain |
| **196** | 15360 | **0** in this domain |
| 210…252 | smaller | **0** |

So among all multiples of 14, **only Δ = 42** produces exact one-symbol aliases
from one-byte reachable contexts in this probe. Differences **126** and **196**
still appear as third-symbol aliases in the exhaustive three-byte catalogue
(`docs/THREE_BYTE_RESULTS.md`); they are context-depth dependent, not universal
phase-0 free gifts.

### 2.5 Structural root cause (design-level)

The same message byte enters **both**:

1. `control` (linearly, mod 256), and  
2. `amount_source` (again, as an integer summand),

while amount is reduced **mod 7** and axis is essentially a **parity** of a
rotated control. That double use creates an additive relation  
`Δamount_source ≈ Δcontrol + Δsymbol` that is periodically solvable when
`Δsymbol` is a multiple of 14 and the control trajectory stays on a modular
rail for six phases.

This is a **formula design defect**, not a bad constant in the Perfect Value
Cube.

---

## 3. Redesign constraints (P1-e, offline only)

Any successor candidate (**new identifier**, new vectors) should:

1. Preserve the project boundary: no SHA/Keccak/AES/BLAKE/ChaCha/S-box import.
2. Preserve intersecting rotation geometry and foldback/closure architecture
   unless a later decision expands the redesign.
3. **Eliminate** cheap one-symbol aliases:
   - exhaustive scan of all 256×255 ordered pairs from many contexts (initial,
     one-byte, two-byte, reverse-reachable) must find **zero** exact six-move
     state collisions for a single absorbed symbol; or document residual rate
     with a mathematical bound.
4. Prefer structural fixes over magic constants, for example (research options,
   not yet chosen):
   - do **not** feed `symbol` into both `control` and `amount_source` linearly;
   - mix symbol with probe/geometry via non-additive byte ops before `% 7`;
   - make axis selection depend on more than one selector bit / phase mixing;
   - avoid amount modulus 7 aligning with a simple double-Δ relation.
5. Re-run the entire v0→v1 statistical battery before any deep campaign on the
   successor (multiplicity, positional bias, avalanche, two-byte exhaust).

**Hard rule:** redesign experiments must not alter `SPECIFICATION.md` or official
vectors of PVC-RotHash-1. Implement prototypes under a research-only path or a
future branch/candidate name.

---

## 4. Attack workstreams (still on frozen RotHash-1)

| ID | Stream | Near-term action |
|---|---|---|
| P1-a | Multi-path foldback distance | Beam/LSH over many bridged seeds, not one path |
| P1-b | Dual reverse alias | Search return-symbol pairs with Δ∈{42,126,196} from common forward states |
| P1-c | Alias taxonomy | Inherited vs local; depth; bridge necessity |
| P1-d | Independent model | This document + `scripts/controller_alias_analysis.py` |
| P1-e | Redesign prototypes | Separate research harness; zero vector drift on main candidate |

### Exit memo requirements

A short `docs/CONTROLLER_DECISION_MEMO.md` (to be written when evidence is ready)
must cite:

- alias root cause (this file §2);
- best after-foldback / digest attack costs from P1-a/b;
- redesign prototype injectivity scan results;
- explicit choice: Keep-1 / Redesign-2 / Negative.

---

## 5. Reproducibility

```bash
# Phase-0 family alignment (canonical)
./build/pvc-alignment-probe --delta 42 --print-limit 8
./build/pvc-alignment-probe --delta 126 --print-limit 0
./build/pvc-alignment-probe --delta 196 --print-limit 0

# Independent modular + catalogue helper
python3 scripts/controller_alias_analysis.py

# Known regression anchors
./build/pvc-tests
```

Environment for the 2026-08-01 modular notes: GCC 14.2 tools already built;
Python 3 standard library only for the analysis script.

---

## 6. Immediate conclusion (not a Gate B decision yet)

1. The `42/126/196` family is explained at the **equation** level well enough to
   guide redesign: double linear use of `symbol` plus mod-7 amount and LSB axis.
2. Δ = 42 is special in one-byte contexts (exactly three full aliases).
3. Δ ∈ {126,196} require deeper context; they remain first-class attack and
   redesign targets.
4. Gate B remains **open**. Parallel attack and redesign continue.
5. Acceptance messaging stays off until Gate B closes.
