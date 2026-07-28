# Reproducibility

## Supported baseline

The candidate uses C++20 and CMake 3.20 or newer. The Python reference requires
Python 3.10 or newer and no third-party packages.

## Clean build

```bash
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release \
  -DPVC_WARNINGS_AS_ERRORS=ON
cmake --build build --parallel 2
ctest --test-dir build --output-on-failure
```

## Cross-implementation conformance

```bash
python3 scripts/verify_vectors.py \
  --cpp build/pvc-hash \
  --vector-dump build/pvc-vector-dump
```

Expected summary:

```text
verified 32 digest vectors and 5 phase vectors
```

## Anchor commands

```bash
./build/pvc-hash --hex ""
./build/pvc-hash --hex 616263
python3 reference/python/pvc_rothash1.py --hex ""
python3 reference/python/pvc_rothash1.py --hex 616263
```

## Quick research suite

```bash
./build/pvc-collision-probe 2
./build/pvc-avalanche-sweep 32000 16
./build/pvc-distribution-probe --exhaustive-two-byte
./build/pvc-three-byte-collision --phase forward --threads 4
./build/pvc-three-byte-collision --phase foldback --threads 4
./build/pvc-digest-beam-search --levels 16 --beam 1024 --threads 8
./build/pvc-barrier-correlation \
  --left 000000 --right 000001 \
  --samples 10000 --suffix-bytes 2 --independent-suffix
```

Some exhaustive and LSH campaigns require substantial CPU time and memory.
Exact commands and bounded domains are recorded in the associated result files.

## Regenerating vectors

The checked-in vector files are frozen. Regeneration is only a conformance
check during the candidate period:

```bash
python3 scripts/generate_vectors.py
git diff --exit-code -- test-vectors/
```

Any vector change means either an implementation defect or an incompatible
algorithm change and must stop the release process.
