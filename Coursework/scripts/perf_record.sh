#!/bin/bash
set -euo pipefail

ROOT_DIR=$(dirname "$0")/..
cd "$ROOT_DIR"

echo "Building bench in prof mode..."
make MODE=prof bench

echo "Recording perf data..."
perf record -F 999 --call-graph dwarf -- ./bench_rsa --keybits 1024 --seeds 10 --iters 10 --warmup 1

echo "Done. Use: perf report"