# Effort Policy — Value-Based Cryptanalysis

**Status:** Binding project working rule  
**Adopted:** 2026-08-01  
**Does not weaken:** honesty, disclosure, freeze discipline, or falsification

## Purpose

The goal is **real** progress toward a credible hash candidate — not infinite
self-attack theater, and not marketing. Time and compute are finite. This
policy decides when to stop a line of work, when to generalize, and what counts
as the next high-value step.

This is **not** a license to hide weaknesses. Known breaks stay in the attack
log. Closed attack *classes* stay documented with their exact domain and cost.

---

## 1. Three kinds of result

| Kind | Meaning | How we treat it |
|---|---|---|
| **Break** | Sub-generic or structural full-phase / digest failure | Record, stop over-claiming, redesign or negative path |
| **Open with budget** | Attack class not finished; more EV remains | Continue only if next step is qualitatively new or expands domain by a clear factor |
| **Budget-closed (inductive)** | Serious, documented effort found no break; mechanism understood enough to predict more of the *same* work will not change the conclusion | Stop grinding; re-open only with a new method or new target phase |

Budget-closed is a **research management** judgment, not a security proof.

---

## 2. When further difficulty is low value

Do **not** automatically escalate the same attack by one more byte, one more
beam width, or one more random seed if all of the following hold:

1. The mechanism of success/failure is already characterized (e.g. first
   differing return step separates; dual return alias absent in full catalogue).
2. Multiple independent finite domains already agree (exhaustive small domain +
   structured extensions + multipath sample).
3. Distance-guided search shows no stable trajectory toward zero.
4. The next experiment is the same idea with only larger constants.

In that case, write an **inductive close** paragraph: domain, cost, mechanism,
what would re-open the class. Then spend effort elsewhere.

---

## 3. What re-opens a closed class

Any of:

- a new *method* (SAT/SMT, different differential object, reverse-only dual
  construction, algebraic invariant);
- a new *target phase* (digest surface, squeeze-only, length inequality);
- a concrete counterexample to the separation mechanism;
- an external report that contradicts the close.

“Run the same tool with 4× budget” alone does not re-open.

---

## 4. Value ranking for this project (acceptance goal)

Highest value first:

1. **Remove or fully bound structural blockers** that experts will reject on
   sight (today: cheap forward multicollisions / controller aliases).
2. **Qualitatively new cryptanalysis** (new model, new phase, solver).
3. **External exposure** (spec freeze package, review invitation) so others
   attack with independent eyes.
4. **Engineering polish** only after 1–3 trend positive.
5. **Repeated near-miss distance shaving** on already-separated foldback pairs —
   lowest priority once budget-closed.

Hiding a blocker by only measuring how far foldback keeps states apart is
**not** progress toward general-purpose acceptance.

---

## 5. Inductive wording template

Use this form in campaign logs (never “therefore secure”):

```text
Attack class: ...
Domains and cost: ...
Mechanism: ...
Result: no break in these domains.
Inductive close: further same-method extension is low expected value because ...
Re-open if: ...
This is not a security proof.
```

---

## 6. Relation to security claims

- Budget-closed ≠ claimed resistance.
- `SECURITY.md` production ban remains until an explicit later policy change.
- Papers and READMEs must keep the distinction between *effort spent* and
  *proven security*.

---

## 7. External consultation (when stuck)

If progress stalls on a **structural** question (not a missing constant tweak),
**ask an outside specialist or peer** rather than chaining low-EV local patches.

Good triggers:

- the same gate fails after 2–3 reasoned experiments in one class;
- statistics or theory suggest the approach cannot pass (e.g. random-like G3);
- the team is optimizing a hit-list instead of a global invariant.

The 2026-08-02 G3 consultation (`docs/EXTERNAL_ADVICE_G3.md`) is the template:
clear gates, what failed, design constraints, concrete questions. Outside advice
does not replace measurement; it chooses the **next class** of design.
