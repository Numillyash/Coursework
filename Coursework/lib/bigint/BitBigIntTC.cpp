#include "BitBigIntTC.hpp"
#include <algorithm>
#include <sstream>

namespace bigint {

// === Constructors ===

BitBigIntTC::BitBigIntTC() : mas_{0, 0} {
    verify_invariants();
}

BitBigIntTC::BitBigIntTC(uint64_t value) {
    if (value == 0) {
        mas_ = {0, 0};
    } else {
        // Convert to binary (LSB first)
        while (value > 0) {
            mas_.push_back(static_cast<uint8_t>(value & 1));
            value >>= 1;
        }
        // Add sign bit (0 for positive)
        mas_.push_back(0);
    }
    verify_invariants();
}

// === Static factory functions ===

BitBigIntTC BitBigIntTC::from_binary_bits(const std::vector<uint8_t>& raw) {
    BitBigIntTC result;
    result.mas_ = raw;
    result.ensure_min_size();
    result.normalize();
    return result;
}

BitBigIntTC BitBigIntTC::zero() {
    return BitBigIntTC();
}

BitBigIntTC BitBigIntTC::one() {
    BitBigIntTC result;
    result.mas_[0] = 1;
    result.verify_invariants();
    return result;
}

// === Representation and inspection ===

std::string BitBigIntTC::to_binary() const {
    if (mas_.empty()) {
        return "(empty - invalid)";
    }

    std::ostringstream oss;

    // All bits except the sign bit (MSB first for readability)
    for (int i = static_cast<int>(mas_.size()) - 2; i >= 0; --i) {
        oss << static_cast<int>(mas_[i]);
    }

    // Add sign indicator
    uint8_t sign = sign_bit();
    oss << " (";
    oss << (sign == 0 ? "positive" : "negative");
    oss << ")";

    return oss.str();
}

bool BitBigIntTC::is_zero() const {
    if (mas_.size() < 2) return false;
    if (mas_.back() != 0) return false; // Sign bit must be 0

    for (size_t i = 0; i < mas_.size() - 1; ++i) {
        if (mas_[i] != 0) return false;
    }
    return true;
}

// === Comparison ===

int BitBigIntTC::compare(const BitBigIntTC& other) const {
    if (mas_.size() < 2 || other.mas_.size() < 2) {
        return 0; // Both invalid, treat as equal
    }

    int my_len = static_cast<int>(mas_.size()) - 2;
    int other_len = static_cast<int>(other.mas_.size()) - 2;

    // Remove leading zeros for positive numbers
    while (my_len > 0 && mas_[my_len] == 0 && mas_[my_len + 1] == 0) --my_len;
    while (other_len > 0 && other.mas_[other_len] == 0 && other.mas_[other_len + 1] == 0) --other_len;

    if (my_len != other_len) {
        return (my_len < other_len) ? -1 : 1;
    }

    for (int i = my_len; i >= 0; --i) {
        if (mas_[i] != other.mas_[i]) {
            return (mas_[i] < other.mas_[i]) ? -1 : 1;
        }
    }
    return 0;
}

bool BitBigIntTC::operator==(const BitBigIntTC& other) const {
    return compare(other) == 0;
}

// === Normalization (ported from normalize in bit_LA.c) ===

void BitBigIntTC::normalize() {
    if (mas_.size() < 2) {
        ensure_min_size();
        return;
    }

    int i;
    int end = static_cast<int>(mas_.size()) - 2;
    uint8_t sign_byte = mas_.back();

    if (sign_byte) {
        // Negative: find last occurrence of (0, 1) or (1, 0) pair
        for (i = static_cast<int>(mas_.size()) - 2; i > 0; i--) {
            end = i;
            if (mas_[i] == 0 || mas_[i - 1] == 0) {
                break;
            }
        }
    } else {
        // Positive: find last 1 bit
        for (i = static_cast<int>(mas_.size()) - 2; i >= 0; i--) {
            end = i;
            if (mas_[i] != 0) {
                break;
            }
        }
    }

    // Resize to end + 2 (data bits [0..end] + sign bit)
    mas_.resize(end + 2);
    mas_.back() = sign_byte;

    ensure_min_size();
    verify_invariants();
}

// === Helper: add_digit (ported from bit_LA.c) ===

void BitBigIntTC::add_digit(uint8_t value) {
    if (mas_.size() == 0) {
        mas_.push_back(value);
        mas_.push_back(0); // sign bit
    } else {
        // Insert value before sign bit, shifting sign bit one position right
        // Save current sign bit position
        uint8_t sign_bit = mas_.back();
        
        // Insert new value before the sign bit
        mas_.insert(mas_.end() - 1, value);
        
        // Restore sign bit at the end
        mas_.back() = sign_bit;
    }

    verify_invariants();
}

// === reverse (ported from bit_LA.c) ===

void BitBigIntTC::reverse() {
    if (mas_.size() <= 1) {
        return;
    }

    // Save original sign bit
    uint8_t original_sign = mas_.back();
    int data_count = static_cast<int>(mas_.size()) - 1;

    // Create temporary with sign-only state (matching legacy init())
    BitBigIntTC prom;
    prom.mas_.assign(1, 0);  // Only sign bit, like legacy init()

    // Reverse: add data bits in backward order
    for (int idx = data_count - 1; idx >= 0; --idx) {
        prom.add_digit(mas_[idx]);
    }

    // Verify: prom must have same size as original after loop
    // (because add_digit adds one element per iteration, starting from size 1)
    assert(prom.mas_.size() == mas_.size() && 
           "Invariant: prom.size() must equal original size after reversing");

    // Copy reversed data back (not changing size of mas_)
    for (int i = 0; i < data_count; ++i) {
        mas_[i] = prom.mas_[i];
    }

    // Restore original sign bit
    mas_.back() = original_sign;

    verify_invariants();
}

// === offset_left (ported from bit_LA.c) ===

void BitBigIntTC::offset_left() {
    reverse();
    add_digit(0);
    
    // Swap data bits at positions [current_count-2] and [current_count-1]
    if (mas_.size() >= 2) {
        std::swap(mas_[mas_.size() - 2], mas_[mas_.size() - 1]);
    }
    
    reverse();
    normalize();
}

// === offset_right (ported from bit_LA.c) ===

void BitBigIntTC::offset_right() {
    if (mas_.size() == 2) {
        // Number has only data_bit + sign_bit; dividing by 2 gives 0
        mas_ = {0, 0};
    } else {
        reverse();
        
        // Move sign bit down one position
        if (mas_.size() > 1) {
            mas_[mas_.size() - 2] = mas_[mas_.size() - 1];
            mas_.pop_back();
        }
        
        reverse();
        normalize();
    }
}

// === Addition (ported from bit_LA.c addition) ===

BitBigIntTC BitBigIntTC::add(const BitBigIntTC& other) const {
    // Create copies to work with (normalize on entry)
    BitBigIntTC summand = *this;
    BitBigIntTC addend = other;
    
    summand.normalize();
    addend.normalize();
    
    // Ensure summand is the larger one (or equal)
    if (summand.mas_.size() < addend.mas_.size()) {
        std::swap(summand, addend);
    }
    
    BitBigIntTC carry;
    carry.mas_.assign(1, 0);  // init() equivalent
    int oper_sign = 0;
    int real_symb = 0;
    int max_symb = 0;
    
    // Both negative: convert to two's complement and recurse
    if (summand.mas_.back() && addend.mas_.back()) {
        summand.additional_code();
        addend.additional_code();
        carry = summand.add(addend);
        carry.additional_code();
        
        carry.normalize();
        return carry;
    }
    
    // Mixed signs or both positive: bit-level addition
    oper_sign = (int)summand.mas_.back() + (int)addend.mas_.back();
    real_symb = static_cast<int>(summand.mas_.size());
    
    while (!addend.is_zero()) {
        max_symb = std::max({static_cast<int>(summand.mas_.size()),
                              static_cast<int>(addend.mas_.size()),
                              static_cast<int>(carry.mas_.size())});
        
        if (max_symb == 2) {
            max_symb++;
            real_symb++;
        }
        
        // Sign-extend summand
        for (int i = max_symb - (int)summand.mas_.size(); i > 0; i--) {
            if (summand.mas_.back()) {
                summand.add_digit(1);
            } else {
                summand.add_digit(0);
            }
        }
        
        // Sign-extend addend
        for (int i = max_symb - (int)addend.mas_.size(); i > 0; i--) {
            if (addend.mas_.back()) {
                addend.add_digit(1);
            } else {
                addend.add_digit(0);
            }
        }
        
        // Sign-extend carry
        for (int i = max_symb - (int)carry.mas_.size(); i > 0; i--) {
            if (carry.mas_.back()) {
                carry.add_digit(1);
            } else {
                carry.add_digit(0);
            }
        }
        
        // carry = summand & addend (bitwise AND)
        for (int i = 0; i < max_symb; i++) {
            carry.mas_[i] = summand.mas_[i] & addend.mas_[i];
        }
        
        // summand = summand ^ addend (bitwise XOR)
        for (int i = 0; i < max_symb; i++) {
            summand.mas_[i] = summand.mas_[i] ^ addend.mas_[i];
        }
        
        // addend = carry << 1 (left shift of carry)
        addend = carry;
        addend.offset_left();
        
        // Truncate result if needed for mixed signs
        if (oper_sign == 1) {
            if ((int)summand.mas_.size() > real_symb) {
                summand.mas_[real_symb] = 0;
            }
        }
    }
    
    summand.normalize();
    return summand;
}

// === Private: Two's complement (additional code) ===

void BitBigIntTC::additional_code() {
    if (!is_zero()) {
        uint8_t addit_digit = 1;
        
        // Invert all bits and add 1
        for (size_t i = 0; i < mas_.size(); i++) {
            mas_[i] = !mas_[i];  // NOT
            mas_[i] = mas_[i] ^ addit_digit;  // XOR with carry
            if (mas_[i]) {
                addit_digit = 0;
            }
        }
    }
}

// === Private helpers ===

void BitBigIntTC::verify_invariants() const {
    // Allow size >= 1 to support legacy init() semantics (temporary sign-only state)
    // Public normalized values will have size >= 2 via ensure_min_size()
    assert(mas_.size() >= 1 && "Invariant: mas_.size() >= 1");

    // Check that all elements are 0 or 1
    for (uint8_t bit : mas_) {
        assert((bit == 0 || bit == 1) && "Invariant: each element is 0 or 1");
    }
}

void BitBigIntTC::ensure_min_size() {
    while (mas_.size() < 2) {
        mas_.push_back(0);
    }
}

} // namespace bigint
