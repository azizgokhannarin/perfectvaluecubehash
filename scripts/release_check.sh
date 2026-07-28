#!/usr/bin/env bash
set -euo pipefail

root="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
build="${root}/build-release-check"
rm -rf "${build}"

cmake -S "${root}" -B "${build}" \
  -DCMAKE_BUILD_TYPE=Release \
  -DPVC_WARNINGS_AS_ERRORS=ON
cmake --build "${build}" --parallel 2
ctest --test-dir "${build}" --output-on-failure
python3 "${root}/scripts/verify_vectors.py" \
  --cpp "${build}/pvc-hash" \
  --vector-dump "${build}/pvc-vector-dump"

"${build}/pvc-hash" --hex "" | grep -Fx \
  7f01eb3ce13131ef290f8428ed725b849f875e49ad6c646cc9f4f1b1a1e5734b
"${build}/pvc-hash" --hex 616263 | grep -Fx \
  f32b2241a950d7e7b2b006ff8ae2d0b08f02db23c0d8fde198dfdf9e9642051f

printf '%s\n' "PVC-RotHash-1 1.0.0-rc1 release check passed"
