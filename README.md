# Perfect Value Cube Hash

`PVC-RotHash-0` is a **research prototype**, not a production cryptographic hash.

The project tests whether the Perfect Value Cube and a state-dependent chain of
intersecting rotations can form a useful one-way permutation structure.

## Design boundary

The candidate algorithm deliberately contains no existing cryptographic
primitive:

- no SHA family,
- no Keccak,
- no AES,
- no ChaCha,
- no BLAKE,
- no imported S-box,
- no external KDF, MAC, or checksum.

Only the following design elements are used:

1. the canonical 8×8×8 Perfect Value Cube;
2. rotations of eight-cell X, Y, and Z lines;
3. a chain in which every new rotation intersects the previous rotation;
4. message- and state-dependent move selection;
5. a 32-byte result taken directly from the cube's four body diagonals.

Standard C++ containers and general statistical tests are used only as
implementation and analysis tools.

## Output

An 8×8×8 cube has four body diagonals. Each diagonal contains eight byte cells,
so their direct concatenation produces the 32-byte experimental digest:

```text
D0 = C(i, i, i)
D1 = C(7-i, i, i)
D2 = C(i, 7-i, i)
D3 = C(i, i, 7-i)
digest = D0 || D1 || D2 || D3
```

for `i = 0..7`.

## Build

```bash
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build
ctest --test-dir build --output-on-failure
```

Sanitizer build:

```bash
cmake -S . -B build-san \
  -DCMAKE_BUILD_TYPE=Debug \
  -DPVC_ENABLE_SANITIZERS=ON
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
./build/pvc-collision-probe 1
./build/pvc-collision-probe 2
./build/pvc-structure-probe "Perfect Value Cube"
```

The tools are intended to disprove the design as early as possible. A passing
test is not evidence of cryptographic security.

## Current security status

**Unreviewed and experimental. Do not use for passwords, signatures, file
integrity, authentication, key derivation, or any security-sensitive purpose.**

The state transformation only permutes the original 512 byte cells. It
therefore preserves the complete value histogram and may preserve or expose
additional algebraic invariants. This is an intentional first-stage design
constraint so the strength and weakness of the rotation concept can be measured
without an established algorithm masking the result.

See:

- [`docs/DESIGN.md`](docs/DESIGN.md)
- [`docs/ATTACK_MODEL.md`](docs/ATTACK_MODEL.md)
- [`docs/EXPERIMENTS.md`](docs/EXPERIMENTS.md)
- [`docs/RESULTS.md`](docs/RESULTS.md)
- [`docs/ATTACK_LOG.md`](docs/ATTACK_LOG.md)
- [`docs/DECISIONS.md`](docs/DECISIONS.md)
- [`SECURITY.md`](SECURITY.md)

## Origin

The canonical cube is copied from:

- `azizgokhannarin/perfectvaluecube`

It contains every byte value from 0 to 255 exactly twice and all 192
axis-parallel eight-cell lines sum to 1020.

## License

Apache License 2.0.
