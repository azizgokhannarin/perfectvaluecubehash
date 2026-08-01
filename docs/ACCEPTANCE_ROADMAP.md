# Acceptance Roadmap — Toward Credible PQC-Era Hash Status

**Status:** Strategic plan (not a security claim)  
**Candidate line:** PVC-RotHash-1 `1.0.0-rc1` and any successor required by evidence  
**Last updated:** 2026-08-01  
**Supersedes for strategy:** the short “Next campaign” section of `docs/ROADMAP.md`  
**Does not supersede:** historical completed work in `docs/ROADMAP.md`

This document defines how the project may move from a frozen research candidate
toward a construction that the cryptographic community could *rationally
consider* for post-quantum-era hash use. It deliberately does **not** assert
that the construction is secure, standardized, or ready for production.

---

## 0. North star and honesty bound

### North star

Produce an original, primitive-free Perfect Value Cube hash whose:

1. known structural weaknesses are either **removed by redesign** or
   **shown not to yield full-hash shortcuts** under multi-year independent attack;
2. security story is **explicit, quantitative, and falsifiable** (classical and
   quantum query models);
3. implementations are **conforming, portable, and reproducible**;
4. external experts can verify claims without trusting the authors.

Only then is any conversation about “acceptance” legitimate.

### Honesty bound (non-negotiable project principles)

These rules already govern the repository and remain binding:

| Principle | Implication for this roadmap |
|---|---|
| Falsification over marketing | Every phase ends with attacks that can kill the candidate. |
| No premature security claims | No “PQC-secure”, “collision-resistant”, or “production-ready” language until evidence and external review support a *carefully worded* claim — and even then, prefer “no better-than-generic attack known after effort X”. |
| Freeze discipline | Algorithm changes require a **new candidate identifier** and new vectors (`docs/SPEC_FREEZE.md`). |
| Exact verification | Fingerprint/LSH tools never report collisions without full-state checks (`docs/DECISIONS.md` D-009). |
| Forward equality ≠ digest collision | Structural findings are documented at the deepest true phase (`D-010`). |
| Production ban until decision gate | `SECURITY.md` remains prohibitive until a later explicit policy change. |

### What “PQC-era acceptance” means here

Hash functions are not selected by a NIST PQC KEM/signature process. In practice,
post-quantum-era acceptance for a **new** hash means all of the following:

1. **No sub-generic classical break** at the claimed parameters after serious
   public cryptanalysis.
2. **Quantum query model is stated honestly** (e.g. Grover for preimage,
   Brassard–Høyer–Tapp / quantum collision bounds for multicollision-style
   search) and output length / application mapping is justified.
3. **Structural multicollision and herding surfaces** are either absent or
   proven not to cross finalization under known techniques.
4. **Independent implementations and vectors** exist.
5. **Peer review** (paper, workshop, external writeups) exists.
6. **Use cases are scoped** (e.g. general-purpose hash, commitment, hash-based
   signature building block) with matching strength and performance.

This roadmap optimizes for that bar. It does **not** optimize for early hype,
productization, or embedding in protocols before the bar is met.

---

## 1. Current position (baseline)

As of PVC-RotHash-1 `1.0.0-rc1`:

| Area | Status |
|---|---|
| Normative spec + dual implementation + official vectors | Done |
| Simple distribution / avalanche distinguishers (v0-class) | Rejected in documented domains |
| Forward controller injectivity | **Broken** — scalable bridged multicollisions |
| Alias family | Differences `42`, `126`, `196` in forward and reverse contexts |
| Foldback as separation layer | Holds in all *tested* finite domains; not proven |
| Full 256-bit digest collision | None known |
| Sub-generic truncated collision | None demonstrated |
| Security proof / reduction | None |
| Independent external review (outside author tooling) | Not yet a completed program |
| Public tag + announcement package | Checklist partially open (`docs/PUBLIC_RELEASE_CHECKLIST.md`) |

**Critical strategic fact:** a scalable Joux-style *forward* multicollision is
already a major obstacle to any acceptance narrative. Even if foldback currently
separates known families, a design whose uniqueness concentrates in one reverse
pass will not pass expert review without either (a) a rigorous separation
argument or (b) elimination of the alias family.

---

## 2. Success criteria (acceptance gates)

Progress is measured only by gates. Soft feelings of “looking good” do not pass.

### Gate A — Scientific integrity package

- Frozen tag published; vectors stable.
- Challenge document live; known weaknesses fully disclosed.
- Anyone can reproduce KATs and phase vectors in under one hour.

### Gate B — Structural blocker resolution

Either:

- **B1 (redesign):** a successor candidate removes the `42/126/196` controller
  alias family (and preferably all cheap one-symbol aliases) in exhaustive
  short-domain and large sampled reverse/forward maps; **or**
- **B2 (proof-like evidence):** under multi-path, SAT, and dual-alias search,
  no better-than-generic path from forward multicollision structure to
  after-foldback or digest collision is found, *and* a clear mathematical
  explanation of separation cost is written and attacked.

Without Gate B, stop talking about acceptance.

### Gate C — Depth cryptanalysis

- Reduced-round ladder: break R0–Rk fully; document the first unbroken preset.
- Independent model (SAT/SMT/CP or pure-Python symbolic) of the move controller.
- Multi-domain truncated collision campaigns at 24–48+ bits vs birthday reference.
- Nonlinear / higher-order distinguisher suite with multiple-testing correction.
- Written negative results for C1–C8 style targets (`CRYPTANALYSIS_CHALLENGE.md`).

### Gate D — External stress

- At least one independent implementation or audit not authored by the project.
- Public review window with documented inbound analysis (or documented silence
  after genuine outreach — silence is weak evidence, not proof).
- Paper draft following `docs/PAPER_PLAN.md` constraints (no overclaim).

### Gate E — Parameter and PQ story

- Classical targets stated as **aspirations**, not theorems.
- Quantum model section: what 256-bit output means for collision vs preimage
  under standard quantum query attacks; whether a 384/512-bit variant is needed
  for long-horizon collision applications.
- Application matrix: which uses are in-scope if analysis remains favorable.

### Gate F — Engineering maturity (only after B–E trending positive)

- Constant-time policy for secret-input uses (if any such use is ever claimed).
- Performance baselines on multiple ISAs.
- Streaming semantics fully specified if retained.
- Packaging, CI, and cross-language conformance.

### Gate G — Policy change to limited recommendation

Only after B–F and human judgment: a **versioned** security policy may soften
from total ban to “experimental, not recommended for high-value secrets” or
similar. Full production recommendation remains a later, separate decision.

---

## 3. Phased program

Each phase has: objective, exit criteria, primary artifacts, and kill criteria.
Phases may partially overlap, but **Gate B must not be skipped**.

### Phase 0 — Close the public scientific package (weeks)

**Objective:** Make the frozen candidate maximally attackable and citable.

Work:

1. Complete `docs/PUBLIC_RELEASE_CHECKLIST.md` (release_check, CI, tag, archive).
2. Publish `v1.0.0-rc1` and the public-review announcement.
3. Ensure `docs/KNOWN_CRYPTOANALYSIS.md` and `CRYPTANALYSIS_CHALLENGE.md` are the
   single front doors for outsiders.
4. Pin an “independent review” issue/discussion.

**Exit:** Gate A.  
**Kill:** Spec/impl/vector discrepancy that cannot be resolved without silent
algorithm change → fix as bug or mint new candidate; never silent drift.

### Phase 1 — Decision campaign on the controller (1–3 months)

**Objective:** Resolve the central open decision in `docs/ROADMAP.md`:
keep, redesign, or abandon the move controller.

**Living campaign log:** `docs/PHASE1_CONTROLLER_CAMPAIGN.md`  
**Independent analysis helper:** `scripts/controller_alias_analysis.py`

Workstreams (parallel):

| ID | Workstream | Success signal |
|---|---|---|
| P1-a | Multi-path foldback beam/LSH over many independent bridged paths | Distribution of min distances; any exact merge is a full-phase break |
| P1-b | Dual reverse-alias search (paired return symbols with Δ∈{42,126,196}) | Exact after-foldback merge or strong negative evidence |
| P1-c | Alias taxonomy: inherited vs local; depth; bridge necessity | Clear map of multicollision cost |
| P1-d | Independent pure model of amount/axis equations (no shared C++ control) | Reproduced aliases; search for other Δ families |
| P1-e | Controller redesign prototypes **off the frozen branch** (research only) | Measure whether simple formula changes kill one-symbol aliases without reintroducing v0 distinguishers |

**Exit (choose one):**

- **Path Keep-1:** Gate B2 evidence package → continue Phase 2 on RotHash-1.
- **Path Redesign-2:** mint **PVC-RotHash-2** (or renamed candidate) with new
  vectors; re-run v0-class statistical battery and short-domain injectivity
  scans before deep investment.
- **Path Negative:** retain as published negative result; stop acceptance push.

**Kill:** Practical after-foldback merge or full digest collision on the
canonical candidate. That is a *successful research outcome*, then redesign or
stop.

### Phase 2 — Depth cryptanalysis on the chosen candidate (3–12 months)

**Objective:** Gate C.

Work:

1. SAT/SMT/CP models for reduced presets; complexity ladder vs R-index.
2. Digest-surface campaigns expanded orders of magnitude; multi-seed.
3. Differential / rotational trails aimed at **squeeze observations**, not only
   cube Hamming distance.
4. Histogram-preserving and invariant-subset attacks (rotation-only state).
5. Length-framing, herding, and expandable-message attempts that must cross
   foldback + length encoding.
6. 40/48-bit truncated campaigns toward birthday scale; publish censored runs
   honestly.

**Exit:** Written campaign reports with reproducible commands; first unbroken
reduced preset characterized.  
**Kill:** Scalable sub-generic collision/preimage/distinguisher.

### Phase 3 — Externalization (overlaps late Phase 1–2)

**Objective:** Gate D.

Work:

1. Preprint / workshop paper under `docs/PAPER_PLAN.md` rules.
2. Outreach to independent cryptanalysts; offer clear challenge targets and
   credit policy.
3. Optional third implementation (e.g. another language) for conformance only.
4. Optional formal specification style cleanup (RFC-like) without algorithm drift.

**Exit:** External review artifacts or documented outreach attempt + timeline.  
**Kill:** External break that authors cannot refute; then redesign or stop.

### Phase 4 — PQ parameter and application story (after Phase 2 trending)

**Objective:** Gate E.

Work:

1. Document classical aspirational targets vs measured attack cost (never equate).
2. Quantum section:
   - preimage: Grover scaling;
   - collision: quantum collision-search bounds;
   - whether long-term collision-sensitive apps need larger digests.
3. If needed, define **length-extension variants** (e.g. 384/512-bit squeeze)
   as *separate candidate identifiers*, not silent options.
4. Application matrix: allowed / discouraged / forbidden uses under current
   evidence.

**Exit:** `docs/SECURITY_TARGET.md` revised with PQ subsection still free of
false claims.  
**Kill:** Realization that required parameters destroy performance or design
identity without gaining a security argument — revisit Path Negative.

### Phase 5 — Engineering for scrutiny (only if Gates B–E hold)

**Objective:** Gate F.

Work: performance, portability, API stability proposal, constant-time analysis
if secret inputs are ever contemplated, packaging.

**Exit:** Engineering report + benchmarks.  
**Not an exit:** “fast enough” without cryptanalytic standing.

### Phase 6 — Community acceptance track (years, contingent)

**Objective:** Organic credibility, not self-certification.

Possible tracks (none promised):

- academic publication and continued challenges;
- independent libraries and interoperability tests;
- discussion in hash-design / lightweight / alternative-primitives venues;
- only much later, any standards liaison.

**Exit:** Outside parties cite, implement, and attack the work on their own.  
There is no single checkbox that “accepts” a hash into the world.

---

## 4. Immediate execution order (next 30–60 days)

Ordered by leverage under current uncertainty:

1. **Finish Gate A** — tag and public package (cheap, unblocks outsiders).
2. **Start P1-d** — independent mathematical model of the amount/axis controller
   (explains `42/126/196`; guides redesign).
3. **Start P1-e** — offline redesign experiments that eliminate one-symbol
   aliases while preserving design boundary (no SHA/AES/Keccak/…).
4. **Run P1-a/P1-b** — multi-path and dual-alias searches on frozen RotHash-1
   (protects against false confidence if we keep the candidate).
5. **Write a controller decision memo** — Keep-1 / Redesign-2 / Negative with
   evidence citations.

Do **not** spend early energy on product APIs, logos, or production integration.

---

## 5. Resource model

| Resource | Minimum viable | Preferred |
|---|---|---|
| Human | Author + AI-assisted tooling (disclosed) | + independent reviewers |
| Compute | Multi-core workstation campaigns | Cluster for 40/48-bit and SAT |
| Time to Gate B decision | ~1–3 months focused | Before any “acceptance” messaging |
| Time to credible external standing | Multi-year | Continuous public challenge |

AI assistance remains disclosed (`AUTHORS.md`). AI output is never cited as
cryptanalytic authority.

---

## 6. Communication rules for the acceptance push

1. Lead with **what is broken** and **what is unproven**, then methods and tools.
2. Prefer “no attack found in domain D at cost C” over “secure”.
3. Never imply NIST PQC selection or government endorsement.
4. Never hide the forward multicollision structure.
5. New candidates get new names/versions; old vectors stay for science.

---

## 7. Relationship to existing docs

| Document | Role after this roadmap |
|---|---|
| `docs/ROADMAP.md` | Historical completed work log; keep updating completions |
| `docs/ACCEPTANCE_ROADMAP.md` (this file) | Strategic path and gates |
| `docs/SECURITY_TARGET.md` | Quantitative aspirations and non-claims |
| `docs/SPEC_FREEZE.md` | Frozen-candidate change policy |
| `docs/PAPER_PLAN.md` | Publication evidence structure |
| `CRYPTANALYSIS_CHALLENGE.md` | External attack targets |
| `docs/PUBLIC_RELEASE_CHECKLIST.md` | Gate A checklist |

---

## 8. Locked strategic choices (2026-08-01)

Maintainer decisions for the acceptance push:

| Choice | Decision |
|---|---|
| Primary ambition | **General-purpose hash candidate** (long-horizon SHA-class use case, not a signature-only building block). |
| Controller policy | **Parallel Path:** deep attack on frozen RotHash-1 **and** offline redesign prototypes that aim to kill one-symbol aliases. Evidence chooses Keep-1 vs Redesign-2. |
| Digest length | **256-bit only for now.** Gate E writes an honest quantum-query section; any 384/512-bit squeeze is a *future separate candidate*, not a silent option. |
| Near-term focus (first ~2 weeks) | **Gate A:** public scientific package (`release_check`, tag `v1.0.0-rc1`, announcement, review entry points). |
| After Gate A | Immediately open Phase 1 workstreams P1-a–P1-e without waiting for external silence. |

Default execution line:

> Gate A now · then parallel attack + redesign · no security claims · redesign ready if Gate B2 fails.

---

## 9. One-line summary

**Earn the right to be taken seriously by killing or fully characterizing the
forward multicollision surface, freezing only what survives deep attack, and
never claiming post-quantum security as a slogan.**
