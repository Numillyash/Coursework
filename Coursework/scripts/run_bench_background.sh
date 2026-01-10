#!/bin/bash
# Run overnight benchmark in background with nohup
# Usage: bash run_bench_background.sh

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
BENCH_SCRIPT="$SCRIPT_DIR/bench_overnight.sh"
PROJECT_ROOT="$(dirname "$SCRIPT_DIR")"
LOG_FILE="$PROJECT_ROOT/bench_background.log"

echo "Starting overnight benchmark in background..."
echo "Output log: $LOG_FILE"
echo "Benchmark script: $BENCH_SCRIPT"
echo ""
echo "To monitor progress:"
echo "  tail -f $LOG_FILE"
echo ""
echo "To check status:"
echo "  ps aux | grep bench_overnight"
echo ""

# Run in background and capture output
nohup bash "$BENCH_SCRIPT" > "$LOG_FILE" 2>&1 &
PID=$!

echo "Process started with PID: $PID"
echo "Background process details:"
ps -p "$PID" -o pid,ppid,cmd,etime,time
