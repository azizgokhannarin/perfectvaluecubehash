# Official Candidate Vectors

These files are normative conformance artifacts for PVC-RotHash-1
1.0.0-rc1.

- `official-v1.json` — 32 digest vectors with exact byte inputs.
- `official-v1.rsp` — the same digest vectors in a simple response-file format.
- `phase-vectors-v1.json` — five vectors containing the complete operational
  state after forward, foldback, diagonal closure, orbit closure, and squeeze.

`cube_hex` uses `index = 64*z + 8*y + x`. A state comparison must also include
cursor, previous axis, and symbol index.

Verification:

```bash
python3 scripts/verify_vectors.py \
  --cpp build/pvc-hash \
  --vector-dump build/pvc-vector-dump
```
