#!/usr/bin/env python3
"""PVC-RotHash-2 pure-Python reference (experimental).

Absorb uses systematic Controller S (mixed-radix injectivity channel).
Finalization shape matches PVC-RotHash-1 (foldback, closures, squeeze).

Not production. Not PVC-RotHash-1. Official vectors: test-vectors/official-v2.json
when present.
"""

from __future__ import annotations

import argparse
import importlib.util
import sys
from pathlib import Path

ROOT = Path(__file__).resolve().parents[2]
sys.path.insert(0, str(ROOT / "reference" / "python"))
sys.path.insert(0, str(ROOT / "scripts"))

from pvc_rothash1 import (  # noqa: E402
    State,
    encode_length,
    return_symbol,
)

spec = importlib.util.spec_from_file_location(
    "stage2_smoke_s", ROOT / "scripts" / "stage2_smoke_s.py"
)
stage2 = importlib.util.module_from_spec(spec)
sys.modules["stage2_smoke_s"] = stage2
assert spec.loader is not None
spec.loader.exec_module(stage2)


def hash_bytes(message: bytes) -> bytes:
    return stage2.hash_s(message)


def hash_hex(message: bytes) -> str:
    return hash_bytes(message).hex()


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    source = parser.add_mutually_exclusive_group(required=True)
    source.add_argument("--text")
    source.add_argument("--hex", dest="hex_input")
    source.add_argument("--file", type=Path)
    args = parser.parse_args()
    if args.text is not None:
        message = args.text.encode("utf-8")
    elif args.hex_input is not None:
        message = bytes.fromhex(args.hex_input)
    else:
        message = args.file.read_bytes()
    print(hash_hex(message))
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
