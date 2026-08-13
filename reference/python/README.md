# Independent Python References

Intended for conformance and cryptanalysis, not performance or production.
No third-party packages.

## PVC-RotHash-2 (active draft)

```bash
python3 pvc_rothash2.py --text abc
python3 pvc_rothash2.py --hex 616263
python3 ../../scripts/verify_vectors_v2.py --cpp ../../build/pvc-hash
```

## PVC-RotHash-1 (frozen baseline)

```bash
python3 pvc_rothash1.py --text abc
python3 pvc_rothash1.py --hex 616263
python3 pvc_rothash1.py --file sample.bin
```

```bash
python3 scripts/verify_vectors.py \
  --cpp build/pvc-hash \
  --vector-dump build/pvc-vector-dump
```
