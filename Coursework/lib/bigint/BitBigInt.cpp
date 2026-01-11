#include "BitBigInt.hpp"
#include <algorithm>
#include <sstream>

namespace bigint {

// === Constructors ===

BitBigInt::BitBigInt() : bits_{0, 0} {
    // Zero: one data bit (0) + one sign bit (0)
    verify_invariants();
}

BitBigInt::BitBigInt(uint64_t value) {
    if (value == 0) {
        bits_ = {0, 0};
    } else {
        // Convert to binary (LSB first)
        while (value > 0) {
            bits_.push_back(static_cast<uint8_t>(value & 1));
            value >>= 1;
        }
        // Add sign bit (0 for positive)
        bits_.push_back(0);
    }
    verify_invariants();
}

// === Static factory functions ===

BitBigInt BitBigInt::zero() {
    return BitBigInt();
}

BitBigInt BitBigInt::one() {
    BitBigInt result;
    result.bits_[0] = 1; // Set LSB to 1
    result.verify_invariants();
    return result;
}

// === Representation and inspection ===

std::string BitBigInt::to_binary() const {
    if (bits_.empty()) {
        return "(empty - invalid)";
    }

    // Build string: MSB first (reverse of internal storage)
    std::ostringstream oss;

    // All bits except the sign bit
    for (int i = static_cast<int>(bits_.size()) - 2; i >= 0; --i) {
        oss << static_cast<int>(bits_[i]);
    }

    // Add sign indicator
    uint8_t sign_bit = bits_.back();
    oss << " (";
    oss << (sign_bit == 0 ? "positive" : "negative");
    oss << ")";

    return oss.str();
}

bool BitBigInt::is_zero() const {
    if (bits_.size() < 2) return false; // Invalid state
    if (bits_.back() != 0) return false; // Sign bit must be 0

    // All data bits must be 0
    for (size_t i = 0; i < bits_.size() - 1; ++i) {
        if (bits_[i] != 0) return false;
    }
    return true;
}

// === Normalization ===

void BitBigInt::normalize() {
    if (bits_.size() < 2) {
        ensure_min_size();
        return;
    }

    uint8_t sign_bit = bits_.back();

    if (sign_bit == 0) {
        // Positive number: remove leading zeros
        // Keep at least one data bit (before sign)
        while (bits_.size() > 2 && bits_[bits_.size() - 2] == 0) {
            bits_.erase(bits_.end() - 2);
        }
    } else {
        // Negative number: TODO - implement two's complement normalization
        // For now, just ensure minimum size
    }

    ensure_min_size();
    verify_invariants();
}

// === Private helpers ===

void BitBigInt::verify_invariants() const {
    // Check minimum size
    assert(bits_.size() >= 2 && "Invariant: bits_.size() >= 2");

    // Check that all elements are 0 or 1
    for (uint8_t bit : bits_) {
        assert((bit == 0 || bit == 1) && "Invariant: each element is 0 or 1");
    }
}

void BitBigInt::ensure_min_size() {
    while (bits_.size() < 2) {
        bits_.push_back(0);
    }
}

} // namespace bigint
