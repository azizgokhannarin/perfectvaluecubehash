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

## Version 0.4.0 three-byte structural results

The canonical algorithm and all previous known-answer vectors remain unchanged.
The complete three-byte forward-state domain contained 1,496 exact collision
pairs: 768 inherited common-suffix extensions of the three known two-byte
pairs, and 728 new context-dependent third-symbol aliases.

All 728 new aliases used an identical physical six-move trace. Their absolute
symbol differences were 42, 126, or 196. No pair survived foldback.

A separate exhaustive enumeration of all 16,777,216 three-byte after-foldback
states found zero exact collision. Independent two-byte suffix MITM searches
for each known forward-collision prefix pair covered 2^32 cross combinations
and also found zero after-foldback merge.

Detailed classifications, commands, and limitations are in
`THREE_BYTE_RESULTS.md`.


## Version 0.5.0 foldback-aware and multicollision results

The canonical algorithm and known-answer vectors remain unchanged. Every one of
the 1,496 known three-byte forward collision pairs was tested against the
position-dependent return-symbol transition. No direct dual alias was found.
All common one-byte suffixes produced 382,976 structured extension cases with
zero after-foldback merge. The first 4,096 common two-byte suffixes for every
pair produced 6,127,616 additional cases with zero exact merge.

The minimum cube bit distance immediately after the two differing return
symbols decreased as controlled suffix length increased:

```text
no suffix             = 172 bits
one common byte        = 114 bits
4,096 two-byte samples =  78 bits
```

This does not constitute a collision, but it identifies distance-guided
foldback optimization as a higher-priority attack.

A separate multicollision probe found that 15 of the 1,496 common forward states
contain another symbol alias. Each such relation produces four distinct
messages with one complete forward state. One example is:

```text
176f115b
176f1185
1799115b
17991185
```

All four after-foldback states and full digests are distinct. Exhaustive common
one-byte suffix extension and a 4,096-value two-byte suffix sample for all 15
families found zero after-foldback or digest collision.

Detailed commands, counts, and limitations are in
`FOLDBACK_AWARE_RESULTS.md`.


## Version 0.6.0 foldback separation and bridged multicollision results

The complete 1,496-pair three-byte forward collision catalogue was followed
through foldback one return symbol at a time. Every pair diverged exactly at
the first reverse step processing its last differing original byte. No direct
return alias, delayed divergence, later reconvergence, or final foldback merge
was observed. The initial separation gate changed 49–78 cube cells.

Independent one-byte suffixes were selected on both sides for every pair,
covering 98,041,856 logical cross combinations with zero after-foldback merge.

A common bridge byte between alias levels made the forward multicollision
mechanism scalable. A 32-level path representing a theoretical 2^32-message
forward family was found. The first 16 levels were fully materialized as 65,536
33-byte messages. All reached one exact forward state, while all 65,536
after-foldback states and full digests were distinct.

The result strengthens two conclusions simultaneously: the forward pass is not
a collision-resistance boundary, and the tested construction currently relies
on foldback to separate complete message histories.

## Version 0.7.0 structural security campaign

The canonical algorithm and known-answer vectors remain unchanged.

A distance-guided beam search over one 16-level bridged forward-multicollision
path reduced the after-foldback operational-state distance from 812 bits at the
seed to a global minimum of 224 bits at level 11. The distance later increased;
no exact merge or monotonic convergence was observed.

A projection-LSH search over 8,192 independent two-byte suffixes on each side of
the inherited `176f00` / `179900` forward collision covered 67,108,864 logical
cross pairs and evaluated 13,040,013 projected candidates. The nearest state
was 180 bits apart over the complete operational snapshot. Repeating the same
bounded campaign for local alias `af671b` / `af67df` reached 578 bits. Neither
search produced an exact after-foldback or digest collision.

The move controller was enumerated in 2,304 reachable reverse contexts. Twenty-
four contexts (1.041667%) contained an alias, with at most one pair per context.
All observed differences were 42, 126, or 196. This demonstrates a sparse but
non-empty return alias surface.

Truncated first-collision campaigns measured mean/expected ratios of 1.0520 for
24 bits over eight trials and 1.1275 for 32 bits over four trials. The 40- and
48-bit 100,000-message runs were censored far below their birthday expectations.

Finally, `symbol_index` equals `n` after forward absorption, `2n` after foldback,
and `2n + 330` after canonical finalization. This prevents exact complete-state
equality at equal phases across unequal message lengths, but it does not prove
unequal-length digest collision resistance.

Detailed methods, commands, and limitations are in
`STRUCTURAL_SECURITY_CAMPAIGN.md`.


## v0.8.0 digest-surface campaign

The direct same-forward digest beam evaluated 42,325 candidate pairs and found a
96-bit minimum, equal to the generic 256-bit reference for that candidate count.
The forward-divergent beam evaluated 229,633 pairs and found 94 bits against a
93-bit generic reference.

Four digest-LSH domains each represented 67,108,864 logical cross pairs. Their
best distances were 85, 83, 85, and 83 bits, compared with a generic reference
of 84 bits. No exact digest collision was found and selected pairs remained
forward-divergent.

In 10,000-pair phase-correlation campaigns, final digest distance remained
centered near 128 bits. Pearson correlation between internal-state distance and
digest distance remained between approximately -0.02 and +0.02 at every tested
phase. See `DIGEST_SURFACE_RESULTS.md` for full commands and tables.
