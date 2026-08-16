# Suggested pinned issue: Independent public review of PVC-RotHash-2 0.2.0-draft

**Published:** https://github.com/azizgokhannarin/perfectvaluecubehash/issues/1  
(use this text as the historical draft; live issue may have absolute links)

---

## Title

Independent public review of PVC-RotHash-2 candidate 0.2.0-draft

## Body

PVC-RotHash-2 **0.2.0-draft** is frozen as a **draft public-attack candidate**.

**This is not a production hash.** There is no claim of collision resistance,
preimage resistance, or post-quantum security. The draft freeze exists so
outsiders can attack a stable target.

### Relation to RotHash-1

- **PVC-RotHash-1** `1.0.0-rc1` stays the frozen historical baseline (known
  forward multicollisions; do not use for security).
- **PVC-RotHash-2** changes the absorb controller (systematic **S**) and keeps
  the same finalization shape. It is the active competitive-path candidate.

### Start here

1. [`SPECIFICATION_ROTHASH2.md`](../SPECIFICATION_ROTHASH2.md) — normative absorb
2. [`CRYPTANALYSIS_CHALLENGE_ROTHASH2.md`](../CRYPTANALYSIS_CHALLENGE_ROTHASH2.md) — targets R2-C1…R2-C9
3. [`docs/KNOWN_CRYPTOANALYSIS.md`](KNOWN_CRYPTOANALYSIS.md) — R1 + R2 findings
4. [`docs/STAGE4_DEEP_R2.md`](STAGE4_DEEP_R2.md) — operational Stage 4
5. [`docs/STAGE4_DIGEST_R2.md`](STAGE4_DIGEST_R2.md) — digest-surface Stage 4
6. [`docs/SECURITY_TARGET.md`](SECURITY_TARGET.md) — targets and non-claims
7. [`docs/INDEPENDENT_REVIEW.md`](INDEPENDENT_REVIEW.md) — reviewer workflow

### Already known in-house (budget-limited; not a proof)

- Controller S: G1∧G2∧G3 injectivity gates pass
- Exhaustive 2-byte digests unique; full 2^24 three-byte forward/foldback
  operational pairs = 0
- Multicollision / bridged tools: no seed pairs
- Digest-surface minima near generic binomial references; no exact 256-bit collision
- Truncated 24/32/40-bit costs near birthday in small trial sets

A method that finds an operational merge, a sub-generic truncated attack, or a
full digest collision **is** high value.

### How to report

- Cryptanalysis: use the **Cryptanalysis finding** issue template
- Spec/impl mismatch: use the **Conformance report** issue template
- Practical full-candidate break: private contact before publication is welcome

### Credit

Reproducible findings are recorded in `docs/ATTACK_LOG.md` with preferred
attribution, including strong negative results.

### Repository

https://github.com/azizgokhannarin/perfectvaluecubehash

Tag: `v0.2.0-rothash2-draft`
