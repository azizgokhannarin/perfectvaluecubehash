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


def variant_g_mod7_rail_break(
    control: int, probe: int, geometry: int, symbol: int, previous: int, phase: int
) -> tuple[int, int]:
    """Principled fix aimed at E's residual mod-7 lane rail (campaign §8).

    Keep E's nonlinear lane (it cleared the frozen 42-family at one-byte depth).
    Rebuild amount from a linear form over GF(7) whose symbol coefficient is
    nonzero mod 7, so when Δlane ≡ 0 (mod 7) we still have:

        Δamount_source ≡ c_symbol * Δsymbol  (mod 7)

    with c_symbol ≠ 0. Then amount matches only if Δsymbol ≡ 0 (mod 7).
    The four known E residuals all have Δsymbol ≠ 0 (mod 7), so they cannot
    share amount under that condition. Axis still mixes symbol into the
    selector so even Δsymbol ≡ 0 (mod 7) is not free.

    Coefficients 3,5,1,2,4,6 are all nonzero mod 7. This is still primitive-free.
    """
    lane = u8(rotl8(control ^ symbol, phase) + (probe ^ geometry) + phase * 17)
    lane = u8(lane ^ rotl8(lane, 3) ^ rotl8(symbol, 5))
    selector = (
        lane
        ^ rotl8(probe, phase & 7)
        ^ rotl8(symbol, phase)
        ^ rotl8(control, 3)
    )
    axis = axis_from_selector(previous, selector)
    # Integer sum then % 7; coefficients chosen nonzero mod 7.
    amount_source = (
        3 * lane
        + 5 * symbol
        + 1 * control
        + 2 * probe
        + 4 * geometry
        + 6 * phase
        + 3 * axis
    )
    amount = amount_mod7(amount_source)
    return axis, amount


def mul_odd(value: int, odd: int) -> int:
    """Multiplication by an odd constant is a bijection on Z/256Z."""
    return (value * odd) & 0xFF


class StructuralController:
    """Full absorb-hook controller (not only axis/amount from fixed control)."""

    name: str = "structural"

    def init_control(
        self, symbol: int, symbol_index: int, cursor: tuple[int, int, int]
    ) -> int:
        raise NotImplementedError

    def choose(
        self,
        control: int,
        probe: int,
        geometry: int,
        symbol: int,
        previous: int,
        phase: int,
    ) -> tuple[int, int]:
        raise NotImplementedError

    def evolve_control(
        self,
        control: int,
        axis: int,
        amount: int,
        probe_after: int,
        cursor: tuple[int, int, int],
        phase: int,
        symbol: int,
    ) -> int:
        # Default: RotHash-1 evolution (kept unless a design changes it).
        return u8(
            rotl8(control, 1 + (axis & 1))
            + probe_after
            + amount
            + coordinate_code(cursor)
            + phase * 7
        )


class ControllerH(StructuralController):
    """Structural competitive-path prototype (requirements S1–S5).

    - Control init is a nonlinear injective-friendly mix of symbol (not s+k only).
    - Amount uses odd multiplications on Z/256Z then a balanced residue map
      (not a linear form over GF(7) alone).
    - Axis selector mixes symbol every phase with rotates (not probe-only LSB).
    Still primitive-free: only XOR, rotates, adds, odd muls, fixed arithmetic.
    """

    name = "H"

    def init_control(
        self, symbol: int, symbol_index: int, cursor: tuple[int, int, int]
    ) -> int:
        c = coordinate_code(cursor)
        # Nonlinear in symbol; avoids control = symbol + k affine rail.
        return u8(
            rotl8(symbol, 3)
            ^ rotl8(symbol, 1)
            ^ mul_odd(symbol, 5)
            ^ u8(symbol_index)
            ^ c
            ^ rotl8(c, 2)
        )

    def choose(
        self,
        control: int,
        probe: int,
        geometry: int,
        symbol: int,
        previous: int,
        phase: int,
    ) -> tuple[int, int]:
        # Wide mix for axis; symbol appears with rotates every phase.
        sel = (
            rotl8(symbol, phase)
            ^ rotl8(control, (phase + 1) & 7)
            ^ rotl8(probe, 2)
            ^ geometry
            ^ u8(previous * 0x1D)
            ^ u8(phase * 0x3B)
        )
        sel = u8(sel ^ rotl8(sel, 3) ^ mul_odd(symbol, 9))
        axis = axis_from_selector(previous, sel)

        # Byte mix bijective components, then balanced map to {1..7}.
        lane = u8(
            mul_odd(symbol, 73)
            ^ mul_odd(control, 45)
            ^ mul_odd(probe, 29)
            ^ rotl8(geometry, phase & 7)
            ^ u8(phase * 19)
            ^ u8(axis * 11)
        )
        lane = u8(lane + rotl8(lane, 4) + rotl8(symbol ^ control, 2))
        # Map u8 -> {0..6} without "sum of fields mod 7" identity.
        residue = (mul_odd(lane, 41) ^ (lane >> 3) ^ (lane >> 5) ^ phase) % 7
        amount = 1 + residue
        return axis, amount

    def evolve_control(
        self,
        control: int,
        axis: int,
        amount: int,
        probe_after: int,
        cursor: tuple[int, int, int],
        phase: int,
        symbol: int,
    ) -> int:
        # Feed symbol back nonlinearly so later phases keep symbol sensitivity.
        return u8(
            rotl8(control, 1 + (axis & 1))
            ^ mul_odd(symbol, 3)
            ^ probe_after
            ^ u8(amount * 17)
            ^ coordinate_code(cursor)
            ^ u8(phase * 13)
        )


class ControllerH2(StructuralController):
    """H + fixed public 256→{1..7} table (S5). Table is a project constant."""

    name = "H2"

    def __init__(self) -> None:
        # Balanced: each residue 0..6 appears 36 or 37 times.
        self._amount_table = [
            1 + ((mul_odd(i, 47) ^ (i >> 2) ^ (i * 3)) % 7) for i in range(256)
        ]

    def init_control(
        self, symbol: int, symbol_index: int, cursor: tuple[int, int, int]
    ) -> int:
        return ControllerH().init_control(symbol, symbol_index, cursor)

    def choose(
        self,
        control: int,
        probe: int,
        geometry: int,
        symbol: int,
        previous: int,
        phase: int,
    ) -> tuple[int, int]:
        sel = (
            rotl8(symbol, phase)
            ^ rotl8(control, (phase + 2) & 7)
            ^ probe
            ^ rotl8(geometry, 1)
            ^ u8(phase * 0x51)
        )
        sel = u8(sel ^ mul_odd(sel, 11) ^ symbol)
        axis = axis_from_selector(previous, sel)
        idx = u8(
            mul_odd(symbol, 73)
            ^ mul_odd(control, 19)
            ^ rotl8(probe, phase & 7)
            ^ geometry
            ^ u8(axis * 13)
            ^ u8(phase * 7)
        )
        amount = self._amount_table[idx]
        return axis, amount

    def evolve_control(
        self,
        control: int,
        axis: int,
        amount: int,
        probe_after: int,
        cursor: tuple[int, int, int],
        phase: int,
        symbol: int,
    ) -> int:
        return ControllerH().evolve_control(
            control, axis, amount, probe_after, cursor, phase, symbol
        )


class ControllerH3(StructuralController):
    """Over-hardened H residual fix (axis+amount). Regressed G2/G3; kept for log."""

    name = "H3"

    def init_control(
        self, symbol: int, symbol_index: int, cursor: tuple[int, int, int]
    ) -> int:
        return ControllerH().init_control(symbol, symbol_index, cursor)

    def choose(
        self,
        control: int,
        probe: int,
        geometry: int,
        symbol: int,
        previous: int,
        phase: int,
    ) -> tuple[int, int]:
        sel = (
            rotl8(symbol, phase)
            ^ rotl8(control, (phase + 1) & 7)
            ^ rotl8(probe, 2)
            ^ geometry
            ^ u8(previous * 0x1D)
            ^ u8(phase * 0x3B)
        )
        sel = u8(
            sel
            ^ rotl8(sel, 3)
            ^ mul_odd(symbol, 9)
            ^ mul_odd(control, 3)
            ^ rotl8(symbol ^ control, 5)
        )
        axis = axis_from_selector(previous, sel)

        lane = u8(
            mul_odd(symbol, 73)
            ^ mul_odd(control, 45)
            ^ mul_odd(probe, 29)
            ^ rotl8(geometry, phase & 7)
            ^ u8(phase * 19)
            ^ u8(axis * 11)
        )
        lane2 = u8(lane + rotl8(lane, 4) + rotl8(symbol ^ control, 2))
        residue = (
            mul_odd(lane2, 41)
            ^ mul_odd(symbol, 13)
            ^ mul_odd(control, 21)
            ^ rotl8(symbol, phase)
            ^ (lane2 >> 3)
            ^ (lane2 >> 5)
            ^ phase
        ) % 7
        amount = 1 + residue
        return axis, amount

    def evolve_control(
        self,
        control: int,
        axis: int,
        amount: int,
        probe_after: int,
        cursor: tuple[int, int, int],
        phase: int,
        symbol: int,
    ) -> int:
        return ControllerH().evolve_control(
            control, axis, amount, probe_after, cursor, phase, symbol
        )


class ControllerH4(StructuralController):
    """Minimal H fix: same axis as H; amount residue includes symbol once.

    H residuals matched amount because residue depended only on lane2 (and
    phase). Adding mul_odd(symbol, 13) splits 58/c5 and 6e/c6 on traced
    prefixes without touching the axis path that already passed G1/G2.
    G3 worsened (337/5); kept as experiment log.
    """

    name = "H4"

    def init_control(
        self, symbol: int, symbol_index: int, cursor: tuple[int, int, int]
    ) -> int:
        return ControllerH().init_control(symbol, symbol_index, cursor)

    def choose(
        self,
        control: int,
        probe: int,
        geometry: int,
        symbol: int,
        previous: int,
        phase: int,
    ) -> tuple[int, int]:
        # Axis path identical to ControllerH.
        sel = (
            rotl8(symbol, phase)
            ^ rotl8(control, (phase + 1) & 7)
            ^ rotl8(probe, 2)
            ^ geometry
            ^ u8(previous * 0x1D)
            ^ u8(phase * 0x3B)
        )
        sel = u8(sel ^ rotl8(sel, 3) ^ mul_odd(symbol, 9))
        axis = axis_from_selector(previous, sel)

        lane = u8(
            mul_odd(symbol, 73)
            ^ mul_odd(control, 45)
            ^ mul_odd(probe, 29)
            ^ rotl8(geometry, phase & 7)
            ^ u8(phase * 19)
            ^ u8(axis * 11)
        )
        lane2 = u8(lane + rotl8(lane, 4) + rotl8(symbol ^ control, 2))
        residue = (
            mul_odd(lane2, 41)
            ^ mul_odd(symbol, 13)
            ^ (lane2 >> 3)
            ^ (lane2 >> 5)
            ^ phase
        ) % 7
        amount = 1 + residue
        return axis, amount

    def evolve_control(
        self,
        control: int,
        axis: int,
        amount: int,
        probe_after: int,
        cursor: tuple[int, int, int],
        phase: int,
        symbol: int,
    ) -> int:
        return ControllerH().evolve_control(
            control, axis, amount, probe_after, cursor, phase, symbol
        )


class ControllerH5(StructuralController):
    """H axis; amount = 1 + mul_odd(lane2 XOR symbol, 41) % 7 (single XOR fold)."""

    name = "H5"

    def init_control(
        self, symbol: int, symbol_index: int, cursor: tuple[int, int, int]
    ) -> int:
        return ControllerH().init_control(symbol, symbol_index, cursor)

    def choose(
        self,
        control: int,
        probe: int,
        geometry: int,
        symbol: int,
        previous: int,
        phase: int,
    ) -> tuple[int, int]:
        sel = (
            rotl8(symbol, phase)
            ^ rotl8(control, (phase + 1) & 7)
            ^ rotl8(probe, 2)
            ^ geometry
            ^ u8(previous * 0x1D)
            ^ u8(phase * 0x3B)
        )
        sel = u8(sel ^ rotl8(sel, 3) ^ mul_odd(symbol, 9))
        axis = axis_from_selector(previous, sel)

        lane = u8(
            mul_odd(symbol, 73)
            ^ mul_odd(control, 45)
            ^ mul_odd(probe, 29)
            ^ rotl8(geometry, phase & 7)
            ^ u8(phase * 19)
            ^ u8(axis * 11)
        )
        lane2 = u8(lane + rotl8(lane, 4) + rotl8(symbol ^ control, 2))
        amount = 1 + (mul_odd(lane2 ^ symbol, 41) % 7)
        return axis, amount

    def evolve_control(
        self,
        control: int,
        axis: int,
        amount: int,
        probe_after: int,
        cursor: tuple[int, int, int],
        phase: int,
        symbol: int,
    ) -> int:
        return ControllerH().evolve_control(
            control, axis, amount, probe_after, cursor, phase, symbol
        )


ControllerLike = VariantFn | StructuralController

VARIANTS: dict[str, ControllerLike] = {
    "canonical": variant_canonical,
    "A": variant_a_no_symbol_in_amount,
    "B": variant_b_xor_amount,
    "C": variant_c_dual_bit_axis,
    "D": variant_d_xor_rot_amount,
    "E": variant_e_feedback_lane,
    "E2": variant_e2_double_fold,
    "E3": variant_e3_split_control_amount,
    "F": variant_f_popcount_amount,
    "G": variant_g_mod7_rail_break,
    "H": ControllerH(),
    "H2": ControllerH2(),
    "H3": ControllerH3(),
    "H4": ControllerH4(),
    "H5": ControllerH5(),
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


def absorb_symbol(state: ProtoState, symbol: int, controller: ControllerLike) -> None:
    if isinstance(controller, StructuralController):
        control = controller.init_control(
            symbol, state.symbol_index, state.cursor
        )
        for phase in range(6):
            probe = state.cube[
                state.cursor[2] * 64 + state.cursor[1] * 8 + state.cursor[0]
            ]
            index_byte = (state.symbol_index >> ((phase & 7) * 8)) & 0xFF
            geometry = u8(coordinate_code(state.cursor) + phase * 29 + index_byte)
            axis, amount = controller.choose(
                control, probe, geometry, symbol, state.previous_axis, phase
            )
            rotate_line(state.cube, axis, state.cursor, amount)
            state.cursor = advance(state.cursor, axis, amount)
            probe_after = state.cube[
                state.cursor[2] * 64 + state.cursor[1] * 8 + state.cursor[0]
            ]
            control = controller.evolve_control(
                control,
                axis,
                amount,
                probe_after,
                state.cursor,
                phase,
                symbol,
            )
            state.previous_axis = axis
        state.symbol_index += 1
        return

    # Legacy VariantFn path (axis/amount only; RotHash-1 control rails).
    variant = controller
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


def count_one_byte_context_aliases(
    controller: ControllerLike,
) -> tuple[int, list[tuple]]:
    """Match scripts/controller_alias_analysis.py domain: first then second symbol."""
    found: list[tuple] = []
    for first in range(256):
        base = initial_state()
        absorb_symbol(base, first, controller)
        keys: dict[tuple, int] = {}
        for symbol in range(256):
            st = ProtoState(
                cube=list(base.cube),
                cursor=base.cursor,
                previous_axis=base.previous_axis,
                symbol_index=base.symbol_index,
            )
            absorb_symbol(st, symbol, controller)
            key = state_key(st)
            if key in keys:
                found.append((first, keys[key], symbol))
            else:
                keys[key] = symbol
    return len(found), found


def count_initial_one_symbol_aliases(controller: ControllerLike) -> int:
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
        absorb_symbol(st, symbol, controller)
        key = state_key(st)
        if key in keys:
            collisions += 1
        else:
            keys[key] = symbol
    return collisions


def count_aliases_after_prefix(
    controller: ControllerLike, prefix: bytes
) -> list[tuple[int, int]]:
    base = initial_state()
    for byte in prefix:
        absorb_symbol(base, byte, controller)
    keys: dict[tuple, int] = {}
    pairs: list[tuple[int, int]] = []
    for symbol in range(256):
        st = ProtoState(
            cube=list(base.cube),
            cursor=base.cursor,
            previous_axis=base.previous_axis,
            symbol_index=base.symbol_index,
        )
        absorb_symbol(st, symbol, controller)
        key = state_key(st)
        if key in keys:
            pairs.append((keys[key], symbol))
        else:
            keys[key] = symbol
    return pairs


def sample_two_byte_context_aliases(
    controller: ControllerLike, sample_prefixes: int
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
        pairs = count_aliases_after_prefix(controller, prefix)
        total_pairs += len(pairs)
        for left, right in pairs[:2]:
            if len(examples) < 8:
                examples.append((prefix[0], prefix[1], left, right))
        tested += 1
    return tested, total_pairs, examples


def full_two_byte_alias_catalogue(
    controller: ControllerLike,
) -> tuple[int, object]:
    """Exact one-symbol aliases after every two-byte prefix (slow, ~10–15 min)."""
    from collections import Counter

    pair_counts: Counter = Counter()
    total = 0
    for counter in range(65536):
        if counter % 8192 == 0:
            print(f"  two_byte_full progress {counter}/65536", flush=True)
        prefix = bytes(((counter >> 8) & 0xFF, counter & 0xFF))
        for left, right in count_aliases_after_prefix(controller, prefix):
            pair_counts[(left, right)] += 1
            total += 1
    return total, pair_counts


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument(
        "--variants",
        default="H,H2",
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
        controller = VARIANTS[name]
        initial_collisions = count_initial_one_symbol_aliases(controller)
        print(f"\nvariant={name}")
        print(f"  G1_initial_one_symbol_alias_pairs={initial_collisions}")
        if args.deep:
            n, samples = count_one_byte_context_aliases(controller)
            print(f"  G2_one_byte_context_alias_pairs={n}")
            for sample in samples[:6]:
                print(f"    example context={sample[0]:02x} {sample[1]:02x}/{sample[2]:02x}")
            if n > 6:
                print(f"    ... {n - 6} more")
        if args.two_byte_samples > 0:
            tested, pairs, examples = sample_two_byte_context_aliases(
                controller, args.two_byte_samples
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
            total, pair_counts = full_two_byte_alias_catalogue(controller)
            print(f"  G3_two_byte_full_alias_instances={total}")
            print(f"  G3_two_byte_full_unique_pairs={len(pair_counts)}")
            for (left, right), count in pair_counts.most_common():
                print(
                    f"    pair={left:02x}/{right:02x} "
                    f"delta={((right - left) & 0xFF)} count={count}"
                )
            g1 = initial_collisions == 0
            g2 = (not args.deep) or (n == 0)
            g3 = total == 0
            print(
                f"  gates: G1={'PASS' if g1 else 'FAIL'} "
                f"G2={'PASS' if g2 else 'FAIL' if args.deep else 'skip'} "
                f"G3={'PASS' if g3 else 'FAIL'}"
            )

    print("\ninterpretation:")
    print("  - competitive path: G1∧G2∧G3 must PASS (docs/CONTROLLER_REQUIREMENTS.md)")
    print("  - canonical fails G2/G3; E/G fail G3; H/H2 are structural attempts")
    print("  - meeting gates is necessary, not a security proof")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
