# PVC-RotHash-1 Candidate Specification 1.0.0-rc1

**Status:** Frozen candidate for public, independent cryptanalysis
**Algorithm identifier:** `PVC-RotHash-1`
**Output size:** 256 bits
**Candidate freeze date:** 2026-07-28

This document is the normative definition of PVC-RotHash-1. The C++ and Python
implementations are informative conformance implementations. If an
implementation and this document disagree, the discrepancy must be reported;
the candidate is not silently changed.

PVC-RotHash-1 is experimental. This specification does not claim collision,
second-preimage, preimage, or quantum security. It must not be used in a
production security boundary.

## 1. Conventions

The words **MUST**, **MUST NOT**, **SHOULD**, and **MAY** are normative.

- `u8(x)` means `x mod 256`, represented as an unsigned byte.
- `ROTL8(x,s)` rotates an 8-bit value left by `s mod 8` bits.
- `a XOR b` is bitwise XOR on bytes.
- All byte additions are reduced with `u8` when assigned to a byte.
- `% 7` is the ordinary non-negative remainder in `{0,...,6}`.
- Message length is the number of input bytes and MUST fit in an unsigned
  64-bit integer.
- Coordinate components are integers in `{0,...,7}`.
- Axes are encoded `X=0`, `Y=1`, and `Z=2`.
- Arrays are zero-indexed.

## 2. Canonical parameters

| Parameter | Value |
|---|---:|
| Cube side | 8 |
| Cube cells | 512 |
| Digest bytes | 32 |
| Moves per absorbed symbol | 6 |
| Diagonal-closure symbols | 64 |
| Orbit-closure symbols | 128 |
| Squeeze symbols per output byte | 4 |
| Output bytes | 32 |

The complete number of absorbed symbols for an `n`-byte message is:

```text
2*n + 330
```

and the complete number of line rotations is:

```text
6 * (2*n + 330)
```

## 3. Cube coordinates and storage

A cube cell is addressed as `C(x,y,z)`. Linear storage is normative only for
serialization and test vectors:

```text
index(x,y,z) = 64*z + 8*y + x
```

Thus `x` changes fastest, then `y`, then `z`.

The coordinate code is:

```text
coord_code(x,y,z) = u8((x << 5) XOR (y << 2) XOR z)
```

The storage coordinate for a linear index `i` is:

```text
x = i & 7
y = (i >> 3) & 7
z = (i >> 6) & 7
```

## 4. Canonical Perfect Value Cube

The initial state MUST contain the following 512 bytes. Each block is one
`z` layer; rows are increasing `y`, and values in a row are increasing `x`.

### Layer z=0

```text
 68  69 182 183 184 185  74  75
 84  85 169 168 167 166  90  91
107 154 102 152 151 105 149 100
123 138 137 119 120 134 133 116
139 122 121 135 136 118 117 132
155 106 150 104 103 153 101 148
164 165  89  88  87  86 170 171
180 181  70  71  72  73 186 187
```

### Layer z=1

```text
 64  65 178 179 188 189  78  79
 80  81 173 172 163 162  94  95
111 158  98 156 147 109 145  96
127 142 141 115 124 130 129 112
143 126 125 131 140 114 113 128
159 110 146 108  99 157  97 144
160 161  93  92  83  82 174 175
176 177  66  67  76  77 190 191
```

### Layer z=2

```text
  4   5 246 247 248 249  10  11
 20  21 233 232 231 230  26  27
 43 218  38 216 215  41 213  36
 59 202 201  55  56 198 197  52
203  58  57 199 200  54  53 196
219  42 214  40  39 217  37 212
228 229  25  24  23  22 234 235
244 245   6   7   8   9 250 251
```

### Layer z=3

```text
  0   1 242 243 252 253  14  15
 16  17 237 236 227 226  30  31
 47 222  34 220 211  45 209  32
 63 206 205  51  60 194 193  48
207  62  61 195 204  50  49 192
223  46 210  44  35 221  33 208
224 225  29  28  19  18 238 239
240 241   2   3  12  13 254 255
```

### Layer z=4

```text
255 254  13  12   3   2 241 240
239 238  18  19  28  29 225 224
208  33 221  35  44 210  46 223
192  49  50 204 195  61  62 207
 48 193 194  60  51 205 206  63
 32 209  45 211 220  34 222  47
 31  30 226 227 236 237  17  16
 15  14 253 252 243 242   1   0
```

### Layer z=5

```text
251 250   9   8   7   6 245 244
235 234  22  23  24  25 229 228
212  37 217  39  40 214  42 219
196  53  54 200 199  57  58 203
 52 197 198  56  55 201 202  59
 36 213  41 215 216  38 218  43
 27  26 230 231 232 233  21  20
 11  10 249 248 247 246   5   4
```

### Layer z=6

```text
191 190  77  76  67  66 177 176
175 174  82  83  92  93 161 160
144  97 157  99 108 146 110 159
128 113 114 140 131 125 126 143
112 129 130 124 115 141 142 127
 96 145 109 147 156  98 158 111
 95  94 162 163 172 173  81  80
 79  78 189 188 179 178  65  64
```

### Layer z=7

```text
187 186  73  72  71  70 181 180
171 170  86  87  88  89 165 164
148 101 153 103 104 150 106 155
132 117 118 136 135 121 122 139
116 133 134 120 119 137 138 123
100 149 105 151 152 102 154 107
 91  90 166 167 168 169  85  84
 75  74 185 184 183 182  69  68
```

The canonical cube contains each byte value exactly twice. Each of its 192
axis-parallel lines sums to 1020. These facts describe the starting constant;
intermediate states are not required to preserve line sums. Line rotations do
preserve the two-copy byte histogram.

The four canonical body diagonals, concatenated as defined in Section 7, are:

```text
44512633332651444b5e293c3c295e4bb4a1d6c3c3d6a1b4bbaed9ccccd9aebb
```

## 5. Line rotation

`ROTATE_LINE(C, axis, p, amount)` selects the unique eight-cell line parallel
to `axis` and passing through `p`.

For each source coordinate `i` on that axis, its old value moves to:

```text
(i + amount) mod 8
```

on the same line. Canonical absorbed moves always use amounts 1 through 7.

After a move, the cursor is advanced on the same axis by the same amount:

```text
ADVANCE((x,y,z), X, a) = ((x+a) mod 8, y, z)
ADVANCE((x,y,z), Y, a) = (x, (y+a) mod 8, z)
ADVANCE((x,y,z), Z, a) = (x, y, (z+a) mod 8)
```

## 6. Operational state

The operational state is:

```text
S = (C, cursor, previous_axis, symbol_index)
```

Initial values are:

```text
C             = canonical Perfect Value Cube
cursor        = (0,0,0)
previous_axis = X
symbol_index  = 0
```

The full state, including `symbol_index`, is relevant when reporting internal
collisions.

## 7. Body diagonals

`DIAGONALS(C)` returns 32 bytes in this order, for `i=0,...,7`:

```text
D0[i] = C(i,   i,   i)
D1[i] = C(7-i, i,   i)
D2[i] = C(i,   7-i, i)
D3[i] = C(i,   i,   7-i)
```

The returned array is:

```text
D0 || D1 || D2 || D3
```

## 8. Choosing the next axis

Consecutive moves MUST use different axes. Given `previous_axis` and a byte
`selector`, let `second = selector & 1` and use:

| Previous | `second=0` | `second=1` |
|---|---|---|
| X | Y | Z |
| Y | Z | X |
| Z | X | Y |

## 9. Absorbing one symbol

`ABSORB_SYMBOL(S, symbol)` performs six moves. All variables named as bytes are
reduced with `u8` on assignment.

```text
control = u8(symbol + u8(symbol_index) + coord_code(cursor))

for phase = 0..5:
    probe_before = C(cursor)
    index_byte = (symbol_index >> (8 * (phase & 7))) & 0xff
    geometry = u8(coord_code(cursor) + 29*phase + index_byte)

    selector = ROTL8(control, phase) XOR probe_before XOR geometry
    axis = CHOOSE_OTHER_AXIS(previous_axis, selector)

    amount_source = control
                  + probe_before
                  + geometry
                  + symbol
                  + 11*axis

    amount = 1 + (amount_source % 7)

    ROTATE_LINE(C, axis, cursor, amount)
    cursor = ADVANCE(cursor, axis, amount)
    probe_after = C(cursor)

    control = u8(
        ROTL8(control, 1 + (axis & 1))
        + probe_after
        + amount
        + coord_code(cursor)
        + 7*phase
    )

    previous_axis = axis

symbol_index = symbol_index + 1
```

## 10. Forward pass

For message bytes `M[0],...,M[n-1]`, in increasing order:

```text
ABSORB_SYMBOL(S, M[i])
```

## 11. Foldback pass

The return symbol at original position `i` is:

```text
RETURN(M[i], i) = M[i]
                XOR PERFECT_DIAGONALS[i & 31]
                XOR u8(8*i + (i >> 3))
```

The message is then processed in reverse original order:

```text
for i = n-1 down to 0:
    ABSORB_SYMBOL(S, RETURN(M[i], i))
```

The return map is a position-specific XOR bijection. That fact does not make the
complete foldback transition injective.

## 12. Length encoding

`LEN[0..7]` is the little-endian unsigned 64-bit encoding of `n`:

```text
LEN[i] = (n >> (8*i)) & 0xff
```

## 13. Diagonal closure

First absorb the geometry marker:

```text
ABSORB_SYMBOL(S, 252)
```

Then, for `i=0,...,7`:

```text
framed = u8(LEN[i] + PERFECT_DIAGONALS[4*i] + 8*i)
ABSORB_SYMBOL(S, framed)
```

Then absorb the side marker:

```text
ABSORB_SYMBOL(S, 8)
```

For `i=0,...,63`, recompute `D = DIAGONALS(C)` and define:

```text
folded = u8(
    ROTL8(D[i & 31], 1)
    + ROTL8(D[(i+11) & 31], 3)
    + ROTL8(D[(7*i+3) & 31], 5)
)

symbol = folded
       XOR PERFECT_DIAGONALS[(13*i) & 31]
       XOR LEN[i & 7]
       XOR u8(8*i + (i >> 2))

ABSORB_SYMBOL(S, symbol)
```

## 14. Full-cube orbit closure

For `i=0,...,127`, let `offset = i & 127`, and map the four linear positions:

```text
p0 = storage_coord(offset)
p1 = storage_coord(offset + 128)
p2 = storage_coord(offset + 256)
p3 = storage_coord(offset + 384)
```

Read `v0=C(p0)`, ..., `v3=C(p3)`, and recompute `D=DIAGONALS(C)`.

```text
pair_a = u8(ROTL8(v0,1) + ROTL8(v1,3))
pair_b = u8(ROTL8(v2,5) + ROTL8(v3,7))

symbol = pair_a
       XOR pair_b
       XOR D[(5*i + (i >> 3)) & 31]
       XOR LEN[i & 7]
       XOR coord_code(cursor)
       XOR u8(4*i + (i >> 5))

ABSORB_SYMBOL(S, symbol)
```

Across the canonical 128 iterations, every physical cube position participates
once as one of `p0`, `p1`, `p2`, or `p3`.

## 15. Four-diagonal squeeze

Before output, let `D=DIAGONALS(C)` and initialize:

```text
chain = D[0]
      XOR ROTL8(D[9],1)
      XOR ROTL8(D[18],3)
      XOR ROTL8(D[27],5)
      XOR LEN[0]
```

For output index `i=0,...,31`:

```text
lane = i >> 3
position = i & 7
D = DIAGONALS(C)

a = D[((lane+0) & 3)*8 + position]
b = D[((lane+1) & 3)*8 + ((position+1+lane) & 7)]
c = D[((lane+2) & 3)*8 + ((position+3+2*lane) & 7)]
d = D[((lane+3) & 3)*8 + ((position+5+3*lane) & 7)]

pair_a = u8(ROTL8(a,1) + ROTL8(b,3))
pair_b = u8(ROTL8(c,5) + ROTL8(d,7))
cross  = u8(D[(7*i+3) & 31] + D[(13*i+1) & 31])

OUT[i] = u8(
    ROTL8(pair_a XOR pair_b XOR chain, 1+position)
    + ROTL8(cross, 1+2*lane)
    + u8(8*i + lane)
)
```

After emitting `OUT[i]`, absorb four symbols. For `j=0,...,3`, recompute
`D=DIAGONALS(C)` and set:

```text
diagonal_lane = j & 3
left  = D[diagonal_lane*8 + ((position+j+lane) & 7)]
right = D[((diagonal_lane+1) & 3)*8
          + ((7-position+lane+j) & 7)]

symbol = u8(
    ROTL8(left, 1+2*diagonal_lane)
    + ROTL8(right, 7-2*diagonal_lane)
    + ROTL8(OUT[i], 1+(j & 7))
    + chain
    + LEN[(i+j) & 7]
    + u8(4*i+j)
)

ABSORB_SYMBOL(S, symbol)
```

After the four symbols, recompute `D=DIAGONALS(C)` and update:

```text
chain = u8(
    ROTL8(chain, 1+(i & 7))
    + OUT[i]
    + D[(5*i+7) & 31]
    + C(cursor)
    + coord_code(cursor)
)
```

The digest is `OUT[0] || ... || OUT[31]`.

## 16. Streaming API semantics

An API MAY accept incremental updates, but it MUST behave exactly as if the
complete concatenated byte string were processed by Sections 10 through 15.
Because the foldback pass consumes the original message in reverse, an
implementation may buffer the entire message or use external storage.

Text encoding is outside the algorithm. A text API MUST state its encoding.
The reference command-line tools use the exact bytes of files, hexadecimal
input, or UTF-8 for the Python `--text` option. The C++ `--text` option uses the
bytes supplied by the process argument.

## 17. Conformance

A conforming implementation MUST match:

- every digest in `test-vectors/official-v1.json`;
- every phase state in `test-vectors/phase-vectors-v1.json`;
- the cube storage order and operational metadata in this document.

The two anchor vectors are:

```text
M = empty
H = 7f01eb3ce13131ef290f8428ed725b849f875e49ad6c646cc9f4f1b1a1e5734b

M = 616263  (ASCII "abc")
H = f32b2241a950d7e7b2b006ff8ae2d0b08f02db23c0d8fde198dfdf9e9642051f
```

## 18. Candidate-change policy

This specification is frozen for independent cryptanalysis. A change to any
constant, equation, parameter, state field, phase order, or output rule creates
a different candidate and MUST use a new algorithm identifier or major design
revision. Editorial corrections that do not alter any official vector may be
made transparently.
