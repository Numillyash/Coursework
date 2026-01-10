# Benchmark Suite

Automated performance testing infrastructure for RSA modular exponentiation.

## Scripts

### `bench_overnight.sh` — Overnight Benchmark Suite
Runs comprehensive benchmarks across multiple key sizes and random seeds.

**Configuration:**
- **64-bit**: 5 seeds × 10 iterations (≈10 sec)
- **128-bit**: 5 seeds × 10 iterations (≈1.5 min)  
- **256-bit**: 3 seeds × 5 iterations (≈3 min)
- **512-bit**: 2 seeds × 2 iterations (≈30 min)

**Usage:**
```bash
bash scripts/bench_overnight.sh
```

**Output:**
- JSONL format: `bench_results/bench_YYYYMMDD_HHMMSS.jsonl`
- Each line: `{"test":"...", "keybits":..., "seed":..., "ns":...}`
- Real-time progress printed to console
- Approx. total runtime: **~35 minutes**

### `run_bench_background.sh` — Background Runner
Start benchmark in the background with `nohup` while you're away.

**Usage:**
```bash
bash scripts/run_bench_background.sh
```

**Monitor progress:**
```bash
tail -f bench_background.log
ps aux | grep bench_overnight
```

### `analyze_bench.sh` — Results Analyzer
Parse and summarize benchmark results.

**Usage:**
```bash
bash scripts/analyze_bench.sh
```

**Output format:**
```
Test: modexp_public
Keybits | Avg Time (ms) | Min (ms) | Max (ms) | Count
--------|---------------|----------|----------|-------
     64 |       297.516 |  278.666 |  318.702 |      5
    128 |      1127.245 | 1058.937 | 1344.484 |      5
    256 |      2258.490 | 2223.561 | 2323.837 |      3
    512 |      3722.626 | 3625.800 | 3819.451 |      2
```

## Quick Test

For fast validation before overnight run:

```bash
cd Coursework
./bench_rsa --keybits 64 --seeds 2 --iters 5 --warmup 1
```

## Build Modes

Benchmarks compile with `-O2 -g -fno-omit-frame-pointer` (prof mode):
```bash
make MODE=prof bench
```

## Benchmark Binary

`bench_rsa` — Deterministic modular exponentiation benchmark.

**Parameters:**
- `--keybits N`: Key size in bits (64, 128, 256, 512)
- `--seeds N`: Number of random seeds (1-10)
- `--iters N`: Iterations per seed
- `--warmup N`: Warmup iterations (not counted)

**Tests:**
1. `modexp_public`: 65537 (typical RSA public exponent)
2. `modexp_private_like`: Random long exponent (similar to private key)

**PRNG:** Deterministic splitmix64 for reproducibility.

## Results Storage

All benchmark results saved to `bench_results/`:
```
bench_results/
├── bench_20260110_170621.jsonl
├── bench_20260110_185024.jsonl
└── ...
```

Each file is immutable; new runs create new files with timestamped names.

## Analysis Pipeline

1. Run overnight benchmark → JSONL results
2. Run analyzer → summary statistics (avg/min/max)
3. Export to CSV or plotting tool for trend analysis

Example (manual extraction):
```bash
grep "modexp_private_like" bench_results/bench_*.jsonl | \
  jq -r '[.keybits, .ns/1e6] | @csv' | sort -t, -k1 -n
```

## Performance Expectations

Approximate execution times (per seed, per iteration):

| Keybits | Public (ms) | Private (ms) |
|---------|-------------|--------------|
| 64      | 0.3         | 1.5          |
| 128     | 1.1         | 12           |
| 256     | 2.2         | 48           |
| 512     | 3.7         | 161          |

Total overnight run: ~35 minutes (mostly 512-bit private exponentiation)
