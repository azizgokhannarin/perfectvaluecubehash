#!/usr/bin/env python3
"""Verify PVC-RotHash-2 official vectors with the Python reference and optional C++ CLI.

Not production. RotHash-1 vectors are intentionally out of scope here.
"""

from __future__ import annotations

import argparse
import json
import subprocess
import sys
import tempfile
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
sys.path.insert(0, str(ROOT / "reference" / "python"))
from pvc_rothash2 import hash_hex  # noqa: E402


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--cpp", type=Path, help="path to pvc-hash executable")
    args = parser.parse_args()

    official_path = ROOT / "test-vectors" / "official-v2.json"
    official = json.loads(official_path.read_text())
    if official.get("algorithm") != "PVC-RotHash-2":
        print("FAIL: unexpected algorithm field", file=sys.stderr)
        return 1

    failures: list[str] = []
    for vector in official["vectors"]:
        vector_id = vector.get("id") or vector.get("name") or vector.get("message_hex")
        message = bytes.fromhex(vector.get("message_hex") or vector.get("input_hex") or "")
        expected = vector["digest_hex"].lower()
        actual = hash_hex(message)
        if actual != expected:
            failures.append(
                f"python digest mismatch: {vector_id} expected={expected} actual={actual}"
            )
        if args.cpp:
            with tempfile.NamedTemporaryFile() as tmp:
                tmp.write(message)
                tmp.flush()
                cpp = subprocess.check_output(
                    [str(args.cpp), "--rothash2", "--file", tmp.name],
                    text=True,
                ).strip().lower()
            if cpp != expected:
                failures.append(
                    f"c++ digest mismatch: {vector_id} expected={expected} actual={cpp}"
                )

    if failures:
        for failure in failures:
            print(f"FAIL: {failure}", file=sys.stderr)
        return 1

    impl = "python"
    if args.cpp:
        impl = "python + c++"
    print(f"verified {len(official['vectors'])} RotHash-2 digest vectors ({impl})")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
