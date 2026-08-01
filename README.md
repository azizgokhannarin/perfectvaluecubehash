# Perfect Value Cube Hash

`PVC-RotHash-1 1.0.0-rc1` is a **frozen, falsification-oriented hash candidate
published for independent cryptanalysis**. The algorithm is unchanged from the
analyzed PVC-RotHash-1 line; this release adds a normative specification, an
independent pure-Python implementation, official digest and phase vectors, and
a public-review package.

It is not a production hash and makes no security claim.

## Public-review entry points

- [`SPECIFICATION.md`](SPECIFICATION.md) — frozen normative candidate.
- [`CRYPTANALYSIS_CHALLENGE.md`](CRYPTANALYSIS_CHALLENGE.md) — high-value attack targets.
- [`docs/INDEPENDENT_REVIEW.md`](docs/INDEPENDENT_REVIEW.md) — reviewer workflow.
- [`docs/KNOWN_CRYPTOANALYSIS.md`](docs/KNOWN_CRYPTOANALYSIS.md) — consolidated findings.
- [`docs/SECURITY_TARGET.md`](docs/SECURITY_TARGET.md) — targets and explicit non-claims.
- [`docs/RESEARCH_GOAL.md`](docs/RESEARCH_GOAL.md) — competitive hash ambition; RotHash-1 is baseline, not the product.
- [`docs/TRUST_PATH_ROADMAP.md`](docs/TRUST_PATH_ROADMAP.md) — stage-by-stage path to a hash people can rationally trust.
- [`docs/CONTROLLER_REQUIREMENTS.md`](docs/CONTROLLER_REQUIREMENTS.md) — hard injectivity gates G1–G3 for any successor.
- [`docs/ACCEPTANCE_ROADMAP.md`](docs/ACCEPTANCE_ROADMAP.md) — public-review packaging and long-horizon gates.
- [`test-vectors/`](test-vectors/) — official digest and phase vectors.
- [`reference/python/`](reference/python/) — independent standard-library reference.

The candidate is frozen under [`docs/SPEC_FREEZE.md`](docs/SPEC_FREEZE.md).
Algorithm changes require a new candidate identifier; analysis and portability
improvements may continue without changing the official vectors.

The project studies whether the canonical Perfect Value Cube, a state-dependent
chain of intersecting line rotations, and an original four-diagonal squeeze can
produce a useful 256-bit hash construction without embedding an established
cryptographic primitive.

## Design boundary

The candidate algorithm contains no SHA, Keccak, AES, ChaCha, BLAKE, imported
S-box, KDF, MAC, checksum, or external pseudorandom generator.

Its algorithmic ingredients are:

1. the canonical 8×8×8 Perfect Value Cube;
2. cyclic rotations of eight-cell X, Y, and Z lines;
3. a chain where every new line intersects the previous line;
4. message-, position-, cursor-, and state-dependent move selection;
5. a forward message pass and reverse foldback pass;
6. a self-fed diagonal closure;
7. a 128-symbol full-cube orbit closure;
8. a 32-step squeeze driven only by the four body diagonals.

Generic arithmetic and bit operations are part of this original construction.
Standard library random generators occur only in analysis executables and are
not used by the candidate hash.

## Why version 1 exists

Version 0 returned the 32 body-diagonal cells of one final cube directly.
Independent and local testing found two structural distinguishers:

- no byte could occur more than twice in a digest, because each cube value
  exists exactly twice;
- short messages retained a strong positional preference for the byte that
  originally occupied an output coordinate.

Version 1 keeps the four-diagonal concept but no longer concatenates raw cells.
Each output byte is derived from all four diagonals, and four new intersecting
rotation symbols separate consecutive bytes. The 32 bytes therefore come from
32 evolving cube states.

Before squeezing, a 128-symbol orbit closure samples four quarters of the cube,
so every one of the 512 physical cells participates as a direct sample.

## Output construction

For output position `i`, the current four body diagonals are read:

```text
D0 = C(j, j, j)
D1 = C(7-j, j, j)
D2 = C(j, 7-j, j)
D3 = C(j, j, 7-j)
```

Selected cells from all four diagonals are combined with geometry-derived byte
rotations, byte addition, the squeeze chain value, and the output position.
After the byte is emitted, four diagonal-rooted symbols are absorbed into the
same intersecting rotation chain before the next output byte is produced.

The operational definition is `src/engine.cpp`; `src/hash.cpp` fixes the public API to the canonical parameters.

## Build

```bash
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release \
  -DPVC_WARNINGS_AS_ERRORS=ON
cmake --build build
ctest --test-dir build --output-on-failure
```

Cross-implementation conformance:

```bash
python3 scripts/verify_vectors.py \
  --cpp build/pvc-hash \
  --vector-dump build/pvc-vector-dump
```

Expected output:

```text
verified 32 digest vectors and 5 phase vectors
```

Sanitizer build:

```bash
cmake -S . -B build-san \
  -DCMAKE_BUILD_TYPE=Debug \
  -DPVC_ENABLE_SANITIZERS=ON \
  -DPVC_WARNINGS_AS_ERRORS=ON
cmake --build build-san
ctest --test-dir build-san --output-on-failure
```

## Usage

```bash
./build/pvc-hash --text "hello"
./build/pvc-hash --file document.bin
./build/pvc-hash --text "hello" --trace
./build/pvc-hash --text "hello" --dump-cube
```

## Reduced-round research interface

`include/pvc/research.hpp` exposes runtime parameters, exact operational-state
snapshots, phase checkpoints, and six reduced-round presets. The normal
`RotHash1` API remains fixed to the canonical configuration.

See [`docs/CRYPTANALYSIS_FRAMEWORK.md`](docs/CRYPTANALYSIS_FRAMEWORK.md).

Version 0.8.0 results are in [`docs/DIGEST_SURFACE_RESULTS.md`](docs/DIGEST_SURFACE_RESULTS.md).

## Analysis tools

```bash
./build/pvc-avalanche "Perfect Value Cube"
./build/pvc-avalanche-sweep 32000 16
./build/pvc-collision-probe 1
./build/pvc-collision-probe 2
./build/pvc-distribution-probe --exhaustive-two-byte
./build/pvc-distribution-probe 100000 16
./build/pvc-state-bias-probe 50000 16
./build/pvc-structure-probe "Perfect Value Cube"
./build/pvc-reduced-round-probe
./build/pvc-transition-collision --preset R5-canonical --depth 2
./build/pvc-phase-collision --preset R5-canonical --message-bytes 2
./build/pvc-truncated-collision --preset R5-canonical --bits 24 --limit 50000
./build/pvc-differential-search --preset R5-canonical --samples 4
./build/pvc-predecessor-enumerator --preset R5-canonical
./build/pvc-related-input-probe --preset R5-canonical
./build/pvc-foldback-merge-search --left 176f --right 1799 --suffix-bytes 2
./build/pvc-three-byte-collision --phase forward --threads 4
./build/pvc-three-byte-collision --phase foldback --threads 4
./build/pvc-alignment-probe --delta 42
./build/pvc-constrained-merge-search --left 176f --right 1799 --suffix-bytes 2
./build/pvc-foldback-aware-alias --suffix-bytes 1 --suffix-limit 256 --threads 8
./build/pvc-multicollision-probe --suffix-bytes 1 --suffix-limit 256 --threads 8
./build/pvc-foldback-separation-profile --threads 8
./build/pvc-independent-suffix-catalog --threads 8
./build/pvc-bridged-multicollision --levels 32 --materialize-levels 16 --threads 8
./build/pvc-foldback-beam-search --levels 16 --beam 1024 --threads 8
./build/pvc-foldback-lsh-search --left 176f00 --right 179900 --suffix-bytes 2 --suffix-limit 8192 --projections 16 --projection-bytes 4
./build/pvc-return-alias-surface --messages 256 --message-bytes 8
./build/pvc-truncated-campaign --bits 24,32 --trials 8 --limit 150000
./build/pvc-length-framing-probe --max-length 64
./build/pvc-digest-beam-search --levels 16 --beam 1024 --threads 8
./build/pvc-divergent-digest-beam --left 000000 --right 000001 --depth 8 --beam 128 --branch 16
./build/pvc-digest-lsh-search --left 000000 --right 000001 --suffix-bytes 2 --suffix-limit 8192 --projections 32 --projection-bytes 1
./build/pvc-barrier-correlation --left 000000 --right 000001 --samples 10000 --suffix-bytes 2 --independent-suffix
./build/pvc-dual-return-alias --suffix-bytes 1 --suffix-limit 256
./build/pvc-multipath-foldback-sample --seed-limit 128 --suffix-samples 64
python3 scripts/controller_redesign_prototypes.py --variants S --deep --two-byte-full
python3 scripts/stage2_smoke_s.py
```

These tools are intended to disprove the design. Passing them is not evidence
of cryptographic security.

## Current measured status

On the documented GCC 14.2.0 test environment:

- all 65,536 two-byte inputs produced distinct digests;
- 32,000 single-bit differential trials had mean distance `127.9508/256`
  and standard deviation `8.0574`;
- exhaustive two-byte output distributions had mean per-position chi-square
  `255.4556`;
- 100,000 deterministic 16-byte test messages had mean per-position
  chi-square `254.5854`;
- equal-byte and complementary-byte pair rates matched their random-reference
  expectations closely;
- digests containing a byte at least three times occurred at the expected
  nonzero rate, removing version 0's multiplicity distinguisher;
- final-cube cells no longer showed the canonical-value preference reported
  against version 0 in the tested sample.

These results only reject specific simple distinguishers. They do not establish
preimage, second-preimage, or collision resistance.

Version 0.3.0 structural analysis additionally found:

- three exact forward-state merges among all 65,536 canonical two-byte messages;
- the three pairs are `17 6f`/`17 99`, `25 1c`/`25 46`, and
  `a2 6f`/`a2 99`;
- reverse foldback separates all three merges in that finite domain;
- appending every possible common two-byte suffix to each pair found no
  after-foldback collision;
- no after-foldback, closure, final-state, or digest collision was found in the
  complete two-byte domain;
- reduced presets R0 through R2 produce full collisions, while R3 through R5 do
  not in the same exhaustive two-byte experiment;
- 24-bit truncated collision times for R5 remain broadly compatible with the
  generic birthday scale in the initial 20-trial sample.

See [`docs/REDUCED_ROUND_RESULTS.md`](docs/REDUCED_ROUND_RESULTS.md).

Version 0.4.0 extended the structural search to the full three-byte domain:

- 1,496 exact forward-state pairs were found among all 16,777,216 messages;
- 768 are inherited extensions of the three known two-byte merges;
- 728 are new context-dependent third-symbol aliases;
- all 728 new aliases execute identical six-move physical paths, with byte
  differences 42, 126, or 196;
- the complete three-byte after-foldback domain contained zero exact state
  collisions;
- independent two-byte suffix MITM searches covered 2^32 cross combinations
  for each known prefix pair and found zero after-foldback merges.

See [`docs/THREE_BYTE_RESULTS.md`](docs/THREE_BYTE_RESULTS.md).

Version 0.5.0 coupled the forward aliases to the foldback equations:

- none of the 1,496 known three-byte forward pairs was also a direct return-
  transition alias;
- every common one-byte suffix was tested for all pairs, covering 382,976
  structured four-byte cases with zero after-foldback merge;
- 6,127,616 sampled common two-byte extension cases also produced zero exact
  merge;
- 15 common forward states supported a second controller alias, producing
  explicit four-message forward multicollisions;
- all four branches remained distinct after foldback and in the full digest;
- all common one-byte suffixes and the first 4,096 two-byte suffixes were tested
  for every four-way family with zero collision.

See [`docs/FOLDBACK_AWARE_RESULTS.md`](docs/FOLDBACK_AWARE_RESULTS.md).

Version 0.6.0 made the foldback boundary and forward multicollision mechanism
more explicit:

- all 1,496 known forward pairs diverge exactly at the first reverse step that
  processes their last differing byte;
- no direct return alias, delayed divergence, or later reconvergence was found;
- the first differing return transition changes at least 49 cube cells;
- all 98,041,856 independent one-byte suffix cross pairs for the complete
  catalogue produced distinct after-foldback states;
- one-byte common bridges permit a 32-level forward collision path, representing
  a theoretical 2^32-message forward multicollision family;
- a fully materialized 65,536-message subset had one forward state but 65,536
  distinct after-foldback states and digests.

See [`docs/FOLDBACK_SEPARATION_RESULTS.md`](docs/FOLDBACK_SEPARATION_RESULTS.md)
and [`docs/BRIDGED_MULTICOLLISION_RESULTS.md`](docs/BRIDGED_MULTICOLLISION_RESULTS.md).


Version 0.7.0 targeted foldback directly with distance-guided and
bidirectional-style searches:

- beam search inside a bridged forward-multicollision family reached a minimum
  after-foldback distance of 224 bits, with no exact merge and no monotonic
  convergence toward zero;
- a bounded independent two-byte suffix search covered 67,108,864 logical
  pairs for `176f00`/`179900` and reached 180 bits, with no exact merge;
- 24 aliases were found in 2,304 sampled reachable reverse contexts (1.0417%);
  all differences were 42, 126, or 196;
- 24- and 32-bit first-collision samples remained broadly compatible with the
  generic birthday scale;
- symbol-index framing prevents identical complete operational states at equal
  phases for unequal message lengths.

See [`docs/STRUCTURAL_SECURITY_CAMPAIGN.md`](docs/STRUCTURAL_SECURITY_CAMPAIGN.md).

## Security status

**Do not use this project for passwords, authentication, signatures, file
integrity, key derivation, proof-of-work, or any production security boundary.**

See:

- [`docs/DESIGN.md`](docs/DESIGN.md)
- [`docs/ATTACK_MODEL.md`](docs/ATTACK_MODEL.md)
- [`docs/EXPERIMENTS.md`](docs/EXPERIMENTS.md)
- [`docs/RESULTS.md`](docs/RESULTS.md)
- [`docs/ATTACK_LOG.md`](docs/ATTACK_LOG.md)
- [`docs/DECISIONS.md`](docs/DECISIONS.md)
- [`docs/CRYPTANALYSIS_FRAMEWORK.md`](docs/CRYPTANALYSIS_FRAMEWORK.md)
- [`docs/STRUCTURAL_SECURITY_CAMPAIGN.md`](docs/STRUCTURAL_SECURITY_CAMPAIGN.md)
- [`docs/REDUCED_ROUND_RESULTS.md`](docs/REDUCED_ROUND_RESULTS.md)
- [`docs/THREE_BYTE_RESULTS.md`](docs/THREE_BYTE_RESULTS.md)
- [`docs/FOLDBACK_AWARE_RESULTS.md`](docs/FOLDBACK_AWARE_RESULTS.md)
- [`docs/FOLDBACK_SEPARATION_RESULTS.md`](docs/FOLDBACK_SEPARATION_RESULTS.md)
- [`docs/BRIDGED_MULTICOLLISION_RESULTS.md`](docs/BRIDGED_MULTICOLLISION_RESULTS.md)
- [`SECURITY.md`](SECURITY.md)

## Origin

The canonical state is copied from `azizgokhannarin/perfectvaluecube`. It
contains every byte value from 0 to 255 exactly twice, and all 192 initial
axis-parallel lines sum to 1020.

## License

Apache License 2.0.
