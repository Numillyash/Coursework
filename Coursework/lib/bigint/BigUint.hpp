#pragma once

#include <cstddef>
#include <cstdint>
#include <utility>
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
    BigUint mul_schoolbook(const BigUint& other) const;
    BigUint square() const;
    std::pair<BigUint, BigUint> divmod(const BigUint& divisor) const;
    BigUint div(const BigUint& divisor) const;
    BigUint mod(const BigUint& divisor) const;
    BigUint shift_left_bits(size_t bits) const;
    BigUint shift_right_bits(size_t bits) const;
    bool is_even() const noexcept;
    bool is_odd() const noexcept;

    static BigUint gcd(BigUint a, BigUint b);
    static BigUint mod_add(
            const BigUint& a, const BigUint& b, const BigUint& modulus);
    static BigUint mod_sub(
            const BigUint& a, const BigUint& b, const BigUint& modulus);
    static BigUint mod_mul(
            const BigUint& a, const BigUint& b, const BigUint& modulus);

    uint64_t to_uint64_for_testing() const;

private:
    std::vector<Limb> limbs_;

    explicit BigUint(std::vector<Limb> limbs);

    void normalize();
    void verify_invariants() const;
};

} // namespace bigint
