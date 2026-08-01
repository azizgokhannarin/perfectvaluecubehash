#!/usr/bin/env python3
"""Independent controller-alias analysis for PVC-RotHash-1 (research only).

Re-implements absorb_symbol from SPECIFICATION.md using only the Python
standard library and the published Perfect Value Cube constant. It does not
import the C++ engine and must not be treated as a security proof.

Usage:
  python3 scripts/controller_alias_analysis.py
"""

from __future__ import annotations

import argparse
from dataclasses import dataclass
from pathlib import Path
import sys

# Allow running from repo root or scripts/.
ROOT = Path(__file__).resolve().parents[1]
sys.path.insert(0, str(ROOT / "reference" / "python"))

from pvc_rothash1 import (  # noqa: E402
    State,
    absorb_symbol,
    choose_other_axis,
    coordinate_code,
    rotl8,
    u8,
)

KNOWN_DELTAS = (42, 126, 196)
MULTIPLES_OF_14 = tuple(d for d in range(14, 256, 14))


@dataclass(frozen=True)
class MoveView:
    phase: int
    axis: int
    amount: int
    cursor_before: tuple[int, int, int]
    cursor_after: tuple[int, int, int]


def absorb_symbol_traced(state: State, symbol: int) -> list[MoveView]:
    """SPEC §9 with a physical-move trace (no shared C++ control flow)."""
    control = u8(symbol + u8(state.symbol_index) + coordinate_code(state.cursor))
    moves: list[MoveView] = []
    for phase in range(6):
        cursor_before = state.cursor
        probe_before = state.at(state.cursor)
        index_byte = (state.symbol_index >> ((phase & 7) * 8)) & 0xFF
        geometry = u8(coordinate_code(state.cursor) + phase * 29 + index_byte)
        selector = rotl8(control, phase) ^ probe_before ^ geometry
        axis = choose_other_axis(state.previous_axis, selector)
        amount_source = control + probe_before + geometry + symbol + axis * 11
        amount = 1 + (amount_source % 7)
        state.rotate_line(axis, state.cursor, amount)
        state.cursor = (
            advance_local(state.cursor, axis, amount)
        )
        probe_after = state.at(state.cursor)
        moves.append(
            MoveView(
                phase=phase,
                axis=axis,
                amount=amount,
                cursor_before=cursor_before,
                cursor_after=state.cursor,
            )
        )
        control = u8(
            rotl8(control, 1 + (axis & 1))
            + probe_after
            + amount
            + coordinate_code(state.cursor)
            + phase * 7
        )
        state.previous_axis = axis
    state.symbol_index += 1
    return moves


def advance_local(
    coord: tuple[int, int, int], axis: int, amount: int
) -> tuple[int, int, int]:
    x, y, z = coord
    amount &= 7
    if axis == 0:
        return (x + amount) & 7, y, z
    if axis == 1:
        return x, (y + amount) & 7, z
    return x, y, (z + amount) & 7


def state_key(state: State) -> tuple:
    return (
        tuple(state.cube),
        state.cursor,
        state.previous_axis,
        state.symbol_index,
    )


def physical_equal(a: list[MoveView], b: list[MoveView]) -> bool:
    if len(a) != len(b):
        return False
    return all(
        x.phase == y.phase
        and x.axis == y.axis
        and x.amount == y.amount
        and x.cursor_before == y.cursor_before
        and x.cursor_after == y.cursor_after
        for x, y in zip(a, b)
    )


def phase0_necessary(d: int) -> bool:
    """Even and multiple of 7 ⇒ multiple of 14 (see campaign doc §2.2)."""
    return d > 0 and d % 14 == 0


def scan_one_byte_contexts(deltas: tuple[int, ...]) -> dict[int, list[tuple[int, int, int]]]:
    """For each first symbol, try second symbols differing by d (non-wrapping)."""
    found: dict[int, list[tuple[int, int, int]]] = {d: [] for d in deltas}
    for first in range(256):
        base = State.initial()
        absorb_symbol(base, first)
        for d in deltas:
            for symbol in range(0, 256 - d):
                left = State(
                    list(base.cube),
                    base.cursor,
                    base.previous_axis,
                    base.symbol_index,
                )
                right = State(
                    list(base.cube),
                    base.cursor,
                    base.previous_axis,
                    base.symbol_index,
                )
                tL = absorb_symbol_traced(left, symbol)
                tR = absorb_symbol_traced(right, symbol + d)
                if state_key(left) == state_key(right):
                    assert physical_equal(tL, tR)
                    found[d].append((first, symbol, symbol + d))
    return found


def verify_known_anchors() -> None:
    """Regression anchors from docs/THREE_BYTE_RESULTS.md / tests."""
    anchors = [
        (0x17, 0x6F, 0x99),
        (0x25, 0x1C, 0x46),
        (0xA2, 0x6F, 0x99),
    ]
    for first, a, b in anchors:
        d = b - a
        assert d == 42, (first, a, b, d)
        base = State.initial()
        absorb_symbol(base, first)
        left = State(list(base.cube), base.cursor, base.previous_axis, base.symbol_index)
        right = State(list(base.cube), base.cursor, base.previous_axis, base.symbol_index)
        absorb_symbol_traced(left, a)
        absorb_symbol_traced(right, b)
        if state_key(left) != state_key(right):
            raise SystemExit(f"anchor failed: {first:02x} {a:02x}/{b:02x}")


def print_modular_summary() -> None:
    print("PVC-RotHash-1 controller alias analysis (independent Python)")
    print("phase0_necessary: d even and d≡0 (mod 7) ⇒ d≡0 (mod 14)")
    print("multiples_of_14:", ",".join(str(d) for d in MULTIPLES_OF_14))
    print(
        "known_family_subset_of_14:",
        all(phase0_necessary(d) for d in KNOWN_DELTAS),
    )
    print(
        "root_cause_sketch: symbol enters control and amount_source linearly; "
        "amount uses integer sum mod 7; axis uses selector LSB"
    )


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument(
        "--scan-multiples-of-14",
        action="store_true",
        help="scan all one-byte contexts for every multiple of 14 (slow-ish)",
    )
    parser.add_argument(
        "--deltas",
        default="42,126,196",
        help="comma-separated deltas for the one-byte context scan",
    )
    args = parser.parse_args()

    print_modular_summary()
    verify_known_anchors()
    print("known_anchors: ok (17:6f/99, 25:1c/46, a2:6f/99)")

    if args.scan_multiples_of_14:
        deltas = MULTIPLES_OF_14
    else:
        deltas = tuple(int(x) for x in args.deltas.split(",") if x.strip())

    print(f"scanning one-byte contexts for deltas={deltas} ...")
    found = scan_one_byte_contexts(deltas)
    for d in deltas:
        pairs = found[d]
        print(f"delta={d}: exact_one_symbol_aliases={len(pairs)}")
        for first, a, b in pairs[:8]:
            print(f"  context={first:02x} symbols={a:02x}/{b:02x}")
        if len(pairs) > 8:
            print(f"  ... {len(pairs) - 8} more")

    # Sanity: only 42 among default known family should appear in this domain.
    if set(deltas) >= set(KNOWN_DELTAS):
        if len(found.get(42, [])) != 3:
            raise SystemExit(f"expected 3 delta-42 aliases, got {len(found.get(42, []))}")
        if found.get(126) or found.get(196):
            raise SystemExit("unexpected 126/196 aliases in one-byte domain")

    print("analysis_complete")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
