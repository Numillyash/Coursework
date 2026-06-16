#include <cstdint>
#include <cstdlib>
#include <iostream>
#include <limits>
#include <numeric>
#include <random>
#include <stdexcept>
#include <string>
#include <tuple>
#include <vector>

#include "BigUint.hpp"
#include "BitBigIntTC.hpp"
#include "LimbOps.hpp"

using bigint::BigUint;
using bigint::BitBigIntTC;
using bigint::limb::dlimb_t;
using bigint::limb::limb_t;

namespace {

[[noreturn]] void fail(const std::string& message) {
    std::cerr << "FAIL: " << message << std::endl;
    std::exit(1);
}

void expect(bool condition, const std::string& message) {
    if (!condition)
        fail(message);
}

template <typename T>
void expect_eq(T actual, T expected, const std::string& message) {
    if (actual != expected) {
        std::cerr << "FAIL: " << message << std::endl;
        std::cerr << "  actual:   " << actual << std::endl;
        std::cerr << "  expected: " << expected << std::endl;
        std::exit(1);
    }
}

void expect_limbs(
        const BigUint& value,
        const std::vector<BigUint::Limb>& expected,
        const std::string& message) {
    if (value.limbs() != expected) {
        std::cerr << "FAIL: " << message << std::endl;
        std::cerr << "  actual:   [";
        for (size_t i = 0; i < value.limbs().size(); ++i) {
            if (i)
                std::cerr << ",";
            std::cerr << value.limbs()[i];
        }
        std::cerr << "]\n  expected: [";
        for (size_t i = 0; i < expected.size(); ++i) {
            if (i)
                std::cerr << ",";
            std::cerr << expected[i];
        }
        std::cerr << "]" << std::endl;
        std::exit(1);
    }
}

void test_section(const char* name) {
    std::cout << "\n=== " << name << " ===" << std::endl;
}

std::string limbs_to_debug(const BigUint& value) {
    std::string result = "[";
    for (size_t i = 0; i < value.limbs().size(); ++i) {
        if (i)
            result += ",";
        result += std::to_string(value.limbs()[i]);
    }
    result += "]";
    return result;
}

BitBigIntTC tc_from_biguint(const BigUint& value) {
    if (value.is_zero())
        return BitBigIntTC::from_binary_bits({0, 0});

    std::vector<uint8_t> bits;
    bits.reserve(value.bit_length() + 1);
    for (size_t i = 0; i < value.bit_length(); ++i)
        bits.push_back(value.test_bit(i) ? 1 : 0);
    bits.push_back(0);
    return BitBigIntTC::from_binary_bits(bits);
}

BigUint biguint_from_positive_tc(const BitBigIntTC& value) {
    if (value.sign_bit() != 0) {
        std::cerr << "FAIL: BitBigIntTC oracle produced negative value: "
                  << value.to_binary() << std::endl;
        std::exit(1);
    }

    BigUint result;
    const auto& raw = value.raw();
    if (raw.empty())
        return result;

    for (size_t i = 0; i + 1 < raw.size(); ++i) {
        if (raw[i])
            result.set_bit(i);
    }
    return result;
}

void expect_same_as_tc(
        const BigUint& actual,
        const BitBigIntTC& expected,
        const std::string& message) {
    BigUint converted = biguint_from_positive_tc(expected);
    if (actual != converted) {
        std::cerr << "FAIL: " << message << std::endl;
        std::cerr << "  BigUint: " << limbs_to_debug(actual) << std::endl;
        std::cerr << "  TC:      " << expected.to_binary() << std::endl;
        std::cerr << "  TC->BU:  " << limbs_to_debug(converted) << std::endl;
        std::exit(1);
    }
}

void expect_divmod_invariant(
        const BigUint& dividend,
        const BigUint& divisor,
        const std::string& message) {
    auto dm = dividend.divmod(divisor);
    BigUint recomposed = dm.first.mul_schoolbook(divisor).add(dm.second);
    if (recomposed != dividend || dm.second.compare(divisor) >= 0) {
        std::cerr << "FAIL: " << message << std::endl;
        std::cerr << "  dividend: " << limbs_to_debug(dividend) << std::endl;
        std::cerr << "  divisor:  " << limbs_to_debug(divisor) << std::endl;
        std::cerr << "  quotient: " << limbs_to_debug(dm.first) << std::endl;
        std::cerr << "  rem:      " << limbs_to_debug(dm.second) << std::endl;
        std::exit(1);
    }
}

void expect_mod_result(
        const BigUint& result,
        const BigUint& modulus,
        const std::string& message) {
    if (modulus.is_zero() || result.compare(modulus) >= 0) {
        std::cerr << "FAIL: " << message << std::endl;
        std::cerr << "  result:  " << limbs_to_debug(result) << std::endl;
        std::cerr << "  modulus: " << limbs_to_debug(modulus) << std::endl;
        std::exit(1);
    }
}

std::vector<BigUint> oracle_values() {
    std::vector<BigUint> values = {
        BigUint(0),
        BigUint(1),
        BigUint(UINT32_MAX),
        BigUint(uint64_t{UINT32_MAX} + 1),
        BigUint(uint64_t{UINT32_MAX} + 2),
        BigUint(UINT64_MAX),
        BigUint::from_limbs({UINT32_MAX}),
        BigUint::from_limbs({UINT32_MAX, UINT32_MAX}),
        BigUint::from_limbs({UINT32_MAX, UINT32_MAX, UINT32_MAX}),
        BigUint::from_limbs({0xaaaaaaaaU, 0x55555555U, 0xffffffffU}),
        BigUint::from_limbs(
                {0x12345678U, 0x9abcdef0U, 0x0badc0deU, 0xffffffffU}),
    };

    const std::vector<size_t> power_bits = {
        0, 1, 31, 32, 33, 63, 64, 65, 100, 127, 128,
        191, 255, 256, 257, 384, 511, 512
    };
    for (size_t bit : power_bits)
        values.push_back(BigUint(1).shift_left_bits(bit));

    const std::vector<size_t> random_bits = {
        1, 2, 3, 4, 5, 7, 8, 15, 16, 31, 32, 33, 63, 64, 65,
        96, 127, 128, 191, 255, 256, 257, 384, 512
    };
    const std::vector<uint64_t> seeds = {
        0x12345678ULL, 0xdeadbeefULL, 0xc0ffeeULL
    };
    for (uint64_t seed : seeds) {
        std::mt19937_64 rng(seed);
        for (size_t bits : random_bits) {
            BigUint value;
            for (size_t bit = 0; bit < bits; ++bit) {
                if ((rng() & 1U) != 0)
                    value.set_bit(bit);
            }
            if (bits > 0)
                value.set_bit(bits - 1);
            values.push_back(value);
        }
    }

    return values;
}

BitBigIntTC tc_shift_left(BitBigIntTC value, size_t shift) {
    for (size_t i = 0; i < shift; ++i)
        value.offset_left();
    return value;
}

BitBigIntTC tc_shift_right(BitBigIntTC value, size_t shift) {
    for (size_t i = 0; i < shift; ++i)
        value.offset_right();
    return value;
}

void test_limb_ops() {
    test_section("LimbOps");

    limb_t carry = 99;
    expect_eq(bigint::limb::add_carry(0, 0, 0, carry), limb_t{0},
            "add_carry 0+0 result");
    expect_eq(carry, limb_t{0}, "add_carry 0+0 carry");

    expect_eq(bigint::limb::add_carry(1, 1, 0, carry), limb_t{2},
            "add_carry 1+1 result");
    expect_eq(carry, limb_t{0}, "add_carry 1+1 carry");

    expect_eq(bigint::limb::add_carry(
                    std::numeric_limits<limb_t>::max(), 1, 0, carry),
            limb_t{0}, "add_carry max+1 result");
    expect_eq(carry, limb_t{1}, "add_carry max+1 carry");

    expect_eq(bigint::limb::add_carry(
                    std::numeric_limits<limb_t>::max(),
                    std::numeric_limits<limb_t>::max(), 0, carry),
            std::numeric_limits<limb_t>::max() - 1,
            "add_carry max+max result");
    expect_eq(carry, limb_t{1}, "add_carry max+max carry");

    expect_eq(bigint::limb::add_carry(
                    std::numeric_limits<limb_t>::max(), 0, 1, carry),
            limb_t{0}, "add_carry carry_in result");
    expect_eq(carry, limb_t{1}, "add_carry carry_in carry");

    limb_t borrow = 99;
    expect_eq(bigint::limb::sub_borrow(1, 1, 0, borrow), limb_t{0},
            "sub_borrow 1-1 result");
    expect_eq(borrow, limb_t{0}, "sub_borrow 1-1 borrow");

    expect_eq(bigint::limb::sub_borrow(0, 1, 0, borrow),
            std::numeric_limits<limb_t>::max(), "sub_borrow 0-1 result");
    expect_eq(borrow, limb_t{1}, "sub_borrow 0-1 borrow");

    expect_eq(bigint::limb::sub_borrow(0, 0, 1, borrow),
            std::numeric_limits<limb_t>::max(),
            "sub_borrow borrow propagation result");
    expect_eq(borrow, limb_t{1}, "sub_borrow borrow propagation borrow");

    expect_eq(bigint::limb::sub_borrow(
                    std::numeric_limits<limb_t>::max(), 1, 0, borrow),
            std::numeric_limits<limb_t>::max() - 1,
            "sub_borrow max-1 result");
    expect_eq(borrow, limb_t{0}, "sub_borrow max-1 borrow");

    limb_t lo = 99;
    limb_t hi = 99;
    bigint::limb::mul_wide(0, 12345, lo, hi);
    expect_eq(lo, limb_t{0}, "mul_wide zero lo");
    expect_eq(hi, limb_t{0}, "mul_wide zero hi");

    bigint::limb::mul_wide(1, std::numeric_limits<limb_t>::max(), lo, hi);
    expect_eq(lo, std::numeric_limits<limb_t>::max(), "mul_wide one*max lo");
    expect_eq(hi, limb_t{0}, "mul_wide one*max hi");

    bigint::limb::mul_wide(
            std::numeric_limits<limb_t>::max(),
            std::numeric_limits<limb_t>::max(), lo, hi);
    expect_eq(lo, limb_t{1}, "mul_wide max*max lo");
    expect_eq(hi, std::numeric_limits<limb_t>::max() - 1,
            "mul_wide max*max hi");

    const limb_t a = 0x12345678U;
    const limb_t b = 0x9abcdef0U;
    bigint::limb::mul_wide(a, b, lo, hi);
    dlimb_t product = static_cast<dlimb_t>(a) * static_cast<dlimb_t>(b);
    expect_eq(lo, static_cast<limb_t>(product), "mul_wide constant lo");
    expect_eq(hi, static_cast<limb_t>(product >> 32),
            "mul_wide constant hi");
}

void test_construction() {
    test_section("construction / representation");

    expect(BigUint().limbs().empty(), "default zero has empty limbs");
    expect(BigUint(0).limbs().empty(), "BigUint(0) has empty limbs");
    expect_limbs(BigUint(1), {1}, "BigUint(1)");
    expect_limbs(BigUint(UINT32_MAX), {UINT32_MAX}, "BigUint(UINT32_MAX)");
    expect_limbs(BigUint(uint64_t{UINT32_MAX} + 1), {0, 1},
            "BigUint(UINT32_MAX + 1)");
    expect_limbs(BigUint(UINT64_MAX), {UINT32_MAX, UINT32_MAX},
            "BigUint(UINT64_MAX)");
    expect_limbs(BigUint::from_limbs({0}), {}, "from_limbs({0})");
    expect_limbs(BigUint::from_limbs({1, 0, 0}), {1},
            "from_limbs({1,0,0})");
    expect(BigUint::zero().is_zero(), "zero() is zero");
    expect_limbs(BigUint::one(), {1}, "one()");
}

void test_compare() {
    test_section("compare / equality");

    expect_eq(BigUint(7).compare(BigUint(7)), 0, "compare equal");
    expect_eq(BigUint(7).compare(BigUint(8)), -1, "compare shorter/smaller");
    expect_eq(BigUint(8).compare(BigUint(7)), 1, "compare greater");
    expect_eq(BigUint(0).compare(BigUint(0)), 0, "compare zero");
    expect_eq(BigUint(0).compare(BigUint(1)), -1, "zero < one");
    expect_eq(BigUint(1).compare(BigUint(0)), 1, "one > zero");
    expect(BigUint::from_limbs({1, 0, 0}) == BigUint(1),
            "normalized leading-zero equality");
    expect(BigUint(1) != BigUint(2), "operator!=");
}

void test_bit_length() {
    test_section("bit_length");

    expect_eq(BigUint(0).bit_length(), size_t{0}, "bit_length 0");
    expect_eq(BigUint(1).bit_length(), size_t{1}, "bit_length 1");
    expect_eq(BigUint(2).bit_length(), size_t{2}, "bit_length 2");
    expect_eq(BigUint(uint64_t{1} << 31).bit_length(), size_t{32},
            "bit_length 2^31");
    expect_eq(BigUint(uint64_t{1} << 32).bit_length(), size_t{33},
            "bit_length 2^32");
    expect_eq(BigUint(uint64_t{1} << 63).bit_length(), size_t{64},
            "bit_length 2^63");
    expect_eq(BigUint(UINT64_MAX).bit_length(), size_t{64},
            "bit_length UINT64_MAX");
}

void test_bits() {
    test_section("test_bit / set_bit");

    BigUint value;
    value.set_bit(0);
    expect_limbs(value, {1}, "set bit 0");
    value.set_bit(31);
    expect(value.test_bit(31), "test bit 31");
    expect_limbs(value, {0x80000001U}, "set bit 31 layout");
    value.set_bit(32);
    expect_limbs(value, {0x80000001U, 1}, "set bit 32 layout");
    value.set_bit(63);
    expect(value.test_bit(63), "test bit 63");
    value.set_bit(100);
    expect(value.test_bit(100), "test bit 100");
    expect(!value.test_bit(99), "test unset bit 99");

    BigUint before = value;
    value.set_bit(100);
    expect(value == before, "setting already-set bit is stable");
}

void test_add() {
    test_section("add");

    for (uint64_t a = 0; a <= 1000; ++a) {
        for (uint64_t b = 0; b <= 1000; ++b) {
            BigUint sum = BigUint(a).add(BigUint(b));
            expect_eq(sum.to_uint64_for_testing(), a + b,
                    "exhaustive small add");
        }
    }

    expect_limbs(BigUint(UINT32_MAX).add(BigUint(1)), {0, 1},
            "UINT32_MAX + 1");
    expect_limbs(BigUint::from_limbs({UINT32_MAX, UINT32_MAX}).add(
                         BigUint(1)),
            {0, 0, 1}, "carry across multiple limbs");
    expect_limbs(BigUint::from_limbs({0, UINT32_MAX}).add(
                         BigUint::from_limbs({0, 1})),
            {0, 0, 1}, "carry in high limb");
}

void test_sub_abs() {
    test_section("sub_abs");

    expect_limbs(BigUint(7).sub_abs(BigUint(7)), {}, "equal subtraction");
    expect_limbs(BigUint(9).sub_abs(BigUint(3)), {6}, "no borrow");
    expect_limbs(BigUint::from_limbs({0, 1}).sub_abs(BigUint(1)),
            {UINT32_MAX}, "borrow within one high limb");
    expect_limbs(BigUint::from_limbs({0, 0, 1}).sub_abs(BigUint(1)),
            {UINT32_MAX, UINT32_MAX}, "borrow across limbs");

    for (uint64_t a = 0; a <= 1000; ++a) {
        for (uint64_t b = 0; b <= a; ++b) {
            BigUint diff = BigUint(a).sub_abs(BigUint(b));
            expect_eq(diff.to_uint64_for_testing(), a - b,
                    "exhaustive small sub_abs");
        }
    }

    bool threw = false;
    try {
        (void)BigUint(1).sub_abs(BigUint(2));
    } catch (const std::invalid_argument&) {
        threw = true;
    }
    expect(threw, "sub_abs lhs < rhs throws invalid_argument");
}

void test_shifts() {
    test_section("shifts");

    const std::vector<size_t> shifts = {0, 1, 31, 32, 33, 63, 64};
    for (size_t shift : shifts) {
        uint64_t base = 0x12345ULL;
        BigUint value(base);
        BigUint left = value.shift_left_bits(shift);
        BigUint right = left.shift_right_bits(shift);
        expect(right == value, "shift left/right roundtrip");
    }

    expect_limbs(BigUint(1).shift_left_bits(0), {1}, "left shift 0");
    expect_limbs(BigUint(1).shift_left_bits(1), {2}, "left shift 1");
    expect_limbs(BigUint(1).shift_left_bits(31), {0x80000000U},
            "left shift 31");
    expect_limbs(BigUint(1).shift_left_bits(32), {0, 1}, "left shift 32");
    expect_limbs(BigUint(1).shift_left_bits(33), {0, 2}, "left shift 33");
    expect_limbs(BigUint(1).shift_left_bits(63), {0, 0x80000000U},
            "left shift 63");
    expect_limbs(BigUint(1).shift_left_bits(64), {0, 0, 1},
            "left shift 64");

    BigUint high = BigUint(1).shift_left_bits(100);
    expect(high.shift_right_bits(101).is_zero(),
            "right shift more than bit length");
    expect(BigUint(0).shift_left_bits(64).is_zero(), "shift left zero");
    expect(BigUint(0).shift_right_bits(64).is_zero(), "shift right zero");
}

void test_large_representation() {
    test_section("deterministic large representation");

    BigUint value;
    const std::vector<size_t> bits = {0, 31, 32, 63, 64, 100, 127, 128};
    for (size_t bit : bits)
        value.set_bit(bit);

    expect_eq(value.bit_length(), size_t{129}, "large bit_length");
    expect_eq(value.limb_count(), size_t{5}, "large limb_count");
    expect_limbs(value, {
            0x80000001U,
            0x80000001U,
            0x00000001U,
            0x80000010U,
            0x00000001U,
    }, "large limb layout");

    for (size_t bit : bits)
        expect(value.test_bit(bit), "large test set bit");
    expect(!value.test_bit(126), "large test unset bit");

    bool threw = false;
    try {
        (void)value.to_uint64_for_testing();
    } catch (const std::overflow_error&) {
        threw = true;
    }
    expect(threw, "to_uint64_for_testing overflow");
}

void test_multiplication_basic() {
    test_section("mul_schoolbook basic");

    expect_limbs(BigUint(0).mul_schoolbook(BigUint(0)), {}, "0 * 0");
    expect_limbs(BigUint(0).mul_schoolbook(BigUint(12345)), {}, "0 * x");
    expect_limbs(BigUint(12345).mul_schoolbook(BigUint(0)), {}, "x * 0");
    expect_limbs(BigUint(1).mul_schoolbook(BigUint(UINT64_MAX)),
            {UINT32_MAX, UINT32_MAX}, "1 * UINT64_MAX");
    expect_eq(BigUint(2).mul_schoolbook(BigUint(3)).to_uint64_for_testing(),
            uint64_t{6}, "2 * 3");
    expect_limbs(BigUint(UINT32_MAX).mul_schoolbook(BigUint(2)),
            {UINT32_MAX - 1, 1}, "UINT32_MAX * 2");
    expect_limbs(BigUint(UINT32_MAX).mul_schoolbook(BigUint(UINT32_MAX)),
            {1, UINT32_MAX - 1}, "UINT32_MAX * UINT32_MAX");
    expect_limbs(BigUint(UINT64_MAX).mul_schoolbook(BigUint(1)),
            {UINT32_MAX, UINT32_MAX}, "UINT64_MAX * 1");
    expect_limbs(BigUint(UINT64_MAX).mul_schoolbook(BigUint(UINT64_MAX)),
            {1, 0, UINT32_MAX - 1, UINT32_MAX},
            "UINT64_MAX * UINT64_MAX");
}

void test_multiplication_exhaustive_small() {
    test_section("mul_schoolbook exhaustive uint64_t-safe");

    for (uint64_t a = 0; a <= 1000; a += 7) {
        for (uint64_t b = 0; b <= 1000; b += 11) {
            BigUint product = BigUint(a).mul_schoolbook(BigUint(b));
            expect_eq(product.to_uint64_for_testing(), a * b,
                    "deterministic small multiplication");
        }
    }
}

void test_multiplication_boundaries() {
    test_section("mul_schoolbook limb boundaries");

    BigUint two32 = BigUint(1).shift_left_bits(32);
    expect_limbs(two32.mul_schoolbook(two32), {0, 0, 1},
            "2^32 * 2^32");

    expect_limbs(BigUint(UINT32_MAX).square(), {1, UINT32_MAX - 1},
            "(2^32 - 1)^2");
    expect_limbs(BigUint(UINT64_MAX).square(),
            {1, 0, UINT32_MAX - 1, UINT32_MAX},
            "(2^64 - 1)^2");

    const std::vector<size_t> powers = {
        0, 1, 31, 32, 33, 63, 64, 65, 100, 127, 128
    };
    for (size_t lhs_bit : powers) {
        for (size_t rhs_bit : powers) {
            BigUint lhs = BigUint(1).shift_left_bits(lhs_bit);
            BigUint rhs = BigUint(1).shift_left_bits(rhs_bit);
            BigUint product = lhs.mul_schoolbook(rhs);
            expect(product.test_bit(lhs_bit + rhs_bit),
                    "power-of-two product has expected bit");
            expect_eq(product.bit_length(), lhs_bit + rhs_bit + 1,
                    "power-of-two product bit_length");
        }
    }
}

void test_multiplication_dense_patterns() {
    test_section("mul_schoolbook dense limb patterns");

    BigUint dense1 = BigUint::from_limbs({UINT32_MAX});
    BigUint dense2 = BigUint::from_limbs({UINT32_MAX, UINT32_MAX});
    BigUint dense3 = BigUint::from_limbs(
            {UINT32_MAX, UINT32_MAX, UINT32_MAX});

    expect_limbs(dense1.mul_schoolbook(BigUint(2)), {UINT32_MAX - 1, 1},
            "dense1 * 2");
    expect_limbs(dense2.mul_schoolbook(BigUint(2)),
            {UINT32_MAX - 1, UINT32_MAX, 1}, "dense2 * 2");
    expect_limbs(dense3.mul_schoolbook(BigUint(2)),
            {UINT32_MAX - 1, UINT32_MAX, UINT32_MAX, 1}, "dense3 * 2");

    expect_limbs(dense1.square(), {1, UINT32_MAX - 1}, "dense1 square");
    expect_limbs(dense2.square(), {1, 0, UINT32_MAX - 1, UINT32_MAX},
            "dense2 square");
    expect_limbs(dense3.square(),
            {1, 0, 0, UINT32_MAX - 1, UINT32_MAX, UINT32_MAX},
            "dense3 square");

    expect_limbs(dense3.mul_schoolbook(dense1),
            {1, UINT32_MAX, UINT32_MAX, UINT32_MAX - 1},
            "dense3 * dense1");
}

void test_multiplication_identities() {
    test_section("mul_schoolbook commutativity and distributivity");

    const std::vector<BigUint> values = {
        BigUint(0),
        BigUint(1),
        BigUint(2),
        BigUint(17),
        BigUint(UINT32_MAX),
        BigUint(uint64_t{UINT32_MAX} + 1),
        BigUint(UINT64_MAX),
        BigUint::from_limbs({3, 0, 7}),
    };

    for (const BigUint& a : values) {
        for (const BigUint& b : values) {
            expect(a.mul_schoolbook(b) == b.mul_schoolbook(a),
                    "multiplication commutativity");
        }
    }

    const std::vector<BigUint> dist_values = {
        BigUint(0),
        BigUint(1),
        BigUint(3),
        BigUint(17),
        BigUint(UINT32_MAX),
        BigUint::from_limbs({5, 1}),
    };
    for (const BigUint& a : dist_values) {
        for (const BigUint& b : dist_values) {
            for (const BigUint& c : dist_values) {
                BigUint lhs = a.mul_schoolbook(b.add(c));
                BigUint rhs = a.mul_schoolbook(b).add(a.mul_schoolbook(c));
                expect(lhs == rhs, "a*(b+c) == a*b + a*c");
            }
        }
    }
}

void test_square() {
    test_section("square");

    const std::vector<BigUint> values = {
        BigUint(0),
        BigUint(1),
        BigUint(2),
        BigUint(UINT32_MAX),
        BigUint(UINT64_MAX),
        BigUint::from_limbs({UINT32_MAX, UINT32_MAX, UINT32_MAX}),
        BigUint(1).shift_left_bits(31),
        BigUint(1).shift_left_bits(32),
        BigUint(1).shift_left_bits(64),
        BigUint(1).shift_left_bits(128),
    };

    for (const BigUint& value : values)
        expect(value.square() == value.mul_schoolbook(value),
                "square equals mul_schoolbook self-product");

    expect_limbs(BigUint(0).square(), {}, "zero square");
    expect_limbs(BigUint(1).square(), {1}, "one square");
    expect_limbs(BigUint(1).shift_left_bits(64).square(), {0, 0, 0, 0, 1},
            "power of two square");
}

void test_divmod_basic() {
    test_section("divmod basic");

    auto zero_by_one = BigUint(0).divmod(BigUint(1));
    expect_limbs(zero_by_one.first, {}, "0 / 1 quotient");
    expect_limbs(zero_by_one.second, {}, "0 / 1 remainder");

    auto one_by_one = BigUint(1).divmod(BigUint(1));
    expect_limbs(one_by_one.first, {1}, "1 / 1 quotient");
    expect_limbs(one_by_one.second, {}, "1 / 1 remainder");

    auto five_by_two = BigUint(5).divmod(BigUint(2));
    expect_limbs(five_by_two.first, {2}, "5 / 2 quotient");
    expect_limbs(five_by_two.second, {1}, "5 / 2 remainder");

    BigUint x = BigUint::from_limbs({0x12345678U, 0x9abcdef0U});
    auto x_by_x = x.divmod(x);
    expect_limbs(x_by_x.first, {1}, "x / x quotient");
    expect_limbs(x_by_x.second, {}, "x / x remainder");

    BigUint larger = x.shift_left_bits(1);
    auto x_by_larger = x.divmod(larger);
    expect_limbs(x_by_larger.first, {}, "x / larger quotient");
    expect(x_by_larger.second == x, "x / larger remainder");

    auto x_by_one = x.divmod(BigUint(1));
    expect(x_by_one.first == x, "x / 1 quotient");
    expect_limbs(x_by_one.second, {}, "x / 1 remainder");

    bool threw = false;
    try {
        (void)BigUint(1).divmod(BigUint::zero());
    } catch (const std::invalid_argument&) {
        threw = true;
    }
    expect(threw, "division by zero throws invalid_argument");
}

void test_divmod_uint64_oracle() {
    test_section("divmod uint64_t oracle");

    for (uint64_t a = 0; a <= 1000; ++a) {
        for (uint64_t b = 1; b <= 1000; ++b) {
            auto dm = BigUint(a).divmod(BigUint(b));
            expect_eq(dm.first.to_uint64_for_testing(), a / b,
                    "exhaustive small div quotient");
            expect_eq(dm.second.to_uint64_for_testing(), a % b,
                    "exhaustive small div remainder");
        }
    }

    const std::vector<std::pair<uint64_t, uint64_t>> cases = {
        {UINT32_MAX, 2},
        {uint64_t{1} << 32, 2},
        {uint64_t{1} << 32, UINT32_MAX},
        {UINT64_MAX, UINT32_MAX},
        {UINT64_MAX, uint64_t{1} << 32},
    };

    for (const auto& test : cases) {
        auto dm = BigUint(test.first).divmod(BigUint(test.second));
        expect_eq(dm.first.to_uint64_for_testing(), test.first / test.second,
                "uint64 boundary div quotient");
        expect_eq(dm.second.to_uint64_for_testing(), test.first % test.second,
                "uint64 boundary div remainder");
        expect_divmod_invariant(BigUint(test.first), BigUint(test.second),
                "uint64 boundary invariant");
    }
}

void test_divmod_limb_cases() {
    test_section("divmod limb and power cases");

    BigUint two32 = BigUint(1).shift_left_bits(32);
    BigUint two64 = BigUint(1).shift_left_bits(64);
    auto two64_by_two32 = two64.divmod(two32);
    expect(two64_by_two32.first == two32, "2^64 / 2^32 quotient");
    expect_limbs(two64_by_two32.second, {}, "2^64 / 2^32 remainder");

    BigUint two100 = BigUint(1).shift_left_bits(100);
    auto two100_by_two32 = two100.divmod(two32);
    expect(two100_by_two32.first == BigUint(1).shift_left_bits(68),
            "2^100 / 2^32 quotient");
    expect_limbs(two100_by_two32.second, {}, "2^100 / 2^32 remainder");

    BigUint two128_minus_one = BigUint(1).shift_left_bits(128).sub_abs(
            BigUint(1));
    BigUint two64_minus_one = BigUint(1).shift_left_bits(64).sub_abs(
            BigUint(1));
    auto mersenne_dm = two128_minus_one.divmod(two64_minus_one);
    expect(mersenne_dm.first == BigUint(1).shift_left_bits(64).add(
                   BigUint(1)),
            "(2^128-1)/(2^64-1) quotient");
    expect_limbs(mersenne_dm.second, {}, "(2^128-1)/(2^64-1) remainder");

    const std::vector<std::pair<BigUint, BigUint>> cases = {
        {BigUint::from_limbs({UINT32_MAX}), BigUint(3)},
        {BigUint::from_limbs({UINT32_MAX, UINT32_MAX}), BigUint(17)},
        {BigUint::from_limbs({UINT32_MAX, UINT32_MAX, UINT32_MAX}),
                BigUint::from_limbs({0x12345678U, 1})},
        {BigUint::from_limbs({0xaaaaaaaaU, 0x55555555U, 0xffffffffU}),
                BigUint::from_limbs({0xffffU, 0x10U})},
        {BigUint::from_limbs(
                 {0x12345678U, 0x9abcdef0U, 0x0badc0deU, 0xffffffffU}),
                BigUint::from_limbs({0x11111111U, 0x22222222U})},
    };

    for (const auto& test : cases)
        expect_divmod_invariant(test.first, test.second,
                "dense/uneven divmod invariant");
}

void test_divmod_bitbiginttc_oracle() {
    test_section("divmod BitBigIntTC positive oracle");

    const std::vector<BigUint> dividends = {
        BigUint(0),
        BigUint(1),
        BigUint(5),
        BigUint(UINT32_MAX),
        BigUint(uint64_t{1} << 32),
        BigUint(UINT64_MAX),
        BigUint(1).shift_left_bits(96).add(BigUint(12345)),
        BigUint::from_limbs({0xaaaaaaaaU, 0x55555555U, 0xffffffffU}),
    };
    const std::vector<BigUint> divisors = {
        BigUint(1),
        BigUint(2),
        BigUint(3),
        BigUint(UINT32_MAX),
        BigUint(uint64_t{1} << 32),
        BigUint::from_limbs({0x12345678U, 1}),
    };

    for (const BigUint& dividend : dividends) {
        for (const BigUint& divisor : divisors) {
            auto dm = dividend.divmod(divisor);
            auto tc_dm = tc_from_biguint(dividend).divmod(
                    tc_from_biguint(divisor));
            expect_same_as_tc(dm.first, tc_dm.q,
                    "divmod quotient vs BitBigIntTC");
            expect_same_as_tc(dm.second, tc_dm.r,
                    "divmod remainder vs BitBigIntTC");
            expect_divmod_invariant(dividend, divisor,
                    "BitBigIntTC oracle divmod invariant");
        }
    }
}

void test_div_mod_wrappers() {
    test_section("div/mod wrappers");

    const std::vector<std::pair<BigUint, BigUint>> cases = {
        {BigUint(0), BigUint(1)},
        {BigUint(5), BigUint(2)},
        {BigUint(12345), BigUint(17)},
        {BigUint(UINT64_MAX), BigUint(UINT32_MAX)},
        {BigUint::from_limbs({0x12345678U, 0x9abcdef0U}),
                BigUint::from_limbs({0x1111U, 1})},
    };

    for (const auto& test : cases) {
        auto dm = test.first.divmod(test.second);
        expect(test.first.div(test.second) == dm.first,
                "div wrapper matches divmod quotient");
        expect(test.first.mod(test.second) == dm.second,
                "mod wrapper matches divmod remainder");
    }

    expect_limbs(BigUint(0).mod(BigUint(1)), {}, "0 mod 1");
    expect_limbs(BigUint(5).div(BigUint(2)), {2}, "5 div 2");
    expect_limbs(BigUint(5).mod(BigUint(2)), {1}, "5 mod 2");

    BigUint x = BigUint::from_limbs({0xaaaaaaaaU, 0x55555555U});
    expect_limbs(x.mod(x), {}, "x mod x");
    BigUint larger = x.shift_left_bits(1);
    expect(x.mod(larger) == x, "x mod larger");

    bool div_threw = false;
    try {
        (void)BigUint(1).div(BigUint::zero());
    } catch (const std::invalid_argument&) {
        div_threw = true;
    }
    expect(div_threw, "div by zero throws");

    bool mod_threw = false;
    try {
        (void)BigUint(1).mod(BigUint::zero());
    } catch (const std::invalid_argument&) {
        mod_threw = true;
    }
    expect(mod_threw, "mod by zero throws");
}

void test_parity() {
    test_section("is_even / is_odd");

    expect(BigUint(0).is_even(), "0 is even");
    expect(!BigUint(0).is_odd(), "0 is not odd");
    expect(BigUint(1).is_odd(), "1 is odd");
    expect(!BigUint(1).is_even(), "1 is not even");
    expect(BigUint(2).is_even(), "2 is even");
    expect(BigUint(UINT32_MAX).is_odd(), "UINT32_MAX is odd");
    expect(BigUint(uint64_t{1} << 32).is_even(), "2^32 is even");

    BigUint value;
    value.set_bit(0);
    expect(value.is_odd(), "set bit 0 makes odd");
    value = BigUint();
    value.set_bit(31);
    expect(value.is_even(), "set bit 31 is even");
    value.set_bit(0);
    expect(value.is_odd(), "set bit 31 plus bit 0 is odd");
    value = BigUint();
    value.set_bit(32);
    expect(value.is_even(), "set bit 32 is even");
    value.set_bit(100);
    expect(value.is_even(), "set bit 32 and 100 is even");
}

void test_gcd_uint64_oracle() {
    test_section("gcd uint64_t oracle");

    expect_limbs(BigUint::gcd(BigUint(0), BigUint(0)), {}, "gcd(0,0)");
    expect_limbs(BigUint::gcd(BigUint(42), BigUint(0)), {42}, "gcd(a,0)");
    expect_limbs(BigUint::gcd(BigUint(0), BigUint(42)), {42}, "gcd(0,b)");
    expect_limbs(BigUint::gcd(BigUint(1), BigUint(UINT64_MAX)), {1},
            "gcd(1,x)");
    expect(BigUint::gcd(BigUint(UINT64_MAX), BigUint(UINT64_MAX))
                    == BigUint(UINT64_MAX),
            "gcd(x,x)");
    expect_limbs(BigUint::gcd(BigUint(12), BigUint(18)), {6},
            "gcd(12,18)");
    expect_limbs(BigUint::gcd(BigUint(17), BigUint(31)), {1},
            "gcd(17,31)");
    expect_limbs(BigUint::gcd(BigUint(65537), BigUint(3120)), {1},
            "gcd(65537,3120)");

    for (uint64_t a = 0; a <= 300; ++a) {
        for (uint64_t b = 0; b <= 300; ++b) {
            BigUint g = BigUint::gcd(BigUint(a), BigUint(b));
            expect_eq(g.to_uint64_for_testing(), std::gcd(a, b),
                    "exhaustive small gcd");
        }
    }

    for (size_t i = 0; i <= 16; ++i) {
        for (size_t j = 0; j <= 16; ++j) {
            BigUint lhs = BigUint(1).shift_left_bits(i);
            BigUint rhs = BigUint(1).shift_left_bits(j);
            BigUint expected = BigUint(1).shift_left_bits(i < j ? i : j);
            expect(BigUint::gcd(lhs, rhs) == expected,
                    "gcd powers of two");
        }
    }
}

void test_gcd_large_cases() {
    test_section("gcd large cases");

    BigUint two128 = BigUint(1).shift_left_bits(128);
    BigUint two64 = BigUint(1).shift_left_bits(64);
    expect(BigUint::gcd(two128, two64) == two64,
            "gcd(2^128,2^64)");

    BigUint two128_minus_one = two128.sub_abs(BigUint(1));
    BigUint two64_minus_one = two64.sub_abs(BigUint(1));
    expect(BigUint::gcd(two128_minus_one, two64_minus_one)
                    == two64_minus_one,
            "gcd(2^128-1,2^64-1)");

    BigUint dense = BigUint::from_limbs(
            {UINT32_MAX, UINT32_MAX, UINT32_MAX});
    expect(BigUint::gcd(dense, dense) == dense,
            "gcd(dense,dense)");

    BigUint large = BigUint::from_limbs(
            {0x12345678U, 0x9abcdef0U, 0x0badc0deU, 0xffffffffU});
    expect_limbs(BigUint::gcd(large, BigUint(1)), {1},
            "gcd(large,1)");
}

void test_gcd_bitbiginttc_oracle() {
    test_section("gcd BitBigIntTC positive oracle");

    const std::vector<std::pair<BigUint, BigUint>> cases = {
        {BigUint(12), BigUint(18)},
        {BigUint(17), BigUint(31)},
        {BigUint(65537), BigUint(3120)},
        {BigUint(1).shift_left_bits(128), BigUint(1).shift_left_bits(64)},
        {BigUint(1).shift_left_bits(128).sub_abs(BigUint(1)),
                BigUint(1).shift_left_bits(64).sub_abs(BigUint(1))},
        {BigUint::from_limbs({UINT32_MAX, UINT32_MAX, UINT32_MAX}),
                BigUint::from_limbs({UINT32_MAX, UINT32_MAX})},
    };

    for (const auto& test : cases) {
        BigUint actual = BigUint::gcd(test.first, test.second);
        BitBigIntTC expected = tc_from_biguint(test.first)
                .euclide_algorithm_compat_for_testing(
                        tc_from_biguint(test.second));
        expect_same_as_tc(actual, expected, "gcd vs BitBigIntTC Euclid");
    }
}

void test_mod_helpers_zero_and_one() {
    test_section("mod_add/mod_sub/mod_mul zero and modulus-one cases");

    bool add_threw = false;
    try {
        (void)BigUint::mod_add(BigUint(1), BigUint(2), BigUint::zero());
    } catch (const std::invalid_argument&) {
        add_threw = true;
    }
    expect(add_threw, "mod_add zero modulus throws");

    bool sub_threw = false;
    try {
        (void)BigUint::mod_sub(BigUint(1), BigUint(2), BigUint::zero());
    } catch (const std::invalid_argument&) {
        sub_threw = true;
    }
    expect(sub_threw, "mod_sub zero modulus throws");

    bool mul_threw = false;
    try {
        (void)BigUint::mod_mul(BigUint(1), BigUint(2), BigUint::zero());
    } catch (const std::invalid_argument&) {
        mul_threw = true;
    }
    expect(mul_threw, "mod_mul zero modulus throws");

    BigUint a = BigUint::from_limbs({0x12345678U, 0x9abcdef0U});
    BigUint b = BigUint::from_limbs({0xffffffffU, 0x0badc0deU});
    expect_limbs(BigUint::mod_add(a, b, BigUint(1)), {}, "mod_add mod 1");
    expect_limbs(BigUint::mod_sub(a, b, BigUint(1)), {}, "mod_sub mod 1");
    expect_limbs(BigUint::mod_mul(a, b, BigUint(1)), {}, "mod_mul mod 1");
}

void test_mod_helpers_uint64_oracle() {
    test_section("mod_add/mod_sub/mod_mul uint64_t oracle");

    const std::vector<std::tuple<uint64_t, uint64_t, uint64_t>> cases = {
        {0, 0, 1},
        {1, 2, 3},
        {5, 9, 7},
        {UINT32_MAX, 2, UINT32_MAX - 1ULL},
        {uint64_t{1} << 32, uint64_t{1} << 32, UINT32_MAX},
        {(uint64_t{1} << 32) + 1, UINT32_MAX, uint64_t{1} << 32},
        {UINT64_MAX, 1, UINT32_MAX},
        {UINT64_MAX, UINT32_MAX, UINT64_MAX - 58},
        {UINT64_MAX, UINT64_MAX - 1, UINT64_MAX},
    };

    auto check_case = [](uint64_t a, uint64_t b, uint64_t m) {
        unsigned __int128 aw = a;
        unsigned __int128 bw = b;
        unsigned __int128 mw = m;

        uint64_t expected_add = static_cast<uint64_t>((aw + bw) % mw);
        uint64_t a_mod = a % m;
        uint64_t b_mod = b % m;
        uint64_t expected_sub = static_cast<uint64_t>(
                (static_cast<unsigned __int128>(a_mod) + mw - b_mod) % mw);
        uint64_t expected_mul = static_cast<uint64_t>((aw * bw) % mw);

        BigUint modulus(m);
        BigUint add = BigUint::mod_add(BigUint(a), BigUint(b), modulus);
        BigUint sub = BigUint::mod_sub(BigUint(a), BigUint(b), modulus);
        BigUint mul = BigUint::mod_mul(BigUint(a), BigUint(b), modulus);

        expect_eq(add.to_uint64_for_testing(), expected_add,
                "uint64 mod_add oracle");
        expect_eq(sub.to_uint64_for_testing(), expected_sub,
                "uint64 mod_sub oracle");
        expect_eq(mul.to_uint64_for_testing(), expected_mul,
                "uint64 mod_mul oracle");
        expect_mod_result(add, modulus, "uint64 mod_add result range");
        expect_mod_result(sub, modulus, "uint64 mod_sub result range");
        expect_mod_result(mul, modulus, "uint64 mod_mul result range");
    };

    for (const auto& test : cases)
        check_case(std::get<0>(test), std::get<1>(test), std::get<2>(test));

    for (uint64_t a = 0; a <= 200; a += 10) {
        for (uint64_t b = 0; b <= 200; b += 11) {
            for (uint64_t m = 1; m <= 200; ++m)
                check_case(a, b, m);
        }
    }
}

void test_mod_helpers_large_cases() {
    test_section("mod_add/mod_sub/mod_mul large cases and invariants");

    const std::vector<std::tuple<BigUint, BigUint, BigUint>> cases = {
        {BigUint(0), BigUint(1), BigUint(3)},
        {BigUint(UINT32_MAX), BigUint((uint64_t{1} << 32) + 1),
                BigUint(UINT32_MAX)},
        {BigUint(UINT64_MAX), BigUint(1).shift_left_bits(64),
                BigUint(uint64_t{1} << 32)},
        {BigUint(1).shift_left_bits(100), BigUint(UINT32_MAX),
                BigUint(1).shift_left_bits(96).add(BigUint(17))},
        {BigUint(1).shift_left_bits(255),
                BigUint::from_limbs({UINT32_MAX, UINT32_MAX, UINT32_MAX}),
                BigUint(1).shift_left_bits(128).sub_abs(BigUint(1))},
        {BigUint::from_limbs(
                 {0x12345678U, 0x9abcdef0U, 0x0badc0deU, 0xffffffffU}),
                BigUint::from_limbs(
                        {0xaaaaaaaaU, 0x55555555U, 0xffffffffU}),
                BigUint::from_limbs({0x11111111U, 0x22222222U})},
    };

    for (const auto& test : cases) {
        const BigUint& a = std::get<0>(test);
        const BigUint& b = std::get<1>(test);
        const BigUint& modulus = std::get<2>(test);
        BigUint add = BigUint::mod_add(a, b, modulus);
        BigUint sub = BigUint::mod_sub(a, b, modulus);
        BigUint mul = BigUint::mod_mul(a, b, modulus);

        expect_mod_result(add, modulus, "large mod_add result range");
        expect_mod_result(sub, modulus, "large mod_sub result range");
        expect_mod_result(mul, modulus, "large mod_mul result range");
        expect(add == BigUint::mod_add(b, a, modulus),
                "mod_add commutativity");
        expect(mul == BigUint::mod_mul(b, a, modulus),
                "mod_mul commutativity");
    }

    BigUint large = BigUint(1).shift_left_bits(255).add(
            BigUint::from_limbs({0x12345678U, 0x9abcdef0U}));
    BigUint small_modulus = BigUint(UINT32_MAX);
    expect_mod_result(BigUint::mod_add(large, large, small_modulus),
            small_modulus, "huge operands small modulus add");
    expect_mod_result(BigUint::mod_sub(large, BigUint(12345), small_modulus),
            small_modulus, "huge operands small modulus sub");
    expect_mod_result(BigUint::mod_mul(large, large, small_modulus),
            small_modulus, "huge operands small modulus mul");

    BigUint huge_modulus = BigUint(1).shift_left_bits(300).add(BigUint(19));
    expect(BigUint::mod_add(BigUint(5), BigUint(7), huge_modulus)
                    == BigUint(12),
            "mod_add with larger modulus keeps direct sum");
    expect(BigUint::mod_sub(BigUint(5), BigUint(7), huge_modulus)
                    == huge_modulus.sub_abs(BigUint(2)),
            "mod_sub wraps under larger modulus");
    expect(BigUint::mod_mul(BigUint(5), BigUint(7), huge_modulus)
                    == BigUint(35),
            "mod_mul with larger modulus keeps direct product");
}

BitBigIntTC tc_mod_sub(
        const BigUint& a, const BigUint& b, const BigUint& modulus) {
    BitBigIntTC tc_modulus = tc_from_biguint(modulus);
    BitBigIntTC a_mod = tc_from_biguint(a).divmod(tc_modulus).r;
    BitBigIntTC b_mod = tc_from_biguint(b).divmod(tc_modulus).r;
    if (a_mod.compare(b_mod) >= 0)
        return a_mod.sub(b_mod).divmod(tc_modulus).r;

    BitBigIntTC diff = b_mod.sub(a_mod);
    return tc_modulus.sub(diff).divmod(tc_modulus).r;
}

void test_mod_helpers_bitbiginttc_oracle() {
    test_section("mod_add/mod_sub/mod_mul BitBigIntTC positive oracle");

    const std::vector<std::tuple<BigUint, BigUint, BigUint>> cases = {
        {BigUint(0), BigUint(1), BigUint(1)},
        {BigUint(5), BigUint(9), BigUint(7)},
        {BigUint(UINT32_MAX), BigUint(uint64_t{1} << 32), BigUint(3)},
        {BigUint(uint64_t{1} << 32), BigUint(UINT32_MAX),
                BigUint(UINT32_MAX)},
        {BigUint(1).shift_left_bits(96).add(BigUint(12345)), BigUint(17),
                BigUint::from_limbs({0x12345678U, 1})},
        {BigUint::from_limbs({0xaaaaaaaaU, 0x55555555U, 0xffffffffU}),
                BigUint(UINT32_MAX), BigUint::from_limbs({0x12345678U, 1})},
    };

    for (const auto& test : cases) {
        const BigUint& a = std::get<0>(test);
        const BigUint& b = std::get<1>(test);
        const BigUint& modulus = std::get<2>(test);
        BitBigIntTC tc_modulus = tc_from_biguint(modulus);
        BitBigIntTC tc_a = tc_from_biguint(a);
        BitBigIntTC tc_b = tc_from_biguint(b);

        expect_same_as_tc(BigUint::mod_add(a, b, modulus),
                tc_a.add(tc_b).divmod(tc_modulus).r,
                "mod_add vs BitBigIntTC");
        expect_same_as_tc(BigUint::mod_sub(a, b, modulus),
                tc_mod_sub(a, b, modulus),
                "mod_sub vs BitBigIntTC");
        expect_same_as_tc(BigUint::mod_mul(a, b, modulus),
                tc_a.multiplication_compat_for_testing(tc_b)
                        .divmod(tc_modulus).r,
                "mod_mul vs BitBigIntTC");
    }
}

void test_bitbiginttc_oracle() {
    test_section("BitBigIntTC positive oracle");

    const std::vector<BigUint> values = oracle_values();
    const std::vector<size_t> shifts = {0, 1, 31, 32, 33, 63, 64, 65, 127};

    for (const BigUint& value : values) {
        BitBigIntTC tc_value = tc_from_biguint(value);
        expect_same_as_tc(value, tc_value, "BigUint -> TC -> BigUint");

        for (size_t shift : shifts) {
            BigUint shifted_left = value.shift_left_bits(shift);
            expect_same_as_tc(shifted_left, tc_shift_left(tc_value, shift),
                    "shift_left_bits vs BitBigIntTC offset_left");

            BigUint shifted_right = value.shift_right_bits(shift);
            expect_same_as_tc(shifted_right, tc_shift_right(tc_value, shift),
                    "shift_right_bits vs BitBigIntTC offset_right");
        }

        BigUint squared = value.square();
        BitBigIntTC tc_square = tc_value.multiplication_compat_for_testing(
                tc_value);
        expect_same_as_tc(squared, tc_square, "square vs BitBigIntTC");
    }

    const size_t pair_limit = 18;
    for (size_t i = 0; i < pair_limit && i < values.size(); ++i) {
        for (size_t j = 0; j < pair_limit && j < values.size(); ++j) {
            const BigUint& lhs = values[i];
            const BigUint& rhs = values[j];
            BitBigIntTC tc_lhs = tc_from_biguint(lhs);
            BitBigIntTC tc_rhs = tc_from_biguint(rhs);

            expect_same_as_tc(lhs.add(rhs), tc_lhs.add(tc_rhs),
                    "add vs BitBigIntTC");

            if (lhs.compare(rhs) >= 0) {
                expect_same_as_tc(lhs.sub_abs(rhs), tc_lhs.sub(tc_rhs),
                        "sub_abs vs BitBigIntTC");
            }

            expect_same_as_tc(lhs.mul_schoolbook(rhs),
                    tc_lhs.multiplication_compat_for_testing(tc_rhs),
                    "mul_schoolbook vs BitBigIntTC");

            if (!rhs.is_zero()) {
                auto dm = lhs.divmod(rhs);
                auto tc_dm = tc_lhs.divmod(tc_rhs);
                expect_same_as_tc(dm.first, tc_dm.q,
                        "divmod quotient vs BitBigIntTC corpus");
                expect_same_as_tc(dm.second, tc_dm.r,
                        "divmod remainder vs BitBigIntTC corpus");
            }
        }
    }
}

} // namespace

int main() {
    std::cout << "BigUint Stage 1A/1B/1C/2A/2B/2C Tests\n";
    std::cout << "======================\n" << std::endl;

    test_limb_ops();
    test_construction();
    test_compare();
    test_bit_length();
    test_bits();
    test_add();
    test_sub_abs();
    test_shifts();
    test_large_representation();
    test_multiplication_basic();
    test_multiplication_exhaustive_small();
    test_multiplication_boundaries();
    test_multiplication_dense_patterns();
    test_multiplication_identities();
    test_square();
    test_divmod_basic();
    test_divmod_uint64_oracle();
    test_divmod_limb_cases();
    test_divmod_bitbiginttc_oracle();
    test_div_mod_wrappers();
    test_parity();
    test_gcd_uint64_oracle();
    test_gcd_large_cases();
    test_gcd_bitbiginttc_oracle();
    test_mod_helpers_zero_and_one();
    test_mod_helpers_uint64_oracle();
    test_mod_helpers_large_cases();
    test_mod_helpers_bitbiginttc_oracle();
    test_bitbiginttc_oracle();

    std::cout << "\nAll BigUint tests PASSED" << std::endl;
    return 0;
}
