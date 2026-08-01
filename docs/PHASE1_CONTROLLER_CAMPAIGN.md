# Phase 1 — Controller Decision Campaign

**Status:** Active — foldback class budget-closed; redesign preferred  
**Started:** 2026-08-01  
**Frozen algorithm:** PVC-RotHash-1 `1.0.0-rc1` (unchanged)  
**Strategy:** `docs/ACCEPTANCE_ROADMAP.md` · effort: `docs/EFFORT_POLICY.md`  
**Provisional decision:** `docs/CONTROLLER_DECISION_MEMO.md`

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

---

## 7. Campaign results — 2026-08-01 (Phase 1 wave 2)

Frozen PVC-RotHash-1 only for attack tools. Redesign prototypes are offline.

### 7.1 P1-b Dual return-alias (`pvc-dual-return-alias`)

Full three-byte forward catalogue (`1496` pairs), canonical parameters:

| Check | Result |
|---|---:|
| Direct return-gate merges (one reverse step from common forward state) | **0** |
| After-foldback merges, bare pairs | **0** |
| After-foldback merges, all common 1-byte suffixes (`1496×256`) | **0** |
| Min direct return operational-state distance | 181 bits |
| Min after-foldback operational-state distance (with 1-byte suffixes) | 208 bits |
| Common-forward states with a further family-delta one-symbol alias | 15 (matches known second-level multicollision seeds) |

Return-symbol absolute arithmetic deltas are **not** concentrated on `{42,126,196}`
(the return map preserves XOR differences, not arithmetic differences). No
foldback-compatible dual was found in this domain.

### 7.2 P1-a Multi-path foldback sample (`pvc-multipath-foldback-sample`)

`128` independent forward seeds, optional common 1-byte suffixes (`64` samples
each):

| Metric | Bits |
|---|---:|
| Bare after-foldback mean / p50 / p10 / min | 753 / 783 / 573 / 521 |
| Best common-suffix mean / p50 / p10 | 580 / 618 / 438 |
| Global minimum (seed + common suffix) | **282** (`176f2d51` / `17992d51`) |
| Exact after-foldback merges | **0** |

Distances remain large; sampling many seeds does not show a collapse to zero in
this budget. Beam/LSH on multiple bridged paths remains open for deeper work.

### 7.3 P1-e Offline redesign prototypes

`scripts/controller_redesign_prototypes.py` — one-symbol alias counts:

| Variant | Idea | 1-byte contexts | 2-byte sample (4096 prefixes) |
|---|---|---:|---:|
| canonical | frozen controller | **3** | 62 |
| A | drop symbol from amount | 7 | — |
| B | XOR amount mix | 167 | — |
| C | dual-bit axis | 5 | — |
| D | XOR/rotate amount | 1 | — |
| **E** | feedback lane before mod 7 | **0** | **14** |
| F | popcount amount | 465 | — |

Variant **E** eliminates the known one-byte-context surface and reduces sampled
two-byte residual aliases (~4× fewer than canonical in a 4096-prefix sample) but
**does not** achieve injectivity. Residual pairs use non-`42` deltas (e.g.
`36/71`). E is a **lead redesign sketch**, not a successor candidate.

### 7.4 Tools added

```bash
./build/pvc-dual-return-alias --suffix-bytes 1 --suffix-limit 256
./build/pvc-multipath-foldback-sample --seed-limit 128 --suffix-samples 64
python3 scripts/controller_redesign_prototypes.py --variants E --deep --two-byte-samples 4096
```

### 7.5 Updated conclusion

- RotHash-1 still has no known after-foldback merge in expanded dual/multipath
  checks; the foldback gate remains standing under these attacks.
- Simple formula patches often **worsen** alias counts; only careful non-linear
  mixing (variant E) improved the short-domain surface.
- Per `docs/EFFORT_POLICY.md`, the foldback-vs-known-forward class is
  **Phase-1 budget-closed** (inductive; not a proof). More same-method suffix
  grinding is deferred.
- Provisional Gate B (`docs/CONTROLLER_DECISION_MEMO.md`): prefer **Redesign-2**
  for general-purpose acceptance; high-value work is alias-free controller
  prototypes (E-class+), not endless foldback near-miss distance.

### 7.6 Inductive close (foldback vs known forward multicollisions)

```text
Attack class: known forward multicollisions → after-foldback merge
Domains and cost: exhaustive 3-byte foldback; 1496 dual-return + 1-byte
  suffixes; prior 2^32 MITM samples; 128-seed multipath; prior beam/LSH
Mechanism: return XOR preservation; large first reverse divergence; no
  catalogue one-step return dual
Result: no break in these domains
Inductive close: further same-method extension is low EV
Re-open if: SAT dual, multi-step reverse merge, digest break, external contradict
This is not a security proof.
```

---

## 8. Redesign variant E — residual alias mechanism (2026-08-01)

Offline only. Not a candidate. Exact cube-state equality for every reported pair.

### 8.1 Full two-byte domain catalogue under E

All `2^16` two-byte prefixes; for each, all 256 next symbols; exact state match:

| Metric | Value |
|---|---:|
| Alias instances | **183** |
| Unique symbol pairs | **4** |
| Physical six-move path identical | **183 / 183** |
| One-byte-context aliases (prior) | **0** |

| Symbol pair | Δ | Instances |
|---|---:|---:|
| `36` / `71` | 59 | 78 |
| `2b` / `2c` | 1 | 36 |
| `61` / `80` | 31 | 36 |
| `2d` / `ae` | 129 | 33 |

So E collapses the classical `42/126/196` one-byte surface but leaves a
**tiny, discrete residual family** at depth two — not a diffuse random failure.

### 8.2 Mechanism (principled, not cosmetic)

Variant E builds a feedback `lane`, then:

```text
selector = lane XOR rotl(probe, ...)
axis     = CHOOSE_OTHER_AXIS(prev, selector)   // LSB only
amount   = 1 + ((lane + rotl(geometry,1) + axis) % 7)
```

Traced residuals (example `36/71` on a live context):

- `lane` values on the two sides can **differ**;
- when they differ by a multiple of 7, **amount matches**;
- selector LSBs still match, so **axis matches**;
- the full six-move physical path therefore matches → exact state collision.

This is the same *modular rail* idea as the frozen controller’s amount `% 7`,
relocated into the E lane. Killing the old `2d ≡ 0 (mod 7)` double-add of
`symbol` is not enough if a later mix still collides mod 7 for six phases.

### 8.3 Blind patches E4/E5 (stopped under effort policy)

Targeted-looking amount rewires without a full residual theory:

| Variant | One-byte aliases | Two-byte aliases | Verdict |
|---|---:|---:|---|
| E | 0 | 183 (4 pairs) | **lead sketch** |
| E4 | 2 | 194 | worse — discarded |
| E5 | 5 | 260 | worse — discarded |
| E6 | scan aborted | — | not completed; low EV after E4/E5 |

Further random ARX tweaks are **budget-closed** for this redesign fork until a
fix that *provably* breaks “Δlane ≡ 0 (mod 7) for six phases” is written down
and tested once.

### 8.4 High-value next redesign step (when resumed)

1. Formalize a single amount/axis rule that forbids simultaneous
   (same axis LSB) ∧ (amount equal) whenever `symbol_L ≠ symbol_R` for the
   four residual pairs *and* the old 42-family (one design, one full scan).
2. Prefer structural separation (e.g. amount depends on a byte-bijective mix of
   `(control, symbol)` before `% 7`, or axis uses more than one mixed bit with
   proven sensitivity to Δsymbol) over constant thrash.
3. Only after zero aliases on full one- and two-byte domains: cheap avalanche /
   two-byte digest smoke, then consider a named successor candidate.

### 8.5 Acceptance reading

- E is evidence that **the frozen controller’s alias surface is not inevitable**.
- E is **not** ready to mint RotHash-2.
- Honest status: lead offline sketch with a fully listed residual catalogue.

---

## 9. Science-first goal + principled variant G (2026-08-01)

Goal ordering locked in `docs/RESEARCH_GOAL.md`: publishable science first;
acceptance only if a repaired controller earns it. Design **family continues**;
RotHash-1 controller remains a disclosed defect.

### 9.1 Comparative full two-byte one-symbol alias scan

Same methodology for all rows: every 2-byte prefix × all next symbols; exact
operational state equality. One-byte context scan also run.

| Variant | 1-byte aliases | 2-byte instances | Unique pairs | Notes |
|---|---:|---:|---:|---|
| canonical (frozen) | 3 | **728** | 10 | Only Δ ∈ {42,126,196}; matches known local catalogue size |
| E | 0 | **183** | 4 | Residual mod-7 lane rail (§8) |
| **G** | 0 | **187** | 4 | New pairs; did **not** beat E |

G pairs (all physical-path style expected; not expanded here):

```text
3c/ac (Δ=112, n=53), 87/fb (Δ=116, n=51),
3c/bc (Δ=128, n=42), 4b/a8 (Δ=93, n=41)
```

### 9.2 What G tried (principled, not thrash)

G keeps E’s lane, then sets

```text
amount_source = 3·lane + 5·symbol + 1·control + 2·probe
              + 4·geometry + 6·phase + 3·axis
amount = 1 + (amount_source % 7)
```

with all displayed coefficients **nonzero mod 7**, so that when Δlane ≡ 0
(mod 7), Δamount_source ≡ 5·Δsymbol (mod 7) and E’s four residual Δsymbol
values (none ≡ 0 mod 7) cannot share amount. That calculation is sound for
*that* obstruction; the scan shows **other** joint (axis, amount, path)
collisions appear instead.

### 9.3 Scientific conclusion (publishable)

1. **Architecture** (cube, rotations, foldback, closures, squeeze) remains a
   legitimate research object.
2. **Controller injectivity** is the central hard problem; amount-mod-7 / LSB
   axis structure repeatedly reintroduces finite alias families under redesign.
3. Progress is real and quantitative: 728 → 183 residual instances (E), without
   restoring the one-byte 42-anchors.
4. A single linear-mod-7 coefficient fix (G) is **insufficient** for zero
   two-byte aliases; next redesign must change the *shape* of the choice
   (e.g. stronger axis dependence, or abandoning bare `% 7` amount), not only
   coefficients — or the paper stops at this comparative negative for G.

### 9.4 Effort stop (superseded by §10)

Coefficient thrash stopped. Structural path opened in §10.

---

## 10. Competitive program + structural controller H (2026-08-01)

Goal reaffirmed: build toward a **serious general-purpose / PQC-era hash**, not
a “failed attempt” paper (`docs/RESEARCH_GOAL.md`). Hard gates in
`docs/CONTROLLER_REQUIREMENTS.md`.

### 10.1 Structural prototype H

H changes more than amount coefficients:

- nonlinear **control init** (not `symbol + index + coord` alone);
- odd multiplications on `Z/256Z` + residue map for amount;
- symbol-sensitive axis mix every phase;
- nonlinear control evolution feeding symbol back each phase.

H2 (fixed 256→{1..7} table) **failed** G2/G3 and is discarded as lead.

### 10.2 Gate results

| Variant | G1 | G2 | G3 instances / pairs |
|---|---|---|---|
| canonical | pass | fail | 728 / 10 |
| E | pass | pass | 183 / 4 |
| G | pass | pass | 187 / 4 |
| **H** | **pass** | **pass** | **161 / 2** (`58/c5`×82, `6e/c6`×79) |
| H2 | pass | fail 57 | 10890 / 78 |

### 10.3 Reading for the competitive path

- **Progress is real:** first controller to pass G1∧G2 with only two G3 pairs.
- **Not done:** G3 must be zero before minting any successor candidate.
- **Do not** mint RotHash-2 or claim Keccak-class standing until G3 passes and
  ST smoke tests run.

### 10.4 H residual mechanism (traced)

Prefixes: `0247` → `58/c5` (Δ=109); `0056` → `6e/c6` (Δ=88). Counts 82 and 79.

On both pairs, **all six phases** share identical `(axis, amount)` while
`control` and intermediate `lane2` differ. Amount collides because:

```text
residue = (mul_odd(lane2, 41) XOR (lane2>>3) XOR (lane2>>5) XOR phase) mod 7
```

depends only on `lane2` (and phase), not on a direct symbol term that would
separate those two symbols when `lane2` already collides mod the residue map.

### 10.5 Targeted follow-ups (H3–H5)

| Variant | Change | G1 | G2 | G3 | Verdict |
|---|---|---|---|---|---|
| H3 | harden axis + heavy residue mix | pass | fail 1 | 183/3 | worse |
| H4 | H axis + `mul_odd(symbol,13)` in residue | pass | pass | **337/5** | H residuals split; **G3 worse** |
| H5 | amount = `1+mul_odd(lane2⊕symbol,41)%7` | pass | fail 1 | 243/3 | worse |

**Conclusion:** Splitting the two known H pairs on sample prefixes is easy;
doing so **without** increasing the full two-byte alias count is not. H stays
the competitive-path lead (best G1∧G2 with smallest G3). Further work must
attack the residue collision class globally, not only two symbol pairs.
