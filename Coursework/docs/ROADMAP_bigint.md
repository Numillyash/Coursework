# BigInt Architecture Roadmap

This document outlines the long-term architecture and optimization strategy for the bigint library, including migration from the legacy bit-level representation to modern RSA-optimized engines.

## Current State

- **Legacy backend** (`lib/bigint/bit_LA.c/h`): bit-level storage, bit-by-bit operations
- **New backend** (`lib/bigint/BitBigInt` in C++): initial skeleton with LSB-first bit vectors
- **Use case**: RSA cryptography (2048-bit, 4096-bit keys)

## Phase 1: BitBigInt Skeleton → Full Implementation

**Status**: In progress (BitBigInt basic operations)

- [x] RAII wrapper (`std::vector` for memory management)
- [x] Basic I/O (to_binary, from uint64_t)
- [x] Comparison (operator==, compare)
- [ ] Arithmetic kernels: shl1, shr1, add_unsigned (bit-level)
- [ ] Full addition/subtraction
- [ ] Multiplication (naive first, then Karatsuba/FFT)
- [ ] Modular arithmetic

**Rationale**: Establish clean C++ interface before optimizing for RSA-specific use cases.

---

## Phase 2: RSA Fast-Path with Fixed-Width Engines

**Target**: Production-ready modular exponentiation (modpow) for RSA 2048/4096.

### A) Fixed-Width Unsigned Engines

Dedicate specialized types for RSA arithmetic without dynamic allocation:

#### U2048: 256-bit and 2048-bit unsigned integer

```cpp
namespace bigint {
  // 2048 bits = 64 × uint32_t limbs (little-endian word order)
  struct U2048 {
    uint32_t limbs[64];  // each = word
  };

  // Operations: add, sub, mul, sqr, montgomery_reduce, mod_exp
  // All stack-allocated, no malloc/free
}
```

**Benefits**:

- No heap allocation overhead
- Predictable memory layout for SIMD optimization
- Constant-time potential (for side-channel resistance)

#### U4096: 4096-bit unsigned (intermediate storage)

```cpp
  struct U4096 {
    uint32_t limbs[128];  // for mul/sqr result before reduction
  };
```

**Use case**: Multiplication result (2048 × 2048 → 4096) before Montgomery reduction.

### B) ModIntRSA2048: RSA-specific composition type

Use **composition over inheritance** to keep designs independent:

```cpp
struct ModContext2048 {
  U2048 modulus_n;
  U2048 r_squared;      // R² mod n (Montgomery param)
  uint32_t n_inv;       // Montgomery: μ = -n⁻¹ mod 2³²
};

struct ModIntRSA2048 {
  U2048 value;
  const ModContext2048* context;  // shared, read-only

  ModIntRSA2048 operator*(const ModIntRSA2048& other) const;
  ModIntRSA2048 powmod(const U2048& exponent) const;
};
```

**Goals**:

- Zero-copy modular operations
- Montgomery multiplication (word-by-word, no big-number div)
- Side-channel resistant (constant-time path options)
- Benchmark target: < 1ms for RSA-2048 signature

### C) Implementation Strategy

1. **Montgomery multiplication** (school book):

   - Input: A, B (< n), n, μ = -n⁻¹ mod 2³²
   - Output: A·B·R⁻¹ mod n (R = 2^(word_bits × limbs))
   - Loop: 64 iterations (one per limb), no division

2. **Exponentiation** (binary/windowed):

   - Left-to-right or right-to-left with precomputed windows
   - No variable-time branches for constant-time RSA

3. **Reduction** (conditional subtraction at end):
   - If result ≥ n, subtract n (single word-level comparison + sub)

---

## Phase 3: Windowed Storage Optimization

**Goal**: Support arbitrary-precision arithmetic without expensive allocations during computation.

### A) Window-based buffer layout

Instead of `std::vector<uint8_t> bits_`, use:

```cpp
class BitBigIntWindowed {
private:
  std::vector<uint8_t> buffer_;  // allocated once, reused
  size_t start_;                  // first valid bit index
  size_t length_;                 // number of valid bits
  size_t capacity_;               // total buffer size (bits)

public:
  // O(1) left-shift by word boundary:
  void shift_left_words(size_t words) {
    start_ += words * 8;  // if headroom allows
  }

  // O(1) right-shift similarly
  void shift_right_words(size_t words) {
    start_ -= words * 8;  // if permitted
  }

  // Fallback: recenter buffer (rare)
  void recenter();
};
```

**Benefits**:

- O(1) shifts when used strategically (e.g., multiplication by powers of 2)
- Reusable buffer for intermediate results
- Predictable memory pattern for cache locality

### B) Headroom and recenter policy

- **Headroom**: Extra capacity before `start_` and after `start_ + length_`
- **Thresholds**:
  - If operation needs shift > headroom, call `recenter()` (moves bits, costs O(n))
  - Keep headroom ≥ 2× expected intermediate growth

### C) Debug assertions

Protect against silent corruption:

```cpp
#ifdef BIGINT_DEBUG
  void verify_invariants() const {
    assert(start_ + length_ <= capacity_);
    assert(length_ >= 2);  // min: data + sign
    assert(all_bits_in_0_or_1());
  }
#endif
```

---

## Phase 4: Two's Complement Backend (TC)

**Purpose**: Unified signed integer interface with native negative support.

### A) TC Representation

```cpp
class BigIntTC {
  std::vector<uint8_t> bits_;  // two's complement
  // No separate sign bit; sign encoded in MSB + representation
};
```

**Relationship to legacy number**:

- Legacy `number` uses sign-bit (1 = negative)
- `BigIntTC` uses true two's complement
- 1:1 mapping for conversion

### B) Migration path

1. **BitBigInt (current)**: sign-magnitude, non-negative focus
2. **BigIntTC**: two's complement, full signed arithmetic
3. **RSA fast-path**: uses BigIntTC for reduction/intermediate values
4. **Drop legacy**: Once all crypto operations use new backend

---

## Phase 5: Cryptographic Operations

Once arithmetic is solid:

- [ ] GCD / Extended GCD (for key generation)
- [ ] Modular inverse (for decryption)
- [ ] Primality testing (for key generation validation)
- [ ] Chinese Remainder Theorem optimization (RSA-CRT)

---

## Summary: Architecture Layers

```
┌─────────────────────────────────────────────────┐
│  RSA Signature (Main API)                       │
│  - sign(msg, private_key)                       │
│  - verify(msg, sig, public_key)                 │
└──────────────────┬──────────────────────────────┘
                   │
┌──────────────────┴──────────────────────────────┐
│  ModIntRSA2048 (Composition)                    │
│  - powmod, mul, mod operations                  │
│  - constant-time checks                         │
└──────────────────┬──────────────────────────────┘
                   │
┌──────────────────┴──────────────────────────────┐
│  U2048 / U4096 (Fixed-width engines)            │
│  - Montgomery mul, add, sub                      │
│  - No allocation, stack-based                   │
└──────────────────┬──────────────────────────────┘
                   │
┌──────────────────┴──────────────────────────────┐
│  BigIntTC (Windowed + arbitrary precision)      │
│  - Handles general bigint ops                   │
│  - Fallback for non-RSA sizes                   │
└──────────────────┬──────────────────────────────┘
                   │
┌──────────────────┴──────────────────────────────┐
│  BitBigInt (LSB-first bit vectors) + Legacy     │
│  - Tests, reference implementation              │
│  - Phase 1: Full functionality before optimizing│
└─────────────────────────────────────────────────┘
```

---

## Implementation Guidelines

### Memory Safety

- No raw pointers; use `std::vector`, `std::span` (C++20)
- Stack-allocate fixed-width types (U2048, U4096)
- For heap: use unique_ptr with pool allocators if needed

### Correctness

- Unit tests for each primitive (add, mul, mod)
- Cross-validate new vs. legacy for medium-size inputs
- Property-based tests: (a + b) - b == a, etc.

### Performance

- Benchmark hot paths: Montgomery mul, modexp
- Profile memory allocation patterns
- Cache-aware loop unrolling (word-level operations)

### Security (RSA context)

- Constant-time comparison (no early exit on mismatch)
- Constant-time multiplication (no branch-dependent loop count)
- Mask intermediate values in reduction steps (timing side-channel)

---

## Timeline & Priorities

1. **Near term** (now):
   - Complete BitBigInt basic arithmetic (shl1, add, sub)
   - Write tests vs. legacy
2. **Medium term** (1–2 weeks):
   - U2048/U4096 prototypes
   - Montgomery multiplication
   - Basic modexp benchmark
3. **Long term** (if needed):
   - Windowed storage optimization
   - Two's complement unification
   - RSA-CRT and other advanced optimizations

---

## References

- Legacy implementation: `lib/bigint/bit_LA.c` (current reference)
- New skeleton: `lib/bigint/BitBigInt.hpp/cpp`
- Benchmark suite: `scripts/bench_overnight.sh` (performance tracking)

For the latest status, see git log on branch `BitBigInt`.
