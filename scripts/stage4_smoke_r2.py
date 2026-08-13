#!/usr/bin/env python3
"""Stage 4 smoke — PVC-RotHash-2 small-domain falsification checks.

Reuses the dual Python/C++ references. This is a smoke gate, not a security proof.

Checks:
  SF1  Exhaustive one-byte digests are unique (256 messages).
  SF2  Sampled two-byte digests unique on a fixed grid (4096 messages by default).
  SF3  Known RotHash-1 forward/context multicollision families produce distinct
       RotHash-2 digests pairwise.
  SF4  Optional: call C++ pvc-collision-probe --rothash2 1 if --cpp given.

Exit 0 on all pass. Not production.
"""

from __future__ import annotations

import argparse
import subprocess
import sys
from itertools import combinations
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
sys.path.insert(0, str(ROOT / "reference" / "python"))
from pvc_rothash2 import hash_hex  # noqa: E402


# Known RotHash-1 structural families (disclosed; digests under R2 must differ).
R1_FORWARD_PAIR = (bytes.fromhex("176f"), bytes.fromhex("1799"))
R1_CONTEXT_PAIR = (bytes.fromhex("af671b"), bytes.fromhex("af67df"))
R1_FOUR_WAY = [
    bytes.fromhex("176f115b"),
    bytes.fromhex("176f1185"),
    bytes.fromhex("1799115b"),
    bytes.fromhex("17991185"),
]


def check_sf1() -> tuple[bool, str]:
    digests: dict[str, int] = {}
    for value in range(256):
        d = hash_hex(bytes([value]))
        if d in digests:
            return False, f"SF1 FAIL collision {digests[d]:02x} vs {value:02x} dig={d}"
        digests[d] = value
    return True, "SF1 PASS: 256 unique one-byte digests"


def check_sf2(grid: int) -> tuple[bool, str]:
    # Regular grid over 256x256: grid^2 samples (default 64^2 = 4096).
    step = max(1, 256 // grid)
    digests: dict[str, tuple[int, int]] = {}
    count = 0
    for first in range(0, 256, step):
        for second in range(0, 256, step):
            msg = bytes([first, second])
            d = hash_hex(msg)
            if d in digests:
                a, b = digests[d]
                return (
                    False,
                    f"SF2 FAIL collision {a:02x}{b:02x} vs {first:02x}{second:02x} dig={d}",
                )
            digests[d] = (first, second)
            count += 1
    return True, f"SF2 PASS: {count} unique two-byte grid digests (step={step})"


def check_sf3() -> tuple[bool, str]:
    families = {
        "r1-forward": list(R1_FORWARD_PAIR),
        "r1-context": list(R1_CONTEXT_PAIR),
        "r1-four-way": R1_FOUR_WAY,
    }
    for name, messages in families.items():
        digests = [hash_hex(m) for m in messages]
        for (i, di), (j, dj) in combinations(enumerate(digests), 2):
            if di == dj:
                return (
                    False,
                    f"SF3 FAIL {name}: messages {i} and {j} share digest {di}",
                )
    return True, "SF3 PASS: known RotHash-1 families have pairwise-distinct RotHash-2 digests"


def check_sf4(cpp: Path | None) -> tuple[bool, str]:
    if cpp is None:
        return True, "SF4 SKIP: no --cpp (C++ one-byte probe)"
    try:
        out = subprocess.check_output(
            [str(cpp), "--rothash2", "1"],
            text=True,
            stderr=subprocess.STDOUT,
        )
    except subprocess.CalledProcessError as exc:
        return False, f"SF4 FAIL: collision-probe exit {exc.returncode}: {exc.output}"
    except FileNotFoundError:
        return False, f"SF4 FAIL: executable not found: {cpp}"
    if "No collision" not in out and "no collision" not in out.lower():
        return False, f"SF4 FAIL unexpected output: {out.strip()}"
    return True, "SF4 PASS: C++ collision-probe --rothash2 1 reports no collisions"


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument(
        "--grid",
        type=int,
        default=64,
        help="two-byte grid side length (default 64 → 4096 messages)",
    )
    parser.add_argument(
        "--cpp",
        type=Path,
        help="path to pvc-collision-probe (optional SF4)",
    )
    parser.add_argument(
        "--full-two-byte",
        action="store_true",
        help="SF2 uses exhaustive 65536 two-byte messages (slow)",
    )
    args = parser.parse_args()

    grid = 256 if args.full_two_byte else args.grid
    results: list[tuple[bool, str]] = []
    results.append(check_sf1())
    results.append(check_sf2(grid))
    results.append(check_sf3())
    results.append(check_sf4(args.cpp))

    all_ok = True
    for ok, msg in results:
        print(msg)
        all_ok = all_ok and ok

    if all_ok:
        print("Stage 4 smoke (RotHash-2): PASS")
        return 0
    print("Stage 4 smoke (RotHash-2): FAIL", file=sys.stderr)
    return 1


if __name__ == "__main__":
    raise SystemExit(main())
