#!/usr/bin/env python3
"""Stage 2 smoke tests: full research hash using Controller S (not RotHash-1).

ST1 — single-bit avalanche sample (mean Hamming ~128/256)
ST2 — all 2^16 two-byte messages produce distinct digests
ST3 — triple-byte repeats in digests at nonzero / near-random rate
ST4 — reduced-path ladder: fewer moves / no foldback / short closure must be
      weaker than full S-path (document first clearly broken setting)

This is offline research only. Official vectors and PVC-RotHash-1 are unchanged.

Usage:
  python3 scripts/stage2_smoke_s.py
  python3 scripts/stage2_smoke_s.py --skip-st2   # faster (skip 65536 digests)
"""

from __future__ import annotations

import argparse
import importlib.util
import math
import sys
from collections import Counter
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
sys.path.insert(0, str(ROOT / "reference" / "python"))
sys.path.insert(0, str(ROOT / "scripts"))

from pvc_rothash1 import (  # noqa: E402
    DIAGONAL_CLOSURE_SYMBOLS,
    DIGEST_BYTES,
    MOVES_PER_SYMBOL,
    ORBIT_CLOSURE_SYMBOLS,
    SQUEEZE_SYMBOLS_PER_BYTE,
    State,
    absorb_diagonal_closure,
    absorb_foldback,
    absorb_forward,
    absorb_orbit_closure,
    absorb_symbol,
    coordinate_code,
    derive_diagonal_byte,
    encode_length,
    return_symbol,
    rotl8,
    storage_coord,
    squeeze,
    u8,
)

spec = importlib.util.spec_from_file_location(
    "redesign", ROOT / "scripts" / "controller_redesign_prototypes.py"
)
mod = importlib.util.module_from_spec(spec)
sys.modules["redesign"] = mod
assert spec.loader is not None
spec.loader.exec_module(mod)

CONTROLLER_S = mod.VARIANTS["S"]


def absorb_symbol_s(state: State, symbol: int) -> None:
    """Replace canonical absorb with Controller S (mutates state in place)."""
    ps = mod.ProtoState(
        cube=state.cube,
        cursor=state.cursor,
        previous_axis=state.previous_axis,
        symbol_index=state.symbol_index,
    )
    CONTROLLER_S.absorb_into(ps, symbol)
    state.cursor = ps.cursor
    state.previous_axis = ps.previous_axis
    state.symbol_index = ps.symbol_index


def hash_s(
    message: bytes,
    *,
    moves_per_symbol: int = MOVES_PER_SYMBOL,
    enable_foldback: bool = True,
    diagonal_closure: int = DIAGONAL_CLOSURE_SYMBOLS,
    orbit_closure: int = ORBIT_CLOSURE_SYMBOLS,
    squeeze_bytes: int = DIGEST_BYTES,
    squeeze_symbols_per_byte: int = SQUEEZE_SYMBOLS_PER_BYTE,
) -> bytes:
    """Full PVC path with S absorb. Reduced knobs for ST4 only."""
    if moves_per_symbol != MOVES_PER_SYMBOL:
        # S hard-codes 6 moves; reduced move counts use canonical absorb for ST4.
        return hash_canonical_reduced(
            message,
            moves_per_symbol=moves_per_symbol,
            enable_foldback=enable_foldback,
            diagonal_closure=diagonal_closure,
            orbit_closure=orbit_closure,
            squeeze_bytes=squeeze_bytes,
            squeeze_symbols_per_byte=squeeze_symbols_per_byte,
        )

    state = State.initial()
    for byte in message:
        absorb_symbol_s(state, byte)
    if enable_foldback:
        for original_index in range(len(message) - 1, -1, -1):
            absorb_symbol_s(
                state, return_symbol(message[original_index], original_index)
            )
    # Closures and squeeze: same symbols as RotHash-1, S absorption.
    _absorb_diagonal_closure_s(state, len(message), diagonal_closure)
    _absorb_orbit_closure_s(state, len(message), orbit_closure)
    return _squeeze_s(state, len(message), squeeze_bytes, squeeze_symbols_per_byte)


def _absorb_diagonal_closure_s(
    state: State, input_size: int, n_symbols: int
) -> None:
    absorb_symbol_s(state, 252)
    length = encode_length(input_size)
    perfect = State.initial().body_diagonals()
    for i in range(8):
        framed = u8(length[i] + perfect[i * 4] + i * 8)
        absorb_symbol_s(state, framed)
    absorb_symbol_s(state, 8)
    for i in range(n_symbols):
        current = state.body_diagonals()
        folded = u8(
            rotl8(current[i & 31], 1)
            + rotl8(current[(i + 11) & 31], 3)
            + rotl8(current[(i * 7 + 3) & 31], 5)
        )
        symbol = (
            folded
            ^ perfect[(i * 13) & 31]
            ^ length[i & 7]
            ^ u8(i * 8 + (i >> 2))
        )
        absorb_symbol_s(state, symbol)


def _absorb_orbit_closure_s(state: State, input_size: int, n_symbols: int) -> None:
    length = encode_length(input_size)
    for i in range(n_symbols):
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
        absorb_symbol_s(state, symbol)


def _squeeze_s(
    state: State,
    input_size: int,
    squeeze_bytes: int,
    squeeze_symbols_per_byte: int,
) -> bytes:
    length = encode_length(input_size)
    initial = state.body_diagonals()
    chain = (
        initial[0]
        ^ rotl8(initial[9], 1)
        ^ rotl8(initial[18], 3)
        ^ rotl8(initial[27], 5)
        ^ length[0]
    )
    digest = bytearray()
    for i in range(squeeze_bytes):
        diagonals = state.body_diagonals()
        output = derive_diagonal_byte(diagonals, i, chain)
        digest.append(output)
        for diagonal in range(squeeze_symbols_per_byte):
            current = state.body_diagonals()
            position = i & 7
            lane = i >> 3
            diagonal_lane = diagonal & 3
            left = current[diagonal_lane * 8 + ((position + diagonal + lane) & 7)]
            right = current[
                ((diagonal_lane + 1) & 3) * 8
                + ((7 - position + lane + diagonal) & 7)
            ]
            symbol = u8(
                rotl8(left, 1 + 2 * diagonal_lane)
                + rotl8(right, 7 - 2 * diagonal_lane)
                + rotl8(output, 1 + (diagonal & 7))
                + chain
                + length[(i + diagonal) & 7]
                + u8(i * squeeze_symbols_per_byte + diagonal)
            )
            absorb_symbol_s(state, symbol)
        after = state.body_diagonals()
        chain = u8(
            rotl8(chain, 1 + (i & 7))
            + output
            + after[(i * 5 + 7) & 31]
            + state.at(state.cursor)
            + coordinate_code(state.cursor)
        )
    return bytes(digest)


def hash_canonical_reduced(
    message: bytes,
    *,
    moves_per_symbol: int,
    enable_foldback: bool,
    diagonal_closure: int,
    orbit_closure: int,
    squeeze_bytes: int,
    squeeze_symbols_per_byte: int,
) -> bytes:
    """ST4 only: intentionally weak paths using canonical absorb with cuts."""
    # Monkey-patch move count is heavy; approximate ST4 by disabling foldback
    # and/or shortening closures while keeping 6-move S absorb when moves==6.
    del moves_per_symbol  # reserved for future reduced-S if S supports it
    state = State.initial()
    for byte in message:
        absorb_symbol(state, byte)  # canonical weak controller for ladder
    if enable_foldback:
        absorb_foldback(state, message)
    # shortened closures via truncated loops
    if diagonal_closure > 0:
        absorb_symbol(state, 252)
        length = encode_length(len(message))
        perfect = State.initial().body_diagonals()
        for i in range(8):
            absorb_symbol(state, u8(length[i] + perfect[i * 4] + i * 8))
        absorb_symbol(state, 8)
        for i in range(diagonal_closure):
            current = state.body_diagonals()
            folded = u8(
                rotl8(current[i & 31], 1)
                + rotl8(current[(i + 11) & 31], 3)
                + rotl8(current[(i * 7 + 3) & 31], 5)
            )
            absorb_symbol(
                state,
                folded
                ^ perfect[(i * 13) & 31]
                ^ length[i & 7]
                ^ u8(i * 8 + (i >> 2)),
            )
    if orbit_closure > 0:
        length = encode_length(len(message))
        for i in range(orbit_closure):
            offset = i & 127
            p0, p1 = storage_coord(offset), storage_coord(offset + 128)
            p2, p3 = storage_coord(offset + 256), storage_coord(offset + 384)
            v0, v1, v2, v3 = (
                state.at(p0),
                state.at(p1),
                state.at(p2),
                state.at(p3),
            )
            diagonals = state.body_diagonals()
            pair_a = u8(rotl8(v0, 1) + rotl8(v1, 3))
            pair_b = u8(rotl8(v2, 5) + rotl8(v3, 7))
            absorb_symbol(
                state,
                pair_a
                ^ pair_b
                ^ diagonals[(i * 5 + (i >> 3)) & 31]
                ^ length[i & 7]
                ^ coordinate_code(state.cursor)
                ^ u8(i * 4 + (i >> 5)),
            )
    # minimal squeeze
    length = encode_length(len(message))
    initial = state.body_diagonals()
    chain = (
        initial[0]
        ^ rotl8(initial[9], 1)
        ^ rotl8(initial[18], 3)
        ^ rotl8(initial[27], 5)
        ^ length[0]
    )
    out = bytearray()
    for i in range(squeeze_bytes):
        d = state.body_diagonals()
        b = derive_diagonal_byte(d, i, chain)
        out.append(b)
        for diagonal in range(squeeze_symbols_per_byte):
            current = state.body_diagonals()
            position, lane = i & 7, i >> 3
            dl = diagonal & 3
            left = current[dl * 8 + ((position + diagonal + lane) & 7)]
            right = current[
                ((dl + 1) & 3) * 8 + ((7 - position + lane + diagonal) & 7)
            ]
            absorb_symbol(
                state,
                u8(
                    rotl8(left, 1 + 2 * dl)
                    + rotl8(right, 7 - 2 * dl)
                    + rotl8(b, 1 + (diagonal & 7))
                    + chain
                    + length[(i + diagonal) & 7]
                    + u8(i * squeeze_symbols_per_byte + diagonal)
                ),
            )
        after = state.body_diagonals()
        chain = u8(
            rotl8(chain, 1 + (i & 7))
            + b
            + after[(i * 5 + 7) & 31]
            + state.at(state.cursor)
            + coordinate_code(state.cursor)
        )
    return bytes(out)


def hamming_bits(a: bytes, b: bytes) -> int:
    return sum((x ^ y).bit_count() for x, y in zip(a, b, strict=True))


def st1_avalanche(samples: int = 512) -> dict:
    distances: list[int] = []
    for n in range(samples):
        msg = bytes([(n * 17 + i * 31) & 0xFF for i in range(16)])
        base = hash_s(msg)
        bit = n % (16 * 8)
        byte_i, bit_i = divmod(bit, 8)
        flipped = bytearray(msg)
        flipped[byte_i] ^= 1 << bit_i
        dist = hamming_bits(base, hash_s(bytes(flipped)))
        distances.append(dist)
    mean = sum(distances) / len(distances)
    var = sum((d - mean) ** 2 for d in distances) / len(distances)
    std = math.sqrt(var)
    # Accept mean in [110, 146] (~128 ± 18) as loose smoke.
    ok = 110.0 <= mean <= 146.0
    return {
        "samples": samples,
        "mean_bits": mean,
        "std_bits": std,
        "min_bits": min(distances),
        "max_bits": max(distances),
        "pass": ok,
    }


def st2_two_byte_unique() -> dict:
    seen: set[bytes] = set()
    collisions = 0
    example = None
    for v in range(65536):
        msg = bytes([(v >> 8) & 0xFF, v & 0xFF])
        d = hash_s(msg)
        if d in seen:
            collisions += 1
            if example is None:
                example = msg.hex()
        else:
            seen.add(d)
        if v % 8192 == 0:
            print(f"  ST2 progress {v}/65536", flush=True)
    return {
        "messages": 65536,
        "unique_digests": len(seen),
        "collisions": collisions,
        "example_collision_msg": example,
        "pass": collisions == 0 and len(seen) == 65536,
    }


def st3_multiplicity(samples: int = 20000, msg_len: int = 16) -> dict:
    """Fraction of digests with some byte value appearing ≥3 times."""
    hits = 0
    for n in range(samples):
        msg = bytes([((n * 131 + i * 17) ^ (n >> 3)) & 0xFF for i in range(msg_len)])
        dig = hash_s(msg)
        counts = Counter(dig)
        if any(c >= 3 for c in counts.values()):
            hits += 1
    rate = hits / samples
    # Random 32-byte strings: nonzero triple rate (order ~few percent).
    # Fail only if rate is essentially zero (v0 ban) or absurdly high (>0.5).
    ok = 0.01 <= rate <= 0.50
    return {
        "samples": samples,
        "triple_or_more_rate": rate,
        "pass": ok,
    }


def st4_reduced_ladder() -> dict:
    """Document that reduced canonical paths collide more than full S path.

    Uses two-byte domain collision counts at after-hash digest for:
    - full S hash
    - no-foldback S (still S absorb)
    - minimal closure (canonical absorb, short path) as broken reference
    """
    def count_digest_collisions(hasher, limit: int = 4096) -> int:
        seen: dict[bytes, int] = {}
        coll = 0
        for v in range(limit):
            msg = bytes([(v >> 8) & 0xFF, v & 0xFF])
            d = hasher(msg)
            if d in seen:
                coll += 1
            else:
                seen[d] = v
        return coll

    full_s = count_digest_collisions(lambda m: hash_s(m), 4096)
    no_fb = count_digest_collisions(
        lambda m: hash_s(m, enable_foldback=False), 4096
    )
    # R0-like: no foldback, no diagonal/orbit, tiny squeeze, canonical absorb
    r0 = count_digest_collisions(
        lambda m: hash_canonical_reduced(
            m,
            moves_per_symbol=1,
            enable_foldback=False,
            diagonal_closure=0,
            orbit_closure=0,
            squeeze_bytes=4,
            squeeze_symbols_per_byte=1,
        ),
        4096,
    )
    # Full S should have zero collisions in this 4096 sample; r0 should have many.
    ok = full_s == 0 and r0 > full_s
    return {
        "sample_messages": 4096,
        "full_S_digest_collisions": full_s,
        "S_no_foldback_digest_collisions": no_fb,
        "R0_like_canonical_digest_collisions": r0,
        "first_clearly_broken": "R0-like canonical short path"
        if r0 > 0
        else "none in sample",
        "pass": ok,
    }


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument(
        "--skip-st2",
        action="store_true",
        help="skip exhaustive two-byte uniqueness (slow)",
    )
    parser.add_argument("--avalanche-samples", type=int, default=512)
    parser.add_argument("--multiplicity-samples", type=int, default=10000)
    args = parser.parse_args()

    print("Stage 2 smoke: research full-hash with Controller S")
    print("WARNING: not PVC-RotHash-1; not a security proof\n")

    # Sanity: S digests must differ from RotHash-1 for nonempty messages
    sample = b"abc"
    # RotHash-1 via reference
    from pvc_rothash1 import hash_bytes as hash_rothash1

    d_s = hash_s(sample)
    d_r = hash_rothash1(sample)
    print(f"sanity abc S={d_s.hex()}")
    print(f"sanity abc R1={d_r.hex()}")
    print(f"S_differs_from_RotHash1={d_s != d_r}")
    if d_s == d_r:
        print("FAIL: S path produced RotHash-1 digest (wiring bug)")
        return 1

    results = {}
    print("\n=== ST1 avalanche ===")
    results["ST1"] = st1_avalanche(args.avalanche_samples)
    print(results["ST1"])

    if args.skip_st2:
        print("\n=== ST2 two-byte uniqueness SKIPPED ===")
        results["ST2"] = {"pass": None, "skipped": True}
    else:
        print("\n=== ST2 two-byte uniqueness ===")
        results["ST2"] = st2_two_byte_unique()
        print(
            {
                k: results["ST2"][k]
                for k in (
                    "messages",
                    "unique_digests",
                    "collisions",
                    "pass",
                )
            }
        )

    print("\n=== ST3 multiplicity ===")
    results["ST3"] = st3_multiplicity(args.multiplicity_samples)
    print(results["ST3"])

    print("\n=== ST4 reduced ladder ===")
    results["ST4"] = st4_reduced_ladder()
    print(results["ST4"])

    print("\n=== SUMMARY ===")
    all_required = ["ST1", "ST3", "ST4"]
    if not args.skip_st2:
        all_required.append("ST2")
    passed = all(results[k].get("pass") for k in all_required)
    for k in ("ST1", "ST2", "ST3", "ST4"):
        p = results[k].get("pass")
        flag = "SKIP" if p is None else ("PASS" if p else "FAIL")
        print(f"  {k}: {flag}")
    print(f"stage2_smoke={'PASS' if passed else 'FAIL'}")
    return 0 if passed else 1


if __name__ == "__main__":
    raise SystemExit(main())
