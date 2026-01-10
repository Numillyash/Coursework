#!/bin/bash
# Parse and analyze benchmark results from JSON lines

set -e

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_ROOT="$(dirname "$SCRIPT_DIR")"
OUTPUT_DIR="$PROJECT_ROOT/bench_results"

if [ ! -d "$OUTPUT_DIR" ]; then
    echo "No results directory found: $OUTPUT_DIR"
    exit 1
fi

# Get latest benchmark file
LATEST_LOG=$(ls -t "$OUTPUT_DIR"/bench_*.jsonl 2>/dev/null | head -1)

if [ -z "$LATEST_LOG" ]; then
    echo "No benchmark results found in $OUTPUT_DIR"
    exit 1
fi

echo "=== Benchmark Results Analysis ==="
echo "File: $(basename "$LATEST_LOG")"
echo ""

# Summary by test type and keybits
echo "Summary:"
echo "--------"

for test_type in modexp_public modexp_private_like; do
    echo ""
    echo "Test: $test_type"
    echo "Keybits | Avg Time (ms) | Min (ms) | Max (ms) | Count"
    echo "--------|---------------|----------|----------|-------"
    
    # Group by keybits and compute stats
    grep "\"test\":\"$test_type\"" "$LATEST_LOG" | \
    jq -r '[.keybits, .ns] | @csv' | \
    sort -t, -k1 -n | \
    awk -F, '
        {
            keybits = $1
            ns = $2
            if (keybits != prev_keybits && prev_keybits != "") {
                avg = total / count
                printf "%7d | %13.3f | %8.3f | %8.3f | %6d\n", 
                    prev_keybits, avg/1e6, min/1e6, max/1e6, count
                count = 0
                total = 0
                min = 999999999999
                max = 0
            }
            
            if (count == 0) min = ns
            
            count++
            total += ns
            if (ns < min) min = ns
            if (ns > max) max = ns
            prev_keybits = keybits
        }
        END {
            if (prev_keybits != "") {
                avg = total / count
                printf "%7d | %13.3f | %8.3f | %8.3f | %6d\n", 
                    prev_keybits, avg/1e6, min/1e6, max/1e6, count
            }
        }
    '
done

echo ""
echo "Raw results count:"
wc -l < "$LATEST_LOG"
echo "lines logged"
