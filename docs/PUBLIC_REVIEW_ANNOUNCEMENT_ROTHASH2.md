# Suggested Public Announcement — PVC-RotHash-2 0.2.0-draft

PVC-RotHash-2 **0.2.0-draft** is available for independent cryptanalysis.

It is a 256-bit experimental hash in the Perfect Value Cube / intersecting
rotation family. Absorb uses systematic Controller **S** (mixed-radix
injectivity channel). Foldback, closures, and four-diagonal squeeze keep the
same shape as PVC-RotHash-1.

**PVC-RotHash-1 remains frozen as a historical baseline** with known forward
multicollisions. RotHash-2 is the **successor candidate** on the competitive
research path — still experimental, still not for production.

The repository includes:

- normative absorb draft (`SPECIFICATION_ROTHASH2.md`);
- C++ `RotHash2` and independent pure-Python reference;
- official digest vectors (`test-vectors/official-v2.json`);
- dual-implementation CTest verification;
- in-house Stage 4 operational and digest-surface campaigns (budget-limited);
- explicit public challenge targets (`CRYPTANALYSIS_CHALLENGE_ROTHASH2.md`).

**Important:** this is not a production hash and no collision, preimage, or
post-quantum security claim is made. In-house tools found no structural break in
stated budgets; that is **not** a security proof. The draft freeze exists so
external researchers can attack a stable target.

Repository:
https://github.com/azizgokhannarin/perfectvaluecubehash

Suggested tag:
`v0.2.0-rothash2-draft`

Start here:

1. `CRYPTANALYSIS_CHALLENGE_ROTHASH2.md`
2. `SPECIFICATION_ROTHASH2.md`
3. `docs/STAGE4_DEEP_R2.md` and `docs/STAGE4_DIGEST_R2.md`
4. `docs/INDEPENDENT_REVIEW.md` (workflow; targets are R2-C1…)
