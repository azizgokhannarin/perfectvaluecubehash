#!/usr/bin/env bash
set -euo pipefail

root="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
build="${root}/build-release-check"
rm -rf "${build}"

cmake -S "${root}" -B "${build}" \
  -DCMAKE_BUILD_TYPE=Release \
  -DPVC_WARNINGS_AS_ERRORS=ON \
  -DPVC_BUILD_TOOLS=ON \
  -DPVC_BUILD_TESTS=ON
cmake --build "${build}" --parallel "$(nproc 2>/dev/null || echo 2)"
ctest --test-dir "${build}" --output-on-failure \
  -E 'pvc-stage4-smoke-rothash2'

# --- PVC-RotHash-1 (frozen baseline) ---
python3 "${root}/scripts/verify_vectors.py" \
  --cpp "${build}/pvc-hash" \
  --vector-dump "${build}/pvc-vector-dump"

"${build}/pvc-hash" --hex "" | grep -Fx \
  7f01eb3ce13131ef290f8428ed725b849f875e49ad6c646cc9f4f1b1a1e5734b
"${build}/pvc-hash" --hex 616263 | grep -Fx \
  f32b2241a950d7e7b2b006ff8ae2d0b08f02db23c0d8fde198dfdf9e9642051f

# --- PVC-RotHash-2 (0.2.0-draft) ---
python3 "${root}/scripts/verify_vectors_v2.py" \
  --cpp "${build}/pvc-hash"

"${build}/pvc-hash" --rothash2 --hex "" | grep -Fx \
  9c4f0899255220a60bac6df7661cc50583f25f076ad37254b270b54685ce864a
"${build}/pvc-hash" --rothash2 --hex 616263 | grep -Fx \
  9c1b502e8eac4ea07e18265ea30f888c4d5fd8ae81fa1ed453c2c099d4d68fdb

printf '%s\n' "PVC-RotHash-1 1.0.0-rc1 + PVC-RotHash-2 0.2.0-draft release check passed"
