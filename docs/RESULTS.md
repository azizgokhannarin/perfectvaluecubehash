# Results for PVC-RotHash-0 0.1.0

Environment:

```text
Compiler: GCC 14.2.0
Language: C++20
Builds: Release and ASan/UBSan Debug
```

## Build and unit tests

```text
Release build: passed
Warnings as errors: passed
CTest: 1/1 passed
ASan/UBSan build: passed
ASan/UBSan CTest: 1/1 passed
```

## Known-answer samples

```text
H("") =
490f069052e5ac8bababdeeff7c47e4b
482da201fc7743a9db88c3328cd9da33

H("abc") =
ef1f164375e49a44956adb8714ad35ff
febd0631abc1b583660497a7c43a8f99
```

These values are implementation regression vectors, not standard test vectors.

## Chain-structure probe

Input:

```text
Perfect Value Cube
```

Result:

```text
moves                         : 468
consecutive axes differ       : yes
each line meets previous line : yes
0..255 each still occur twice : yes
all 192 sums still 1020       : no
```

The final line sums are expected to lose the initial balance because individual
line rotations move values across orthogonal lines.

## Avalanche probe

Input:

```text
Perfect Value Cube
```

Every input bit was flipped independently.

```text
trials        : 144
bit distance  : mean=128.26 min=106 max=149 of 256
byte distance : mean=31.87 min=31 max=32 of 32
```

This is encouraging statistical behavior for this sample, but it does not
establish cryptographic security.

## Exhaustive collision probes

```text
All 256 one-byte inputs:
no collision found

All 65,536 two-byte inputs:
no collision found
```

The forward-only predecessor of this design did have an immediate two-byte
collision. See `ATTACK_LOG.md`.

## Current interpretation

The current prototype passes its first implementation and small-domain tests.
Its major unresolved risks include:

- collisions caused by projecting 512 bytes to 32 diagonal bytes;
- algebraic relations between rotation chains;
- preserved full-state histogram;
- symmetry attacks inherited from the canonical cube;
- meet-in-the-middle attacks on reversible move sequences;
- distinguishers caused by the constrained output distribution.
