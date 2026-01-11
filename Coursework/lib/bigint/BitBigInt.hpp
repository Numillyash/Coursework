#pragma once

#include <vector>
#include <cstdint>
#include <string>
#include <cassert>

namespace bigint {

/**
 * BitBigInt - A multi-precision integer class using bit-level storage.
 *
 * Internal representation:
 * - Stores bits in a vector<uint8_t> where each element is 0 or 1
 * - Bit-endian: LSB first (index 0 is least significant bit)
 * - Sign bit: stored as the last element (bits_.back())
 *   * 0 = positive or zero
 *   * 1 = negative (two's complement style, though we don't use full two's comp)
 *
 * Invariant:
 * - bits_.size() >= 2 (minimum: data_bit + sign_bit)
 * - Zero is represented as {0, 0} (bit=0, sign=0)
 * - All elements in bits_ are either 0 or 1
 *
 * Design decisions for phase 1:
 * - NO external dependencies on legacy number type for computation
 * - RAII: std::vector for automatic memory management
 * - Copy/move semantics: default (provided by std::vector)
 * - No exceptions: use assertions for invariants
 *
 * Architecture roadmap: see docs/ROADMAP_bigint.md
 * - Phase 1 (now): BitBigInt basic arithmetic
 * - Phase 2 (near): RSA-specific fixed-width engines (U2048, U4096)
 * - Phase 3: Windowed storage optimization
 * - Phase 4: Two's complement backend unification
 */
class BitBigInt final {
public:
    // === Constructors ===

    /// Default constructor: creates zero
    BitBigInt();

    /// Construct from unsigned 64-bit integer
    /// Converts value to binary representation (LSB first)
    explicit BitBigInt(uint64_t value);

    // Deleted to be explicit: use static factory functions or explicit constructors
    BitBigInt(const BitBigInt&) = default;
    BitBigInt(BitBigInt&&) = default;
    BitBigInt& operator=(const BitBigInt&) = default;
    BitBigInt& operator=(BitBigInt&&) = default;

    ~BitBigInt() = default;

    // === Static factory functions ===

    /// Create zero
    static BitBigInt zero();

    /// Create one
    static BitBigInt one();

    // === Representation and inspection ===

    /// Return bits as binary string for debugging
    /// Format: "...b1b0" (MSB first on the left) + sign indicator
    /// Example: "101 (positive)" or "011 (negative)"
    std::string to_binary() const;

    /// Check if value is zero
    bool is_zero() const;

    /// Access raw bits (LSB first)
    /// Intended for testing and invariant checking
    const std::vector<uint8_t>& raw_bits() const { return bits_; }

    // === Normalization and maintenance ===

    /// Normalize: remove leading zero bits (for positive numbers)
    /// Postcondition: maintains invariant bits_.size() >= 2
    /// TODO: handle negative numbers (sign_bit == 1) properly
    void normalize();

    // === Arithmetic operations (TODO - scaffolding for future) ===

    // TODO: BitBigInt operator+(const BitBigInt& other) const;
    // TODO: BitBigInt operator-(const BitBigInt& other) const;
    // TODO: BitBigInt operator*(const BitBigInt& other) const;
    // TODO: BitBigInt operator/(const BitBigInt& divisor) const;
    // TODO: BitBigInt modpow(const BitBigInt& exponent, const BitBigInt& modulus) const;

    // === Comparison operations (TODO) ===

    /// Compare with another BigInt
    /// Returns: -1 if this < other, 0 if equal, +1 if this > other
    /// Currently only handles non-negative numbers (sign_bit == 0)
    int compare(const BitBigInt& other) const;

    /// Equality operator
    bool operator==(const BitBigInt& other) const;

    // TODO: bool operator!=(const BitBigInt& other) const;
    // TODO: bool operator<(const BitBigInt& other) const;
    // TODO: bool operator<=(const BitBigInt& other) const;
    // TODO: bool operator>(const BitBigInt& other) const;
    // TODO: bool operator>=(const BitBigInt& other) const;

private:
    std::vector<uint8_t> bits_; ///< Bit storage: LSB first, sign bit last

    /// Verify invariants (used internally and in debug builds)
    void verify_invariants() const;

    /// Helper: ensure minimum size (at least 2 elements for data + sign)
    void ensure_min_size();
};

} // namespace bigint

// TODO(perf/storage): Consider "windowed storage" for fast shifts.
// Idea: Keep limbs in a preallocated buffer (e.g., 2–8 KB) with [start, len] window.
// Shifts by whole limbs can be implemented by moving 'start' (O(1)) instead of memmove.
// When headroom is insufficient (growth/2N intermediates), recenter window or fallback to resize.
// Notes:
// - RSA/modarith frequently uses shifts; legacy code spends lots of time in offset/normalize-like ops.
// - Must support intermediates up to ~2*N limbs for mul/sqr before reduction.
// - Provide debug asserts and a safe fallback path to avoid silent corruption.
