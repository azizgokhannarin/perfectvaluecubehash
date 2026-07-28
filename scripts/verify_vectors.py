#!/usr/bin/env python3
"""Verify official vectors with the Python reference and optional C++ CLI."""

from __future__ import annotations

import argparse
import json
import subprocess
import sys
import tempfile
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
sys.path.insert(0, str(ROOT / "reference" / "python"))
from pvc_rothash1 import hash_hex, inspect  # noqa: E402


def main() -> int:
    parser = argparse.ArgumentParser()
    parser.add_argument("--cpp", type=Path, help="path to pvc-hash executable")
    parser.add_argument("--vector-dump", type=Path, help="path to pvc-vector-dump executable")
    args = parser.parse_args()

    official = json.loads((ROOT / "test-vectors" / "official-v1.json").read_text())
    failures: list[str] = []
    for vector in official["vectors"]:
        message = bytes.fromhex(vector["input_hex"])
        actual = hash_hex(message)
        if actual != vector["digest_hex"]:
            failures.append(f"python digest mismatch: {vector['id']}")
        if args.cpp:
            with tempfile.NamedTemporaryFile() as tmp:
                tmp.write(message)
                tmp.flush()
                cpp = subprocess.check_output([str(args.cpp), "--file", tmp.name], text=True).strip()
            if cpp != vector["digest_hex"]:
                failures.append(f"c++ digest mismatch: {vector['id']}")

    phases = json.loads((ROOT / "test-vectors" / "phase-vectors-v1.json").read_text())
    for expected in phases["phases"]:
        message = bytes.fromhex(expected["input_hex"])
        actual = inspect(message)
        actual["id"] = expected["id"]
        if actual != expected:
            failures.append(f"python phase mismatch: {expected['id']}")
        if args.vector_dump:
            raw = subprocess.check_output(
                [str(args.vector_dump), "--hex", expected["input_hex"]], text=True
            )
            cpp = json.loads(raw)
            cpp["id"] = expected["id"]
            if cpp != expected:
                failures.append(f"c++ phase mismatch: {expected['id']}")

    if failures:
        for failure in failures:
            print(f"FAIL: {failure}", file=sys.stderr)
        return 1
    print(
        f"verified {len(official['vectors'])} digest vectors and "
        f"{len(phases['phases'])} phase vectors"
    )
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
