#include <cstdint>
#include <cstdlib>
#include <iostream>
#include <limits>
#include <random>
#include <stdexcept>
#include <string>
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
        }
    }
}

} // namespace

int main() {
    std::cout << "BigUint Stage 1A/1B/1C Tests\n";
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
    test_bitbiginttc_oracle();

    std::cout << "\nAll BigUint tests PASSED" << std::endl;
    return 0;
}
