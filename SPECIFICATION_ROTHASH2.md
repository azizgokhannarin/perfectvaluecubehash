# PVC-RotHash-2 Candidate Specification

**Status:** Experimental **0.2.0-draft** public-attack freeze  
**Algorithm identifier:** `PVC-RotHash-2`  
**Candidate:** `0.2.0-draft`  
**Tag:** `v0.2.0-rothash2-draft`  
**Digest size:** 256 bits  
**Relation to RotHash-1:** Same finalization shape; **different absorb controller**

This document is the normative absorb definition for PVC-RotHash-2. Foldback,
length framing, diagonal closure, orbit closure, and four-diagonal squeeze are
**identical** to `SPECIFICATION.md` (PVC-RotHash-1) §§10–15 except that every
`ABSORB_SYMBOL` call uses the **systematic controller** below instead of §9.

**Production use is prohibited.** No collision, preimage, or post-quantum claim.
Official digests: `test-vectors/official-v2.json`. Public challenge:
`CRYPTANALYSIS_CHALLENGE_ROTHASH2.md`.

---

## 1. Identity

| Item | Value |
|---|---|
| Cube, coordinates, diagonals | Same as RotHash-1 §§3–7 |
| Moves per symbol | 6 |
| Closure / orbit / squeeze counts | 64 / 128 / 4 per output byte |
| Symbol total for `n`-byte message | `2*n + 330` |

---

## 2. Systematic ABSORB_SYMBOL (Controller S)

Public permutation (nothing-up-my-sleeve):

```text
Π(s) = u8(s * 41 + 17)     # odd multiply is a bijection on Z/256Z
```

At the **start** of absorbing `symbol` (before any of the six moves), let:

```text
probe0 = C(cursor)
context_seed = u8(
    u8(symbol_index)
    XOR coord_code(cursor)
    XOR u8(previous_axis * 13)
    XOR u8(probe0 * 19)      # mul_odd(probe0, 19)
)

for i = 0..2:
    g_i = u8(coord_code(cursor) + 29*i + u8(symbol_index) + u8(previous_axis * 11))
    mixed_i = u8( mul_odd(probe0, 29) XOR mul_odd(context_seed, 45)
                  XOR ROTL8(g_i, i) XOR u8(19*i) )
    c_i = mixed_i % 7

t  = Π(symbol)
d0 = t % 7
d1 = (t / 7) % 7
d2 = t / 49                 # integer division; d2 in 0..5 for t in 0..255
r_i = 1 + ((d_i + c_i) % 7) for i = 0..2
```

**Critical:** `c_i` and `context_seed` **MUST NOT** depend on `symbol`.

Diffusion control init (may depend on symbol):

```text
control = u8(
    ROTL8(symbol, 3) XOR ROTL8(symbol, 1) XOR mul_odd(symbol, 5)
    XOR u8(symbol_index) XOR coord_code(cursor) XOR ROTL8(coord_code(cursor), 2)
)
```

For `phase = 0..5`:

1. Read `probe = C(cursor)`, build `geometry` as in RotHash-1 §9.
2. Axis (diffusion):

```text
sel = ROTL8(symbol, phase)
    XOR ROTL8(control, phase+1)
    XOR ROTL8(probe, 2)
    XOR geometry
    XOR u8(previous_axis * 0x1D)
    XOR u8(phase * 0x3B)
sel = u8(sel XOR ROTL8(sel, 3) XOR mul_odd(symbol, 9))
axis = CHOOSE_OTHER_AXIS(previous_axis, sel)
```

3. Amount:

```text
if phase < 3:
    amount = r_phase
else:
    lane = u8( mul_odd(symbol,73) XOR mul_odd(control,45) XOR mul_odd(probe,29)
               XOR ROTL8(geometry, phase) XOR u8(19*phase) XOR u8(11*axis) )
    lane = u8(lane + ROTL8(lane, 4) + ROTL8(symbol XOR control, 2))
    amount = 1 + ((mul_odd(lane,41) XOR (lane>>3) XOR (lane>>5) XOR phase) % 7)
```

4. `ROTATE_LINE`, `ADVANCE` cursor, update control:

```text
control = u8(
    ROTL8(control, 1+(axis&1))
    XOR mul_odd(symbol, 3)
    XOR probe_after
    XOR u8(amount * 17)
    XOR coord_code(cursor)
    XOR u8(phase * 13)
)
previous_axis = axis
```

5. After six phases: `symbol_index ← symbol_index + 1`.

`mul_odd(x, k)` means `u8(x * k)` with odd `k`.

---

## 3. Rest of the hash

Apply RotHash-1 §§10–15 with the above `ABSORB_SYMBOL`.

---

## 4. Conformance anchors

Implementations MUST match C++ `RotHash2` and Python `reference/python/pvc_rothash2.py`.

```text
M = empty
H = (see test-vectors/official-v2.json after generation)

M = 616263  ("abc")
H = 9c1b502e8eac4ea07e18265ea30f888c4d5fd8ae81fa1ed453c2c099d4d68fdb
```

---

## 5. Change policy

Any change to absorb equations creates a new candidate ID. Editorial fixes that
do not change vectors may be transparent.
