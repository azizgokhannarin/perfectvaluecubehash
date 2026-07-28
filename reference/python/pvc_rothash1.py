#!/usr/bin/env python3
"""Independent pure-Python reference for PVC-RotHash-1 candidate 1.0.0-rc1.

This file intentionally uses only Python's standard language features. It is
written from the normative specification and exists to cross-check the C++
implementation. It is not optimized and must not be used for production.
"""

from __future__ import annotations

import argparse
from dataclasses import dataclass
from pathlib import Path
from typing import Iterable, Sequence

SIDE = 8
CELLS = 512
DIGEST_BYTES = 32
MOVES_PER_SYMBOL = 6
DIAGONAL_CLOSURE_SYMBOLS = 64
ORBIT_CLOSURE_SYMBOLS = 128
SQUEEZE_SYMBOLS_PER_BYTE = 4

X, Y, Z = 0, 1, 2

PERFECT_CUBE: tuple[int, ...] = (
    68,69,182,183,184,185,74,75,
    84,85,169,168,167,166,90,91,
    107,154,102,152,151,105,149,100,
    123,138,137,119,120,134,133,116,
    139,122,121,135,136,118,117,132,
    155,106,150,104,103,153,101,148,
    164,165,89,88,87,86,170,171,
    180,181,70,71,72,73,186,187,

    64,65,178,179,188,189,78,79,
    80,81,173,172,163,162,94,95,
    111,158,98,156,147,109,145,96,
    127,142,141,115,124,130,129,112,
    143,126,125,131,140,114,113,128,
    159,110,146,108,99,157,97,144,
    160,161,93,92,83,82,174,175,
    176,177,66,67,76,77,190,191,

    4,5,246,247,248,249,10,11,
    20,21,233,232,231,230,26,27,
    43,218,38,216,215,41,213,36,
    59,202,201,55,56,198,197,52,
    203,58,57,199,200,54,53,196,
    219,42,214,40,39,217,37,212,
    228,229,25,24,23,22,234,235,
    244,245,6,7,8,9,250,251,

    0,1,242,243,252,253,14,15,
    16,17,237,236,227,226,30,31,
    47,222,34,220,211,45,209,32,
    63,206,205,51,60,194,193,48,
    207,62,61,195,204,50,49,192,
    223,46,210,44,35,221,33,208,
    224,225,29,28,19,18,238,239,
    240,241,2,3,12,13,254,255,

    255,254,13,12,3,2,241,240,
    239,238,18,19,28,29,225,224,
    208,33,221,35,44,210,46,223,
    192,49,50,204,195,61,62,207,
    48,193,194,60,51,205,206,63,
    32,209,45,211,220,34,222,47,
    31,30,226,227,236,237,17,16,
    15,14,253,252,243,242,1,0,

    251,250,9,8,7,6,245,244,
    235,234,22,23,24,25,229,228,
    212,37,217,39,40,214,42,219,
    196,53,54,200,199,57,58,203,
    52,197,198,56,55,201,202,59,
    36,213,41,215,216,38,218,43,
    27,26,230,231,232,233,21,20,
    11,10,249,248,247,246,5,4,

    191,190,77,76,67,66,177,176,
    175,174,82,83,92,93,161,160,
    144,97,157,99,108,146,110,159,
    128,113,114,140,131,125,126,143,
    112,129,130,124,115,141,142,127,
    96,145,109,147,156,98,158,111,
    95,94,162,163,172,173,81,80,
    79,78,189,188,179,178,65,64,

    187,186,73,72,71,70,181,180,
    171,170,86,87,88,89,165,164,
    148,101,153,103,104,150,106,155,
    132,117,118,136,135,121,122,139,
    116,133,134,120,119,137,138,123,
    100,149,105,151,152,102,154,107,
    91,90,166,167,168,169,85,84,
    75,74,185,184,183,182,69,68,
)

if len(PERFECT_CUBE) != CELLS:
    raise RuntimeError("canonical cube must contain 512 bytes")


def u8(value: int) -> int:
    return value & 0xFF


def rotl8(value: int, amount: int) -> int:
    shift = amount & 7
    value &= 0xFF
    if shift == 0:
        return value
    return ((value << shift) | (value >> (8 - shift))) & 0xFF


def index(coord: tuple[int, int, int]) -> int:
    x, y, z = coord
    return z * 64 + y * 8 + x


def storage_coord(i: int) -> tuple[int, int, int]:
    return i & 7, (i >> 3) & 7, (i >> 6) & 7


def coordinate_code(coord: tuple[int, int, int]) -> int:
    x, y, z = coord
    return ((x << 5) ^ (y << 2) ^ z) & 0xFF


def choose_other_axis(previous: int, selector: int) -> int:
    second = bool(selector & 1)
    if previous == X:
        return Z if second else Y
    if previous == Y:
        return X if second else Z
    return Y if second else X


def advance(coord: tuple[int, int, int], axis: int, amount: int) -> tuple[int, int, int]:
    x, y, z = coord
    amount %= 8
    if axis == X:
        x = (x + amount) & 7
    elif axis == Y:
        y = (y + amount) & 7
    else:
        z = (z + amount) & 7
    return x, y, z


@dataclass
class State:
    cube: list[int]
    cursor: tuple[int, int, int] = (0, 0, 0)
    previous_axis: int = X
    symbol_index: int = 0

    @classmethod
    def initial(cls) -> "State":
        return cls(list(PERFECT_CUBE))

    def at(self, coord: tuple[int, int, int]) -> int:
        return self.cube[index(coord)]

    def rotate_line(self, axis: int, point: tuple[int, int, int], amount: int) -> None:
        shift = amount & 7
        if shift == 0:
            return
        x0, y0, z0 = point
        coords: list[tuple[int, int, int]] = []
        for i in range(8):
            if axis == X:
                coords.append((i, y0, z0))
            elif axis == Y:
                coords.append((x0, i, z0))
            else:
                coords.append((x0, y0, i))
        old = [self.at(c) for c in coords]
        for i, value in enumerate(old):
            self.cube[index(coords[(i + shift) & 7])] = value

    def body_diagonals(self) -> list[int]:
        out: list[int] = []
        for i in range(8):
            out.append(self.at((i, i, i)))
        for i in range(8):
            out.append(self.at((7 - i, i, i)))
        for i in range(8):
            out.append(self.at((i, 7 - i, i)))
        for i in range(8):
            out.append(self.at((i, i, 7 - i)))
        return out


def absorb_symbol(state: State, symbol: int) -> None:
    control = u8(symbol + u8(state.symbol_index) + coordinate_code(state.cursor))
    for phase in range(MOVES_PER_SYMBOL):
        probe_before = state.at(state.cursor)
        index_byte = (state.symbol_index >> ((phase & 7) * 8)) & 0xFF
        geometry = u8(coordinate_code(state.cursor) + phase * 29 + index_byte)
        selector = rotl8(control, phase) ^ probe_before ^ geometry
        axis = choose_other_axis(state.previous_axis, selector)
        amount_source = control + probe_before + geometry + symbol + axis * 11
        amount = 1 + (amount_source % 7)
        state.rotate_line(axis, state.cursor, amount)
        state.cursor = advance(state.cursor, axis, amount)
        probe_after = state.at(state.cursor)
        control = u8(
            rotl8(control, 1 + (axis & 1))
            + probe_after
            + amount
            + coordinate_code(state.cursor)
            + phase * 7
        )
        state.previous_axis = axis
    state.symbol_index += 1


def encode_length(size: int) -> list[int]:
    if size < 0 or size >= 1 << 64:
        raise ValueError("message length must fit in uint64")
    return [(size >> (8 * i)) & 0xFF for i in range(8)]


def return_symbol(byte: int, original_index: int) -> int:
    diagonals = State.initial().body_diagonals()
    return byte ^ diagonals[original_index & 31] ^ u8(original_index * 8 + (original_index >> 3))


def absorb_forward(state: State, message: bytes) -> None:
    for byte in message:
        absorb_symbol(state, byte)


def absorb_foldback(state: State, message: bytes) -> None:
    for original_index in range(len(message) - 1, -1, -1):
        absorb_symbol(state, return_symbol(message[original_index], original_index))


def absorb_diagonal_closure(state: State, input_size: int) -> None:
    absorb_symbol(state, 252)
    length = encode_length(input_size)
    perfect_diagonals = State.initial().body_diagonals()
    for i in range(8):
        framed = u8(length[i] + perfect_diagonals[i * 4] + i * 8)
        absorb_symbol(state, framed)
    absorb_symbol(state, 8)
    for i in range(DIAGONAL_CLOSURE_SYMBOLS):
        current = state.body_diagonals()
        folded = u8(
            rotl8(current[i & 31], 1)
            + rotl8(current[(i + 11) & 31], 3)
            + rotl8(current[(i * 7 + 3) & 31], 5)
        )
        symbol = folded ^ perfect_diagonals[(i * 13) & 31] ^ length[i & 7] ^ u8(i * 8 + (i >> 2))
        absorb_symbol(state, symbol)


def absorb_orbit_closure(state: State, input_size: int) -> None:
    length = encode_length(input_size)
    for i in range(ORBIT_CLOSURE_SYMBOLS):
        offset = i & 127
        p0 = storage_coord(offset)
        p1 = storage_coord(offset + 128)
        p2 = storage_coord(offset + 256)
        p3 = storage_coord(offset + 384)
        v0, v1, v2, v3 = state.at(p0), state.at(p1), state.at(p2), state.at(p3)
        diagonals = state.body_diagonals()
        pair_a = u8(rotl8(v0, 1) + rotl8(v1, 3))
        pair_b = u8(rotl8(v2, 5) + rotl8(v3, 7))
        symbol = (
            pair_a
            ^ pair_b
            ^ diagonals[(i * 5 + (i >> 3)) & 31]
            ^ length[i & 7]
            ^ coordinate_code(state.cursor)
            ^ u8(i * 4 + (i >> 5))
        )
        absorb_symbol(state, symbol)


def derive_diagonal_byte(diagonals: Sequence[int], output_index: int, chain: int) -> int:
    lane = output_index >> 3
    position = output_index & 7
    a = diagonals[((lane + 0) & 3) * 8 + position]
    b = diagonals[((lane + 1) & 3) * 8 + ((position + 1 + lane) & 7)]
    c = diagonals[((lane + 2) & 3) * 8 + ((position + 3 + 2 * lane) & 7)]
    d = diagonals[((lane + 3) & 3) * 8 + ((position + 5 + 3 * lane) & 7)]
    pair_a = u8(rotl8(a, 1) + rotl8(b, 3))
    pair_b = u8(rotl8(c, 5) + rotl8(d, 7))
    cross = u8(diagonals[(output_index * 7 + 3) & 31] + diagonals[(output_index * 13 + 1) & 31])
    return u8(
        rotl8(pair_a ^ pair_b ^ chain, 1 + position)
        + rotl8(cross, 1 + lane * 2)
        + u8(output_index * 8 + lane)
    )


def squeeze(state: State, input_size: int) -> bytes:
    length = encode_length(input_size)
    initial = state.body_diagonals()
    chain = initial[0] ^ rotl8(initial[9], 1) ^ rotl8(initial[18], 3) ^ rotl8(initial[27], 5) ^ length[0]
    digest = bytearray()
    for i in range(DIGEST_BYTES):
        diagonals = state.body_diagonals()
        output = derive_diagonal_byte(diagonals, i, chain)
        digest.append(output)
        for diagonal in range(SQUEEZE_SYMBOLS_PER_BYTE):
            current = state.body_diagonals()
            position = i & 7
            lane = i >> 3
            diagonal_lane = diagonal & 3
            left = current[diagonal_lane * 8 + ((position + diagonal + lane) & 7)]
            right = current[((diagonal_lane + 1) & 3) * 8 + ((7 - position + lane + diagonal) & 7)]
            symbol = u8(
                rotl8(left, 1 + 2 * diagonal_lane)
                + rotl8(right, 7 - 2 * diagonal_lane)
                + rotl8(output, 1 + (diagonal & 7))
                + chain
                + length[(i + diagonal) & 7]
                + u8(i * SQUEEZE_SYMBOLS_PER_BYTE + diagonal)
            )
            absorb_symbol(state, symbol)
        after = state.body_diagonals()
        chain = u8(
            rotl8(chain, 1 + (i & 7))
            + output
            + after[(i * 5 + 7) & 31]
            + state.at(state.cursor)
            + coordinate_code(state.cursor)
        )
    return bytes(digest)


def _state_record(state: State) -> dict[str, object]:
    return {
        "cube_hex": bytes(state.cube).hex(),
        "cursor": list(state.cursor),
        "previous_axis": ("X", "Y", "Z")[state.previous_axis],
        "symbol_index": state.symbol_index,
    }


def inspect(message: bytes) -> dict[str, object]:
    state = State.initial()
    absorb_forward(state, message)
    after_forward = _state_record(state)
    absorb_foldback(state, message)
    after_foldback = _state_record(state)
    absorb_diagonal_closure(state, len(message))
    after_diagonal_closure = _state_record(state)
    absorb_orbit_closure(state, len(message))
    after_orbit_closure = _state_record(state)
    digest = squeeze(state, len(message))
    return {
        "input_hex": message.hex(),
        "input_length": len(message),
        "digest_hex": digest.hex(),
        "after_forward": after_forward,
        "after_foldback": after_foldback,
        "after_diagonal_closure": after_diagonal_closure,
        "after_orbit_closure": after_orbit_closure,
        "final": _state_record(state),
    }


def hash_bytes(message: bytes) -> bytes:
    return bytes.fromhex(str(inspect(message)["digest_hex"]))


def hash_hex(message: bytes) -> str:
    return hash_bytes(message).hex()


def _parse_args() -> argparse.Namespace:
    parser = argparse.ArgumentParser(description="PVC-RotHash-1 pure-Python reference")
    source = parser.add_mutually_exclusive_group(required=True)
    source.add_argument("--text", help="UTF-8 text input")
    source.add_argument("--hex", dest="hex_input", help="hexadecimal byte input")
    source.add_argument("--file", type=Path, help="binary file input")
    return parser.parse_args()


def main() -> int:
    args = _parse_args()
    if args.text is not None:
        message = args.text.encode("utf-8")
    elif args.hex_input is not None:
        try:
            message = bytes.fromhex(args.hex_input)
        except ValueError as error:
            raise SystemExit(f"invalid hex input: {error}") from error
    else:
        message = args.file.read_bytes()
    print(hash_hex(message))
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
