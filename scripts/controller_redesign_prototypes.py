#!/usr/bin/env python3
"""Offline controller redesign prototypes (NOT PVC-RotHash-1).

These variants deliberately change only the amount/axis mixing formulas to test
whether the one-symbol alias surface can be removed without importing an
external cryptographic primitive. They must never write official vectors or
claim to replace the frozen candidate.

Usage:
  python3 scripts/controller_redesign_prototypes.py
  python3 scripts/controller_redesign_prototypes.py --variants A,B,C --deep
"""

from __future__ import annotations

import argparse
import copy
from dataclasses import dataclass
from pathlib import Path
import sys
from typing import Callable

ROOT = Path(__file__).resolve().parents[1]
sys.path.insert(0, str(ROOT / "reference" / "python"))

from pvc_rothash1 import (  # noqa: E402
    State,
    choose_other_axis,
    coordinate_code,
    rotl8,
    u8,
)

VariantFn = Callable[[int, int, int, int, int, int], tuple[int, int]]
# (control, probe, geometry, symbol, previous_axis, phase) -> (axis, amount)


def axis_from_selector(previous: int, selector: int) -> int:
    return choose_other_axis(previous, selector)


def amount_mod7(source: int) -> int:
    return 1 + (source % 7)


def variant_canonical(
    control: int, probe: int, geometry: int, symbol: int, previous: int, phase: int
) -> tuple[int, int]:
    """Frozen RotHash-1 controller (baseline for comparison)."""
    selector = rotl8(control, phase) ^ probe ^ geometry
    axis = axis_from_selector(previous, selector)
    amount = amount_mod7(control + probe + geometry + symbol + axis * 11)
    return axis, amount


def variant_a_no_symbol_in_amount(
    control: int, probe: int, geometry: int, symbol: int, previous: int, phase: int
) -> tuple[int, int]:
    """Remove linear second use of symbol in amount_source."""
    del symbol  # intentionally unused in amount
    selector = rotl8(control, phase) ^ probe ^ geometry
    axis = axis_from_selector(previous, selector)
    amount = amount_mod7(control + probe + geometry + axis * 11)
    return axis, amount


def variant_b_xor_amount(
    control: int, probe: int, geometry: int, symbol: int, previous: int, phase: int
) -> tuple[int, int]:
    """Mix symbol into amount via XOR rather than integer addition."""
    selector = rotl8(control, phase) ^ probe ^ geometry
    axis = axis_from_selector(previous, selector)
    mixed = control ^ rotl8(symbol, phase) ^ probe ^ geometry ^ (axis * 11)
    amount = amount_mod7(mixed)
    return axis, amount


def variant_c_dual_bit_axis(
    control: int, probe: int, geometry: int, symbol: int, previous: int, phase: int
) -> tuple[int, int]:
    """Axis from two mixed bits; amount from rotated XOR lane."""
    lane = rotl8(control, phase) ^ rotl8(symbol, 3) ^ probe ^ geometry
    # Map two bits to a forced different axis: bit0 chooses among two options.
    selector = (lane ^ (lane >> 1) ^ phase) & 0xFF
    axis = axis_from_selector(previous, selector)
    amount = amount_mod7(lane + rotl8(probe, 1) + (phase * 13))
    return axis, amount


def variant_d_xor_rot_amount(
    control: int, probe: int, geometry: int, symbol: int, previous: int, phase: int
) -> tuple[int, int]:
    """Break double-additive Δcontrol+Δsymbol by XOR/rotate amount mix."""
    selector = rotl8(control, phase) ^ probe ^ geometry ^ rotl8(symbol, 1)
    axis = axis_from_selector(previous, selector)
    mixed = (
        rotl8(control, 1)
        ^ rotl8(symbol, 3)
        ^ probe
        ^ geometry
        ^ u8(axis * 11)
    )
    amount = amount_mod7(mixed)
    return axis, amount


def variant_e_feedback_lane(
    control: int, probe: int, geometry: int, symbol: int, previous: int, phase: int
) -> tuple[int, int]:
    """Lead redesign sketch (not a candidate).

    Clears the frozen one-byte 42-family surface. Full two-byte scan still finds
    exactly four residual physical-path pairs (see PHASE1_CONTROLLER_CAMPAIGN §8):
    36/71, 2b/2c, 61/80, 2d/ae — driven by lane deltas that are 0 mod 7 with
    matching axis LSB. Do not “fix” by random ARX thrash; target that rail.
    """
    lane = u8(rotl8(control ^ symbol, phase) + (probe ^ geometry) + phase * 17)
    lane = u8(lane ^ rotl8(lane, 3) ^ rotl8(symbol, 5))
    selector = lane ^ rotl8(probe, phase & 7)
    axis = axis_from_selector(previous, selector)
    amount = amount_mod7(lane + rotl8(geometry, 1) + axis)
    return axis, amount


def variant_e2_double_fold(
    control: int, probe: int, geometry: int, symbol: int, previous: int, phase: int
) -> tuple[int, int]:
    """E + second ARX-style fold; still primitive-free."""
    x = u8(control + rotl8(symbol, 1) + phase * 19)
    y = u8(probe + rotl8(geometry, 2) + (previous * 37))
    lane = u8(rotl8(x ^ y, phase) + rotl8(x + y, 3))
    lane = u8(lane ^ rotl8(lane, 5) ^ rotl8(symbol ^ probe, 2))
    selector = lane ^ rotl8(control, phase & 7) ^ geometry
    axis = axis_from_selector(previous, selector)
    amount = amount_mod7(lane ^ rotl8(geometry + axis, 1) ^ (phase * 9))
    return axis, amount


def variant_e3_split_control_amount(
    control: int, probe: int, geometry: int, symbol: int, previous: int, phase: int
) -> tuple[int, int]:
    """Axis from control/probe; amount from symbol/probe with no shared linear sum."""
    sel_lane = rotl8(control, phase) ^ probe ^ geometry ^ (previous * 13)
    axis = axis_from_selector(previous, sel_lane)
    amt_lane = rotl8(symbol, phase) ^ rotl8(probe, 3) ^ geometry ^ u8(phase * 29)
    amt_lane = u8(amt_lane + rotl8(amt_lane, 1) + rotl8(control, 5))
    amount = amount_mod7(amt_lane ^ (axis * 17) ^ rotl8(symbol, 7))
    return axis, amount


def variant_f_popcount_amount(
    control: int, probe: int, geometry: int, symbol: int, previous: int, phase: int
) -> tuple[int, int]:
    """Amount from popcount mix — destroys additive mod-7 rails."""
    selector = rotl8(control, phase) ^ probe ^ geometry ^ symbol
    axis = axis_from_selector(previous, selector)
    mixed = (control ^ rotl8(symbol, phase) ^ probe ^ geometry ^ (axis * 17)) & 0xFF
    # 1..7 from popcount of mixed and a second fold bit.
    weight = mixed.bit_count()
    amount = 1 + ((weight + ((mixed >> 3) & 1) + phase) % 7)
    return axis, amount


VARIANTS: dict[str, VariantFn] = {
    "canonical": variant_canonical,
    "A": variant_a_no_symbol_in_amount,
    "B": variant_b_xor_amount,
    "C": variant_c_dual_bit_axis,
    "D": variant_d_xor_rot_amount,
    "E": variant_e_feedback_lane,
    "E2": variant_e2_double_fold,
    "E3": variant_e3_split_control_amount,
    "F": variant_f_popcount_amount,
}


@dataclass
class ProtoState:
    cube: list[int]
    cursor: tuple[int, int, int]
    previous_axis: int
    symbol_index: int


def advance(coord: tuple[int, int, int], axis: int, amount: int) -> tuple[int, int, int]:
    x, y, z = coord
    amount &= 7
    if axis == 0:
        return (x + amount) & 7, y, z
    if axis == 1:
        return x, (y + amount) & 7, z
    return x, y, (z + amount) & 7


def rotate_line(cube: list[int], axis: int, point: tuple[int, int, int], amount: int) -> None:
    # Mirror reference implementation.
    st = State(cube, point, 0, 0)
    st.rotate_line(axis, point, amount)
    cube[:] = st.cube


def absorb_symbol(state: ProtoState, symbol: int, variant: VariantFn) -> None:
    control = u8(symbol + u8(state.symbol_index) + coordinate_code(state.cursor))
    for phase in range(6):
        probe = state.cube[
            state.cursor[2] * 64 + state.cursor[1] * 8 + state.cursor[0]
        ]
        index_byte = (state.symbol_index >> ((phase & 7) * 8)) & 0xFF
        geometry = u8(coordinate_code(state.cursor) + phase * 29 + index_byte)
        axis, amount = variant(
            control, probe, geometry, symbol, state.previous_axis, phase
        )
        rotate_line(state.cube, axis, state.cursor, amount)
        state.cursor = advance(state.cursor, axis, amount)
        probe_after = state.cube[
            state.cursor[2] * 64 + state.cursor[1] * 8 + state.cursor[0]
        ]
        control = u8(
            rotl8(control, 1 + (axis & 1))
            + probe_after
            + amount
            + coordinate_code(state.cursor)
            + phase * 7
        )
        state.previous_axis = axis
    state.symbol_index += 1


def initial_state() -> ProtoState:
    base = State.initial()
    return ProtoState(
        cube=list(base.cube),
        cursor=base.cursor,
        previous_axis=base.previous_axis,
        symbol_index=base.symbol_index,
    )


def state_key(state: ProtoState) -> tuple:
    return (
        tuple(state.cube),
        state.cursor,
        state.previous_axis,
        state.symbol_index,
    )


def count_one_byte_context_aliases(variant: VariantFn) -> tuple[int, list[tuple]]:
    """Match scripts/controller_alias_analysis.py domain: first then second symbol."""
    found: list[tuple] = []
    for first in range(256):
        base = initial_state()
        absorb_symbol(base, first, variant)
        # Map second symbol -> state key
        keys: dict[tuple, int] = {}
        for symbol in range(256):
            st = ProtoState(
                cube=list(base.cube),
                cursor=base.cursor,
                previous_axis=base.previous_axis,
                symbol_index=base.symbol_index,
            )
            absorb_symbol(st, symbol, variant)
            key = state_key(st)
            if key in keys:
                found.append((first, keys[key], symbol))
            else:
                keys[key] = symbol
    return len(found), found


def count_initial_one_symbol_aliases(variant: VariantFn) -> int:
    keys: dict[tuple, int] = {}
    collisions = 0
    base = initial_state()
    for symbol in range(256):
        st = ProtoState(
            cube=list(base.cube),
            cursor=base.cursor,
            previous_axis=base.previous_axis,
            symbol_index=base.symbol_index,
        )
        absorb_symbol(st, symbol, variant)
        key = state_key(st)
        if key in keys:
            collisions += 1
        else:
            keys[key] = symbol
    return collisions


def count_aliases_after_prefix(
    variant: VariantFn, prefix: bytes
) -> list[tuple[int, int]]:
    base = initial_state()
    for byte in prefix:
        absorb_symbol(base, byte, variant)
    keys: dict[tuple, int] = {}
    pairs: list[tuple[int, int]] = []
    for symbol in range(256):
        st = ProtoState(
            cube=list(base.cube),
            cursor=base.cursor,
            previous_axis=base.previous_axis,
            symbol_index=base.symbol_index,
        )
        absorb_symbol(st, symbol, variant)
        key = state_key(st)
        if key in keys:
            pairs.append((keys[key], symbol))
        else:
            keys[key] = symbol
    return pairs


def sample_two_byte_context_aliases(
    variant: VariantFn, sample_prefixes: int
) -> tuple[int, int, list[tuple]]:
    """Sample two-byte prefixes; return (prefixes_tested, alias_pairs, examples)."""
    total_pairs = 0
    examples: list[tuple] = []
    step = max(1, 65536 // max(1, sample_prefixes))
    tested = 0
    for counter in range(0, 65536, step):
        if tested >= sample_prefixes:
            break
        prefix = bytes(((counter >> 8) & 0xFF, counter & 0xFF))
        pairs = count_aliases_after_prefix(variant, prefix)
        total_pairs += len(pairs)
        for left, right in pairs[:2]:
            if len(examples) < 8:
                examples.append((prefix[0], prefix[1], left, right))
        tested += 1
    return tested, total_pairs, examples


def full_two_byte_alias_catalogue(
    variant: VariantFn,
) -> tuple[int, Counter]:
    """Exact one-symbol aliases after every two-byte prefix (slow, ~10–15 min)."""
    from collections import Counter as CounterType

    pair_counts: CounterType = CounterType()
    total = 0
    for counter in range(65536):
        if counter % 8192 == 0:
            print(f"  two_byte_full progress {counter}/65536", flush=True)
        prefix = bytes(((counter >> 8) & 0xFF, counter & 0xFF))
        for left, right in count_aliases_after_prefix(variant, prefix):
            pair_counts[(left, right)] += 1
            total += 1
    return total, pair_counts


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument(
        "--variants",
        default="canonical,E",
        help="comma-separated variant ids",
    )
    parser.add_argument(
        "--deep",
        action="store_true",
        help="also scan all one-byte contexts (slow)",
    )
    parser.add_argument(
        "--two-byte-samples",
        type=int,
        default=0,
        help="sample this many two-byte prefixes for one-symbol aliases",
    )
    parser.add_argument(
        "--two-byte-full",
        action="store_true",
        help="exhaustive 2^16 two-byte prefix scan (very slow; use sparingly)",
    )
    args = parser.parse_args()

    print("Controller redesign prototypes (research only; not RotHash-1)")
    print("WARNING: results do not change official vectors or the frozen candidate")

    names = [x.strip() for x in args.variants.split(",") if x.strip()]
    for name in names:
        if name not in VARIANTS:
            raise SystemExit(f"unknown variant {name}; choose from {list(VARIANTS)}")
        variant = VARIANTS[name]
        initial_collisions = count_initial_one_symbol_aliases(variant)
        print(f"\nvariant={name}")
        print(f"  initial_context_one_symbol_alias_pairs={initial_collisions}")
        if args.deep:
            n, samples = count_one_byte_context_aliases(variant)
            print(f"  one_byte_context_alias_pairs={n}")
            for sample in samples[:6]:
                print(f"    example context={sample[0]:02x} {sample[1]:02x}/{sample[2]:02x}")
            if n > 6:
                print(f"    ... {n - 6} more")
        if args.two_byte_samples > 0:
            tested, pairs, examples = sample_two_byte_context_aliases(
                variant, args.two_byte_samples
            )
            print(
                f"  two_byte_prefix_samples={tested} "
                f"one_symbol_alias_pairs={pairs}"
            )
            for ex in examples:
                print(
                    f"    example prefix={ex[0]:02x}{ex[1]:02x} "
                    f"{ex[2]:02x}/{ex[3]:02x}"
                )
        if args.two_byte_full:
            total, pair_counts = full_two_byte_alias_catalogue(variant)
            print(f"  two_byte_full_alias_instances={total}")
            print(f"  two_byte_full_unique_pairs={len(pair_counts)}")
            for (left, right), count in pair_counts.most_common():
                print(
                    f"    pair={left:02x}/{right:02x} "
                    f"delta={((right - left) & 0xFF)} count={count}"
                )

    print("\ninterpretation:")
    print("  - canonical: 3 one-byte-context pairs when --deep")
    print("  - E: 0 one-byte; full two-byte has 4 residual pairs (campaign §8)")
    print("  - redesign goal: zero one- and two-byte one-symbol aliases")
    print("  - any successor still needs avalanche/distribution/foldback campaigns")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
