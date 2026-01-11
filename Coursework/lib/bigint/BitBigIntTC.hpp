#pragma once

#include <vector>
#include <cstdint>
#include <string>
#include <cassert>

namespace bigint {

/**
 * BitBigIntTC - Two's Complement BigInt backend
 *
 * Legacy-compatible arbitrary-precision integer using bit-level storage.
 * Designed as a 1:1 port of the legacy number/bit_LA.c implementation to C++.
 *
 * Internal representation (identical to legacy):
 * - Stores bits in a vector<uint8_t> where each element is 0 or 1
 * - Bit-endian: LSB first (index 0 is least significant bit)
 * - Sign bit: stored as the last element (mas_.back())
 *   * 0 = non-negative
 *   * 1 = negative
 * - Sign extension: for negative numbers, all high bits are 1 (two's complement)
 *
 * Invariant:
 * - mas_.size() >= 2 (minimum: data_bit + sign_bit)
 * - Zero is represented as {0, 0}
 * - All elements in mas_ are either 0 or 1
 *
 * Design:
 * - NO algorithmic changes from legacy: 1:1 function porting
 * - RAII: std::vector for memory management (replaces malloc/free/clear_mem)
 * - Copy/move semantics: default
 * - No exceptions: use assertions for invariants
 *
 * This backend is used for:
 * - Full arbitrary-precision arithmetic (unlike BitBigInt which starts simpler)
 * - Testing equivalence with legacy bit_LA.c
 * - Foundation for RSA fast-path (Phase 2 in ROADMAP_bigint.md)
 */
class BitBigIntTC final {
public:
    // === Constructors ===

    /// Default constructor: creates zero {0, 0}
    BitBigIntTC();

    /// Construct from unsigned 64-bit integer
    explicit BitBigIntTC(uint64_t value);

    // Copy/move: use default
    BitBigIntTC(const BitBigIntTC&) = default;
    BitBigIntTC(BitBigIntTC&&) = default;
    BitBigIntTC& operator=(const BitBigIntTC&) = default;
    BitBigIntTC& operator=(BitBigIntTC&&) = default;

    ~BitBigIntTC() = default;

    // === Static factory functions ===

    /// Create from raw bit vector (will normalize)
    static BitBigIntTC from_binary_bits(const std::vector<uint8_t>& raw);

    /// Create zero
    static BitBigIntTC zero();

    /// Create one
    static BitBigIntTC one();

    // === Representation and inspection ===

    /// Access raw bits (LSB first, sign bit as last)
    const std::vector<uint8_t>& raw() const noexcept { return mas_; }

    /// Number of bits (including sign bit)
    int current_count() const noexcept { return static_cast<int>(mas_.size()); }

    /// Get sign bit (0 = non-negative, 1 = negative)
    uint8_t sign_bit() const noexcept {
        return mas_.empty() ? 0 : mas_.back();
    }

    /// Return bits as binary string for debugging
    std::string to_binary() const;

    /// Check if value is zero
    bool is_zero() const;

    // === Comparison operations ===

    /// Compare with another number: -1 if this < other, 0 if equal, +1 if greater
    int compare(const BitBigIntTC& other) const;

    /// Equality operator
    bool operator==(const BitBigIntTC& other) const;

    // === Normalization and bit shifting ===

    /// Normalize: remove leading sign-extended bits
    /// Port of normalize(number*) from bit_LA.c
    void normalize();

    /// Reverse the bit order (in-place)
    /// Port of reverse(number*) from bit_LA.c
    /// Note: Only reverses data bits (not the sign bit)
    void reverse();

    /// Left shift: multiply by 2 (binary 10)
    /// Port of offset_left(number*) from bit_LA.c
    void offset_left();

    /// Right shift: divide by 2 (rounding towards zero for negatives)
    /// Port of offset_right(number*) from bit_LA.c
    void offset_right();

    // === Arithmetic operations ===

    /// Add two numbers (port of addition(number*, number*) from bit_LA.c)
    /// Returns the sum of this and other
    BitBigIntTC add(const BitBigIntTC& other) const;

    /// Subtract other from this (port of difference(number*, number*) from bit_LA.c)
    /// Returns this - other
    BitBigIntTC sub(const BitBigIntTC& other) const;

    // TODO: BitBigIntTC operator*(const BitBigIntTC& other) const;
    // TODO: BitBigIntTC operator/(const BitBigIntTC& divisor) const;
    // TODO: BitBigIntTC operator%(const BitBigIntTC& modulus) const;
    // TODO: BitBigIntTC powmod(const BitBigIntTC& exponent, const BitBigIntTC& modulus) const;

    /// Helper: add a digit before the sign bit (used during operations, exposed for testing)
    /// Similar to add_digit(number*, uint8_t) in legacy bit_LA.c
    void add_digit(uint8_t value);

private:
    std::vector<uint8_t> mas_; ///< Bit storage: LSB first, sign bit last

    /// Two's complement (invert all bits and add 1)
    /// Port of additional_code(number*) from bit_LA.c
    void additional_code();

    /// Verify invariants (debug only)
    void verify_invariants() const;

    /// Ensure minimum size (at least 2 elements)
    void ensure_min_size();
};

} // namespace bigint

