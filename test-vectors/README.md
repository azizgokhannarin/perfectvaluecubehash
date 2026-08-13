# Official Candidate Vectors

## PVC-RotHash-2 0.2.0-draft (active successor)

- `official-v2.json` — 29 digest vectors (Controller S absorb).
- `official-v2.rsp` — the same digests in a simple response-file format.

```bash
python3 scripts/verify_vectors_v2.py --cpp build/pvc-hash
./build/pvc-hash --rothash2 --hex 616263
```

## PVC-RotHash-1 1.0.0-rc1 (historical baseline)

- `official-v1.json` — 32 digest vectors with exact byte inputs.
- `official-v1.rsp` — the same digest vectors in a simple response-file format.
- `phase-vectors-v1.json` — five vectors containing the complete operational
  state after forward, foldback, diagonal closure, orbit closure, and squeeze.

`cube_hex` uses `index = 64*z + 8*y + x`. A state comparison must also include
cursor, previous axis, and symbol index.

```bash
python3 scripts/verify_vectors.py \
  --cpp build/pvc-hash \
  --vector-dump build/pvc-vector-dump
```
