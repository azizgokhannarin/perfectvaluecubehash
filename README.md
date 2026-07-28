# Perfect Value Cube Hash

`PVC-RotHash-1` is a **falsification-oriented cryptographic research prototype**.
It is not a production hash and makes no security claim.

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

The operational definition is `src/hash.cpp`.

## Build

```bash
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release \
  -DPVC_WARNINGS_AS_ERRORS=ON
cmake --build build
ctest --test-dir build --output-on-failure
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
- [`SECURITY.md`](SECURITY.md)

## Origin

The canonical state is copied from `azizgokhannarin/perfectvaluecube`. It
contains every byte value from 0 to 255 exactly twice, and all 192 initial
axis-parallel lines sum to 1020.

## License

Apache License 2.0.
