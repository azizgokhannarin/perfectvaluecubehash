# Bridged Forward Multicollision Results

Environment: GCC 14.2.0, Release build, warnings as errors.

Version 0.6.0 extends the earlier immediate-alias multicollision search. The
previous tool only asked whether the current common forward state itself had
another symbol alias. The new search also permits one common bridge byte before
the next alias.

A common bridge preserves a forward collision:

```text
F(M_1) = F(M_2) = S
F(M_1 || b) = F(M_2 || b)
```

If the bridged state has a controller alias `x/y`, every existing branch can be
extended with either value, doubling the multicollision family.

## Constructive 32-level path

The search found a path with 32 independent alias levels, representing a
forward multicollision family of theoretical size:

```text
2^32 messages
```

The path begins with:

```text
seed: 003282 / 0032ac
bridge b8, alias 37/fb
bridge 30, alias 21/e5
bridge 87, alias 19/97
bridge 83, alias 4b/75
...
```

Thirty of the 31 extension levels use one common bridge byte. One level has an
immediate alias. The complete path is emitted by:

```bash
./build/pvc-bridged-multicollision --levels 32 --materialize-levels 16
```

This supersedes the earlier statement that no third collision level had been
found. No third *immediate* alias existed in that bounded search, but one-byte
common bridges make deeper chains easy to construct.

## Materialized 16-level family

The first 16 collision levels were fully materialized:

```text
messages                         = 65,536
message length                   = 33 bytes
forward-state mismatches         = 0
unique after-foldback states     = 65,536
after-foldback collisions        = 0
unique full digests              = 65,536
full digest collisions           = 0
```

Thus all 65,536 messages have one exact complete forward state, while foldback
separates every branch in this materialized family.

## Independent one-byte suffix catalogue

Every one of the 1,496 known three-byte forward-collision pairs was extended
with an independently selected one-byte suffix on each side:

```text
1,496 * 256 * 256 = 98,041,856 logical cross pairs
fingerprint candidate pairs     = 0
exact after-foldback merges     = 0
```

This exhausts the independent one-byte suffix space for the complete known
three-byte catalogue, rather than only the three original two-byte prefixes.

## Interpretation

The forward pass has a scalable Joux-like multicollision mechanism once common
bridge bytes are allowed. Forward non-injectivity is therefore not sparse in a
security-relevant sense: a short search can construct exponentially large
families sharing one forward state.

This is not yet a full hash break because foldback depends on the complete
message history and separates all materialized branches. It does, however,
make the design concentration risk sharper:

```text
forward absorption supplies diffusion but no collision resistance;
foldback is the component preventing the constructed multicollisions from
surviving to finalization.
```

Future attacks should optimize the bridge and alias choices for return-pass
compatibility rather than merely maximizing forward family size.
