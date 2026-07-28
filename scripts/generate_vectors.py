#!/usr/bin/env python3
"""Generate the frozen PVC-RotHash-1 candidate vector corpus.

Run only when intentionally cutting a new incompatible candidate. The generated
files are checked into version control and reviewed as normative artifacts.
"""

from __future__ import annotations

import json
import subprocess
import sys
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
PYREF = ROOT / "reference" / "python"
sys.path.insert(0, str(PYREF))

from pvc_rothash1 import hash_hex  # noqa: E402


def cases() -> list[tuple[str, bytes, str]]:
    return [
        ("empty", b"", "Empty message"),
        ("single-00", bytes([0x00]), "Single zero byte"),
        ("single-ff", bytes([0xFF]), "Single 0xff byte"),
        ("two-0001", bytes.fromhex("0001"), "Two-byte ascending boundary"),
        ("two-0100", bytes.fromhex("0100"), "Two-byte reversed boundary"),
        ("ascii-a", b"a", "ASCII a"),
        ("ascii-abc", b"abc", "ASCII abc"),
        ("ascii-message-digest", b"message digest", "Classic short ASCII phrase"),
        ("ascii-pvc", b"Perfect Value Cube", "Project name"),
        ("ascii-quick-fox", b"The quick brown fox jumps over the lazy dog", "Classic sentence"),
        ("utf8-turkish", "Mükemmel Değer Küpü".encode("utf-8"), "UTF-8 Turkish phrase"),
        ("zeros-8", bytes(8), "Eight zero bytes"),
        ("zeros-32", bytes(32), "Thirty-two zero bytes"),
        ("zeros-64", bytes(64), "Sixty-four zero bytes"),
        ("ff-8", bytes([0xFF]) * 8, "Eight 0xff bytes"),
        ("ff-32", bytes([0xFF]) * 32, "Thirty-two 0xff bytes"),
        ("incrementing-8", bytes(range(8)), "00..07"),
        ("incrementing-16", bytes(range(16)), "00..0f"),
        ("incrementing-32", bytes(range(32)), "00..1f"),
        ("incrementing-64", bytes(range(64)), "00..3f"),
        ("incrementing-128", bytes(range(128)), "00..7f"),
        ("incrementing-256", bytes(range(256)), "00..ff"),
        ("decrementing-32", bytes(range(31, -1, -1)), "1f..00"),
        ("alternating-aa55-32", bytes([0xAA, 0x55]) * 16, "Alternating aa55"),
        ("forward-alias-left", bytes.fromhex("176f"), "Known forward-state alias left"),
        ("forward-alias-right", bytes.fromhex("1799"), "Known forward-state alias right"),
        ("context-alias-left", bytes.fromhex("af671b"), "Known context alias left"),
        ("context-alias-right", bytes.fromhex("af67df"), "Known context alias right"),
        ("four-way-1", bytes.fromhex("176f115b"), "Known four-way forward family member 1"),
        ("four-way-2", bytes.fromhex("176f1185"), "Known four-way forward family member 2"),
        ("four-way-3", bytes.fromhex("1799115b"), "Known four-way forward family member 3"),
        ("four-way-4", bytes.fromhex("17991185"), "Known four-way forward family member 4"),
    ]


def main() -> int:
    out_dir = ROOT / "test-vectors"
    out_dir.mkdir(parents=True, exist_ok=True)
    vectors = []
    for name, message, note in cases():
        vectors.append({
            "id": name,
            "note": note,
            "input_length": len(message),
            "input_hex": message.hex(),
            "digest_hex": hash_hex(message),
        })
    document = {
        "algorithm": "PVC-RotHash-1",
        "candidate": "1.0.0-rc1",
        "digest_bytes": 32,
        "encoding": "input_hex is the exact byte string; digest_hex is lowercase hexadecimal",
        "vectors": vectors,
    }
    (out_dir / "official-v1.json").write_text(json.dumps(document, indent=2) + "\n")

    rsp = ["# PVC-RotHash-1 candidate 1.0.0-rc1 official vectors", ""]
    for vector in vectors:
        rsp.extend([
            f"COUNT = {vector['id']}",
            f"LEN = {vector['input_length']}",
            (f"MSG = {vector['input_hex']}" if vector["input_hex"] else "MSG ="),
            f"MD = {vector['digest_hex']}",
            "",
        ])
    (out_dir / "official-v1.rsp").write_text("\n".join(rsp))

    phase_ids = {"empty", "ascii-abc", "incrementing-32", "forward-alias-left", "forward-alias-right"}
    vector_dump = ROOT / "build" / "pvc-vector-dump"
    if not vector_dump.exists():
        raise SystemExit(f"build {vector_dump} before generating phase vectors")
    phase_vectors = []
    for vector in vectors:
        if vector["id"] not in phase_ids:
            continue
        raw = subprocess.check_output([str(vector_dump), "--hex", vector["input_hex"]], text=True)
        record = json.loads(raw)
        record["id"] = vector["id"]
        phase_vectors.append(record)
    phase_document = {
        "algorithm": "PVC-RotHash-1",
        "candidate": "1.0.0-rc1",
        "storage_order": "cube_hex is z-major, then y, then x; index = 64*z + 8*y + x",
        "phases": phase_vectors,
    }
    (out_dir / "phase-vectors-v1.json").write_text(json.dumps(phase_document, indent=2) + "\n")
    print(f"wrote {len(vectors)} digest vectors and {len(phase_vectors)} phase vectors")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
