#pragma once

#include <cstddef>
#include <cstdint>
#include <vector>

#include "LimbOps.hpp"

namespace bigint {

class BigUint final {
public:
    using Limb = limb::limb_t;
    static constexpr unsigned LIMB_BITS = limb::LIMB_BITS;

    BigUint();
    explicit BigUint(uint64_t value);

    static BigUint zero();
    static BigUint one();
    static BigUint from_limbs(std::vector<Limb> limbs);

    const std::vector<Limb>& limbs() const noexcept;
    bool is_zero() const noexcept;
    size_t limb_count() const noexcept;
    size_t bit_length() const;
    bool test_bit(size_t bit) const;
    void set_bit(size_t bit);

    int compare(const BigUint& other) const;
    bool operator==(const BigUint& other) const;
    bool operator!=(const BigUint& other) const;

    BigUint add(const BigUint& other) const;
    BigUint sub_abs(const BigUint& other) const;
    BigUint shift_left_bits(size_t bits) const;
    BigUint shift_right_bits(size_t bits) const;

    uint64_t to_uint64_for_testing() const;

private:
    std::vector<Limb> limbs_;

    explicit BigUint(std::vector<Limb> limbs);

    void normalize();
    void verify_invariants() const;
};

} // namespace bigint
