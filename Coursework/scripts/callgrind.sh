#!/bin/bash
set -euo pipefail

ROOT_DIR=$(dirname "$0")/..
cd "$ROOT_DIR"

echo "Building bench in prof mode..."
make MODE=prof bench

echo "Running valgrind/callgrind..."
valgrind --tool=callgrind --callgrind-out-file=callgrind.out ./bench_rsa --keybits 1024 --seeds 1 --iters 3 --warmup 1

echo "Done. Use: kcachegrind callgrind.out"