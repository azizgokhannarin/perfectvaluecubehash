# Reduced-Round Cryptanalysis Framework

## Purpose

Version 0.7.0 does not change the canonical `PVC-RotHash-1` algorithm or its
known-answer vectors. It adds a separate research interface for deliberately
weakening the construction and observing where structural failures appear.

Passing these tools is not a security proof. Their purpose is to produce exact,
reproducible counterexamples when reduced or full configurations fail.

## Research parameters

```cpp
struct HashParameters {
    std::size_t moves_per_symbol;
    std::size_t diagonal_closure_symbols;
    std::size_t orbit_closure_symbols;
    std::size_t squeeze_bytes;
    std::size_t squeeze_symbols_per_byte;
    bool enable_foldback;
};
```

The public `RotHash1` API always uses the canonical constants. Runtime
parameters are accepted only through `pvc/research.hpp`.

## Presets

| Preset | Moves/symbol | Foldback | Diagonal closure | Orbit closure | Output | Squeeze symbols/byte |
|---|---:|:---:|---:|---:|---:|---:|
| R0-minimal | 1 | no | 0 | 0 | 4 B | 1 |
| R1-foldback | 2 | yes | 8 | 0 | 8 B | 1 |
| R2-small | 3 | yes | 16 | 16 | 8 B | 2 |
| R3-medium | 4 | yes | 32 | 32 | 16 B | 2 |
| R4-near | 5 | yes | 64 | 64 | 16 B | 4 |
| R5-canonical | 6 | yes | 64 | 128 | 32 B | 4 |

These presets are experimental landmarks, not claimed security levels.

## Exact state snapshots

An operational snapshot contains:

- all 512 cube bytes;
- cursor coordinates;
- previous axis;
- symbol index.

Phase checkpoints are available after:

- forward absorption;
- reverse foldback;
- diagonal closure;
- orbit closure;
- every squeeze byte;
- finalization.

Collision tools may use a compact non-cryptographic analysis fingerprint for
indexing, but every reported state collision is verified against the complete
snapshot.

## Tools

### Reduced-round matrix

```bash
./build/pvc-reduced-round-probe
```

Exhausts all one-byte messages for each preset and reports digest collisions,
final-state collisions, and one-bit avalanche.

### Local transition collision

```bash
./build/pvc-transition-collision --preset R5-canonical --depth 2
```

Searches all one- or two-symbol continuations from one exact start state. When
it finds a forward convergence, it also checks whether foldback, orbit closure,
final state, or digest equality survives.

### Phase collision enumeration

```bash
./build/pvc-phase-collision --preset R5-canonical --message-bytes 2
```

Exhausts the one- or two-byte message domain and counts exact operational-state
collisions separately at the principal phase boundaries.

### Truncated collision scaling

```bash
./build/pvc-truncated-collision \
  --preset R5-canonical --bits 24 --limit 50000 --trial 0
```

Searches for a collision in the first 1–64 output bits and compares the first
observed repeat with the generic birthday expectation. A truncated collision is
not a full digest collision.

### Differential phase search

```bash
./build/pvc-differential-search \
  --preset R5-canonical --samples 4 --message-bytes 16 --mode single
```

Measures cube bit and byte differences at every phase for single-bit or paired
input differences. It reports exact state or digest convergence if observed.

### Reachable predecessor enumeration

```bash
./build/pvc-predecessor-enumerator \
  --preset R5-canonical --prefixes 256 --prefix-bytes 1 --symbols 256
```

Builds a finite reachable transition graph from sampled prefixes and measures
how many predecessor `(state, symbol)` pairs merge into one exact next state.
This is an empirical precursor to a meet-in-the-middle analysis, not a complete
inverse-state enumerator.

### Related-input and multiset probe

```bash
./build/pvc-related-input-probe --preset R5-canonical
```

Tests reverse, complement, cyclic message rotation, and complete permutation
domains for several repeated-byte multisets.


### Foldback merge-extension search

```bash
./build/pvc-foldback-merge-search \
  --preset R5-canonical --left 176f --right 1799 --suffix-bytes 2
```

Starts from a verified forward-state collision, appends every common suffix in
the selected domain, and checks whether the two paths become equal after
foldback. It also reports the closest observed foldback cube distance when no
collision is found.

## Next step

The next stage is a targeted meet-in-the-middle experiment that uses the
observed forward transition merges while preserving foldback constraints. The
current results show that a naive split at the end of forward absorption is not
enough, because foldback deliberately separates the known convergent prefixes.


## Version 0.4 exhaustive and MITM tools

`pvc-three-byte-collision` uses a research-only 64-bit screening fingerprint,
a radix sort, and complete state verification to make the full 2^24 domain
practical. `--phase forward` classifies controller aliases; `--phase foldback`
searches the complete equal-length state domain immediately before finalization.

`pvc-alignment-probe` compares physical move prefixes for controlled symbol
differences. `pvc-constrained-merge-search` independently varies left and right
suffixes and matches complete after-foldback states using a meet-in-the-middle
table. Analysis fingerprints never determine a reported collision: complete
state equality is required.


## Version 0.5 foldback-aware tools

`pvc-foldback-aware-alias` reconstructs the complete three-byte forward
collision catalogue and applies the corresponding position-dependent return
symbols from the exact common state. It can insert common suffixes before the
return-symbol test, directly testing whether controlled forward aliases can also
alias during reverse traversal.

```bash
./build/pvc-foldback-aware-alias \
  --prefix-count 65536 --threads 8 --suffix-bytes 1 --suffix-limit 256
```

`pvc-multicollision-probe` searches every common three-byte forward state for a
second one-symbol alias, constructs four-message forward multicollisions, and
tests their foldback states and digests under common suffix extensions.

```bash
./build/pvc-multicollision-probe \
  --prefix-count 65536 --threads 8 --max-levels 5 \
  --suffix-bytes 1 --suffix-limit 256
```

Both tools may use non-cryptographic fingerprints only to screen candidates.
Every reported alias or collision is checked using the complete operational
state.
