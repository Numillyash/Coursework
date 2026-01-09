#!/bin/bash
set -euo pipefail

ROOT_DIR=$(dirname "$0")/..
cd "$ROOT_DIR"

echo "Building bench in prof mode..."
make MODE=prof bench

echo "Running perf stat (may require sudo)"
perf stat -r 10 -d -- ./bench_rsa --keybits 1024 --seeds 10 --iters 30 --warmup 3
