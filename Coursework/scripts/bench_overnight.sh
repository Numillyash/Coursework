#!/bin/bash
# Long-running benchmark suite for overnight profiling
# Tests multiple key sizes and seeds, outputs to JSON log

set -e

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_ROOT="$(dirname "$SCRIPT_DIR")"
BENCH_BIN="$PROJECT_ROOT/bench_rsa"
OUTPUT_DIR="$PROJECT_ROOT/bench_results"
TIMESTAMP=$(date +%Y%m%d_%H%M%S)
LOG_FILE="$OUTPUT_DIR/bench_${TIMESTAMP}.jsonl"

# Ensure output directory exists
mkdir -p "$OUTPUT_DIR"

echo "=== Overnight Benchmark Suite ==="
echo "Start time: $(date)"
echo "Output: $LOG_FILE"
echo ""

# Build benchmark binary if needed
if [ ! -f "$BENCH_BIN" ]; then
    echo "[*] Building bench_rsa..."
    cd "$PROJECT_ROOT"
    make MODE=prof bench >/dev/null 2>&1
    cd - >/dev/null
fi

# Benchmark configuration
# Keep 512-bit seeds small since each takes ~30min
declare -A CONFIG
CONFIG[64]="seeds=5 iters=10 warmup=2"      # 64-bit: fast, many samples
CONFIG[128]="seeds=5 iters=10 warmup=2"     # 128-bit: fast, many samples
CONFIG[256]="seeds=3 iters=5 warmup=1"      # 256-bit: medium, moderate samples
CONFIG[512]="seeds=2 iters=2 warmup=0"      # 512-bit: slow, minimal samples (2 seeds × 2 iters ≈ 1hr)

# Run benchmarks in order
for keybits in 64 128 256 512; do
    echo ""
    echo "================================================"
    echo "Testing ${keybits}-bit keys"
    echo "Configuration: ${CONFIG[$keybits]}"
    echo "Start: $(date '+%H:%M:%S')"
    echo "================================================"
    
    # Parse config
    eval "${CONFIG[$keybits]}"
    
    # Run benchmark and capture output
    "$BENCH_BIN" \
        --keybits "$keybits" \
        --seeds "$seeds" \
        --iters "$iters" \
        --warmup "$warmup" \
        2>/dev/null | tee -a "$LOG_FILE"
    
    # Show progress
    lines_logged=$(wc -l < "$LOG_FILE")
    echo "Logged $lines_logged results | End: $(date '+%H:%M:%S')"
done

echo ""
echo "================================================"
echo "Benchmark complete!"
echo "Total duration: $(($(date +%s) - $(stat -c %Y "$LOG_FILE")))"
echo "End time: $(date)"
echo "Results saved to: $LOG_FILE"
echo "================================================"
