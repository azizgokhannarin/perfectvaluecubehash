# Suggested pinned issue: Independent public review of PVC-RotHash-1 1.0.0-rc1

Use this text when opening a pinned GitHub issue or discussion.

---

## Title

Independent public review of PVC-RotHash-1 candidate 1.0.0-rc1

## Body

PVC-RotHash-1 **1.0.0-rc1** is frozen for independent cryptanalysis.

**This is not a production hash.** There is no claim of collision resistance,
preimage resistance, or post-quantum security. The freeze exists so outsiders can
attack a stable target.

### Start here

1. [`SPECIFICATION.md`](../SPECIFICATION.md) — normative algorithm
2. [`docs/INDEPENDENT_REVIEW.md`](INDEPENDENT_REVIEW.md) — reviewer workflow
3. [`CRYPTANALYSIS_CHALLENGE.md`](../CRYPTANALYSIS_CHALLENGE.md) — targets C1–C8
4. [`docs/KNOWN_CRYPTOANALYSIS.md`](KNOWN_CRYPTOANALYSIS.md) — what is already known
5. [`docs/SECURITY_TARGET.md`](SECURITY_TARGET.md) — targets and non-claims
6. [`docs/ACCEPTANCE_ROADMAP.md`](ACCEPTANCE_ROADMAP.md) — long-horizon honesty gates

### Already known (do not re-report as full breaks)

- Forward-state collisions and large bridged forward multicollisions
- Controller aliases with differences `42`, `126`, and `196`
- Reduced-round collisions on presets R0–R2 in small domains
- Near-state / near-digest distances consistent with generic references

A method that turns any of the above into an after-foldback merge or a full
digest collision **is** high value.

### How to report

- Cryptanalysis: use the **Cryptanalysis finding** issue template
- Spec/impl mismatch: use the **Conformance report** issue template
- Practical full-candidate break: private contact before publication is welcome

### Credit

Reproducible findings are recorded in `docs/ATTACK_LOG.md` with preferred
attribution, including strong negative results.

### Repository

https://github.com/azizgokhannarin/perfectvaluecubehash

Tag: `v1.0.0-rc1`
