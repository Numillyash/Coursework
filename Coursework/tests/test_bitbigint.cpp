#include <iostream>
#include <cassert>
#include <iomanip>
#include "../lib/bigint/BitBigInt.hpp"

using namespace bigint;

// === Test helpers ===

void test_section(const char* name) {
    std::cout << "\n=== " << name << " ===" << std::endl;
}

void assert_true(bool condition, const char* message) {
    if (!condition) {
        std::cerr << "FAIL: " << message << std::endl;
        exit(1);
    }
    std::cout << "PASS: " << message << std::endl;
}

void assert_true_str(bool condition, const std::string& message) {
    assert_true(condition, message.c_str());
}

// === Tests ===

void test_default_constructor() {
    test_section("Default Constructor");

    BitBigInt zero;
    assert_true(zero.is_zero(), "BitBigInt() creates zero");
    assert_true(zero.raw_bits().size() >= 2, "Zero has minimum size");
}

void test_uint64_constructor() {
    test_section("uint64_t Constructor");

    BitBigInt one(1);
    assert_true(one.raw_bits()[0] == 1, "BitBigInt(1) has LSB = 1");
    assert_true(!one.is_zero(), "BitBigInt(1) is not zero");

    BitBigInt zero(0);
    assert_true(zero.is_zero(), "BitBigInt(0) is zero");

    // Test a small number: 5 = 101b (LSB first: 1, 0, 1, sign)
    BitBigInt five(5);
    const auto& bits = five.raw_bits();
    assert_true(bits[0] == 1, "5: LSB (bit 0) = 1");
    assert_true(bits[1] == 0, "5: bit 1 = 0");
    assert_true(bits[2] == 1, "5: bit 2 = 1");
    assert_true(bits[3] == 0, "5: sign bit = 0 (positive)");
}

void test_static_factories() {
    test_section("Static Factory Functions");

    BitBigInt z = BitBigInt::zero();
    assert_true(z.is_zero(), "BitBigInt::zero() creates zero");

    BitBigInt o = BitBigInt::one();
    assert_true(!o.is_zero(), "BitBigInt::one() is not zero");
    assert_true(o.raw_bits()[0] == 1, "BitBigInt::one() has LSB = 1");
}

void test_to_binary() {
    test_section("Binary String Representation");

    BitBigInt zero(0);
    std::string zero_str = zero.to_binary();
    std::cout << "  BitBigInt(0).to_binary() = \"" << zero_str << "\"" << std::endl;
    assert_true(zero_str.find("positive") != std::string::npos, "Zero is positive");

    BitBigInt one(1);
    std::string one_str = one.to_binary();
    std::cout << "  BitBigInt(1).to_binary() = \"" << one_str << "\"" << std::endl;
    assert_true(one_str.find("1") != std::string::npos, "One contains '1'");
    assert_true(one_str.find("positive") != std::string::npos, "One is positive");

    BitBigInt five(5);
    std::string five_str = five.to_binary();
    std::cout << "  BitBigInt(5).to_binary() = \"" << five_str << "\"" << std::endl;
    assert_true(five_str.find("101") != std::string::npos, "Five's binary contains '101'");
}

void test_normalize() {
    test_section("Normalization");

    BitBigInt num(5);
    num.normalize();
    assert_true(!num.is_zero(), "normalize() preserves non-zero");
    assert_true(num.raw_bits().size() >= 2, "normalize() maintains minimum size");

    BitBigInt zero(0);
    zero.normalize();
    assert_true(zero.is_zero(), "normalize() preserves zero");
}

void test_invariants() {
    test_section("Invariant Verification");

    // Test that various constructions maintain invariants
    for (uint64_t val : {0UL, 1UL, 2UL, 5UL, 42UL, 1000UL, 1000000UL}) {
        BitBigInt num(val);
        const auto& bits = num.raw_bits();

        // Minimum size
        std::string msg_size = "Size >= 2 for " + std::to_string(val);
        assert_true_str(bits.size() >= 2, msg_size);

        // All bits are 0 or 1
        for (size_t i = 0; i < bits.size(); ++i) {
            std::string msg_bit = "Bit " + std::to_string(i) + " is 0 or 1 for " + std::to_string(val);
            assert_true_str(bits[i] == 0 || bits[i] == 1, msg_bit);
        }

        // Sign bit (last) must be 0 for positive values
        std::string msg_sign = "Sign bit = 0 for positive value " + std::to_string(val);
        assert_true_str(bits.back() == 0, msg_sign);
    }
}

void test_copy_move() {
    test_section("Copy and Move Semantics");

    BitBigInt original(42);
    BitBigInt copy = original;

    assert_true(!copy.is_zero(), "Copy is not zero");
    assert_true(copy.raw_bits()[0] == original.raw_bits()[0],
                "Copy has same LSB as original");

    BitBigInt moved = std::move(copy);
    assert_true(!moved.is_zero(), "Moved object is not zero");
}

// === Main ===

int main() {
    std::cout << "BitBigInt Test Suite" << std::endl;
    std::cout << "====================\n" << std::endl;

    try {
        test_default_constructor();
        test_uint64_constructor();
        test_static_factories();
        test_to_binary();
        test_normalize();
        test_invariants();
        test_copy_move();

        std::cout << "\n============================================" << std::endl;
        std::cout << "All tests PASSED!" << std::endl;
        std::cout << "============================================" << std::endl;
        return 0;
    } catch (const std::exception& e) {
        std::cerr << "Exception: " << e.what() << std::endl;
        return 1;
    }
}
