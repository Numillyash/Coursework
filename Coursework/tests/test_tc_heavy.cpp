#include <array>
#include <cstdint>
#include <cstdlib>
#include <iostream>
#include <random>
#include <sstream>
#include <string>
#include <vector>

#include "legacy_bridge.hpp"

extern "C" {
number module_pow(number *a, number *t, number *b);
number euclide_algorithm_modifyed(number *value1, number *value2, number *values);
number euclide_algorithm(number *value1, number *value2);
}

using namespace bigint;
using namespace bigint::test;

namespace {

std::string current_case = "startup";

[[noreturn]] void fail(const std::string& message) {
    std::cerr << "FAIL: " << message << std::endl;
    std::cerr << "  case: " << current_case << std::endl;
    std::exit(1);
}

void set_case(const std::string& label) {
    current_case = label;
}

void expect(bool condition, const std::string& message) {
    if (!condition)
        fail(message);
}

void assert_same_binary(const std::string& label, const number& legacy,
        const BitBigIntTC& tc) {
    const std::string legacy_str = legacy_to_binary(legacy);
    const std::string tc_str = tc_to_binary(tc);
    if (legacy_str != tc_str) {
        std::cerr << "FAIL: " << label << std::endl;
        std::cerr << "  case:   " << current_case << std::endl;
        std::cerr << "  Legacy: " << legacy_str << std::endl;
        std::cerr << "  TC:     " << tc_str << std::endl;
        std::exit(1);
    }
}

std::vector<int> corpus_bit_sizes() {
    return {
        1, 2, 3, 4, 5, 7, 8, 15, 16, 31, 32,
        63, 64, 127, 128, 191, 255, 256, 257, 384, 512
    };
}

std::vector<uint64_t> corpus_seeds() {
    return {0x12345678ULL, 0xdeadbeefULL, 0xc0ffeeULL};
}

std::vector<uint8_t> positive_random_bits(
        int data_bits,
        std::mt19937_64& rng) {
    std::vector<uint8_t> bits(static_cast<size_t>(data_bits) + 1, 0);
    for (int i = 0; i < data_bits; ++i)
        bits[static_cast<size_t>(i)] = static_cast<uint8_t>(rng() & 1U);
    bits[static_cast<size_t>(data_bits) - 1] = 1;
    bits.back() = 0;
    return bits;
}

std::vector<uint8_t> positive_power_bits(int data_bits) {
    std::vector<uint8_t> bits(static_cast<size_t>(data_bits) + 1, 0);
    bits[static_cast<size_t>(data_bits) - 1] = 1;
    return bits;
}

std::vector<uint8_t> positive_all_ones_bits(int data_bits) {
    std::vector<uint8_t> bits(static_cast<size_t>(data_bits) + 1, 1);
    bits.back() = 0;
    return bits;
}

number normalized_legacy_from_positive_bits(const std::vector<uint8_t>& bits) {
    number result = legacy_from_bits(bits);
    normalize(&result);
    return result;
}

number make_legacy_value(const std::vector<uint8_t>& positive_bits,
        bool negative) {
    number positive = normalized_legacy_from_positive_bits(positive_bits);
    if (!negative)
        return positive;

    number zero = int_to_number(0);
    number result = difference(&zero, &positive);
    normalize(&result);
    clear_mem(&zero);
    clear_mem(&positive);
    return result;
}

BitBigIntTC make_tc_value(const std::vector<uint8_t>& positive_bits,
        bool negative) {
    BitBigIntTC positive = BitBigIntTC::from_binary_bits(positive_bits);
    if (!negative)
        return positive;
    BitBigIntTC result = BitBigIntTC::zero().sub(positive);
    result.normalize();
    return result;
}

number copy_number(number& value) {
    return copy(&value);
}

void assert_tc_equal(const std::string& label, const BitBigIntTC& lhs,
        const BitBigIntTC& rhs) {
    if (!lhs.is_equal(rhs)) {
        std::cerr << "FAIL: " << label << std::endl;
        std::cerr << "  case: " << current_case << std::endl;
        std::cerr << "  lhs:  " << tc_to_binary(lhs) << std::endl;
        std::cerr << "  rhs:  " << tc_to_binary(rhs) << std::endl;
        std::exit(1);
    }
}

void run_arithmetic_case(const std::string& label,
        const std::vector<uint8_t>& lhs_bits,
        bool lhs_negative,
        const std::vector<uint8_t>& rhs_bits,
        bool rhs_negative,
        bool run_divmod) {
    set_case(label + " lhs=" + bits_to_debug(lhs_bits)
            + (lhs_negative ? " neg" : " pos")
            + " rhs=" + bits_to_debug(rhs_bits)
            + (rhs_negative ? " neg" : " pos"));

    number legacy_lhs = make_legacy_value(lhs_bits, lhs_negative);
    number legacy_rhs = make_legacy_value(rhs_bits, rhs_negative);
    BitBigIntTC tc_lhs = make_tc_value(lhs_bits, lhs_negative);
    BitBigIntTC tc_rhs = make_tc_value(rhs_bits, rhs_negative);

    number lhs_for_add = copy_number(legacy_lhs);
    number rhs_for_add = copy_number(legacy_rhs);
    number legacy_sum = addition(&lhs_for_add, &rhs_for_add);
    BitBigIntTC tc_sum = tc_lhs.add(tc_rhs);
    assert_same_binary(label + " addition", legacy_sum, tc_sum);
    clear_mem(&legacy_sum);
    clear_mem(&lhs_for_add);
    clear_mem(&rhs_for_add);

    number lhs_for_diff = copy_number(legacy_lhs);
    number rhs_for_diff = copy_number(legacy_rhs);
    number legacy_diff = difference(&lhs_for_diff, &rhs_for_diff);
    normalize(&legacy_diff);
    BitBigIntTC tc_diff = tc_lhs.sub(tc_rhs);
    tc_diff.normalize();
    assert_same_binary(label + " difference", legacy_diff, tc_diff);
    clear_mem(&legacy_diff);
    clear_mem(&lhs_for_diff);
    clear_mem(&rhs_for_diff);

    number lhs_for_mul = copy_number(legacy_lhs);
    number rhs_for_mul = copy_number(legacy_rhs);
    number legacy_product = multiplication(&lhs_for_mul, &rhs_for_mul);
    BitBigIntTC tc_product =
            tc_lhs.multiplication_compat_for_testing(tc_rhs);
    assert_same_binary(label + " multiplication", legacy_product, tc_product);
    clear_mem(&legacy_product);
    clear_mem(&lhs_for_mul);
    clear_mem(&rhs_for_mul);

    if (!lhs_negative && !rhs_negative) {
        BitBigIntTC algebra_sum = tc_lhs.add(tc_rhs);
        BitBigIntTC algebra_roundtrip = algebra_sum.sub(tc_rhs);
        algebra_roundtrip.normalize();
        assert_tc_equal(label + " algebra (a+b)-b == a",
                algebra_roundtrip, tc_lhs);
    }

    if (run_divmod) {
        number lhs_for_div = copy_number(legacy_lhs);
        number rhs_for_div = copy_number(legacy_rhs);
        number legacy_r = init();
        number legacy_q = division_with_module(
                &lhs_for_div, &rhs_for_div, &legacy_r);
        normalize(&legacy_q);
        normalize(&legacy_r);

        auto dm = tc_lhs.divmod(tc_rhs);
        assert_same_binary(label + " divmod quotient", legacy_q, dm.q);
        assert_same_binary(label + " divmod remainder", legacy_r, dm.r);

        if (!lhs_negative && !rhs_negative) {
            BitBigIntTC recomposed =
                    dm.q.multiplication_compat_for_testing(tc_rhs).add(dm.r);
            recomposed.normalize();
            assert_tc_equal(label + " algebra q*divisor+r == dividend",
                    recomposed, tc_lhs);

            auto exact_dm = tc_product.divmod(tc_rhs);
            assert_tc_equal(label + " algebra (a*b)/b quotient == a",
                    exact_dm.q, tc_lhs);
            expect(exact_dm.r.is_zero(),
                    label + " algebra (a*b)%b expected zero remainder");
        }

        clear_mem(&legacy_q);
        clear_mem(&legacy_r);
        clear_mem(&lhs_for_div);
        clear_mem(&rhs_for_div);
    }

    clear_mem(&legacy_lhs);
    clear_mem(&legacy_rhs);
}

void test_random_arithmetic_corpus() {
    std::cout << "\n=== deterministic random arithmetic corpus ==="
              << std::endl;

    size_t cases = 0;
    for (uint64_t seed : corpus_seeds()) {
        std::mt19937_64 rng(seed);
        for (int bits : corpus_bit_sizes()) {
            for (int sample = 0; sample < 1; ++sample) {
                auto lhs = positive_random_bits(bits, rng);
                auto rhs = positive_random_bits(bits, rng);
                std::vector<std::pair<bool, bool>> signs;
                if (bits <= 64) {
                    signs = {{false, false}, {false, true},
                            {true, false}, {true, true}};
                } else if (bits <= 128) {
                    signs = {{false, false}, {true, false}};
                } else {
                    signs = {{false, false}};
                }

                for (const auto& sign_pair : signs) {
                    const bool run_divmod = bits <= 128;
                    std::ostringstream label;
                    label << "seed=0x" << std::hex << seed << std::dec
                          << " bits=" << bits
                          << " sample=" << sample;
                    run_arithmetic_case(label.str(), lhs, sign_pair.first,
                            rhs, sign_pair.second, run_divmod);
                    ++cases;
                }
            }
        }
    }

    std::cout << "PASS: random arithmetic corpus cases=" << cases
              << " seeds=0x12345678,0xdeadbeef,0xc0ffee" << std::endl;
}

void test_threshold_stress() {
    std::cout << "\n=== multiplication threshold stress ===" << std::endl;

    std::vector<int> current_counts = {4, 5, 6, 254, 255, 256, 257, 258};
    size_t cases = 0;

    for (int count : current_counts) {
        int data_bits = count - 1;
        std::vector<std::pair<std::string, std::vector<uint8_t>>> values = {
            {"power", positive_power_bits(data_bits)},
            {"dense", positive_all_ones_bits(data_bits)}
        };

        for (const auto& lhs : values) {
            for (const auto& rhs : values) {
                run_arithmetic_case(
                        "threshold current_count=" + std::to_string(count)
                                + " " + lhs.first + "*" + rhs.first,
                        lhs.second, false, rhs.second, false,
                        count <= 128);
                ++cases;
            }
        }
    }

    run_arithmetic_case("threshold uneven small*huge",
            positive_all_ones_bits(4), false,
            positive_power_bits(512), false,
            false);
    run_arithmetic_case("threshold uneven huge*small",
            positive_power_bits(512), false,
            positive_all_ones_bits(4), false,
            false);
    cases += 2;

    std::cout << "PASS: multiplication threshold stress cases=" << cases
              << std::endl;
}

void test_module_pow_corpus() {
    std::cout << "\n=== module_pow corpus ===" << std::endl;

    std::vector<std::array<int, 3>> cases = {
        {2, 0, 5}, {5, 0, 5}, {2, 1, 5}, {2, 3, 5},
        {3, 4, 7}, {65, 17, 3233}, {2790, 2753, 3233},
        {0, 1, 7}, {1, 100, 7}, {2, 3, 1}
    };

    for (uint64_t seed : corpus_seeds()) {
        std::mt19937_64 rng(seed ^ 0x706f77ULL);
        for (int i = 0; i < 16; ++i) {
            int base = static_cast<int>(rng() % 2000);
            int exponent = static_cast<int>(rng() % 32);
            int modulus = static_cast<int>(rng() % 997) + 1;
            cases.push_back({base, exponent, modulus});
        }
    }

    for (const auto& test : cases) {
        set_case("module_pow base=" + std::to_string(test[0])
                + " exponent=" + std::to_string(test[1])
                + " modulus=" + std::to_string(test[2]));

        number legacy_base = int_to_number(test[0]);
        number legacy_exponent = int_to_number(test[1]);
        number legacy_modulus = int_to_number(test[2]);
        number legacy_result = module_pow(
                &legacy_base, &legacy_exponent, &legacy_modulus);

        BitBigIntTC tc_base(static_cast<int64_t>(test[0]));
        BitBigIntTC tc_exponent(static_cast<int64_t>(test[1]));
        BitBigIntTC tc_modulus(static_cast<int64_t>(test[2]));
        BitBigIntTC tc_result =
                tc_base.module_pow_compat_for_testing(tc_exponent, tc_modulus);
        assert_same_binary("module_pow", legacy_result, tc_result);

        if (test[2] > 0) {
            expect(tc_result.compare(tc_modulus) < 0,
                    "module_pow result should be less than positive modulus");
        }

        clear_mem(&legacy_result);
        clear_mem(&legacy_base);
        clear_mem(&legacy_exponent);
        clear_mem(&legacy_modulus);
    }

    std::cout << "PASS: module_pow cases=" << cases.size() << std::endl;
}

void test_euclid_corpus() {
    std::cout << "\n=== Euclid corpus ===" << std::endl;

    std::vector<std::pair<int, int>> cases = {
        {7, 3}, {3, 7}, {12, 8}, {8, 12},
        {21, 14}, {14, 21}, {3120, 17}, {17, 3120},
        {2773, 17}, {65537, 3120}
    };

    for (uint64_t seed : corpus_seeds()) {
        std::mt19937_64 rng(seed ^ 0x676364ULL);
        for (int i = 0; i < 24; ++i) {
            int lhs = static_cast<int>(rng() % 5000) + 1;
            int rhs = static_cast<int>(rng() % 5000) + 1;
            cases.push_back({lhs, rhs});
        }
    }

    for (const auto& test : cases) {
        set_case("euclid lhs=" + std::to_string(test.first)
                + " rhs=" + std::to_string(test.second));

        number legacy_lhs = int_to_number(test.first);
        number legacy_rhs = int_to_number(test.second);
        number legacy_gcd = euclide_algorithm(&legacy_lhs, &legacy_rhs);

        BitBigIntTC tc_lhs(static_cast<int64_t>(test.first));
        BitBigIntTC tc_rhs(static_cast<int64_t>(test.second));
        BitBigIntTC tc_gcd =
                tc_lhs.euclide_algorithm_compat_for_testing(tc_rhs);
        assert_same_binary("euclide_algorithm", legacy_gcd, tc_gcd);

        auto lhs_div_gcd = tc_lhs.divmod(tc_gcd);
        auto rhs_div_gcd = tc_rhs.divmod(tc_gcd);
        expect(lhs_div_gcd.r.is_zero(), "gcd should divide lhs");
        expect(rhs_div_gcd.r.is_zero(), "gcd should divide rhs");

        clear_mem(&legacy_gcd);
        clear_mem(&legacy_lhs);
        clear_mem(&legacy_rhs);
    }

    std::cout << "PASS: Euclid cases=" << cases.size() << std::endl;
}

void test_modified_euclid_rsa_corpus() {
    std::cout << "\n=== modified Euclid RSA-shaped corpus ===" << std::endl;

    std::vector<std::pair<int, int>> cases = {
        {3120, 17}, {2773, 17}, {3233, 17},
        {65537, 17}, {120, 7}, {288, 5}
    };

    for (const auto& test : cases) {
        set_case("modified_euclid phi=" + std::to_string(test.first)
                + " e=" + std::to_string(test.second));

        number legacy_phi = int_to_number(test.first);
        number legacy_e = int_to_number(test.second);
        number legacy_values[4] = {
            int_to_number(1),
            int_to_number(0),
            int_to_number(0),
            int_to_number(1)
        };
        number legacy_gcd = euclide_algorithm_modifyed(
                &legacy_phi, &legacy_e, legacy_values);

        BitBigIntTC tc_phi(static_cast<int64_t>(test.first));
        BitBigIntTC tc_e(static_cast<int64_t>(test.second));
        std::array<BitBigIntTC, 4> tc_values = {
            BitBigIntTC(static_cast<int64_t>(1)),
            BitBigIntTC(static_cast<int64_t>(0)),
            BitBigIntTC(static_cast<int64_t>(0)),
            BitBigIntTC(static_cast<int64_t>(1))
        };
        BitBigIntTC tc_gcd =
                tc_phi.euclide_algorithm_modifyed_compat_for_testing(
                        tc_e, tc_values);

        assert_same_binary("modified Euclid gcd", legacy_gcd, tc_gcd);
        for (size_t i = 0; i < 4; ++i) {
            assert_same_binary("modified Euclid matrix value "
                    + std::to_string(i), legacy_values[i], tc_values[i]);
        }

        if (tc_gcd.is_equal(BitBigIntTC(static_cast<int64_t>(1)))) {
            BitBigIntTC d = tc_values[1];
            if (d.sign_bit() != 0)
                d = d.add(tc_phi);
            BitBigIntTC inverse_check =
                    tc_e.multiplication_compat_for_testing(d).divmod(tc_phi).r;
            assert_tc_equal("RSA-shaped e*d mod phi == 1",
                    inverse_check, BitBigIntTC(static_cast<int64_t>(1)));
        }

        clear_mem(&legacy_gcd);
        clear_mem(&legacy_phi);
        clear_mem(&legacy_e);
        for (number& value : legacy_values)
            clear_mem(&value);
    }

    std::cout << "PASS: modified Euclid RSA-shaped cases="
              << cases.size() << std::endl;
}

} // namespace

int main() {
    std::cout << "BitBigIntTC Heavy Legacy Parity Tests" << std::endl;
    std::cout << "=====================================\n" << std::endl;

    test_random_arithmetic_corpus();
    test_threshold_stress();
    test_module_pow_corpus();
    test_euclid_corpus();
    test_modified_euclid_rsa_corpus();

    std::cout << "\nHeavy parity tests PASSED" << std::endl;
    return 0;
}
