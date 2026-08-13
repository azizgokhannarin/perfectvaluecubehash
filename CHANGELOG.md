# Changelog

## Unreleased — Stage 3 polish + Stage 4 smoke (RotHash-2)

Experimental path only. RotHash-1 official vectors unchanged.

- expanded `test-vectors/official-v2.json` to 29 KATs;
- added `scripts/verify_vectors_v2.py` and CTest
  `pvc-cross-implementation-vectors-v2`;
- `pvc-collision-probe --rothash2` (1/2/r1-pair modes);
- added `scripts/stage4_smoke_r2.py` and CTest `pvc-stage4-smoke-rothash2`
  (SF1–SF4 PASS); docs `docs/STAGE4_SMOKE_R2.md`;
- production still prohibited; no security claim.

## Unreleased — PVC-RotHash-2 draft candidate (Stage 3)

Experimental absorb path only. RotHash-1 official vectors unchanged.

- added `pvc::RotHash2` and `HashParameters::systematic_absorb` (Controller S);
- added `SPECIFICATION_ROTHASH2.md`, `reference/python/pvc_rothash2.py`,
  `test-vectors/official-v2.json`;
- CLI `--rothash2`; research preset `R5-rothash2`;
- regression tests: abc digest matches research S path; separates RotHash-1
  forward-pair digests;
- production still prohibited; no security claim.

## Unreleased — Session handoff for clean resume

Documentation only. Official digests unchanged.

- added `docs/SESSION_HANDOFF.md` capturing program state, decisions, S/ST
  evidence, anti-patterns, and exact Stage 3 resume steps.

## Unreleased — Stage 2 smoke PASS for Controller S full-hash path

Research tooling only. Official digests unchanged.

- added `scripts/stage2_smoke_s.py` (S absorb + RotHash-1-shaped finalization);
- ST1 avalanche mean 128.06/256 (256 trials); ST2 all 65536 two-byte digests
  unique; ST3 triple rate 6.72%; ST4 R0-like path collides, full S does not;
- documented in `docs/STAGE2_SMOKE_S.md`; next: Stage 3 candidate mint planning.

## Unreleased — Controller S passes G1–G3 injectivity

Offline harness only. Official digests unchanged.

- adopted external G3 advice (systematic mixed-radix injectivity channel);
- fixed S so context digits `c_i` never depend on the message symbol;
- **S: G1=PASS G2=PASS G3=PASS** (zero aliases on full two-byte domain);
- recorded consultation-when-stuck policy in `docs/EFFORT_POLICY.md` §7.

## Unreleased — Trust path roadmap

Documentation only. Official digests unchanged.

- added `docs/TRUST_PATH_ROADMAP.md`: stages 0–9 from laboratory baseline to a
  hash that can earn real-world trust (injectivity → smoke → new candidate →
  deep attack → public review → parameters → engineering → careful policy);
- linked from README, RESEARCH_GOAL, and ROADMAP.

## Unreleased — G3 failure-mode deliberation

Documentation only. Official digests unchanged.

- added `docs/CONTROLLER_G3_FAILURE_MODE.md`: why H fails G3, why H3–H5 local
  fixes redistributed collisions, and a single next hypothesis H\* (global
  amount map; freeze H axis/init; one G1–G3 run; stop on fail).

## Unreleased — H residual hunt (H3–H5)

Offline controller harness only. Official digests unchanged.

- traced H G3 residuals `58/c5` and `6e/c6`: six-phase axis+amount match with
  differing `lane2` (residue map independent of symbol);
- H3/H5 regress G2; H4 keeps G1+G2 but G3 rises to 337/5;
- **H remains lead** (G1+G2 pass, G3 161/2).

## Unreleased — Competitive program gates and structural controller H

Documentation and offline controller harness only. Official digests unchanged.

- reoriented `docs/RESEARCH_GOAL.md` to a competitive hash ambition (Keccak-class
  quality bar as aspiration, not a claim);
- added `docs/CONTROLLER_REQUIREMENTS.md` with automated gates G1–G3;
- updated `docs/SECURITY_TARGET.md` for ambition vs claim discipline;
- structural prototypes H / H2 in `scripts/controller_redesign_prototypes.py`;
- H: G1+G2 pass, G3 has 161 aliases in two pairs only (best so far);
- H2: rejected (G2/G3 regression).

## Unreleased — Science-first goal and variant G experiment

Offline redesign and goal docs only. Official digests unchanged.

- locked `docs/RESEARCH_GOAL.md`: publishable science first; acceptance contingent;
- comparative full two-byte scans: canonical 728, E 183, G 187 instances;
- G (nonzero mod-7 amount coefficients) replaces E’s residual pairs with four
  new ones — coefficient-only repair insufficient;
- paper plan gains redesign comparative section.

## Unreleased — Variant E residual catalogue (redesign science)

Offline redesign documentation only. Official digests unchanged.

- full two-byte scan under prototype E: 183 alias instances, exactly four symbol
  pairs, all identical physical six-move paths;
- residual mechanism: lane deltas that are 0 mod 7 with matching axis LSB;
- E4/E5 blind amount rewires worsened counts; further ARX thrash budget-closed;
- recorded in `docs/PHASE1_CONTROLLER_CAMPAIGN.md` §8.

## Unreleased — Value-based effort policy and provisional Gate B

Documentation and offline redesign prototypes only. Official digests and the
frozen algorithm are unchanged.

- added `docs/EFFORT_POLICY.md`: inductive budget-closes, no hiding of weaknesses,
  stop low-EV repetition of the same attack class;
- added provisional `docs/CONTROLLER_DECISION_MEMO.md`: prefer Redesign-2 for
  general-purpose acceptance; Phase-1 budget-close of foldback-vs-known-forward
  multicollision attacks (explicitly not a security proof);
- recorded decisions D-011 and D-012;
- tried redesign E2/E3; neither beat E on two-byte residual aliases — stop
  unprincipled formula thrash until residual mechanism is characterized.

## Unreleased — Phase 1 wave 2 (dual alias, multipath, redesign)

Research tooling and documentation only. Official digests and the frozen
PVC-RotHash-1 algorithm are unchanged.

- added `pvc-dual-return-alias`: no direct return-gate or after-foldback merge on
  all 1,496 three-byte forward pairs (bare and common 1-byte suffixes);
- added `pvc-multipath-foldback-sample`: 128 seeds, min after-foldback distance
  282 bits, zero exact merges in the sampled common-suffix budget;
- expanded offline redesign prototypes A–F; variant E clears one-byte-context
  aliases but still has residual two-byte-context aliases;
- recorded results in `docs/PHASE1_CONTROLLER_CAMPAIGN.md` §7.

## Unreleased — Phase 1 controller campaign kickoff

Research documentation and analysis tooling only. Official digests and the
frozen PVC-RotHash-1 algorithm are unchanged.

- started Phase 1 controller decision campaign
  (`docs/PHASE1_CONTROLLER_CAMPAIGN.md`);
- derived phase-0 necessary conditions for one-symbol aliases (`d ≡ 0 (mod 14)`);
- traced the known Δ=42 alias through six moves and corrected the multi-phase
  modular model (integer byte sums mod 7, not raw u8 deltas alone);
- confirmed with alignment probes that among multiples of 14 only Δ=42 yields
  exact one-byte-context six-move aliases (three pairs);
- added independent `scripts/controller_alias_analysis.py` (Python/spec-based).

## 1.0.0-rc1 packaging follow-up — Acceptance path (post-tag tooling)

Documentation and repository packaging only. Official digests and the frozen
PVC-RotHash-1 algorithm are unchanged.

- added `docs/ACCEPTANCE_ROADMAP.md` with honesty-bound gates toward long-horizon
  general-purpose hash credibility (not a security claim);
- locked maintainer strategy: general-purpose ambition, parallel attack and
  controller-redesign prototypes, 256-bit output for now, Gate A first;
- added GitHub Actions CI (GCC, Clang, ASan/UBSan, Windows MSVC, Python compile);
- added cryptanalysis and conformance issue templates;
- added public-review issue draft and short post-quantum non-claim notes;
- recorded a clean `scripts/release_check.sh` pass for the frozen vectors.

## 1.0.0-rc1 — Frozen public cryptanalysis candidate

- froze the PVC-RotHash-1 algorithm without changing canonical digests;
- added the normative `SPECIFICATION.md` with exact arithmetic, state,
  constants, phases, and conformance requirements;
- added an independent pure-Python implementation using no third-party package;
- added 32 official digest vectors and five full phase-state vectors;
- added cross-implementation vector verification and CTest/CI conformance;
- added the public cryptanalysis challenge, independent-review guide, security
  target, candidate-freeze policy, consolidated cryptanalysis summary, and
  reproducibility guide;
- added issue templates for cryptanalysis and conformance reports;
- added citation, authorship, AI-assistance, and review-credit disclosures;
- added a deterministic release-check script and public-review checklist;
- retained the complete historical attack tooling and documented weaknesses;
- continued to prohibit production use and made no collision/preimage claim.

## 0.8.0 — Digest-surface cryptanalysis

- Kept the canonical PVC-RotHash-1 algorithm and known-answer vectors unchanged.
- Added `pvc-digest-beam-search`, which optimizes final digest distance inside a
  bridged same-forward multicollision family.
- Added `pvc-divergent-digest-beam`, which rejects forward-state convergence and
  directly searches final-digest distance across independently extended messages.
- Added `pvc-digest-lsh-search` for projection-LSH analysis across large,
  forward-divergent suffix families.
- Added `pvc-barrier-correlation` to measure whether forward, foldback, closure,
  orbit, or final-state Hamming distances predict final digest distance.
- Added generic 256-bit binomial minimum-distance references to distinguish a
  real structural advantage from ordinary multiple-comparison effects.
- Added regression tests for a known same-forward digest pair and a known
  forward-divergent digest pair.
- In a 42,325-pair same-forward beam campaign, observed a 96-bit minimum against
  a generic 96-bit reference; no exact digest collision.
- In a 229,633-pair forward-divergent beam campaign, observed a 94-bit minimum
  against a generic 93-bit reference; no exact digest collision.
- Across four 67,108,864-pair logical LSH domains, observed minima of 83–85 bits
  against a generic 84-bit reference; no exact collision and no forward-state
  convergence in the selected pairs.
- Across three 10,000-pair barrier campaigns, measured phase-to-digest Pearson
  correlations between approximately -0.02 and +0.02.
- Documented results and limitations in `docs/DIGEST_SURFACE_RESULTS.md`.

## 0.7.0 — Structural security campaign

- kept the canonical hash algorithm and known-answer vectors unchanged;
- added a foldback-distance beam search over bridged forward-multicollision
  families;
- reduced a selected family's after-foldback distance from 812 to 224 bits,
  without an exact merge or monotonic convergence;
- added a projection-LSH search for independently selected suffixes;
- covered 67,108,864 logical suffix pairs for an inherited collision and found
  a 180-bit nearest after-foldback state, with zero exact merge;
- repeated the bounded search for a local third-symbol alias and found a
  578-bit nearest state;
- mapped 2,304 reachable reverse contexts and found controller aliases in 24
  contexts, all with differences 42, 126, or 196;
- added a multi-trial truncated-collision campaign; 24- and 32-bit results
  remained compatible with generic birthday scaling;
- formalized symbol-index framing across unequal lengths and showed why classic
  identical-operational-state expandable messages cannot cross lengths;
- added regression coverage and CI smoke jobs for the new tools.

## 0.6.0 — Foldback separation anatomy and bridged multicollisions

- kept the canonical hash algorithm and known-answer vectors unchanged;
- formalized that the position-dependent return-symbol map is an XOR bijection
  and preserves byte XOR differences;
- profiled all 1,496 known three-byte forward collisions step by step through
  foldback;
- verified that every pair diverges exactly when the last differing original
  byte is encountered in reverse, with zero delayed divergences, direct return
  aliases, or later reconvergences;
- measured separation-gate and final foldback cube distances and state-component
  divergence;
- exhausted independent one-byte suffixes on both sides for all 1,496 pairs,
  covering 98,041,856 logical cross pairs with zero after-foldback merge;
- extended multicollision construction with one-byte common bridges;
- found a 32-level forward alias path representing a theoretical 2^32-message
  forward multicollision family;
- fully materialized a 16-level, 65,536-message family and verified one common
  forward state, 65,536 distinct after-foldback states, and 65,536 distinct
  full digests;
- added regression coverage for an eight-way bridged multicollision and the
  return-symbol XOR-difference invariant;
- corrected the earlier bounded-search statement: no third immediate alias was
  found, but bridged alias chains extend far beyond two levels.

## 0.5.0 — Foldback-aware aliases and forward multicollisions

- kept the canonical hash algorithm and known-answer vectors unchanged;
- exposed research-only return-symbol derivation and explicit-state foldback;
- added a complete catalogue generator for the 1,496 known three-byte forward
  collision pairs;
- added a foldback-aware alias attack coupling forward aliases to their return
  symbols;
- tested all 1,496 pairs directly and with every common one-byte suffix, finding
  zero return-transition or after-foldback merge;
- tested 6,127,616 structured common two-byte extension cases with zero exact
  merge;
- discovered 15 chainable second-level controller aliases and constructed exact
  four-message forward multicollisions;
- verified that all four branches remain distinct after foldback and in the full
  digest;
- exhausted all common one-byte extensions and sampled 4,096 two-byte extensions
  for all 15 four-way families without an after-foldback or digest collision;
- added regression coverage for the explicit foldback API and a known four-way
  forward multicollision;
- documented the increasing concentration of security in the reverse foldback
  pass.

## 0.4.0 — Three-byte structural and constrained-merge analysis

- kept the canonical hash algorithm and known-answer vectors unchanged;
- added an exhaustive multithreaded three-byte state scanner with exact
  candidate verification;
- enumerated all 16,777,216 three-byte forward states and found 1,496 exact
  forward-state collision pairs;
- classified 768 pairs as inherited common-suffix extensions and 728 as new
  context-dependent third-symbol aliases;
- verified that all 728 new aliases generate identical six-move physical paths;
- identified third-symbol differences 42, 126, and 196;
- exhausted the complete three-byte after-foldback domain with zero exact state
  collisions;
- added a controller-alignment probe and independently reproduced the three
  canonical delta-42 aliases;
- added an independent-suffix constrained meet-in-the-middle search;
- covered 2^32 suffix combinations for each of the three known forward-collision
  prefixes with zero after-foldback merges;
- added a foldback-only research API and new regression tests;
- documented the finite-domain results and remaining concentration of security
  in the foldback stage.

## 0.3.0 — PVC-RotHash-1 cryptanalysis framework

- kept the canonical hash algorithm and known-answer vectors unchanged;
- added runtime research parameters and six reduced-round presets;
- added exact operational snapshots and phase checkpoints;
- added reduced-round, transition-collision, phase-collision, truncated-
  collision, differential, predecessor, and related-input tools;
- documented the canonical two-symbol forward convergence `17 6f` / `17 99`;
- verified that foldback separates all three forward merges in the exhaustive
  canonical two-byte domain;
- documented full-state and digest collapse in reduced presets R0–R2;
- added a foldback merge-extension search and exhausted all common two-byte
  suffixes for the three canonical forward-collision pairs;
- added regression tests preserving both the canonical KATs and the known
  forward-convergence/foldback-separation result;
- expanded CI with research smoke tests.

## 0.2.0 — PVC-RotHash-1

- replaced direct concatenation of one cube's four body diagonals;
- added an original 32-state four-diagonal squeeze;
- made every output byte depend on all four body diagonals;
- inserted four diagonal-rooted rotation symbols between output bytes;
- expanded self-fed diagonal closure from 32 to 64 symbols;
- added a 128-symbol full-cube orbit closure sampling all 512 cells;
- added exhaustive output-distribution analysis;
- added final-state positional-bias analysis;
- added a 32,000-trial avalanche sweep;
- documented version-0 multiplicity and canonical-coordinate distinguishers;
- added regression assertions for empty input and `abc`.

## 0.1.0 — PVC-RotHash-0

- canonical Perfect Value Cube;
- reversible axis-line rotations;
- state-dependent intersecting move chain;
- six moves per absorbed symbol;
- reverse message foldback traversal;
- geometry-derived length closure;
- self-fed 32-symbol closure;
- direct 32-byte output from one final cube's four body diagonals;
- CLI, tests, avalanche probe, collision probe, and structure probe.
