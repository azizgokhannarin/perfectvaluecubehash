# Controller Decision Memo (Provisional)

**Date:** 2026-08-01  
**Candidate:** PVC-RotHash-1 `1.0.0-rc1` (frozen; unchanged by this memo)  
**Ambition:** science-first publishable result; acceptance only if redesign works
(`docs/RESEARCH_GOAL.md`, `docs/ACCEPTANCE_ROADMAP.md`)  
**Effort rule:** `docs/EFFORT_POLICY.md`

This memo records a **provisional** Gate B judgment so effort stops thrashing
and moves to high-value work. It is not a security proof and not a final
abandon/keep/redesign statute until redesign injectivity and one more
qualitatively new attack class are settled.

---

## 1. Facts that are settled

1. **Forward controller is non-injective.** Exact aliases and bridged
   multicollisions (theoretical `2^32` family) are real and disclosed.
2. **Root cause is formula-level:** `symbol` enters both `control` and
   `amount_source` linearly; amount is reduced mod 7; axis is LSB-driven
   (`docs/PHASE1_CONTROLLER_CAMPAIGN.md` §2).
3. **Δ ∈ {42,126,196}** sit in the phase-0 candidate class `d ≡ 0 (mod 14)`;
   Δ=42 alone yields the three one-byte-context full aliases.
4. **Foldback separates known forward collisions in all completed domains**
   listed in §2 below; no exact after-foldback merge is known.

---

## 2. Foldback-vs-known-forward-multicollision — inductive close

**Attack class:** Turn known forward multicollisions (and their common-suffix /
dual-return extensions) into equal after-foldback operational states.

| Evidence | Domain / cost (summary) |
|---|---|
| Exhaustive 3-byte after-foldback | `2^24` messages, 0 merges |
| Direct return-gate dual | 1,496 pairs, 0 merges; min distance 181 bits |
| Common 1-byte suffixes on all pairs | `1496×256`, 0 after-foldback merges |
| Prior independent 2-byte suffix MITM | `3×2^32` on original 2-byte prefixes, 0 |
| Multipath sample | 128 seeds, min 282 bits with common 1-byte samples, 0 merges |
| Prior beam/LSH foldback campaigns | Near states, no merge, no monotone path to 0 |

**Mechanism:** Position-dependent return map preserves XOR differences; first
reverse step on the last differing original byte changes many cube cells; no
catalogue pair is also a one-step return alias from the common forward state.

**Inductive close (project effort policy):**  
Further *same-method* work (more common suffixes, more seeds, slightly larger
beams on the same objective) is **low expected value** for changing this
conclusion. The class is **budget-closed for Phase 1**, not proven safe.

**Re-open if:** SAT/SMT finds a dual; a multi-step reverse alias chain merges;
a digest-level attack succeeds; or an external result contradicts the above.

**This is not a security proof.**

---

## 3. What is *not* closed

| Issue | Status | Acceptance impact |
|---|---|---|
| Forward multicollision surface | **Open / broken injectivity** | Blocks general-purpose credibility while cheap |
| Residual redesign injectivity | Prototype E clears 1-byte domain, not 2-byte | Redesign unfinished |
| Full digest collision | None known; prior surface searches ~generic | Still open class (different method) |
| Preimage / formal bounds | Untouched | Open |

---

## 4. Provisional Gate B choice

Given the **general-purpose** ambition:

### Preferred paths (ordered by `docs/RESEARCH_GOAL.md`)

**P0 — Publish RotHash-1 science (now):** frozen candidate, disclosed forward
multicollisions, foldback budget-close, redesign experiments E/G as
comparative evidence. This is the **default success** if no zero-alias
controller appears soon.

**P1 — Redesign-2 only if injectivity is won:** Experts will not accept a hash
with cheap forward multicollisions. Value is killing aliases at the controller,
then minting a **new** ID.

**Lead sketches (offline, not candidates):**

| Sketch | G1 | G2 | G3 | Status |
|---|---|---|---|---|
| E | pass | pass | 183 / 4 pairs | mod-7 lane residual |
| G | pass | pass | 187 / 4 pairs | coefficient-only; failed |
| H | pass | pass | 161 / 2 | superseded for injectivity |
| H2–H5 | mixed | mixed | worse | discarded |
| **S** | **pass** | **pass** | **0** | **injectivity lead** (mixed-radix) |

S implements systematic coding from external advice: phases 0–2 carry `Π(s)` in
three Z/7 amounts with state-only context translation; phases 3–5 diffuse.
**G1∧G2∧G3 passed 2026-08-02.** Next: ST1–ST4 smoke, then candidate mint only
if smoke holds. Still not a security proof.

### Rejected as primary path: endless Keep-1 foldback grinding

Keep-1 remains valid as a *scientific freeze* for public attack on RotHash-1,
and foldback is budget-closed against the *known-forward* class above. Keep-1
is **not** sufficient for the acceptance north star while forward aliases stand.

### Negative path

Only if redesign cannot remove short-domain aliases without destroying the
design boundary or statistical behavior — then publish as a negative research
result. Not chosen yet.

---

## 5. High-value next steps only

Do these; defer the rest.

| Priority | Work | Why valuable |
|---:|---|---|
| 1 | Harden E-class (or better) until **zero** one- and two-byte context one-symbol aliases in full or near-full scan | Directly attacks the acceptance blocker |
| 2 | On a winning prototype: cheap statistical smoke (avalanche sample, two-byte digest uniqueness) before any rename | Avoids shipping a distinguisher |
| 3 | Qualitatively new attack on frozen RotHash-1 only if cheap: e.g. reduced-round SAT sketch — not more suffix MITM clones | New information |
| 4 | External package remaining Gate A human steps (pinned issue/announce) | Independent eyes |
| — | More multipath/beam on known forward pairs | **Deferred** (budget-closed) |

When (1) succeeds, mint **PVC-RotHash-2** (name TBD) with new vectors; keep
RotHash-1 frozen as historical science.

---

## 6. Explicit non-claims

- Foldback is not proven injective.
- RotHash-1 is not collision-resistant.
- Variant E is not a candidate and has residual aliases.
- No production use.

---

## 7. Sign-off

Provisional decision for effort allocation:

```text
Path = Redesign-2 (preferred for acceptance)
RotHash-1 = remain frozen public attack target
Foldback-vs-known-forward class = Phase-1 budget-closed (not a proof)
Next value = alias-free controller prototype → then new candidate ID
```
