# Session Handoff — Resume Here

**Last updated:** 2026-08-02  
**Purpose:** Permanent memory of this work stream so any future session (human or
agent) continues **exactly** from the current program state with no amnesia.  
**Git tip of work:** check `git log -5 --oneline` on `main` (should include
Stage 2 smoke commit for Controller S).

---

## 1. One-screen status

| Item | Value |
|---|---|
| **Ambition** | Build a hash people can eventually trust (Keccak-class **quality bar**, not a claim). RotHash-1 is **not** that product. |
| **Stage 0** | Program docs locked (trust path, goals, effort policy) |
| **Stage 1** | **PASS** — Controller **S** G1∧G2∧G3 = 0 |
| **Stage 2** | **PASS** — ST1–ST4 smoke with S full-hash path |
| **Stage 3** | **IN PROGRESS / draft minted** — PVC-RotHash-2 in tree (C++/Python/SPEC/v2 vectors) |
| **Stage 4** | **NEXT** — Deep falsification on RotHash-2 |
| **Production** | Still **forbidden** |
| **Security claims** | **None** |

**Exact next work when resuming:**

```text
Stage 3 polish + Stage 4:
  1. Expand official-v2 vectors / phase dumps; CTest cross-verify v2
  2. Optional tag v0.2.0-rothash2-draft
  3. Point attack tools at systematic_absorb / RotHash2
  4. Deep multicollision / truncated / digest campaigns
  5. Update public challenge for RotHash-2
```

Do **not** reopen H3–H5 residual whack-a-mole. Do **not** treat RotHash-1 as
the product. Do **not** claim security after Stage 2/3 alone.

---

## 2. North-star decisions (do not reverse casually)

1. **Goal:** Real competitive hash for eventual safe use — not a “we tried and
   failed” paper as the primary product (`docs/RESEARCH_GOAL.md`,
   `docs/TRUST_PATH_ROADMAP.md`).
2. **RotHash-1** = frozen public baseline + lesson set; known forward
   multicollisions; **do not use for security**.
3. **Trust path stages 0–9** must not be skipped (`docs/TRUST_PATH_ROADMAP.md`).
4. **Controller gates G1–G3** are hard before any new candidate
   (`docs/CONTROLLER_REQUIREMENTS.md`).
5. **Effort policy:** no low-EV thrash; inductive closes; **consult outsiders
   when structurally stuck** (`docs/EFFORT_POLICY.md` §7).
6. **G3 lesson:** random-like `mod 7` / LSB schedulers expect ~hundreds of G3
   collisions; G3=0 needs **structural** injectivity, not pair-local patches
   (`docs/EXTERNAL_ADVICE_G3.md`, `docs/CONTROLLER_G3_FAILURE_MODE.md`).

---

## 3. Technical memory — Controller S

### What S is

- Offline research scheduler in
  `scripts/controller_redesign_prototypes.py` class **`ControllerS`**.
- **Injectivity channel (phases 0–2):**  
  `t = Π(s)` (fixed public byte perm, odd-mul NUMS style),  
  `d0,d1,d2` mixed-radix base 7,  
  `amount_i = 1 + ((d_i + c_i) mod 7)` where **`c_i` is state-only** (no
  message symbol).  
  ⇒ For fixed start-of-symbol context, `s ↦ (amount0,amount1,amount2)` is
  injective by construction.
- **Diffusion (phases 3–5 + axes):** free mixes (may use symbol).
- **Critical bug once fixed:** first S draft put symbol-dependent `control`
  into `c_i` — **wrong**. Correct: `context_seed` from probe/cursor/prev/index
  only.

### Gate evidence

```bash
python3 scripts/controller_redesign_prototypes.py --variants S --deep --two-byte-full
# G1=PASS G2=PASS G3=PASS (0 aliases)

python3 scripts/stage2_smoke_s.py
# ST1–ST4 PASS
```

### Stage 2 numbers (record)

| Test | Result |
|---|---|
| ST1 avalanche (256×16B) | mean **128.06**/256 bits |
| ST2 two-byte digests | **65536** unique, 0 collisions |
| ST3 triple-byte rate | **6.72%** (5000 samples) |
| ST4 ladder | full S coll=0; R0-like coll=20 on 4096 msgs |
| Sanity | S("abc") ≠ RotHash-1("abc") |

Details: `docs/STAGE2_SMOKE_S.md`.

### Full-hash research wiring

`scripts/stage2_smoke_s.py` implements a **research** full path:

- absorb = S for every symbol (message, foldback, closures, squeeze feeds);
- finalization **shape** same as RotHash-1 (foldback formula, 64 diagonal, 128
  orbit, 32-byte four-diagonal squeeze);
- **not** official vectors; not dual-impl freeze package.

---

## 4. Historical path (so we do not re-learn)

| Controller | G1 | G2 | G3 | Note |
|---|---|---|---|---|
| canonical (RotHash-1) | pass | fail | 728/10 | 42/126/196 family |
| E | pass | pass | 183/4 | residual mod-7 lane |
| G | pass | pass | 187/4 | coefficient-only fail |
| H | pass | pass | 161/2 | best pre-S; residuals 58/c5, 6e/c6 |
| H3–H5 | mixed | mixed | worse | local residual thrash |
| **S** | **pass** | **pass** | **0** | **current injectivity lead** |

RotHash-1 structural facts still true:

- Forward multicollisions / bridged families exist and are disclosed.
- Foldback separates known forward pairs in large tested domains (budget-closed
  for same-method grind; not a proof).
- v0 multiplicity/positional distinguishers addressed in RotHash-1 extraction.

---

## 5. Document map (read in this order when resuming)

1. **`docs/SESSION_HANDOFF.md`** ← this file  
2. `docs/TRUST_PATH_ROADMAP.md` — stages 0–9  
3. `docs/STAGE2_SMOKE_S.md` — latest smoke results  
4. `docs/EXTERNAL_ADVICE_G3.md` — why S works in principle  
5. `docs/CONTROLLER_REQUIREMENTS.md` — G1–G3, ST1–ST4  
6. `docs/RESEARCH_GOAL.md` — ambition vs claims  
7. `docs/EFFORT_POLICY.md` — thrash / consult rules  
8. `SPECIFICATION.md` — **RotHash-1 only** until Stage 3 rewrites absorb  

---

## 6. Code map

| Path | Role |
|---|---|
| `src/engine.cpp` | RotHash-1 normative absorb (frozen) |
| `reference/python/pvc_rothash1.py` | RotHash-1 pure Python |
| `scripts/controller_redesign_prototypes.py` | Controllers E/G/H/S; G1–G3 harness |
| `scripts/stage2_smoke_s.py` | Full-hash S path + ST1–ST4 |
| `docs/*` | Program memory and results |

---

## 7. Resume checklist (agent / human)

- [ ] `git pull` and `git log -5 --oneline`  
- [ ] Read this handoff + `TRUST_PATH_ROADMAP.md` § immediate actions  
- [ ] Confirm S still:  
  `python3 scripts/controller_redesign_prototypes.py --variants S --deep`  
  (optional full `--two-byte-full` if distrusting cache)  
- [ ] Confirm smoke: `python3 scripts/stage2_smoke_s.py --skip-st2` for quick,  
  or full ST2 when time allows  
- [ ] Start **Stage 3**: candidate name + SPEC absorb=S + C++ port plan  
- [ ] If stuck on architecture: **consult outside** (do not H6-style thrash)

---

## 8. Session transcript note

Chat UIs may not persist forever. **This file + git history are the durable
memory.** Important commits in this arc include (names approximate):

- Acceptance / trust path packaging  
- Controller campaign, G3 failure mode, external advice  
- **S G1–G3 pass**  
- **Stage 2 smoke pass** (`stage2_smoke_s.py`, `STAGE2_SMOKE_S.md`)

If a local agent session transcript exists under `~/.grok/sessions/`, it is
supplementary only; **do not rely on it as the sole resume source**.

---

## 9. Emotional / process contract (user preference)

- User wants a **real** hash path to eventual trusted use.  
- Prefers **reasoning over thrash**; approved stop-and-analyze.  
- Approves **external consultation** when structurally stuck.  
- Expects **meaningful git commits** and push of progress.  
- Production ban and no premature security claims are non-negotiable.

---

## 10. Closing line for the next session

> PVC-RotHash-2 is drafted in-tree (`RotHash2`, `SPECIFICATION_ROTHASH2.md`,
> `official-v2.json`) with Controller S absorb. Resume at **Stage 3 polish +
> Stage 4 deep attack**, without security marketing and without touching
> RotHash-1 official vectors.
