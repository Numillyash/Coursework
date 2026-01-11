#include <iostream>
#include <random>
#include <cstring>
#include <cassert>
#include <iomanip>

#include "legacy_bridge.hpp"

using namespace bigint;
using namespace bigint::test;

// Random number generator
std::mt19937_64 rng(0xdeadbeef); // Fixed seed for reproducibility

// === Test helpers ===

void test_section(const char* name) {
    std::cout << "\n=== " << name << " ===" << std::endl;
}

void assert_true(bool condition, const std::string& message) {
    if (!condition) {
        std::cerr << "FAIL: " << message << std::endl;
        exit(1);
    }
    std::cout << "PASS: " << message << std::endl;
}

/// Generate random bits for a number (LSB first, sign bit separate)
std::vector<uint8_t> random_bits(size_t num_bits, std::mt19937_64& rng) {
    std::uniform_int_distribution<int> bit_dist(0, 1);
    std::vector<uint8_t> bits;
    
    for (size_t i = 0; i < num_bits; ++i) {
        bits.push_back(bit_dist(rng));
    }
    
    // Add sign bit
    std::uniform_int_distribution<int> sign_dist(0, 1);
    bits.push_back(sign_dist(rng));
    
    return bits;
}

// === Tests ===

void test_normalize_equivalence() {
    test_section("normalize() vs legacy");

    for (int seed = 0; seed < 100; ++seed) {
        // Generate random bits
        size_t num_bits = (rng() % 30) + 5; // 5-35 bits
        auto bits = random_bits(num_bits, rng);

        // Create legacy number
        number legacy_num = init();
        clear_mem(&legacy_num);
        legacy_num.mas = (uint8_t *)malloc(bits.size());
        std::copy(bits.begin(), bits.end(), legacy_num.mas);
        legacy_num.current_count = (int)bits.size();
        legacy_num.size = (int)bits.size();

        // Create TC number
        BitBigIntTC tc_num = BitBigIntTC::from_binary_bits(bits);

        // Apply legacy normalize
        normalize(&legacy_num);

        // TC already normalized in from_binary_bits, so just compare
        std::string legacy_str = legacy_to_binary(legacy_num);
        std::string tc_str = tc_to_binary(tc_num);

        if (legacy_str != tc_str) {
            std::cerr << "FAIL normalize_equivalence seed=" << seed << std::endl;
            std::cerr << "  Input bits: ";
            for (auto b : bits) std::cerr << (int)b;
            std::cerr << std::endl;
            std::cerr << "  Legacy: " << legacy_str << std::endl;
            std::cerr << "  TC:     " << tc_str << std::endl;
            clear_mem(&legacy_num);
            exit(1);
        }

        clear_mem(&legacy_num);
    }

    std::cout << "PASS: 100 random normalize() tests" << std::endl;
}

void test_reverse_equivalence() {
    test_section("reverse() vs legacy");

    for (int seed = 0; seed < 50; ++seed) {
        // Generate random bits
        size_t num_bits = (rng() % 30) + 5;
        auto bits = random_bits(num_bits, rng);

        // Create legacy number
        number legacy_num = init();
        clear_mem(&legacy_num);
        legacy_num.mas = (uint8_t *)malloc(bits.size());
        std::copy(bits.begin(), bits.end(), legacy_num.mas);
        legacy_num.current_count = (int)bits.size();
        legacy_num.size = (int)bits.size();

        // Create TC number
        BitBigIntTC tc_num = BitBigIntTC::from_binary_bits(bits);

        // Apply operations
        normalize(&legacy_num);
        // TC already normalized

        reverse(&legacy_num);
        tc_num.reverse();

        std::string legacy_str = legacy_to_binary(legacy_num);
        std::string tc_str = tc_to_binary(tc_num);

        if (legacy_str != tc_str) {
            std::cerr << "FAIL reverse_equivalence seed=" << seed << std::endl;
            std::cerr << "  Legacy: " << legacy_str << std::endl;
            std::cerr << "  TC:     " << tc_str << std::endl;
            clear_mem(&legacy_num);
            exit(1);
        }

        clear_mem(&legacy_num);
    }

    std::cout << "PASS: 50 random reverse() tests" << std::endl;
}

void test_offset_left_equivalence() {
    test_section("offset_left() vs legacy");

    for (int seed = 0; seed < 50; ++seed) {
        // Generate random bits
        size_t num_bits = (rng() % 30) + 5;
        auto bits = random_bits(num_bits, rng);

        // Create legacy number
        number legacy_num = init();
        clear_mem(&legacy_num);
        legacy_num.mas = (uint8_t *)malloc(bits.size());
        std::copy(bits.begin(), bits.end(), legacy_num.mas);
        legacy_num.current_count = (int)bits.size();
        legacy_num.size = (int)bits.size();

        // Create TC number
        BitBigIntTC tc_num = BitBigIntTC::from_binary_bits(bits);

        // Apply operations
        normalize(&legacy_num);
        // TC already normalized

        offset_left(&legacy_num);
        tc_num.offset_left();

        std::string legacy_str = legacy_to_binary(legacy_num);
        std::string tc_str = tc_to_binary(tc_num);

        if (legacy_str != tc_str) {
            std::cerr << "FAIL offset_left_equivalence seed=" << seed << std::endl;
            std::cerr << "  Legacy: " << legacy_str << std::endl;
            std::cerr << "  TC:     " << tc_str << std::endl;
            clear_mem(&legacy_num);
            exit(1);
        }

        clear_mem(&legacy_num);
    }

    std::cout << "PASS: 50 random offset_left() tests" << std::endl;
}

void test_offset_right_equivalence() {
    test_section("offset_right() vs legacy");

    for (int seed = 0; seed < 50; ++seed) {
        // Generate random bits
        size_t num_bits = (rng() % 30) + 5;
        auto bits = random_bits(num_bits, rng);

        // Create legacy number
        number legacy_num = init();
        clear_mem(&legacy_num);
        legacy_num.mas = (uint8_t *)malloc(bits.size());
        std::copy(bits.begin(), bits.end(), legacy_num.mas);
        legacy_num.current_count = (int)bits.size();
        legacy_num.size = (int)bits.size();

        // Create TC number
        BitBigIntTC tc_num = BitBigIntTC::from_binary_bits(bits);

        // Apply operations
        normalize(&legacy_num);
        // TC already normalized

        offset_right(&legacy_num);
        tc_num.offset_right();

        std::string legacy_str = legacy_to_binary(legacy_num);
        std::string tc_str = tc_to_binary(tc_num);

        if (legacy_str != tc_str) {
            std::cerr << "FAIL offset_right_equivalence seed=" << seed << std::endl;
            std::cerr << "  Legacy: " << legacy_str << std::endl;
            std::cerr << "  TC:     " << tc_str << std::endl;
            clear_mem(&legacy_num);
            exit(1);
        }

        clear_mem(&legacy_num);
    }

    std::cout << "PASS: 50 random offset_right() tests" << std::endl;
}

void test_combined_operations() {
    test_section("Combined operations (normalize → offset → reverse)");

    for (int seed = 0; seed < 30; ++seed) {
        // Generate random bits
        size_t num_bits = (rng() % 30) + 5;
        auto bits = random_bits(num_bits, rng);

        // Create legacy number
        number legacy_num = init();
        clear_mem(&legacy_num);
        legacy_num.mas = (uint8_t *)malloc(bits.size());
        std::copy(bits.begin(), bits.end(), legacy_num.mas);
        legacy_num.current_count = (int)bits.size();
        legacy_num.size = (int)bits.size();

        // Create TC number
        BitBigIntTC tc_num = BitBigIntTC::from_binary_bits(bits);

        // Apply sequence of operations
        normalize(&legacy_num);
        offset_left(&legacy_num);
        reverse(&legacy_num);

        tc_num.offset_left();
        tc_num.reverse();

        std::string legacy_str = legacy_to_binary(legacy_num);
        std::string tc_str = tc_to_binary(tc_num);

        if (legacy_str != tc_str) {
            std::cerr << "FAIL combined_operations seed=" << seed << std::endl;
            std::cerr << "  Legacy: " << legacy_str << std::endl;
            std::cerr << "  TC:     " << tc_str << std::endl;
            clear_mem(&legacy_num);
            exit(1);
        }

        clear_mem(&legacy_num);
    }

    std::cout << "PASS: 30 random combined operation tests" << std::endl;
}

void test_add_digit_equivalence() {
    test_section("add_digit() vs legacy");

    std::uniform_int_distribution<int> digit_dist(0, 1);

    for (int seed = 0; seed < 100; ++seed) {
        // Generate random bits (1..200 data bits + sign)
        size_t num_bits = (rng() % 200) + 1;
        auto bits = random_bits(num_bits, rng);

        // Create legacy number
        number legacy_num = init();
        clear_mem(&legacy_num);
        legacy_num.mas = (uint8_t *)malloc(bits.size());
        std::copy(bits.begin(), bits.end(), legacy_num.mas);
        legacy_num.current_count = (int)bits.size();
        legacy_num.size = (int)bits.size();

        // Normalize to canonical form
        normalize(&legacy_num);

        // Create TC number
        BitBigIntTC tc_num = BitBigIntTC::from_binary_bits(bits);

        // Choose random digit
        uint8_t digit = static_cast<uint8_t>(digit_dist(rng));

        // Store original state for error reporting
        std::string original_legacy_str = legacy_to_binary(legacy_num);

        // Apply add_digit to legacy
        add_digit(&legacy_num, digit);
        normalize(&legacy_num);

        // Apply add_digit to TC
        tc_num.add_digit(digit);
        tc_num.normalize();

        std::string legacy_str = legacy_to_binary(legacy_num);
        std::string tc_str = tc_to_binary(tc_num);

        if (legacy_str != tc_str) {
            std::cerr << "FAIL add_digit_equivalence seed=" << seed << std::endl;
            std::cerr << "  Original: " << original_legacy_str << std::endl;
            std::cerr << "  Digit:    " << (int)digit << std::endl;
            std::cerr << "  Legacy:   " << legacy_str << std::endl;
            std::cerr << "  TC:       " << tc_str << std::endl;
            clear_mem(&legacy_num);
            exit(1);
        }

        clear_mem(&legacy_num);
    }

    std::cout << "PASS: 100 random add_digit() tests" << std::endl;
}

// === Main ===

int main() {
    std::cout << "BitBigIntTC vs Legacy Tests" << std::endl;
    std::cout << "===========================\n" << std::endl;

    try {
        test_normalize_equivalence();
        test_reverse_equivalence();
        test_offset_left_equivalence();
        test_offset_right_equivalence();
        test_combined_operations();
        test_add_digit_equivalence();

        std::cout << "\n============================================" << std::endl;
        std::cout << "All tests PASSED!" << std::endl;
        std::cout << "============================================" << std::endl;
        return 0;
    } catch (const std::exception& e) {
        std::cerr << "Exception: " << e.what() << std::endl;
        return 1;
    }
}
