# Attack Log

This file records attacks and design failures, including failures found before a
version is tagged.

## A-001 — Two-byte collision in the first forward-only prototype

**Status:** fixed in the current experimental branch; retained as design history.

The initial implementation absorbed the message once in forward order and then
entered length/self-fed closure.

Exhaustive two-byte search found:

```text
M1 = 07 0e
M2 = 07 4c

H(M1) = H(M2)
      = 40ef978c7c43ab0050a607ce52b971a6
        b4e19886c379d69944b2f8de2cff0b3b
```

The second-byte move fragments were different, but both fragments converged to
the same cube, cursor, and previous axis before closure. All later moves were
therefore identical.

This directly demonstrated:

```text
different intersecting paths can converge to the same operational state
```

It disproved the assumption that intersection alone makes the path unique.

### Revision

A reverse foldback traversal was added. After the forward message pass, the
message is traversed from end to start using return symbols bound to:

- the original byte;
- its original position;
- canonical body-diagonal values.

After this revision, exhaustive search found no collision among all 65,536
two-byte messages.

This does not prove collision resistance. It only removes the specific
forward-convergence failure in that tested domain.

## Reporting format

Future entries should contain:

- exact version/commit;
- attack class;
- exact reproducer;
- measured complexity;
- effect on security claims;
- whether the issue is fixed, mitigated, or open.
