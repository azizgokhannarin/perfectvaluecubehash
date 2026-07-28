# Independent Python Reference

`pvc_rothash1.py` is a pure-Python implementation of the frozen
PVC-RotHash-1 1.0.0-rc1 specification. It uses no third-party packages and is
structured independently from the C++ engine.

It is intended for conformance and cryptanalysis, not performance or production.

```bash
python3 pvc_rothash1.py --text abc
python3 pvc_rothash1.py --hex 616263
python3 pvc_rothash1.py --file sample.bin
```

Repository-level verification:

```bash
python3 scripts/verify_vectors.py \
  --cpp build/pvc-hash \
  --vector-dump build/pvc-vector-dump
```
