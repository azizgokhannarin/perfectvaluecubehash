# Trust Path Roadmap — From Laboratory to a Hash People Can Rely On

**Date:** 2026-08-02  
**Status:** Binding program roadmap for the competitive ambition  
**North star:** A hash in this design family that experts and practitioners can
**rationally choose to trust** for real security boundaries — not a research toy
and not a slogan.

**Related:**

| Document | Role |
|---|---|
| `docs/RESEARCH_GOAL.md` | Ambition and claim discipline |
| `docs/CONTROLLER_REQUIREMENTS.md` | Hard injectivity gates G1–G3 |
| `docs/CONTROLLER_G3_FAILURE_MODE.md` | Why H fails G3; next experiment contract |
| `docs/SECURITY_TARGET.md` | Targets vs claims |
| `docs/ACCEPTANCE_ROADMAP.md` | Public review and long-horizon gates |
| `docs/EFFORT_POLICY.md` | No low-EV thrash; inductive closes |
| `docs/SPEC_FREEZE.md` | Candidate identity and vector freeze |

PVC-RotHash-1 is **not** on this path as a product. It is the **baseline and
lesson set**. Trust attaches only to a **new candidate** that clears the gates
below.

---

## 0. What “everyone can use safely” actually means

In cryptography there is no certificate that says “unbreakable.” Trust is
**earned** when all of the following hold:

1. **No known structural shortcut** below generic complexity (especially cheap
   multicollisions, length tricks, output distinguishers).
2. **Clear specification** and multiple independent implementations that match
   official vectors.
3. **Sustained public cryptanalysis** (months to years) without a practical
   break at the claimed parameters.
4. **Honest parameter story** (classical and quantum query models).
5. **Engineering maturity** (portability, performance, API, constant-time policy
   if secret inputs are ever in scope).
6. **Social evidence**: independent reviewers, citations, optional standards
   discussion — never self-certification alone.

**Keccak / SHA-3** is the **quality bar** (process and scrutiny), not a promise
that we will match it on day one.

This roadmap turns that bar into **ordered, falsifiable stages**. Skipping a
stage is how projects ship false confidence.

---

## 1. Starting point (truth)

| Item | Status |
|---|---|
| Ambition | Competitive, PQC-era general-purpose hash |
| RotHash-1 | Frozen; usable as science baseline; **do not trust for security** |
| Known disqualifier | Forward controller multicollisions (42/126/196 family, bridges) |
| Best offline scheduler | **S (systematic)**: G1✓ G2✓ **G3✓ (0)** — 2026-08-02 |
| Prior lead H | G1✓ G2✓ G3✗ (161/2); superseded by S for injectivity |
| Foldback vs known forward collisions | Phase-1 budget-closed for same-method grind; not a proof |
| Production policy | Banned until an explicit later change |

**Bottom line:** We have a serious research platform and a clear defect class.
We do **not** yet have a trustable product.

---

## 2. The path in one picture

```text
  [0] Ambition & bars locked          ← YOU ARE HERE (docs exist)
         │
         ▼
  [1] Controller injectivity (G1–G3 = 0)
         │  H* one global amount-map experiment, stop-on-fail
         ▼
  [2] Smoke: avalanche, 2-byte digests, no v0 distinguishers (ST1–ST4)
         │
         ▼
  [3] Mint NEW candidate ID + dual impl + official vectors + freeze
         │  (not silent RotHash-1 edit)
         ▼
  [4] Deep falsification suite (existing tools + new methods)
         │  any structural break → back to [1] or redesign family
         ▼
  [5] Public independent review window (months+)
         │
         ▼
  [6] Parameter & PQ story; optional longer digests as separate IDs
         │
         ▼
  [7] Engineering pack (perf, ports, API, CT policy if needed)
         │
         ▼
  [8] Limited recommendation language (still not “proven secure”)
         │
         ▼
  [9] Organic community trust (years; not a checkbox we control)
```

Only after **[5]–[8]** is it honest to say “we believe this is suitable for
careful real use under stated assumptions.” Even then, absolute guarantees do
not exist.

---

## 3. Stages in detail

### Stage 0 — Program lock (done / maintain)

**Done when:**

- Competitive ambition written (`RESEARCH_GOAL.md`).
- Trust path (this document) and controller gates published.
- RotHash-1 freeze and non-use policy clear.

**Maintain:** update gates when evidence changes; never weaken claim discipline.

---

### Stage 1 — Controller that is not born broken

**Goal:** One-symbol absorption does not admit cheap operational-state
collisions in exhaustive short domains.

| Gate | Requirement |
|---|---|
| G1 | 256 symbols from initial state → 256 states |
| G2 | All 1-byte contexts: 256 next symbols injective |
| G3 | All 2-byte contexts: 256 next symbols injective |
| G4 | Known anti-families absent (RotHash-1 / E / G / H residuals) |

**Method contract** (`CONTROLLER_G3_FAILURE_MODE.md`):

- Lead H’s axis / init / evolve frozen unless a full rethink is declared.
- Next change = **one** global amount-map redesign (hypothesis H\*).
- One full G1–G2–G3 run; on fail → document and stop cascade (no H6 thrash).

**Exit:** G1 ∧ G2 ∧ G3 ∧ G4 all pass.  
**Fail exit:** redesign amount class or larger controller architecture; RotHash-1
remains science-only.

**This stage is non-negotiable.** Without it, “everyone can use it” is fantasy.

---

### Stage 2 — Statistical and reduced-round smoke

**Goal:** New controller does not revive v0-class output failures or collapse
under trivial reduced settings.

| ID | Smoke test |
|---|---|
| ST1 | Single-bit avalanche sample ~ half of 256 bits |
| ST2 | All `2^16` two-byte messages → distinct digests under **full** hash with new controller |
| ST3 | Triple-byte output repeats occur at ~random rate (no multiplicity ban) |
| ST4 | Reduced-round ladder documented; first unbroken preset identified |

**Exit:** ST1–ST4 pass or documented acceptable deviations with no structural
red flag.  
**Fail:** return to Stage 1.

---

### Stage 3 — Birth of a real candidate

**Goal:** A named, frozen algorithm the world can attack.

**Deliverables:**

1. New algorithm identifier (e.g. PVC-RotHash-2 — name TBD).
2. Normative specification (update or new SPEC).
3. C++ and independent pure-Python (or other) implementations.
4. Official digest + phase vectors.
5. `SPEC_FREEZE` entry and candidate version tag.
6. CHANGELOG / security target updated; production still banned.

**Exit:** Cross-implementation verify green; tag cut.  
**Rule:** Changing equations after tag requires a **new** ID.

---

### Stage 4 — Deep falsification (in-house, then continuous)

**Goal:** Try hard to break the candidate with tools that already exist and with
new methods.

**Minimum campaigns:**

| Class | Examples (reuse / extend repo tools) |
|---|---|
| Multicollision / alias | Forward, foldback-aware, bridged, dual-return |
| Differential / related-input | Existing differential and related probes |
| Truncated collision scaling | 24–48+ bit campaigns vs birthday |
| Digest surface | Beam / LSH / barrier correlation |
| Reduced / solver | SAT/SMT or independent model of the controller |
| Invariants | Histogram-preserving / cube geometry attacks |

**Exit criteria for this stage (internal):**

- No exact full-digest collision found better than generic in stated budgets.
- No after-foldback operational merge that implies full collision for equal
  lengths in stated budgets.
- No scalable structural multicollision that survives finalization.
- All negative and positive findings logged (`ATTACK_LOG`, KNOWN_CRYPTOANALYSIS).

**Fail:** any practical structural break → Stage 1 or family rethink.  
**Note:** Passing Stage 4 is still **not** “secure”; it is “we could not break
it with our best effort.”

---

### Stage 5 — Independent public stress

**Goal:** Trust starts outside the author circle.

**Actions:**

1. Public repo, challenge document, issue templates (partly ready).
2. Pinned review invitation; announce experimental status.
3. Leave frozen for a **defined window** (suggest ≥ 6–12 months before any
   “limited use” language).
4. Credit external findings; fix only as new candidates if algorithm changes.

**Exit:** Window completed with documented external engagement (or documented
outreach + silence — weak but honest).  
**Strong exit:** independent writeups / reproductions with no break.

---

### Stage 6 — Parameters and post-quantum honesty

**Goal:** People know what 256 bits means under quantum query models.

| Work | Output |
|---|---|
| Classical targets | Aspirational table vs measured attack costs |
| Quantum notes | Grover preimage; quantum collision scaling |
| Longer digests | 384/512 only as **separate candidate IDs** if needed |
| Application matrix | OK / discouraged / forbidden uses under evidence |

**Exit:** `SECURITY_TARGET.md` (or successor) complete without overclaim.

---

### Stage 7 — Engineering for real users

**Goal:** If cryptanalysis still holds, the artifact is shippable as software.

- Performance baselines (multiple ISAs if possible).
- Packaging, CI, language ports.
- API stability proposal.
- Constant-time / side-channel policy **if** secret-input use is ever allowed.
- FIPS/style process is optional and late; correctness and clarity first.

**Exit:** Reproducible builds, benchmarks, portability notes.

---

### Stage 8 — Policy: limited recommendation (optional, careful)

**Only if Stages 1–7 are green and Stage 5 window is non-trivial:**

- Soften production ban to something like:  
  *“Experimental but no known break after effort X; not for high-value long-term
  secrets; prefer SHA-2/SHA-3 for production defaults.”*
- Or keep ban and position as research reference implementation.

**Never:** “proven secure,” “PQC-secure,” “Keccak replacement.”

---

### Stage 9 — Organic trust (years; not fully under our control)

- Independent libraries, academic citations, continued challenges.
- Optional standards / community processes only after Stage 8 is boringly stable.

**We cannot schedule “everyone trusts it.”** We can only earn the right to ask.

---

## 4. Immediate next actions (this quarter)

Ordered by dependency:

1. **Execute hypothesis H\*** per `CONTROLLER_G3_FAILURE_MODE.md`  
   - one global amount map; freeze H axis/init/evolve;  
   - single G1–G2–G3 run; stop-on-fail.
2. **If G3 passes:** run ST1–ST4 smoke on a research full-hash path using H\*.
3. **If ST pass:** draft RotHash-2 (name TBD) spec diff vs RotHash-1; dual impl
   plan; vector generation.
4. **If any fail:** update failure-mode note; choose next **class** of controller
   change (not micro-variants); keep publishing science of RotHash-1 + failed
   attempts as evidence, not as the product.

Parallel (does not replace 1–3):

- Keep RotHash-1 public package healthy (CI, vectors, challenge).
- Paper outline that **leads with the candidate we will mint**, using RotHash-1
  as prior art / negative controller case — not as the hero product.

---

## 5. What we must not do on this path

| Anti-pattern | Why |
|---|---|
| Ship RotHash-1 for real use | Known structural multicollisions |
| Silent equation edits after freeze | Destroys reviewability |
| “PQC-secure” marketing early | Lies by implication |
| Infinite foldback distance campaigns | Low EV; wrong layer |
| Residual whack-a-mole (H3–H5 style) | Overfits; worsens G3 |
| Claim trust after only in-house tests | Trust requires outsiders |

---

## 6. Realistic time scale

| Milestone | Order-of-magnitude |
|---|---|
| Stage 1 (G3 = 0) | weeks–months of focused design (not guaranteed) |
| Stage 3 (new candidate freeze) | weeks after Stage 1–2 |
| Stage 4–5 (serious public stress) | **many months to years** |
| Stage 8–9 (broad practical trust) | **years**, contingent on no break |

Wanting a trustworthy hash is right. Believing it is a short sprint is wrong.
The roadmap is long **because** the goal is real.

---

## 7. Success and honest failure

**Success:** A frozen successor with no known structural break after deep and
external attack, dual implementations, clear non-claims, and a cautious use
policy people can understand.

**Honest failure:** After serious Stage 1–4 effort, the family cannot eliminate
cheap scheduler collisions or fails public attack. Then publish the science
(architecture + cryptanalysis + failed controllers) and stop claiming a product
path. That is still valuable work — it is not “everyone can use it.”

---

## 8. One-line program statement

**Do not ask the world to trust RotHash-1; build a new candidate that earns
trust gate by gate — injectivity first, then smoke, freeze, deep attack, public
review, parameters, engineering, and only then careful recommendation.**
