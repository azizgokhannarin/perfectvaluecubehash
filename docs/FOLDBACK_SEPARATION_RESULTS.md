# Foldback Separation Results

Environment: GCC 14.2.0, Release build, warnings as errors.

Version 0.6.0 does not change the canonical `PVC-RotHash-1` algorithm or its
known-answer vectors. It profiles exactly how the reverse foldback separates
the complete catalogue of 1,496 known three-byte forward-state collisions.

## Structural return-symbol property

For original byte `b` at index `i`, foldback uses:

```text
r_i(b) = b XOR K_i
```

where `K_i` is derived from the canonical diagonals and the original index.
For a fixed index this is a bijection. Therefore:

```text
r_i(a) XOR r_i(b) = a XOR b
```

The foldback framing never removes a byte difference by itself. It presents the
same XOR difference to the move controller in reverse order. Collision
separation then depends on whether the two return symbols alias in the current
operational state.

## Complete three-byte catalogue profile

```text
forward collision pairs                     = 1,496
return XOR differences preserved             = 1,496
first divergence at expected reverse step    = 1,496
delayed divergence                           = 0
never diverged                               = 0
direct return-transition aliases             = 0
later reconvergences                         = 0
exact after-foldback merges                  = 0
```

The 728 local third-byte aliases diverge on the first reverse symbol. The 768
inherited collisions have an equal common third byte, remain equal for the
first return symbol, and diverge on the second reverse symbol when the original
second-byte difference is encountered.

```text
reverse step 1 divergences = 728
reverse step 2 divergences = 768
```

## Distance at the separation gate

Immediately after the first differing return transition:

```text
cube bit distance min / mean / max  = 172 / 315.0187 / 390
cube byte distance min / mean / max = 49 / 73.1130 / 78
cursor differs                      = 1,495 / 1,496
previous axis differs               = 1,175 / 1,496
```

One pair retains the same cursor at the gate, so cursor divergence is not the
sole separation mechanism. The cube already differs in at least 49 cells for
every pair.

## Distance after complete foldback

```text
cube bit distance min / mean / max  = 310 / 718.5909 / 1,034
cube byte distance min / mean / max = 67 / 166.9579 / 225
cursor differs                      = 1,492 / 1,496
previous axis differs               = 1,000 / 1,496
```

No pair reconverged after its first divergence.

## Interpretation

The empirical separation mechanism is now explicit:

1. forward-colliding messages enter foldback in one common operational state;
2. equal trailing bytes produce equal return symbols and preserve equality;
3. the last differing original byte produces a different return symbol with the
   same XOR difference as the original bytes;
4. none of the 1,496 differing return-symbol pairs aliases in that common state;
5. one return transition creates a 49–78 cell cube difference;
6. the remaining reverse traversal amplifies rather than cancels it.

This is stronger than merely observing different final digests, but remains a
finite-domain result. A state and byte pair that aliases in both the forward and
return contexts would bypass the measured separation gate.
