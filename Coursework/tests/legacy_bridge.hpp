#pragma once

#include <algorithm>
#include <cstdlib>
#include <new>
#include <string>
#include <sstream>
#include <vector>

// Legacy C headers (wrapped for C++ use)
extern "C" {
#include "bit_LA.h"
}

#include "BitBigIntTC.hpp"

namespace bigint {
namespace test {

/**
 * Legacy bridge: functions to convert between legacy number and BitBigIntTC
 * Used only for testing equivalence between old and new implementations
 */

/// Convert legacy number to BitBigIntTC
inline BitBigIntTC tc_from_legacy(const number& n) {
    std::vector<uint8_t> bits(n.mas, n.mas + n.current_count);
    return BitBigIntTC::from_binary_bits(bits);
}

/// Build a legacy number from raw bit storage (LSB first, sign bit last).
inline number legacy_from_bits(const std::vector<uint8_t>& bits) {
    number result = init();
    clear_mem(&result);

    result.mas = (uint8_t *)malloc(sizeof(uint8_t) * bits.size());
    if (result.mas == nullptr) {
        throw std::bad_alloc();
    }

    std::copy(bits.begin(), bits.end(), result.mas);
    result.current_count = (int)bits.size();
    result.size = (int)bits.size();

    return result;
}

/// Convert BitBigIntTC to legacy number
inline number legacy_from_tc(const BitBigIntTC& x) {
    number result = init();
    clear_mem(&result);
    
    const auto& raw = x.raw();
    result.mas = (uint8_t *)malloc(sizeof(uint8_t) * raw.size());
    if (result.mas == nullptr) {
        throw std::bad_alloc();
    }
    
    std::copy(raw.begin(), raw.end(), result.mas);
    result.current_count = (int)raw.size();
    result.size = (int)raw.size();
    
    return result;
}

/// Convert legacy number to binary string for debugging
inline std::string legacy_to_binary(const number& n) {
    if (!n.mas || n.current_count < 2) {
        return "(invalid)";
    }

    std::ostringstream oss;

    // All bits except sign bit (MSB first)
    for (int i = n.current_count - 2; i >= 0; --i) {
        oss << (int)n.mas[i];
    }

    oss << " (";
    oss << (n.mas[n.current_count - 1] == 0 ? "positive" : "negative");
    oss << ")";

    return oss.str();
}

/// Convert BitBigIntTC to binary string (same format as legacy_to_binary)
inline std::string tc_to_binary(const BitBigIntTC& x) {
    return x.to_binary();
}

inline std::string bits_to_debug(const std::vector<uint8_t>& bits) {
    std::ostringstream oss;

    oss << "[";
    for (size_t i = 0; i < bits.size(); ++i) {
        if (i)
            oss << ",";
        oss << (int)bits[i];
    }
    oss << "]";
    return oss.str();
}

} // namespace test
} // namespace bigint
