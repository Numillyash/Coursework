#include "BigUint.hpp"

#include <algorithm>
#include <cassert>
#include <limits>
#include <stdexcept>

namespace bigint {

BigUint::BigUint() = default;

BigUint::BigUint(uint64_t value) {
    while (value != 0) {
        limbs_.push_back(static_cast<Limb>(value));
        value >>= LIMB_BITS;
    }
    verify_invariants();
}

BigUint::BigUint(std::vector<Limb> limbs) : limbs_(std::move(limbs)) {
    normalize();
}

BigUint BigUint::zero() {
    return BigUint();
}

BigUint BigUint::one() {
    return BigUint(static_cast<uint64_t>(1));
}

BigUint BigUint::from_limbs(std::vector<Limb> limbs) {
    return BigUint(std::move(limbs));
}

const std::vector<BigUint::Limb>& BigUint::limbs() const noexcept {
    return limbs_;
}

bool BigUint::is_zero() const noexcept {
    return limbs_.empty();
}

size_t BigUint::limb_count() const noexcept {
    return limbs_.size();
}

size_t BigUint::bit_length() const {
    if (limbs_.empty())
        return 0;

    Limb top = limbs_.back();
    size_t bits = (limbs_.size() - 1) * LIMB_BITS;
    while (top != 0) {
        ++bits;
        top >>= 1;
    }
    return bits;
}

bool BigUint::test_bit(size_t bit) const {
    size_t limb_index = bit / LIMB_BITS;
    if (limb_index >= limbs_.size())
        return false;

    unsigned bit_index = static_cast<unsigned>(bit % LIMB_BITS);
    return ((limbs_[limb_index] >> bit_index) & Limb{1}) != 0;
}

void BigUint::set_bit(size_t bit) {
    size_t limb_index = bit / LIMB_BITS;
    unsigned bit_index = static_cast<unsigned>(bit % LIMB_BITS);
    if (limb_index >= limbs_.size())
        limbs_.resize(limb_index + 1, 0);
    limbs_[limb_index] |= (Limb{1} << bit_index);
    normalize();
}

int BigUint::compare(const BigUint& other) const {
    if (limbs_.size() < other.limbs_.size())
        return -1;
    if (limbs_.size() > other.limbs_.size())
        return 1;

    for (size_t i = limbs_.size(); i > 0; --i) {
        Limb lhs = limbs_[i - 1];
        Limb rhs = other.limbs_[i - 1];
        if (lhs < rhs)
            return -1;
        if (lhs > rhs)
            return 1;
    }
    return 0;
}

bool BigUint::operator==(const BigUint& other) const {
    return limbs_ == other.limbs_;
}

bool BigUint::operator!=(const BigUint& other) const {
    return !(*this == other);
}

BigUint BigUint::add(const BigUint& other) const {
    const size_t n = std::max(limbs_.size(), other.limbs_.size());
    std::vector<Limb> result;
    result.reserve(n + 1);

    Limb carry = 0;
    for (size_t i = 0; i < n; ++i) {
        Limb lhs = i < limbs_.size() ? limbs_[i] : 0;
        Limb rhs = i < other.limbs_.size() ? other.limbs_[i] : 0;
        result.push_back(limb::add_carry(lhs, rhs, carry, carry));
    }
    if (carry != 0)
        result.push_back(carry);

    return BigUint(std::move(result));
}

BigUint BigUint::sub_abs(const BigUint& other) const {
    if (compare(other) < 0)
        throw std::invalid_argument("BigUint::sub_abs requires lhs >= rhs");

    std::vector<Limb> result;
    result.reserve(limbs_.size());

    Limb borrow = 0;
    for (size_t i = 0; i < limbs_.size(); ++i) {
        Limb rhs = i < other.limbs_.size() ? other.limbs_[i] : 0;
        result.push_back(limb::sub_borrow(limbs_[i], rhs, borrow, borrow));
    }

    return BigUint(std::move(result));
}

BigUint BigUint::mul_schoolbook(const BigUint& other) const {
    if (is_zero() || other.is_zero())
        return BigUint::zero();

    std::vector<Limb> result(limbs_.size() + other.limbs_.size(), 0);

    for (size_t i = 0; i < limbs_.size(); ++i) {
        limb::dlimb_t carry = 0;
        for (size_t j = 0; j < other.limbs_.size(); ++j) {
            limb::dlimb_t current =
                    static_cast<limb::dlimb_t>(result[i + j])
                    + static_cast<limb::dlimb_t>(limbs_[i])
                            * static_cast<limb::dlimb_t>(other.limbs_[j])
                    + carry;
            result[i + j] = static_cast<Limb>(current);
            carry = current >> LIMB_BITS;
        }

        size_t k = i + other.limbs_.size();
        while (carry != 0) {
            if (k == result.size())
                result.push_back(0);
            limb::dlimb_t current =
                    static_cast<limb::dlimb_t>(result[k]) + carry;
            result[k] = static_cast<Limb>(current);
            carry = current >> LIMB_BITS;
            ++k;
        }
    }

    return BigUint(std::move(result));
}

BigUint BigUint::square() const {
    return mul_schoolbook(*this);
}

BigUint BigUint::shift_left_bits(size_t bits) const {
    if (is_zero() || bits == 0)
        return *this;

    size_t word_shift = bits / LIMB_BITS;
    unsigned bit_shift = static_cast<unsigned>(bits % LIMB_BITS);

    std::vector<Limb> result(word_shift, 0);
    result.reserve(limbs_.size() + word_shift + 1);

    Limb carry = 0;
    for (Limb limb : limbs_) {
        if (bit_shift == 0) {
            result.push_back(limb);
        } else {
            result.push_back(static_cast<Limb>((limb << bit_shift) | carry));
            carry = static_cast<Limb>(limb >> (LIMB_BITS - bit_shift));
        }
    }
    if (bit_shift != 0 && carry != 0)
        result.push_back(carry);

    return BigUint(std::move(result));
}

BigUint BigUint::shift_right_bits(size_t bits) const {
    if (is_zero() || bits == 0)
        return *this;

    size_t word_shift = bits / LIMB_BITS;
    unsigned bit_shift = static_cast<unsigned>(bits % LIMB_BITS);
    if (word_shift >= limbs_.size())
        return BigUint::zero();

    std::vector<Limb> result(limbs_.size() - word_shift, 0);
    if (bit_shift == 0) {
        for (size_t i = word_shift; i < limbs_.size(); ++i)
            result[i - word_shift] = limbs_[i];
    } else {
        Limb carry = 0;
        for (size_t i = limbs_.size(); i > word_shift; --i) {
            Limb current = limbs_[i - 1];
            result[i - 1 - word_shift] =
                    static_cast<Limb>((current >> bit_shift) | carry);
            carry = static_cast<Limb>(current << (LIMB_BITS - bit_shift));
        }
    }

    return BigUint(std::move(result));
}

uint64_t BigUint::to_uint64_for_testing() const {
    if (limbs_.size() > 2)
        throw std::overflow_error("BigUint does not fit uint64_t");

    uint64_t result = 0;
    if (!limbs_.empty())
        result |= static_cast<uint64_t>(limbs_[0]);
    if (limbs_.size() == 2)
        result |= static_cast<uint64_t>(limbs_[1]) << LIMB_BITS;
    return result;
}

void BigUint::normalize() {
    while (!limbs_.empty() && limbs_.back() == 0)
        limbs_.pop_back();
    verify_invariants();
}

void BigUint::verify_invariants() const {
    assert(limbs_.empty() || limbs_.back() != 0);
}

} // namespace bigint
