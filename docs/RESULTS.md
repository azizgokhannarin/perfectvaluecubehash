# Results for PVC-RotHash-1

Environment:

```text
Compiler: GCC 14.2.0
Language: C++20
Primary build: Release, warnings as errors
Additional builds: Clang, ASan/UBSan Debug
```

## Regression vectors

```text
H("") =
7f01eb3ce13131ef290f8428ed725b84
9f875e49ad6c646cc9f4f1b1a1e5734b

H("abc") =
f32b2241a950d7e7b2b006ff8ae2d0b0
8f02db23c0d8fde198dfdf9e9642051f

H("Perfect Value Cube") =
ea3cf546291874656dd454e2481fc530
fe1c4b2783ad101cf7af4a5521aa3775
```

These are implementation regression vectors, not standardized cryptographic
test vectors.

## Small-domain collision probes

```text
All 256 one-byte inputs: no collision found
All 65,536 two-byte inputs: no collision found
```

## Avalanche

Single-message, all 144 input bits:

```text
mean=128.10 min=107 max=152 of 256
```

32,000 deterministic 16-byte messages, one flipped bit per message:

```text
mean=127.9508
standard deviation=8.0574
minimum=94
maximum=159
```

The binomial reference for 256 independent balanced bits has center 128 and
standard deviation 8. This agreement is a statistical observation only.

## Exhaustive two-byte output distribution

```text
samples                         = 65,536
mean per-position chi-square    = 255.4556
minimum / maximum chi-square    = 192.2656 / 292.6719
support at every position       = 256 of 256
largest value / expected count  = 1.2656
maximum output-bit |z|          = 3.1172
equal byte pairs per digest     = 1.9356
complement pairs per digest     = 1.9357
digests with a triple byte      = 6.7764%
```

The random-reference expectation for both equal and complementary byte pairs is
1.9375 per 32-byte string.

## Randomized test-data distribution

100,000 deterministic test messages of 16 bytes:

```text
mean per-position chi-square    = 254.5854
minimum / maximum chi-square    = 205.4810 / 301.8291
support at every position       = 256 of 256
largest value / expected count  = 1.1981
maximum output-bit |z|          = 2.7891
equal byte pairs per digest     = 1.9314
complement pairs per digest     = 1.9303
digests with a triple byte      = 6.8000%
```

100,000 deterministic test messages of 64 bytes:

```text
mean per-position chi-square    = 253.9971
minimum / maximum chi-square    = 206.8378 / 333.8342
support at every position       = 256 of 256
largest value / expected count  = 1.2032
maximum output-bit |z|          = 3.3014
equal byte pairs per digest     = 1.9396
complement pairs per digest     = 1.9406
digests with a triple byte      = 6.6110%
```

## Final-state positional bias

50,000 deterministic 16-byte messages, all 512 final-cube positions:

```text
mean per-cell chi-square             = 255.3289
minimum / maximum chi-square         = 198.3846 / 331.8835
support at every cell                = 256 of 256
largest value / expected count       = 1.3158
canonical/complement preferred value = 6 of 512 cells
```

50,000 deterministic 64-byte messages, all 512 final-cube positions:

```text
mean per-cell chi-square             = 253.2930
minimum / maximum chi-square         = 189.0560 / 317.7626
support at every cell                = 256 of 256
largest value / expected count       = 1.3670
canonical/complement preferred value = 4 of 512 cells
```

The version-0 report found the canonical value or complement preferred in
nearly every observed output position. That simple positional-memory signature
was not reproduced against version 1 in this sample.

## Interpretation

Version 1 removes the two known version-0 distinguishers in the measured test
domains while retaining strong avalanche behavior. It remains vulnerable in
principle to untested algebraic, higher-order statistical, symmetry,
meet-in-the-middle, and multicollision attacks. No security level is claimed.


## Version 0.3.0 structural results

The canonical algorithm and statistical results from version 0.2.0 are
unchanged. New reduced-round and exact-state experiments found that the
canonical forward path is not injective over two symbols.

```text
17 6f == 17 99
25 1c == 25 46
a2 6f == a2 99
(after forward absorption only)
```

Exhaustive canonical two-byte phase enumeration found three forward-state
merges, then zero collisions after foldback, diagonal closure, orbit closure,
final state, or digest. Testing every common two-byte suffix for each of the
three pairs also produced no after-foldback collision.

Reduced presets R0, R1, and R2 produced full two-byte collisions. R3, R4, and
R5 had forward merges but no after-foldback or final collision in that domain.

Initial 24-bit truncated testing over 20 deterministic canonical domains had a
mean first-collision ratio of `0.9140` relative to the generic birthday
expectation, with a broad range from `0.1900` to `2.0700`. This sample does not
establish a sub-birthday attack.

Detailed tables and commands are in `REDUCED_ROUND_RESULTS.md`.
