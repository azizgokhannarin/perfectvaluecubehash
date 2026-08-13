#!/usr/bin/env bash
# Stage 4 digest-surface + longer truncated campaigns for PVC-RotHash-2.
# Preset: R5-rothash2. Not a security proof.
#
# Usage:
#   scripts/stage4_digest_r2.sh [--build-dir DIR] [--out DIR] [--quick|--full]

set -euo pipefail

ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
BUILD="${ROOT}/build"
OUT="${ROOT}/results/stage4-r2-digest"
MODE="standard"
PRESET="R5-rothash2"

while [[ $# -gt 0 ]]; do
  case "$1" in
    --build-dir) BUILD="$2"; shift 2 ;;
    --out) OUT="$2"; shift 2 ;;
    --quick) MODE="quick"; shift ;;
    --full) MODE="full"; shift ;;
    --preset) PRESET="$2"; shift 2 ;;
    -h|--help) sed -n '2,8p' "$0"; exit 0 ;;
    *) echo "unknown arg: $1" >&2; exit 2 ;;
  esac
done

mkdir -p "$OUT"
SUMMARY="${OUT}/SUMMARY.md"
: >"$SUMMARY"
log() { printf '%s\n' "$*" | tee -a "$SUMMARY"; }

run_tool() {
  local name="$1"; shift
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
  if [[ $rc -ne 0 ]]; then
    if grep -Eq 'path_found=no|No collision|no exact' "$logf" 2>/dev/null; then
      log "EXIT=${rc} (negative / no path) elapsed=${elapsed}s"
      rc=0
    else
      log "EXIT=${rc} elapsed=${elapsed}s"
      tail -n 50 "$logf" | tee -a "$SUMMARY" >/dev/null || true
      log '```'
      return "$rc"
    fi
  else
    log "EXIT=0 elapsed=${elapsed}s"
  fi
  # Key lines
  grep -Ei 'path_found|preset=|pairs|minimum|generic|collision|correlation|mean|bits,|global|best_digest|exact|digest_bits|forward|samples|censored|found' "$logf" \
    | head -n 100 | tee -a "$SUMMARY" >/dev/null || tail -n 40 "$logf" | tee -a "$SUMMARY" >/dev/null || true
  log '```'
  return 0
}

for bin in pvc-digest-beam-search pvc-divergent-digest-beam pvc-digest-lsh-search \
           pvc-barrier-correlation pvc-truncated-campaign; do
  [[ -x "${BUILD}/${bin}" ]] || { echo "missing ${BUILD}/${bin}" >&2; exit 1; }
done

log "# Stage 4 digest-surface — ${PRESET}"
log ""
log "- date: $(date -u +%Y-%m-%dT%H:%M:%SZ)"
log "- mode: ${MODE}"
log "- note: experimental; not production; no security claim"
log ""

failures=0

# Same-forward beam: expected path_found=no under Controller S (no seed pairs).
run_tool digest-beam-same-forward \
  "${BUILD}/pvc-digest-beam-search" --preset "${PRESET}" \
  --levels 8 --beam 64 --prefix-count 4096 --print-limit 0 \
  || failures=$((failures + 1))

if [[ "$MODE" == "quick" ]]; then
  DIV_DEPTH=4; DIV_BEAM=32; DIV_BRANCH=8
  LSH_LIMIT=1024; LSH_PROJ=16
  BAR_SAMPLES=1000
  TRUNC_BITS="24,32,40"; TRUNC_TRIALS=2; TRUNC_LIMIT=100000
elif [[ "$MODE" == "full" ]]; then
  DIV_DEPTH=8; DIV_BEAM=128; DIV_BRANCH=16
  LSH_LIMIT=8192; LSH_PROJ=32
  BAR_SAMPLES=10000
  TRUNC_BITS="24,32,40,48"; TRUNC_TRIALS=8; TRUNC_LIMIT=1000000
else
  DIV_DEPTH=8; DIV_BEAM=128; DIV_BRANCH=16
  LSH_LIMIT=4096; LSH_PROJ=32
  BAR_SAMPLES=5000
  TRUNC_BITS="24,32,40,48"; TRUNC_TRIALS=4; TRUNC_LIMIT=400000
fi

run_tool divergent-beam-000000-000001 \
  "${BUILD}/pvc-divergent-digest-beam" --preset "${PRESET}" \
  --left 000000 --right 000001 \
  --depth "${DIV_DEPTH}" --beam "${DIV_BEAM}" --branch "${DIV_BRANCH}" \
  --print-limit 0 || failures=$((failures + 1))

run_tool divergent-beam-a5c301-5a3cfe \
  "${BUILD}/pvc-divergent-digest-beam" --preset "${PRESET}" \
  --left a5c301 --right 5a3cfe \
  --depth "${DIV_DEPTH}" --beam "${DIV_BEAM}" --branch "${DIV_BRANCH}" \
  --print-limit 0 || failures=$((failures + 1))

for pair in "000000:000001" "102030:102031" "abcdef:abcdee" "5a00c3:5a00c4"; do
  left="${pair%%:*}"
  right="${pair##*:}"
  name="lsh-${left}-${right}"
  run_tool "${name}" \
    "${BUILD}/pvc-digest-lsh-search" --preset "${PRESET}" \
    --left "${left}" --right "${right}" \
    --suffix-bytes 2 --suffix-limit "${LSH_LIMIT}" \
    --projections "${LSH_PROJ}" --projection-bytes 1 \
    || failures=$((failures + 1))
done

run_tool barrier-independent \
  "${BUILD}/pvc-barrier-correlation" --preset "${PRESET}" \
  --left 000000 --right 000001 \
  --samples "${BAR_SAMPLES}" --suffix-bytes 2 --independent-suffix \
  || failures=$((failures + 1))

run_tool barrier-common \
  "${BUILD}/pvc-barrier-correlation" --preset "${PRESET}" \
  --left 000000 --right 000001 \
  --samples "${BAR_SAMPLES}" --suffix-bytes 2 --common-suffix \
  || failures=$((failures + 1))

run_tool truncated-long \
  "${BUILD}/pvc-truncated-campaign" --preset "${PRESET}" \
  --bits "${TRUNC_BITS}" --trials "${TRUNC_TRIALS}" --limit "${TRUNC_LIMIT}" \
  || failures=$((failures + 1))

log ""
log "## Campaign completion"
log ""
log "- tool failures (true errors): ${failures}"
log "- logs: \`${OUT}\`"
log ""
log "Budget-limited negative evidence only. Production prohibited."

echo "Wrote ${SUMMARY}"
exit "$failures"
