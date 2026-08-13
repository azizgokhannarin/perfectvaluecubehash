#!/usr/bin/env bash
# Stage 4 deep entry campaigns against PVC-RotHash-2 (preset R5-rothash2).
#
# Not a security proof. Records tool outputs under results/stage4-r2/ by default.
# Usage:
#   scripts/stage4_deep_r2.sh [--build-dir DIR] [--out DIR] [--quick|--full]
#
# Exit 0 if all campaigns complete without tool crash. Individual "no break"
# summaries are written to SUMMARY.md — interpret scientifically, not as PASS
# for production.

set -euo pipefail

ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
BUILD="${ROOT}/build"
OUT="${ROOT}/results/stage4-r2"
MODE="standard" # quick | standard | full
PRESET="R5-rothash2"

while [[ $# -gt 0 ]]; do
  case "$1" in
    --build-dir) BUILD="$2"; shift 2 ;;
    --out) OUT="$2"; shift 2 ;;
    --quick) MODE="quick"; shift ;;
    --full) MODE="full"; shift ;;
    --preset) PRESET="$2"; shift 2 ;;
    -h|--help)
      sed -n '2,12p' "$0"
      exit 0
      ;;
    *) echo "unknown arg: $1" >&2; exit 2 ;;
  esac
done

mkdir -p "$OUT"
SUMMARY="${OUT}/SUMMARY.md"
: >"$SUMMARY"

log() { printf '%s\n' "$*" | tee -a "$SUMMARY"; }

run_tool() {
  local name="$1"
  shift
  local logf="${OUT}/${name}.log"
  log ""
  log "### ${name}"
  log '```'
  log "cmd: $*"
  local start end rc
  start=$(date +%s)
  set +e
  "$@" >"$logf" 2>&1
  rc=$?
  set -e
  end=$(date +%s)
  local elapsed=$((end - start))
  # Some tools exit 1 to mean "no structural hit found" (e.g. bridged path).
  # That is a successful negative result for Stage 4, not a campaign failure.
  if [[ $rc -ne 0 ]]; then
    if grep -Eq 'path_found=no|No collision|no exact state collision|forward_collision_pairs=0' "$logf" 2>/dev/null; then
      log "EXIT=${rc} (treated as negative evidence) elapsed=${elapsed}s"
      rc=0
    else
      log "EXIT=${rc} elapsed=${elapsed}s  (see ${name}.log)"
      tail -n 40 "$logf" | tee -a "$SUMMARY" >/dev/null || true
      log '```'
      return "$rc"
    fi
  else
    log "EXIT=0 elapsed=${elapsed}s"
  fi
  # Capture a compact tail of key lines.
  if grep -Eiq 'collision|alias|pairs|digest|no exact|unique|found|pass|fail|probability|mean' "$logf"; then
    grep -Ei 'collision|alias|pairs|digest|no exact|unique|found|pass|fail|probability|mean|contexts_with|total_|preset=|bits=|trial' "$logf" \
      | head -n 80 | tee -a "$SUMMARY" >/dev/null || true
  else
    tail -n 30 "$logf" | tee -a "$SUMMARY" >/dev/null || true
  fi
  log '```'
  return 0
}

need() {
  if [[ ! -x "$1" ]]; then
    echo "missing executable: $1 (build tools first)" >&2
    exit 1
  fi
}

for bin in \
  pvc-collision-probe \
  pvc-transition-collision \
  pvc-phase-collision \
  pvc-return-alias-surface \
  pvc-related-input-probe \
  pvc-alignment-probe \
  pvc-multicollision-probe \
  pvc-foldback-aware-alias \
  pvc-dual-return-alias \
  pvc-bridged-multicollision \
  pvc-three-byte-collision \
  pvc-truncated-campaign \
  pvc-differential-search \
  pvc-length-framing-probe \
  pvc-foldback-separation-profile
do
  need "${BUILD}/${bin}"
done

log "# Stage 4 deep campaigns — ${PRESET}"
log ""
log "- date: $(date -u +%Y-%m-%dT%H:%M:%SZ)"
log "- mode: ${MODE}"
log "- build: ${BUILD}"
log "- host: $(hostname) nproc=$(nproc)"
log "- note: experimental; not production; no security claim"
log ""

failures=0

# --- Always-on cheap gates ---
run_tool collision-1byte \
  "${BUILD}/pvc-collision-probe" --rothash2 1 || failures=$((failures + 1))

run_tool collision-r1-pair \
  "${BUILD}/pvc-collision-probe" --rothash2 r1-pair || failures=$((failures + 1))

run_tool collision-2byte \
  "${BUILD}/pvc-collision-probe" --rothash2 2 || failures=$((failures + 1))

run_tool transition-d1 \
  "${BUILD}/pvc-transition-collision" --preset "${PRESET}" --depth 1 || failures=$((failures + 1))

run_tool transition-d2 \
  "${BUILD}/pvc-transition-collision" --preset "${PRESET}" --depth 2 || failures=$((failures + 1))

run_tool phase-1byte \
  "${BUILD}/pvc-phase-collision" --preset "${PRESET}" --message-bytes 1 || failures=$((failures + 1))

if [[ "$MODE" != "quick" ]]; then
  run_tool phase-2byte \
    "${BUILD}/pvc-phase-collision" --preset "${PRESET}" --message-bytes 2 || failures=$((failures + 1))
fi

# --- Surface / related ---
if [[ "$MODE" == "quick" ]]; then
  RAS_MSG=128; RAS_BYTES=4
  REL_SAMPLES=64
  ALIGN_SAMPLES=64
  MULTI_PREFIX=1024
  FA_PREFIX=1024
  DUAL_PREFIX=1024
  BRIDGE_PREFIX=1024
  BRIDGE_LEVELS=4
  THREE_FIRST=8
  TRUNC_BITS="24,32"
  TRUNC_TRIALS=2
  TRUNC_LIMIT=50000
  DIFF_SAMPLES=8
  FSEP_PREFIX=256
else
  RAS_MSG=512; RAS_BYTES=8
  REL_SAMPLES=256
  ALIGN_SAMPLES=256
  MULTI_PREFIX=4096
  FA_PREFIX=4096
  DUAL_PREFIX=4096
  BRIDGE_PREFIX=4096
  BRIDGE_LEVELS=8
  THREE_FIRST=32
  TRUNC_BITS="24,32,40"
  TRUNC_TRIALS=4
  TRUNC_LIMIT=200000
  DIFF_SAMPLES=16
  FSEP_PREFIX=1024
fi

if [[ "$MODE" == "full" ]]; then
  RAS_MSG=2048; RAS_BYTES=8
  REL_SAMPLES=512
  MULTI_PREFIX=16384
  FA_PREFIX=16384
  DUAL_PREFIX=16384
  BRIDGE_PREFIX=16384
  BRIDGE_LEVELS=12
  THREE_FIRST=256
  TRUNC_BITS="24,32,40,48"
  TRUNC_TRIALS=8
  TRUNC_LIMIT=500000
  DIFF_SAMPLES=32
  FSEP_PREFIX=4096
fi

run_tool return-alias-surface \
  "${BUILD}/pvc-return-alias-surface" --preset "${PRESET}" \
  --messages "${RAS_MSG}" --message-bytes "${RAS_BYTES}" || failures=$((failures + 1))

run_tool related-input \
  "${BUILD}/pvc-related-input-probe" --preset "${PRESET}" \
  --samples "${REL_SAMPLES}" || failures=$((failures + 1))

run_tool alignment \
  "${BUILD}/pvc-alignment-probe" --preset "${PRESET}" || failures=$((failures + 1))

run_tool length-framing \
  "${BUILD}/pvc-length-framing-probe" --preset "${PRESET}" || failures=$((failures + 1))

run_tool differential \
  "${BUILD}/pvc-differential-search" --preset "${PRESET}" \
  --samples "${DIFF_SAMPLES}" || failures=$((failures + 1))

run_tool multicollision \
  "${BUILD}/pvc-multicollision-probe" --preset "${PRESET}" \
  --prefix-count "${MULTI_PREFIX}" --max-levels 4 \
  --suffix-bytes 0 --suffix-limit 1 || failures=$((failures + 1))

run_tool foldback-aware \
  "${BUILD}/pvc-foldback-aware-alias" --preset "${PRESET}" \
  --prefix-count "${FA_PREFIX}" --suffix-bytes 0 --suffix-limit 1 || failures=$((failures + 1))

run_tool dual-return \
  "${BUILD}/pvc-dual-return-alias" --preset "${PRESET}" \
  --prefix-count "${DUAL_PREFIX}" --suffix-bytes 0 --suffix-limit 1 \
  --no-family-surface || failures=$((failures + 1))

run_tool bridged \
  "${BUILD}/pvc-bridged-multicollision" --preset "${PRESET}" \
  --prefix-count "${BRIDGE_PREFIX}" --levels "${BRIDGE_LEVELS}" \
  --materialize-levels 0 || failures=$((failures + 1))

run_tool three-byte-forward \
  "${BUILD}/pvc-three-byte-collision" --preset "${PRESET}" \
  --phase forward --first-byte-count "${THREE_FIRST}" || failures=$((failures + 1))

if [[ "$MODE" != "quick" ]]; then
  run_tool three-byte-foldback \
    "${BUILD}/pvc-three-byte-collision" --preset "${PRESET}" \
    --phase foldback --first-byte-count "${THREE_FIRST}" || failures=$((failures + 1))
fi

run_tool truncated-campaign \
  "${BUILD}/pvc-truncated-campaign" --preset "${PRESET}" \
  --bits "${TRUNC_BITS}" --trials "${TRUNC_TRIALS}" --limit "${TRUNC_LIMIT}" \
  || failures=$((failures + 1))

run_tool foldback-separation \
  "${BUILD}/pvc-foldback-separation-profile" --preset "${PRESET}" \
  --prefix-count "${FSEP_PREFIX}" || failures=$((failures + 1))

log ""
log "## Campaign completion"
log ""
log "- tool failures (non-zero exit): ${failures}"
log "- logs directory: \`${OUT}\`"
log ""
log "Interpret zero structural hits as **budget-limited negative evidence**, not"
log "a security claim. Production use remains prohibited."

echo ""
echo "Wrote ${SUMMARY}"
exit "$failures"
